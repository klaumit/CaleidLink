
#include "apvfile.hpp"
#include <cstdlib>
#include <cstddef>
#include <string>
#include <stdexcept>
#include <utility>

#include <ucl/io/filestream.hpp>
#include <ucl/io/locale.hpp>


namespace apv
{

APVFile::file_t::file_t (const unsigned char* data, unsigned n,
    mode_t _mode, unsigned int _submode)
 : mode (_mode), submode (_submode), file (n)
{
    if (data)
        std::memcpy (&file[0], data, n);
}

const APVFile::filetype_t APVFile::file_t::filetypes[] = {
    ftBinary, ftText,   ftText,   ftText,   ftText,   ftBinary,
    ftBinary, ftText,   ftText,   ftText,   ftBinary, ftBinary,
};

void APVFile::_initHeader (fileheader101_t& fhdr) const
{
    std::memcpy (fhdr.header.signature, "APVFILE1", 8);
    fhdr.header.version = 101;
    fhdr.header.headerlen = sizeof (fileheader101_t);
    std::memcpy (fhdr.header.filetype, filetype, 4);
    fhdr.header.dummy = 101;
    fhdr.header.subfilecnt = files.size ();
    fhdr.flags = 0;

    if (fhdr.header.subfilecnt == 0)
        return;

    fhdr.mode = files.front ().mode; // verify that all files are for the same mode
    for (const_iterator i = files.begin (), e = files.end (); i != e; ++i)
        if (i->mode != fhdr.mode)
        {
            fhdr.header.version = 102;
            fhdr.header.headerlen = sizeof (fhdr.header);
            fhdr.mode = moMaxVal;
            break;
        }
}
APVFile::APVFile (void)
{
    std::strcpy (filetype, ".APV");
}
APVFile::~APVFile (void)
{}

void APVFile::clear (void)
{ files.clear (); }

APVFile::iterator APVFile::begin (void)
{ return files.begin (); }
APVFile::iterator APVFile::end (void)
{ return files.end (); }
APVFile::const_iterator APVFile::begin (void) const
{ return files.begin (); }
APVFile::const_iterator APVFile::end (void) const
{ return files.end (); }

void APVFile::addFile (const file_t& file)
{
    files.push_back (file);
}
void APVFile::removeFile (iterator i)
{
    files.erase (i);
}


void APVFile::loadFromFile (const char* filename)
{
    ucl::io::FileStream file (filename, "rb");
    unsigned hlen;
    int dv;
    fileheader101_t lfhdr;
    std::vector <file_t> lfiles;

    file.read (lfhdr.header);

    if (std::memcmp (lfhdr.header.signature, "APVFILE1", 8) != 0)
        throw std::runtime_error (dgettext ("apvfile", "APVFile: Wrong file format"));
    if ((lfhdr.header.version < 100) || (lfhdr.header.version > 102))
        throw std::runtime_error (dgettext ("apvfile", "APVFile: Wrong file format version"));

    if (lfhdr.header.version == 101) // not for 100 or 102
    {
        file.seek (0);
        file.read (lfhdr);
        hlen = sizeof (lfhdr);
    }
    else
    {
        hlen = sizeof (lfhdr.header);
        lfhdr.mode = moMaxVal;
        lfhdr.flags = 0;
    }


    dv = lfhdr.header.headerlen - hlen;
    if (dv > 0)
        file.seek (dv, SEEK_CUR);
    else if (dv < 0)
        throw std::runtime_error (dgettext ("apvfile", "APVFile: Wrong header size"));

    if (lfhdr.header.version < 102)
    {
        std::vector <fileinfo100_t> fdesc;

        fdesc.resize (lfhdr.header.subfilecnt);
        file.readArray (&fdesc[0], fdesc.size ());
        for (unsigned i = 0; i < lfhdr.header.subfilecnt; ++i)
        {
            file.seek (fdesc[i].offs);
            lfiles.push_back (file_t ());
            lfiles.back ().file.resize (fdesc[i].size);
            file.readArray (&(lfiles.back ().file[0]), fdesc[i].size);
            lfiles.back ().mode = lfhdr.mode;
            lfiles.back ().submode = ~0;
        }
    }
    else // >= 102
    {
        std::vector <fileinfo102_t> fdesc;

        fdesc.resize (lfhdr.header.subfilecnt);
        file.readArray (&fdesc[0], fdesc.size ());
        for (unsigned i = 0; i < lfhdr.header.subfilecnt; ++i)
        {
            file.seek (fdesc[i].fi.offs);
            lfiles.push_back (file_t ());
            lfiles.back ().file.resize (fdesc[i].fi.size);
            file.readArray (&(lfiles.back ().file[0]), fdesc[i].fi.size);
            lfiles.back ().mode = fdesc[i].mode;
            lfiles.back ().submode = fdesc[i].submode;
        }
    }

    std::memcpy (filetype, lfhdr.header.filetype, 4);
    filetype[4] = '\0';
    files.swap (lfiles); // do not change the state if an error occurs
}
void APVFile::saveToFile (const char* filename) const
{
    fileheader101_t lfhdr;
    ucl::io::FileStream file (filename, "wb");
    long offs;

    _initHeader (lfhdr);
    
    if (lfhdr.header.version == 101)
        file.write (lfhdr);
    else
        file.write (lfhdr.header);

    offs = file.tell ();
        // TODO: write files
    if (lfhdr.header.version < 102)
    {
        std::vector <fileinfo100_t> fdesc;
        fdesc.resize (lfhdr.header.subfilecnt);
        offs += sizeof (fileinfo100_t) * lfhdr.header.subfilecnt;

        for (unsigned i = 0; i < lfhdr.header.subfilecnt; ++i)
        {
            fdesc[i].offs = offs;
            fdesc[i].size = files[i].file.size ();
            offs += fdesc[i].size;
        }
        file.writeArray (&fdesc[0], fdesc.size ());
        for (unsigned i = 0; i < lfhdr.header.subfilecnt; ++i)
            file.writeArray (&(files[i].file[0]), files[i].file.size ());
    }
    else // >= 102
    {
        std::vector <fileinfo102_t> fdesc;
        fdesc.resize (lfhdr.header.subfilecnt);
        offs += sizeof (fileinfo100_t) * lfhdr.header.subfilecnt;

        for (unsigned i = 0; i < lfhdr.header.subfilecnt; ++i)
        {
            fdesc[i].fi.offs = offs;
            fdesc[i].fi.size = files[i].file.size ();
            fdesc[i].mode = files[i].mode;
            fdesc[i].submode = files[i].submode;
            fdesc[i].flags = 0;
            offs += fdesc[i].fi.size;
        }
        file.writeArray (&fdesc[0], fdesc.size ());
        for (unsigned i = 0; i < lfhdr.header.subfilecnt; ++i)
            file.writeArray (&(files[i].file[0]), files[i].file.size ());
    }
}

void APVFile::setFileType (const char* ftype)
{
    if (std::strlen (ftype) > 4)
        throw std::runtime_error (dgettext ("apvfile", "APVFile: File type description too long"));

    std::strncpy (filetype, ftype, 4);
}

void APVFile::swap (APVFile& rhs)
{
    files.swap (rhs.files);

        // yes, that is inefficient, but we do it for 5 bytes!
    for (unsigned i = 0; i < 4; ++i)
        std::swap (filetype[i], rhs.filetype[i]);
}

APVFile::APVFile (const APVFile& rhs)
{
    *this = rhs;
}

APVFile& APVFile::operator = (const APVFile& rhs)
{
    files = rhs.files;
    for (unsigned i = 0; i < 4; ++i)
        filetype[i] = rhs.filetype[i];
    return *this;
}

APVFile::file_t::file_t (const file_t& rhs)
 : mode (rhs.mode), submode (rhs.submode), file (rhs.file)
{}
APVFile::file_t& APVFile::file_t::operator = (const file_t& rhs)
{
    mode = rhs.mode;
    submode = rhs.submode;
    file = rhs.file;
    return *this;
}

} // namespace apv

