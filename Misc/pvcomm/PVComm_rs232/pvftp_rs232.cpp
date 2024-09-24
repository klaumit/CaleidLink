
#include "plugin/pvftp.hpp"
#include "pvcrc32.h"
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include "charmap.h"
#include <ucl/stl/meta.hpp>
#include <ucl/io/locale.hpp>



namespace apv
{


class PVFTPCommRS232 : public PVFTPComm
{
private:
    ucl::win::WindowsHandle <HANDLE> hDevice;
    DCB                              dcb;

    bool cancelled;

    virtual void recvBlock  (bool binary = true);
    virtual void sendBlock  (bool binary = true);
    virtual bool sendByte   (unsigned char b);
    virtual bool recvByte   (unsigned char& b);
    virtual bool sendInt    (short b);
    virtual bool recvInt    (short& b);
    virtual bool expectByte (unsigned char b);
    virtual int  readInt    (unsigned char*& dptr);
    virtual void writeInt   (unsigned char*& dptr, int i);

    static void* factoryFunc (const void* diff, bool matches);
    static ucl::oo::FactoryObject <PVFTPComm, PVFTPCommRS232> factoryObject;
    static const char* factorytbl[];

    PVFTPCommRS232 (void);


public:
    virtual ~PVFTPCommRS232 (void);

	virtual const char* getConnectedDeviceName (void);
    virtual const char* getConnectedDeviceType (void);

    virtual bool isConnected (void);

    virtual void open (const char* dev, unsigned speed);
    virtual void close (void);

