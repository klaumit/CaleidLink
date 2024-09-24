
#include "plugin/pvftp.hpp"
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include "charmap.h"
#include <ucl/stl/meta.hpp>
#include <ucl/io/locale.hpp>


namespace apv
{


class PVFTPCommUSB : public PVFTPComm
{
    struct DllInterface
    {
        typedef HANDLE (_stdcall * open_t) (int);
        typedef BOOL (_stdcall * handlefunc_t) (HANDLE);
        typedef BOOL (_stdcall * read_t) (HANDLE, LPVOID, DWORD, LPDWORD);
        typedef BOOL (_stdcall * write_t) (HANDLE, LPCVOID, DWORD);

        open_t          open;
        handlefunc_t    close;

        read_t          read;
        write_t         write;

        handlefunc_t    cancelRead;
        handlefunc_t    cancelWrite;

        handlefunc_t    clearError;
    };

	HANDLE hDll;
    HANDLE hPV;
    DllInterface intf;

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

	static ucl::oo::FactoryObject <PVFTPComm, PVFTPCommUSB> factoryObject;
    static const char* factorytbl[];



public:
    PVFTPCommUSB (const char*);
    virtual ~PVFTPCommUSB (void);

    virtual const char* getConnectedDeviceName (void);
    virtual const char* getConnectedDeviceType (void);

    virtual bool isConnected (void);

    virtual void open (const char* dev, unsigned speed);
    virtual void close (void);

