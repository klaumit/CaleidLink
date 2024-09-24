
#include "plugin/pvftp.hpp"
#include "plugin/apvfile.hpp"
#include "pvcrc32.h"
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include "charmap.h"

#include <ucl/oo/factory.hpp>
#include <ucl/io/locale.hpp>



namespace apv
{

UCL_OO_DEFINE_FACTORY (PVFTPComm);
UCL_OO_DEFINE_HELPERINTERFACE (PVFTPComm::HelperIntf);


static submodedesc_t submodes_tel[9] = {
    {"Common",      0, true},
    {"Personal",    1, false},
    {"Company",     2, false},
    {"Category 1",  3, false},
    {"Category 2",  4, false},
    {"Category 3",  5, false},
    {"Category 4",  6, false},
    {"Category 5",  7, false},
    {"Owner",      15, false},
};

static submodedesc_t submodes_memo[6] = {
    {"Common",      0, true},
    {"Category 1",  1, false},
    {"Category 2",  2, false},
    {"Category 3",  3, false},
    {"Category 4",  4, false},
    {"Category 5",  5, false},
};

static submodedesc_t submodes_schedule[13] = {
    {"Common",                          0, true},
    {"Schedule",                        1, false},
    {"Schedule (Term registration)",    2, false},
    {"Reminder",                        3, false},
    {"Holiday setting",                 4, false},
    {"Schedule (Common)",               6, true},
    {"To Do (Common)",                  8, true},
    {"To Do (A)",                       9, false},
    {"To Do (B)",                      10, false},
    {"To Do (C)",                      11, false},
    {"To Do (D)",                      12, false},
    {"To Do (E)",                      13, false},
    {"To Do (F)",                      14, false},
};

static submodedesc_t submodes_expense[3] = {
    {"Account",      1, false},
    {"Transaction",  2, false},
    {"Payment type",  3, false},
};

static submodedesc_t submodes_dualwin[1] = {
    {"Clip",      1, false},
};

#define _DEF_SUBMODE(name) \
    sizeof (name) / sizeof (submodedesc_t), name

modedesc_t modedesc[7] = {
    {"Telephone",           1, 1, _DEF_SUBMODE (submodes_tel)},
    {"Telephone (Company)", 2, 2, _DEF_SUBMODE (submodes_tel)},
    {"Memo",                3, 5, _DEF_SUBMODE (submodes_memo)},
    {"Schedule",            4, 1, _DEF_SUBMODE (submodes_schedule)},
    {"Expense",             7, 0, _DEF_SUBMODE (submodes_expense)},
    {"Dual window",         9, 0, _DEF_SUBMODE (submodes_dualwin)},
    {"Add-in",             11, 0, 0},
};

#undef _DEF_SUBMODE



std::string PVFTPComm::HelperIntf::getOSVersionString (void)
{
    OSVERSIONINFO ovi;
    char buf[128];
    const char* ps;

    ovi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    GetVersionEx (&ovi);
    if (ovi.dwPlatformId == VER_PLATFORM_WIN32_NT)
        std::sprintf (buf, "Microsoft Windows NT %d.%d",
            ovi.dwMajorVersion, ovi.dwMinorVersion);
    else // Win95, Win98, WinMe
    {
        if (ovi.dwMajorVersion == 4)
            switch (ovi.dwMinorVersion)
            {
            case 0: // Windows 95
                ps = " 95";
                break;
            case 10: // Windows 98
                ps = " 98";
                break;
            case 90: // Windows Me
                ps = " Me";
                break;
            default:
                ps = "";
                break;
            }
        else
            ps = ""; // unknown
        std::sprintf (buf, "Microsoft Windows%s", ps);
    }

    return std::string (buf);
}

unsigned PVFTPComm::HelperIntf::convertAnsi2PV (BYTE* file, unsigned size)
{
    const BYTE* src = file;
    BYTE* dst = file;
    const BYTE* end = file + size;

    for (; src < end; ++src, ++dst)
    {
        *dst = charmap_ansi2pv[*src];
        if ((*dst == '\x0A') || (*dst == '\x0D'))
            if ((src + 1 < end) && (src[1] != *dst) && ((src[1] == '\x0A') || (src[1] == '\x0D')))
            {
                ++src;
                --size;
            }
    }

    return size;
}
PVFTPComm::file_t PVFTPComm::HelperIntf::convertPV2Ansi (const BYTE* file, unsigned size)
{
    PVFTPComm::file_t rfile;
    const BYTE* src;
    BYTE* dst;
    const BYTE* end = file + size;
    unsigned sz;

    /*for (src = file, sz = 0; src < end; ++src, ++sz)
        if (charmap_pv2ansi[*src] == '\x0A')
            ++sz;*/
    sz = size;

    rfile.alloc (sz);

    for (src = file, dst = rfile.get (); src < end; ++src, ++dst)
    {
        *dst = charmap_pv2ansi[*src];
        /*if (*dst == '\x0A')
        {
            *dst = '\x0D';
            *(++dst) = '\x0A';
        }*/
    }

    return rfile;
}

void PVFTPComm::HelperIntf::pvcrc32 (const void* sptr, int bytes, short* crc1, short* crc2)
{
    ::pvcrc32 (sptr, bytes, crc1, crc2);
}




void PVFTPComm::HelperIntf::breakComm (PVFTPComm* self, const char* reason)
{
    self->sendByte (PVFTP_CAN);
    self->close ();
    throw std::runtime_error (reason);
}

PVFTPComm::mode_t PVFTPComm::doNamedFile (const char* filename)
{
    unsigned char* fp = buf;
    mode_t m;

    /*busy = true;*/
    while (InterlockedCompareExchange (&busy, 1, 0))
        SwitchToThread ();

    sendByte (PVFTP_COM_NAMEDFILE);
    while (!expectByte (PVFTP_ACK))
        ;

    writeStrToBlock (fp, filename);
    for (unsigned char* p = buf; p < fp; ++p)
        *p = charmap_ansi2pv[*p];
    bufsize = fp - buf;
    sendBlock ();

    recvBlock ();
    fp = buf;
    m.mode = readInt (fp);
    m.submode = readInt (fp);

    busy = 0;
    
    return m;
}
unsigned PVFTPComm::doFileCnt (mode_t mode)
{
    unsigned char* fp = buf;

    /*busy = true;*/
    while (InterlockedCompareExchange (&busy, 1, 0))
        SwitchToThread ();

    sendByte (PVFTP_COM_FILECNT);
    while (!expectByte (PVFTP_ACK))
        ;

    writeInt (fp, mode.mode);
    writeInt (fp, mode.submode);
    bufsize = fp - buf;
    sendBlock ();

    recvBlock ();
    fp = buf;

    busy = 0;
    
    return readInt (fp);
}
PVFTPComm::fplist_t PVFTPComm::doFpList (mode_t mode)
{
    unsigned char* fp = buf;
    fplist_t r (new std::vector <fp_t>);
    unsigned char* fpmax;
    fp_t lastfp;

    /*busy = true;*/
    while (InterlockedCompareExchange (&busy, 1, 0))
        SwitchToThread ();

    sendByte (PVFTP_COM_FPLIST);
    while (!expectByte (PVFTP_ACK))
        ;

    writeInt (fp, mode.mode);
    writeInt (fp, mode.submode);
    bufsize = fp - buf;
    sendBlock ();

    while (true)
    {
        recvBlock ();
        fp = buf;
        fpmax = fp + bufsize;
        while (fp < fpmax)
            r->push_back (lastfp = readInt (fp));
        if (lastfp == fpInvalid)
        {
            r->pop_back ();
            break;
        }
    }

    busy = 0;

    return r;
}
PVFTPComm::namelist_t PVFTPComm::doNameList (mode_t mode)
{
    unsigned char* fp = buf;
    namelist_t r (new std::vector <name_fp_t>);
    unsigned char* fpmax;
    fp_t lastfp;

    /*busy = true;*/
    while (InterlockedCompareExchange (&busy, 1, 0))
        SwitchToThread ();

    sendByte (PVFTP_COM_NAMELIST);
    while (!expectByte (PVFTP_ACK))
        ;

    writeInt (fp, mode.mode);
    writeInt (fp, mode.submode);
    bufsize = fp - buf;
    sendBlock ();


    while (true)
    {
        recvBlock ();
        fp = buf;
        fpmax = fp + bufsize;
        while (fp < fpmax)
        {
            r->push_back (lastfp = readInt (fp));
            
            r->back ().name = readStrFromBlock (fp);
            for (char* p = &(r->back ().name[0]), * l = p + r->back ().name.size ();
                 p < l; ++p)
                *p = static_cast <char> (charmap_pv2ansi[static_cast <unsigned char> (*p)]);

            r->back ().flags = 0;
        }
        if (lastfp == fpInvalid)
        {
            r->pop_back ();
            break;
        }
    }

    busy = 0;

    return r;
}
void PVFTPComm::doFileXchg (mode_t mode, fp_t fp1, fp_t fp2)
{
    unsigned char* fp = buf;

    /*busy = true;*/
    while (InterlockedCompareExchange (&busy, 1, 0))
        SwitchToThread ();

    sendByte (PVFTP_COM_FILEXCHG);
    while (!expectByte (PVFTP_ACK))
        ;

    writeInt (fp, mode.mode);
    writeInt (fp, mode.submode);
    writeInt (fp, fp1);
    writeInt (fp, fp2);
    bufsize = fp - buf;
    sendBlock ();

    busy = 0;
}
void PVFTPComm::doFileRemove (mode_t mode, fp_t fp1)
{
    unsigned char* fp = buf;

    /*busy = true;*/
    while (InterlockedCompareExchange (&busy, 1, 0))
        SwitchToThread ();

    sendByte (PVFTP_COM_FILEREMOVE);
    while (!expectByte (PVFTP_ACK))
        ;

    writeInt (fp, mode.mode);
    writeInt (fp, mode.submode);
    writeInt (fp, fp1);
    bufsize = fp - buf;
    sendBlock ();

    busy = 0;
}
PVFTPComm::file_t PVFTPComm::doFileRead (mode_t mode, fp_t fp1)
{
    unsigned char* fp = buf;
    file_t r;
    bool binary = (APVFile::file_t::getFileType (
        static_cast <APVFile::mode_t> (mode.mode)) == APVFile::ftBinary);

    /*busy = true;*/
    while (InterlockedCompareExchange (&busy, 1, 0))
        SwitchToThread ();

    sendByte (PVFTP_COM_FILEREAD);
    while (!expectByte (PVFTP_ACK))
        ;

    writeInt (fp, mode.mode);
    writeInt (fp, mode.submode);
    writeInt (fp, fp1);
    bufsize = fp - buf;
    sendBlock ();

    recvBlock (binary);
    fp = buf;
    if (binary)
    {
        r.alloc (bufsize);
        std::memcpy (r.get (), buf, bufsize);
    }
    else // text file
        r = HelperIntf::get ().convertPV2Ansi (buf, bufsize);

    busy = 0;
    
    return r;
}
void PVFTPComm::doFileWrite (mode_t mode, fp_t fp1,
    const unsigned char* file, unsigned size)
{
    unsigned char* fp = buf;
    file_t r;
    bool binary = (APVFile::file_t::getFileType (
        static_cast <APVFile::mode_t> (mode.mode)) == APVFile::ftBinary);

    /*busy = true;*/
    while (InterlockedCompareExchange (&busy, 1, 0))
        SwitchToThread ();

    sendByte (PVFTP_COM_FILEWRITE);
    while (!expectByte (PVFTP_ACK))
        ;

    writeInt (fp, mode.mode);
    writeInt (fp, mode.submode);
    writeInt (fp, fp1);
    bufsize = fp - buf;
    sendBlock ();

    bufsize = size;
    std::memcpy (buf, file, size);
    if (!binary)
    {
        bufsize = HelperIntf::get ().convertAnsi2PV (buf, size);
        if (bufsize > 2048)
            throw std::runtime_error ("File is too large!");
    }
    sendBlock (binary);

    busy = 0;
}




} // namespace apv