    virtual void cancel (void);
};


const char* PVFTPCommRS232::factorytbl[] = {
    "COM1",
    "COM2",
    "COM3",
    "COM4",
    "COM5",
};

ucl::oo::FactoryObject <PVFTPComm, PVFTPCommRS232>
    PVFTPCommRS232::factoryObject (factorytbl, ucl::stl::arraysize (factorytbl),
        factoryFunc);

void* PVFTPCommRS232::factoryFunc (const void* diff, bool matches)
{
    const char* param = *static_cast <const char* const*> (diff);
    return (std::strncmp (param, "COM", 3) == 0) ? new PVFTPCommRS232 : 0;
}

PVFTPCommRS232::PVFTPCommRS232 (void)
 : cancelled (false), hDevice (INVALID_HANDLE_VALUE)
{
}

PVFTPCommRS232::~PVFTPCommRS232 (void)
{
    close ();
}

const char* PVFTPCommRS232::getConnectedDeviceName (void)
{ return pv_identity.devname; }
const char* PVFTPCommRS232::getConnectedDeviceType (void)
{ return pv_identity.devclass; }


void PVFTPCommRS232::open (const char* name, unsigned speed)
{
    COMMTIMEOUTS ct;
    unsigned char b;
    unsigned char* bp;
    DWORD mask;
    ident_t my_identity;

    /*busy = true;*/
    while (InterlockedCompareExchange (&busy, 1, 0))
        SwitchToThread ();

    hDevice = CreateFile (name, GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
		throw std::runtime_error (dgettext ("pvcomm.rs232", "Unable to open port"));

    GetCommState (hDevice, &dcb);
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
    dcb.fOutX = FALSE;
    dcb.fInX  = FALSE;
    dcb.fNull = FALSE;
    dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
    dcb.fAbortOnError = FALSE;
    dcb.BaudRate = CBR_38400;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fParity  = FALSE;
    SetCommState (hDevice, &dcb);

    ct.ReadIntervalTimeout = MAXDWORD;
    ct.ReadTotalTimeoutMultiplier = 0;
    ct.ReadTotalTimeoutConstant = 0;
    ct.WriteTotalTimeoutMultiplier = ct.WriteTotalTimeoutConstant = 0;
    SetCommTimeouts (hDevice, &ct);

    SetCommMask (hDevice, EV_RXCHAR);

    
    sendByte (PVFTP_ID);
    Sleep (20);
    while (true)
    {
        if (recvByte (b))
        {
            if (b == PVFTP_ID)
            {
                sendByte (PVFTP_ACK);
                break;
            }
            else if (b == PVFTP_ACK)
                break;
            else
				HelperIntf::get ().breakComm (this,
					dgettext ("pvcomm.rs232", "Communication error!").c_str ());
        }

        if (cancelled)
        {
            hDevice.close ();
            busy = 0;
            cancelled = false;
            return;
        }
        
        Sleep (100);
    }

    recvBlock ();
    bp = buf;
    if (std::strcmp (readStrFromBlock (bp), "AUDACIA PVFTP1") != 0)
		HelperIntf::get ().breakComm (this,
			dgettext ("pvcomm.rs232", "Communication error: Wrong protocol!").c_str ());
	if (readInt (bp) != 150)
		HelperIntf::get ().breakComm (this,
			dgettext ("pvcomm.rs232", "Communication error: Wrong protocol version!").c_str ());



        // replace if compiled under different architecture
    std::strcpy (my_identity.devclass, "x86 PC");
    std::strcpy (my_identity.devname, HelperIntf::get ().getOSVersionString ().c_str ());

    bp = buf;
    writeStrToBlock (bp, my_identity.devname);
    writeStrToBlock (bp, my_identity.devclass);
    writeInt (bp, 0); /* role: 0 - terminal, 1 - client */
    bufsize = bp - buf;
    sendBlock ();

    recvBlock ();
    bp = buf;
    std::strcpy (pv_identity.devname, readStrFromBlock (bp));
    std::strcpy (pv_identity.devclass, readStrFromBlock (bp));

    bp = buf;
    writeInt (bp, speed / 100);
    writeInt (bp, (speed == CBR_38400) ? 0 : 1);
    bufsize = bp - buf;
    sendBlock ();

    if (speed != CBR_38400)
    {
        dcb.BaudRate = speed;
        CloseHandle (hDevice);
        hDevice = CreateFile (name, GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_EXISTING, 0, NULL);
        if (hDevice == INVALID_HANDLE_VALUE)
            throw std::runtime_error (dgettext ("pvcomm.rs232", "Unable to open port"));
        SetCommState (hDevice, &dcb);

        expectByte (PVFTP_ACK);
    }

    connected = true;
    busy = 0;
}

void PVFTPCommRS232::close (void)
{
    if (connected)
    {
        sendByte (PVFTP_ESC);
        Sleep (100);
    }
    connected = false;
    hDevice.close ();
    busy = 0;
}

void PVFTPCommRS232::cancel (void)
{
    cancelled = true;
}

void PVFTPCommRS232::recvBlock (bool binary)
{
    short crc1, crc2;
    short rcrc1, rcrc2;
    unsigned char* bp;
    short bcnt;
    unsigned char ans;
    unsigned blockcnt;

    do
    {
        recvInt (bufsize);
        recvInt (rcrc1);
        recvInt (rcrc2);

        bp = buf;
        bcnt = bufsize;
        if (!binary)
            bp += 24;

        blockcnt = 249;
        while (bcnt-- > 0)
        {
            while (!recvByte (*bp))
                ;
            ++bp;
            if (--blockcnt == 0)
            {
                sendByte (PVFTP_ACK);
                blockcnt = 249;
            }
        }
        if (blockcnt > 0)
            sendByte (PVFTP_ACK);

        if (!binary)
        {
            bp = buf + std::max <short> (bufsize, 24);
            for (unsigned n = 0; n < 24; ++n)
                buf[n] = bp[n];
        }

        HelperIntf::get ().pvcrc32 (buf + (binary ? 0 : 24), bufsize, &crc1, &crc2);
        if ((crc1 != rcrc1) || (crc2 != rcrc2))
            ans = PVFTP_REP;
        else
            ans = PVFTP_ACK;

        sendByte (ans);
    } while (ans == PVFTP_REP);
}

void PVFTPCommRS232::sendBlock (bool binary)
{
    short crc1, crc2;
    unsigned char* bp;
    unsigned char* bend;
    unsigned char ans;
    unsigned l, m;
    DWORD nobw;

    do
    {
        sendInt (bufsize);
        HelperIntf::get ().pvcrc32 (buf, bufsize, &crc1, &crc2);
        sendInt (crc1); sendInt (crc2);

        bp = buf;

        if (!binary)
        {
            bp = buf + std::max <short> (bufsize, 24);
            for (int n = 23; n >= 0; --n)
                bp[n] = buf[n];

            bp = buf + 24;
        }
        
        bend = bp + bufsize;
        
        while (bp < bend)
        {
            l = bend - bp;
            if (l > 249)
                l = 249;
            sendByte (l);
            while (!expectByte (PVFTP_ACK))
                ;
            WriteFile (hDevice, bp, l, &nobw, NULL);
            bp += l;
            while (!expectByte (PVFTP_ACK))
                ;
        }


        while (!recvByte (ans))
            ;

        if ((ans != PVFTP_ACK) && (ans != PVFTP_REP))
			HelperIntf::get ().breakComm (this,
				dgettext ("pvcomm.rs232", "Communication error!").c_str ());
    } while (ans == PVFTP_REP);
}

bool PVFTPCommRS232::sendByte (unsigned char b)
{
    DWORD nobw;

    WriteFile (hDevice, &b, 1, &nobw, NULL);
    return (nobw == 1);
}

bool PVFTPCommRS232::recvByte (unsigned char& b)
{
    DWORD nobr;

    ReadFile (hDevice, &b, 1, &nobr, NULL);
    return (nobr == 1);
}

bool PVFTPCommRS232::sendInt (short b)
{
    bool r = sendByte (b & 0xFF);
    return (r && sendByte (b >> 8));
}
bool PVFTPCommRS232::recvInt (short& b)
{
    unsigned char b1, b2;
    while (!recvByte (b1))
        ;
    while (!recvByte (b2))
        ;
    b = b1 | (b2 << 8);
    return true;
}
int PVFTPCommRS232::readInt (unsigned char*& dptr)
{
    return readFromBlock <short> (dptr);
}
void PVFTPCommRS232::writeInt (unsigned char*& dptr, int i)
{
    writeToBlock <short> (dptr, i);
}


bool PVFTPCommRS232::expectByte (unsigned char b)
{
    unsigned char c;
    if (!recvByte (c))
        return false;
    else
    {
        if (c == PVFTP_CAN)
			HelperIntf::get ().breakComm (this,
				dgettext ("pvcomm.rs232", "Connection terminated!").c_str ());
		if (c != b)
			HelperIntf::get ().breakComm (this,
				dgettext ("pvcomm.rs232", "Communication error!").c_str ());
        return true;
    }
}

bool PVFTPCommRS232::isConnected (void)
{
    DWORD mask;
    BYTE b;

    if (connected)
    {
        /*if (busy)
            return true;*/
        if (InterlockedCompareExchange (&busy, 1, 0))
            return true;

        GetCommMask (hDevice, &mask);
        if (mask & EV_RXCHAR) // client sent some data
        {
            if (!recvByte (b))
            {
                busy = 0;
                return true;
            }
			if (b == PVFTP_CAN)
				HelperIntf::get ().breakComm (this,
					dgettext ("pvcomm.rs232", "Connection terminated!").c_str ());
			if (b == PVFTP_ESC)
            {
                close ();
                return false;
            }
            else
				HelperIntf::get ().breakComm (this,
					dgettext ("pvcomm.rs232", "Communication error!").c_str ());
		}

        busy = 0;
        return true;
    }
    else
        return false;
}





} // namespace apv