    virtual void cancel (void);
};

const char* PVFTPCommUSB::factorytbl[] = {
    "USB",
};

ucl::oo::FactoryObject <PVFTPComm, PVFTPCommUSB>
    PVFTPCommUSB::factoryObject (factorytbl, ucl::stl::arraysize (factorytbl));


PVFTPCommUSB::PVFTPCommUSB (const char*)
 : cancelled (false), hPV (INVALID_HANDLE_VALUE)
{
    hDll = LoadLibrary ("PVUSBAPI.dll");
    if (!hDll)
        throw std::runtime_error (dgettext ("pvcomm.usb", "PVUSBAPI.dll not found!"));

    intf.open = (DllInterface::open_t) GetProcAddress (hDll,
        "?PVUSB_Open@@YGPAXH@Z");
    intf.close = (DllInterface::handlefunc_t) GetProcAddress (hDll,
        "?PVUSB_Close@@YGHPAX@Z");
    intf.read = (DllInterface::read_t) GetProcAddress (hDll,
        "?PVUSB_Read@@YGHPAX0KPAK@Z");
    intf.write = (DllInterface::write_t) GetProcAddress (hDll,
        "?PVUSB_Write@@YGHPAXPBXK@Z");
    intf.cancelRead = (DllInterface::handlefunc_t) GetProcAddress (hDll,
        "?PVUSB_CancelRead@@YGHPAX@Z");
    intf.cancelWrite = (DllInterface::handlefunc_t) GetProcAddress (hDll,
        "?PVUSB_CancelWrite@@YGHPAX@Z");
    intf.clearError = (DllInterface::handlefunc_t) GetProcAddress (hDll,
        "?PVUSB_ClearError@@YGHPAX@Z");
}

PVFTPCommUSB::~PVFTPCommUSB (void)
{
    close ();
    FreeLibrary (hDll);
}

const char* PVFTPCommUSB::getConnectedDeviceName (void)
{ return pv_identity.devname; }
const char* PVFTPCommUSB::getConnectedDeviceType (void)
{ return pv_identity.devclass; }

void PVFTPCommUSB::open (const char* name, unsigned speed)
{
    unsigned char b;
    unsigned char* bp;
    ident_t my_identity;

    while (InterlockedCompareExchange (&busy, 1, 0))
        SwitchToThread ();

    while (!cancelled)
    {
        hPV = (intf.open) (0);
        Sleep (250);
        if (hPV != INVALID_HANDLE_VALUE)
            break;
    }
	if (hPV == INVALID_HANDLE_VALUE)
    {
        cancelled = false;
        busy = 0;
        return;
    }


    sendByte (PVFTP_ID);
	while (true)
    {
        if (recvByte (b))
        {
			if (b == PVFTP_ACK)
                break;
            else
				HelperIntf::get ().breakComm (this,
					dgettext ("pvcomm.usb", "Connection terminated!").c_str ());
        }
		    }

    recvBlock ();
    bp = buf;
    if (std::strcmp (readStrFromBlock (bp), "AUDACIA PVFTP1") != 0)
		HelperIntf::get ().breakComm (this,
			dgettext ("pvcomm.usb", "Communication error: Wrong protocol!").c_str ());
	if (readInt (bp) != 150)
		HelperIntf::get ().breakComm (this,
			dgettext ("pvcomm.usb", "Communication error: Wrong protocol version!").c_str ());



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

    connected = true;
    busy = 0;
}

void PVFTPCommUSB::close (void)
{
    if (hPV != INVALID_HANDLE_VALUE)
    {
        if (connected)
            sendByte (PVFTP_ESC);
        Sleep (100);
        (intf.clearError) (hPV);
        (intf.close) (hPV);
        hPV = NULL;
    }
    connected = false;
    busy = 0;
}

void PVFTPCommUSB::cancel (void)
{
    cancelled = true;
}



void PVFTPCommUSB::recvBlock (bool binary)
{
    short crc1, crc2;
    short rcrc1, rcrc2;
    unsigned char* bp;
    int bcnt, bmax;
    unsigned char ans;
    unsigned blockcnt;
    DWORD len;

    do
    {
        recvInt (bufsize);
        recvInt (rcrc1);
        recvInt (rcrc2);

        bp = buf;
        bmax = bcnt = bufsize < 1024 ? 1024 : 3072;
        
        len = bcnt;
        if (!binary)
            bp += 24;

        blockcnt = 1024;
        while (bcnt > 0)
        {
            if (!(intf.read) (hPV, bp, bcnt, &len))
				HelperIntf::get ().breakComm (this,
					dgettext ("pvcomm.rs232", "Communication error!").c_str ());

            bcnt -= len;
            blockcnt -= len;
            bp += len;

            if (blockcnt == 0)
            {
                sendByte (PVFTP_ACK);
                blockcnt = 1024;
            }
        }

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

void PVFTPCommUSB::sendBlock (bool binary)
{
    short crc1, crc2;
	unsigned char* bp;
	unsigned char ans;
	DWORD nobw;
    unsigned sendsize;

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

        sendsize = (bufsize + 0x03FF) & 0xFC00; // 1024-byte blocks 
        while (sendsize > 0)
        {
            if (!(intf.write) (hPV, bp, 1024))
                HelperIntf::get ().breakComm (this,
				    dgettext ("pvcomm.usb", "Communication error!").c_str ());
            bp += 1024;
            sendsize -= 1024;
            while (!expectByte (PVFTP_ACK))
                ;
		}

        while (!recvByte (ans))
            ;

        if ((ans != PVFTP_ACK) && (ans != PVFTP_REP))
			HelperIntf::get ().breakComm (this,
				dgettext ("pvcomm.usn", "Communication error!").c_str ());
    } while (ans == PVFTP_REP);
}

bool PVFTPCommUSB::sendByte (unsigned char b)
{
    /*DWORD nobw;

    WriteFile (hDevice, &b, 1, &nobw, NULL);
    return (nobw == 1);*/
    return (intf.write) (hPV, &b, 1);
}

bool PVFTPCommUSB::recvByte (unsigned char& b)
{
    DWORD nobr;

    /*ReadFile (hDevice, &b, 1, &nobr, NULL);
    return (nobr == 1);*/
    (intf.read) (hPV, &b, 1, &nobr);
    return (nobr == 1);
}

bool PVFTPCommUSB::sendInt (short b)
{
    bool r = sendByte (b & 0xFF);
    return (r && sendByte (b >> 8));
}
bool PVFTPCommUSB::recvInt (short& b)
{
    unsigned char b1, b2;
    while (!recvByte (b1))
        ;
    while (!recvByte (b2))
        ;
    b = b1 | (b2 << 8);
    return true;
}
int PVFTPCommUSB::readInt (unsigned char*& dptr)
{
    return reverseEndianness (readFromBlock <unsigned> (dptr));
}
void PVFTPCommUSB::writeInt (unsigned char*& dptr, int i)
{
    writeToBlock <int> (dptr, reverseEndianness (i));
}


bool PVFTPCommUSB::expectByte (unsigned char b)
{
    unsigned char c;
    if (!recvByte (c))
        return false;
    else
    {
        if (c == PVFTP_CAN)
			HelperIntf::get ().breakComm (this,
				dgettext ("pvcomm.usb", "Connection terminated!").c_str ());
        if (c != b)
			HelperIntf::get ().breakComm (this,
				dgettext ("pvcomm.usb", "Communication error!").c_str ());
        return true;
    }
}

bool PVFTPCommUSB::isConnected (void)
{
    /*unsigned char b;

    if (connected)
    {
        if (InterlockedCompareExchange (&busy, 1, 0))
            return true;

        if (!recvByte (b))
        {
            busy = 0;
            return true;
        }
        if (b == PVFTP_CAN)
            HelperIntf::get ().breakComm (this, "Connection terminated!");
        if (b == PVFTP_ESC)
        {
            close ();
            return false;
        }
        else
            HelperIntf::get ().breakComm (this, "Communication error!");

        busy = 0;
        return true;
    }
    else
        return false;*/
    return connected;
}



} // namespace apv

