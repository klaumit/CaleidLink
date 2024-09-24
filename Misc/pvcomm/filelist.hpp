
#ifndef _PVCOMM_FILELIST_HPP
#define _PVCOMM_FILELIST_HPP

#include <vcl.h>
#include <list>
#include <vector>
#include <ucl/stl/smartptr.hpp>


class PVCommFileList
{
public:
    struct file_t
    {
        unsigned short  fp;
        unsigned short  index;

        file_t (unsigned short _fp, unsigned short idx)
         : fp (_fp), index (idx)
        {}

        bool operator < (const file_t& rhs)
        { return (index < rhs.index); }
    };

    struct listitem_t
    {
        unsigned short  index;
        std::vector <file_t> files;

        bool operator < (const listitem_t& rhs)
        { return (index < rhs.index); }
        bool operator < (unsigned short index)
        { return (index < index); }

        listitem_t (void) {}
        listitem_t (unsigned short idx, unsigned short fp)
         : index (idx)
        { files.push_back (file_t (fp, idx)); }
    };

private:
    TStringList* slist;
    std::vector <listitem_t> ilist;


public:
    typedef ucl::stl::SmartPtr <std::vector <file_t> > PFiles;

    PVCommFileList (void);
    ~PVCommFileList (void);

    void clear (void);
    void addFile (AnsiString name, unsigned short fp, unsigned short index);
    PFiles collectItems (unsigned n, unsigned* indices);

    const listitem_t& getItem (unsigned idx)
    { return ilist[idx]; }
    unsigned itemCount (void) const
    { return ilist.size (); }

    TStringList* getStringList (void)
    { return slist; }
};



#endif // _PVCOMM_FILELIST_HPP
