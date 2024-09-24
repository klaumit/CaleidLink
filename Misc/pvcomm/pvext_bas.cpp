
#include "plugin/pvext.hpp"

#include <ucl/io/filestream.hpp>
#include <ucl/io/locale.hpp>


namespace apv
{

class PVWriteBAS : PVExtensionWriteHandler
{
private:
    static ucl::oo::FactoryObject <PVExtensionWriteHandler, PVWriteBAS> factoryObject;
    static const char* factorytbl[];

protected:
    virtual void saveAPVFile (const APVFile& apvfile, const char* filename);

public:
    PVWriteBAS (const char*) {}
};

class PVReadBAS : PVExtensionReadHandler
{
private:
    static ucl::oo::FactoryObject <PVExtensionReadHandler, PVReadBAS> factoryObject;
    static const char* factorytbl[];

protected:
    virtual void loadAPVFile (APVFile& apvfile, const char* filename);

public:
    PVReadBAS (const char*) {}
};


const char* PVWriteBAS::factorytbl[] = {
    ".bas",
    ".inc",
    ".app",
};
const char* PVReadBAS::factorytbl[] = {
    ".bas",
    ".inc",
    ".app",
};

ucl::oo::FactoryObject <PVExtensionWriteHandler, PVWriteBAS>
    PVWriteBAS::factoryObject (factorytbl, ucl::stl::arraysize (factorytbl));
ucl::oo::FactoryObject <PVExtensionReadHandler, PVReadBAS>
    PVReadBAS::factoryObject (factorytbl, ucl::stl::arraysize (factorytbl));



void PVWriteBAS::saveAPVFile (const APVFile& apvfile, const char* filename)
{
    ucl::io::FileStream fs (filename, "wt");
    APVFile::const_iterator i = apvfile.begin ();

    if (apvfile.getFileCount () > 1)
        throw std::runtime_error (dgettext ("pvcomm", "PVWriteBAS: Too many files"));

    fs.writeArray (&(i->file[0]), i->file.size ());
}
void PVReadBAS::loadAPVFile (APVFile& apvfile, const char* filename)
{
    ucl::io::FileStream fs (filename, "rt");
    APVFile afile;
    std::size_t s;

    afile.setFileType (".BAS");
    afile.addFile (APVFile::file_t (0, 2050, APVFile::moMemo));

    s = fs.tryReadArray (&(afile.begin ()->file[0]), 2050);
    if (s > 2048)
        throw std::runtime_error (dgettext ("pvcomm", "PVReadBAS: File too large"));

    afile.begin ()->file.resize (s);
    apvfile.swap (afile);
}



} // namespace apv
