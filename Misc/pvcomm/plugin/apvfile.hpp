/*
 *  apvfile.hpp
 *
 *  A file header definition for a CASIO PV container file format
 *  Written by Moritz Beutel
 *  (c) 2008 by AUDACIA Software
 */


#ifndef _APVFILE_HPP
#define _APVFILE_HPP


#include <vector>

namespace apv
{


class APVFile
{
public:
    enum filetype_t
    {
        ftText          = 0x00,
        ftBinary        = 0x01,
    };

    enum mode_t
    {
        moNone          = 0x00,
        moTel           = 0x01,
        moTelCompany    = 0x02,
        moMemo          = 0x03,
        moSchedule      = 0x04,
        moSpreadsheet   = 0x05,
        moMail          = 0x06,
        moExpense       = 0x07,
        moClock         = 0x08,
        moDualWindow    = 0x09,
        moAddIn         = 0x0A,
        moQuickMemo     = 0x0B,

        moMaxVal        = 0xFFFFFFFF,
    };
    struct file_t
    {
    private:
        static const filetype_t filetypes[];

    public:
        mode_t                      mode;
        unsigned int                submode;
        //unsigned short      flags; // not yet required
        std::vector <unsigned char> file;

        filetype_t getFileType (void) const
        { return filetypes[static_cast <unsigned> (mode)]; }
        static filetype_t getFileType (mode_t mode)
        { return filetypes[static_cast <unsigned> (mode)]; }

        file_t (void)
         : mode (moMaxVal)
        {}
        file_t (const unsigned char* data, unsigned n,
            mode_t _mode, unsigned int _submode = ~0);

        file_t (const file_t& rhs);
        file_t& operator = (const file_t& rhs);
    };



private:
    struct fileheader100_t
    {
        unsigned char   signature[8];    // "APVFILE1"
        unsigned int    version;         // 100 | 101 | 102

        unsigned int    headerlen;

        char            filetype[4];     // uppercase file extension
        unsigned int    dummy;           // always 101

        unsigned int    subfilecnt;
    };

    struct fileheader101_t
    {
        fileheader100_t header;

        unsigned int    flags;           // default: 0
        mode_t          mode;            // default: moAddIn
    };

    struct fileinfo100_t
    {
        unsigned int  offs;
        unsigned int  size;
    };

    struct fileinfo102_t
    {
        fileinfo100_t fi;
        mode_t        mode;
        unsigned int  submode;
        unsigned int  flags;
    };

    std::vector <file_t> files;
    char filetype[5];

    void _initHeader (fileheader101_t& fhdr) const;

public:
    typedef std::vector <file_t>::iterator
        iterator;
    typedef std::vector <file_t>::const_iterator
        const_iterator;

    APVFile (void);
    APVFile (const APVFile& rhs);
    ~APVFile (void);

    APVFile& operator = (const APVFile& rhs);

    void clear (void);

    unsigned getFileCount (void) const
    { return files.size (); }

    const char* getFileType (void) const
    { return filetype; }
    void setFileType (const char* ftype);

    iterator begin (void);
    iterator end (void);
    const_iterator begin (void) const;
    const_iterator end (void) const;

    void addFile (const file_t& file);
    void removeFile (iterator i);

    file_t& firstFile (void)
    { return files.front (); }
    const file_t& firstFile (void) const
    { return files.front (); }
    file_t& lastFile (void)
    { return files.back (); }
    const file_t& lastFile (void) const
    { return files.back (); }

    void loadFromFile (const char* filename);
    void saveToFile (const char* filename) const;

    void swap (APVFile& rhs);
};


} // namespace apv

#endif // _APVFILE_HPP



 