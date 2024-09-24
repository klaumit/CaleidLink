
#include "plugin/pvext.hpp"

#include <ucl/stl/smartptr.hpp>

#include <cstring>
#include <stdexcept>


namespace apv
{

UCL_OO_DEFINE_FACTORY (PVExtensionWriteHandler);
UCL_OO_DEFINE_FACTORY (PVExtensionReadHandler);


static std::string extractFileExtension (const char* filename)
{
    const char* ext;
    std::string extstr;
    
    for (ext = filename + std::strlen (filename); ext != filename; --ext)
        if (*ext == '.')
        {
            extstr = ext;
            std::strlwr (&extstr[0]);
            break;
        }

    return extstr;
}

void PVExtensionWriteHandler::saveToFile (const APVFile& apvfile, const char* filename)
{
    ucl::stl::SmartPtr <PVExtensionWriteHandler> wh;
    std::string extstr;

    extstr = extractFileExtension (filename);
    if (!extstr.empty ())
        wh.reset (create (extstr.c_str ()));

    if (wh.get ())
        wh->saveAPVFile (apvfile, filename);
    else // use the default file handler
        apvfile.saveToFile (filename);
}

void PVExtensionReadHandler::loadFromFile (APVFile& apvfile, const char* filename)
{
    ucl::stl::SmartPtr <PVExtensionReadHandler> rh;
    std::string extstr;

    extstr = extractFileExtension (filename);
    if (!extstr.empty ())
        rh.reset (create (extstr.c_str ()));

    if (rh.get ())
        rh->loadAPVFile (apvfile, filename);
    else // use the default file handler
        apvfile.loadFromFile (filename);
}


std::string PVExtensionWriteHandler::getExtensionString (void)
{
    std::string rv;
    FactoryType* factory;

    factory = &getFactory ();
    for (FactoryType::const_iterator i = factory->begin (),
                                     e = factory->end ();
         i != e; ++i)
        for (FactoryType::FactoryImplementation::const_iterator li = i->begin (),
                                                                le = i->end ();
             li != le; ++li)
            rv += std::string ("*") + *li + std::string (";");

        // it could be sooo simple with auto!
    /*for (auto i = factory->begin (), e = factory->end (); i != e; ++i)
        for (auto li = i->begin (), le = e->end (); li != le; ++li)
            rv += std::string ("*") + *li + std::string (";");*/

    rv.resize (rv.size () - 1); // remove the last ';'
    return rv;
}

std::string PVExtensionReadHandler::getExtensionString (void)
{
    std::string rv;
    FactoryType* factory;

    factory = &getFactory ();
    for (FactoryType::const_iterator i = factory->begin (),
                                     e = factory->end ();
         i != e; ++i)
        for (FactoryType::FactoryImplementation::const_iterator li = i->begin (),
                                                                le = i->end ();
             li != le; ++li)
            rv += std::string ("*") + *li + std::string (";");

    rv.resize (rv.size () - 1); // remove the last ';'
    return rv;
}



class PVDefaultWriteExtensions : PVExtensionWriteHandler
{
private:
    static ucl::oo::FactoryObject <PVExtensionWriteHandler, PVDefaultWriteExtensions> factoryObject;
    static const char* factorytbl[];

public:
    PVDefaultWriteExtensions (const char*) {}
};

class PVDefaultReadExtensions : PVExtensionReadHandler
{
private:
    static ucl::oo::FactoryObject <PVExtensionReadHandler, PVDefaultReadExtensions> factoryObject;
    static const char* factorytbl[];

public:
    PVDefaultReadExtensions (const char*) {}
};


const char* PVDefaultWriteExtensions::factorytbl[] = {
    ".apv",
    ".ags",
};
const char* PVDefaultReadExtensions::factorytbl[] = {
    ".apv",
    ".ags",
};

ucl::oo::FactoryObject <PVExtensionWriteHandler, PVDefaultWriteExtensions>
    PVDefaultWriteExtensions::factoryObject (factorytbl, ucl::stl::arraysize (factorytbl));
ucl::oo::FactoryObject <PVExtensionReadHandler, PVDefaultReadExtensions>
    PVDefaultReadExtensions::factoryObject (factorytbl, ucl::stl::arraysize (factorytbl));




} // namespace apv

