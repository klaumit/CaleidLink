
#ifndef _PVFTP_HPP
#define _PVFTP_HPP

#include <vector>
#include <string>

#include <ucl/stl/arrayptr.hpp>
#include <ucl/stl/smartptr.hpp>
#include <ucl/win/winutil.hpp>
#include <ucl/oo/factory.hpp>

#include <windows.h>


namespace apv
{


const unsigned char PVFTP_ACK = 6;
const unsigned char PVFTP_CAN = 24;
const unsigned char PVFTP_REP = 17;
const unsigned char PVFTP_EOT = 4;
const unsigned char PVFTP_ID  = 18;
const unsigned char PVFTP_ESC = 27;
const unsigned char PVFTP_ENQ = 5;
const unsigned char PVFTP_COM = 32;

const unsigned char PVFTP_COM_NAMEDFILE  = PVFTP_COM + 0;
const unsigned char PVFTP_COM_FILECNT    = PVFTP_COM + 1;
const unsigned char PVFTP_COM_NAMELIST   = PVFTP_COM + 2;
const unsigned char PVFTP_COM_FILEXCHG   = PVFTP_COM + 3;
const unsigned char PVFTP_COM_FILEREMOVE = PVFTP_COM + 4;
const unsigned char PVFTP_COM_FILEREAD   = PVFTP_COM + 5;
const unsigned char PVFTP_COM_FILEWRITE  = PVFTP_COM + 6;
const unsigned char PVFTP_COM_FPLIST     = PVFTP_COM + 7;


/* helper functions */
/*template <int Min, int Max, typename T>
 static inline T bound (T val)
{ return (val > Max) ? Max : ((val < Min) ? Min : val); }*/
template <typename T>
    void writeToBlock (unsigned char*& dptr, const T& val);

template <typename T>
    void writeToBlock (unsigned char*& dptr, const T& val)
{
    *reinterpret_cast <T*> (dptr) = val;
    dptr += sizeof (T);
}

template <typename T>
    T readFromBlock (unsigned char*& dptr)
{
    T val = *reinterpret_cast <T*> (dptr);
    dptr += sizeof (T);
    return val;
}

inline void writeStrToBlock (unsigned char*& dptr, const char* const& string)
{
    unsigned l = strlen (string) + 1;
    std::memcpy (dptr, string, l);
    dptr += l;
}

inline const char* readStrFromBlock (unsigned char*& dptr)
{
    const char* r = dptr;
    dptr += strlen (r) + 1;
    return r;
}

inline int reverseEndianness (unsigned i)
{
    return ((i & 0x000000FF) << 24)
         | ((i & 0x0000FF00) << 8)
         | ((i & 0x00FF0000) >> 8)
         | ((i & 0xFF000000) >> 24);
}



    // TODO: put somewhere else
struct submodedesc_t
{
    const char*     desc;
    short           submode;
    bool            read_only;
};
struct modedesc_t
{
    const char*     desc;
    short           mode;
    unsigned        default_submode;
    unsigned        submodecnt;
    submodedesc_t*  submodes;
};

extern modedesc_t modedesc[7];
    // TODO end


class PVFTPComm : public ucl::oo::FactoryBase <PVFTPComm>
{
public:
    struct ident_t
    {
        char devclass[32];
        char devname[64];
    };

    typedef unsigned short fp_t;

    static const fp_t fpInvalid = 0xFFFF;

    struct mode_t
    {
        unsigned char mode;
        unsigned char submode;
    };

    struct name_fp_t
    {
        fp_t           fp;
        unsigned short flags;
        std::string    name;

        name_fp_t (fp_t _fp)
         : fp (_fp)
        {}
    };

    typedef ucl::stl::SmartPtr <std::vector <fp_t> >            fplist_t;
    typedef ucl::stl::SmartPtr <std::vector <name_fp_t> >       namelist_t;
    typedef ucl::stl::ArrayPtr <unsigned char>                  file_t;
    //typedef ucl::stl::SmartPtr <std::vector <unsigned char> >   file_t;


protected:
    PVFTPComm (void)
     : connected (false), busy (0)
    {
        pv_identity.devclass[0] = '\0';
        pv_identity.devname[0] = '\0';
    }

    struct HelperIntf : ucl::oo::HelperInterfaceBase <HelperIntf>
    {
        virtual std::string getOSVersionString (void);
        virtual unsigned convertAnsi2PV (BYTE* file, unsigned size);
        virtual file_t convertPV2Ansi (const BYTE* file, unsigned size);
        virtual void pvcrc32 (const void* sptr, int bytes, short* crc1, short* crc2);

        virtual void breakComm (PVFTPComm* self, const char* reason);
    };

    friend class HelperIntf;
    friend class ucl::oo::HelperInterfaceBase <HelperIntf>;


    virtual void recvBlock  (bool binary = true) = 0;
    virtual void sendBlock  (bool binary = true) = 0;
    virtual bool sendByte   (unsigned char b) = 0;
    virtual bool recvByte   (unsigned char& b) = 0;
    virtual bool sendInt    (short b) = 0;
    virtual bool recvInt    (short& b) = 0;
    virtual bool expectByte (unsigned char b) = 0;
    virtual int  readInt    (unsigned char*& dptr) = 0;
    virtual void writeInt   (unsigned char*& dptr, int i) = 0;


    unsigned char buf[8192];
    short         bufsize;

    ident_t       pv_identity;

    volatile bool connected;
    volatile LONG busy;


public:
    virtual ~PVFTPComm (void) {}

    mode_t      doNamedFile  (const char* filename);
    unsigned    doFileCnt    (mode_t mode);
    fplist_t    doFpList     (mode_t mode);
    namelist_t  doNameList   (mode_t mode);
    void        doFileXchg   (mode_t mode, fp_t fp1, fp_t fp2);
    void        doFileRemove (mode_t mode, fp_t fp);
    file_t      doFileRead   (mode_t mode, fp_t fp);
    void        doFileWrite  (mode_t mode, fp_t fp,
        const unsigned char* file, unsigned size);

    virtual const char* getConnectedDeviceName (void) = 0;
    virtual const char* getConnectedDeviceType (void) = 0;

    virtual void open  (const char* name, unsigned speed) = 0;
    virtual void close (void) = 0;

    virtual bool isConnected (void) = 0;

    virtual void cancel (void) = 0;
};

} // namespace apv

#endif
