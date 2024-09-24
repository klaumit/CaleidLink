
#include "filelist.hpp"
#include <algorithm>


PVCommFileList::PVCommFileList (void)
 : slist (new TStringList)
{
    clear ();          
}
PVCommFileList::~PVCommFileList (void)
{
    delete slist;
}

void PVCommFileList::clear (void)
{
    slist->Clear ();
    ilist.clear ();

    ilist.reserve (50);
}
void PVCommFileList::addFile (AnsiString name, unsigned short fp, unsigned short index)
{
    unsigned n = 0, d;
    std::vector <listitem_t>::iterator i = ilist.begin (), e = ilist.end ();
    const char* p;
    bool done = false;

    
    p = std::strrchr (name.c_str (), '~');
    if (!name.IsEmpty ()) // file part
    {
        if (p)
        {
            d = p - name.c_str ();
            name.SetLength (d);
        }
        for (; i != e; ++i, ++n)
            if (slist->Strings[n] == name)
            {
                i->files.push_back (file_t (fp, index));
                done = true;
                break;
            }
    }

    if (!done)
    { // find place for insertion
        for (; i != e; ++i, ++n)
            if (*i < index)
                break;
        ilist.insert (i, listitem_t (index, fp));
        slist->Insert (n, name);
    }
}
PVCommFileList::PFiles PVCommFileList::collectItems (unsigned n, unsigned* indices)
{
    PFiles files;


    files.alloc ();
    files->reserve (2 * n);

    for (unsigned i = 0; i < n; ++i)
    {
        for (std::vector <file_t>::iterator v = ilist[indices[i]].files.begin (),
                                            e = ilist[indices[i]].files.end ();
             v != e; ++v)
            files->push_back (*v);
    }

    std::sort (files->rbegin (), files->rend ());

    return files;
}


