
#ifndef _PVEXT_HPP
#define _PVEXT_HPP

#include <ucl/oo/factory.hpp>
#include <string>
#include "apvfile.hpp"


namespace apv
{


    // custom APVFile writer
class PVExtensionWriteHandler : protected ucl::oo::FactoryBase <PVExtensionWriteHandler>
{
protected:
    virtual void saveAPVFile (const APVFile& apvfile, const char* filename)
    { apvfile.saveToFile (filename); }

    PVExtensionWriteHandler (void) {}

public:
    static std::string getExtensionString (void);
    static void saveToFile (const APVFile& apvfile, const char* filename);

    virtual ~PVExtensionWriteHandler (void) {}
};


    // custom APVFile reader
class PVExtensionReadHandler : protected ucl::oo::FactoryBase <PVExtensionReadHandler>
{
protected:
    virtual void loadAPVFile (APVFile& apvfile, const char* filename)
    { apvfile.loadFromFile (filename); }

    PVExtensionReadHandler (void) {}

public:
    static std::string getExtensionString (void);
    static void loadFromFile (APVFile& apvfile, const char* filename);

    virtual ~PVExtensionReadHandler (void) {}
};


} // namespace apv

#endif // _PVEXT_HPP
