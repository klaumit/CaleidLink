
#include "plugin/pvcomm.hpp"
#include <ucl/io/locale.hpp>

class PVCommRS232PlugIn : public ucl::oo::PlugInBase
{
public:
    PVCommRS232PlugIn (void)
     : ucl::oo::PlugInBase ("PVComm_rs232.dll")
    {
            // must be called here to make virtual functions available
        registerPlugIn (plugintypedescriptor);
    }
    virtual ~PVCommRS232PlugIn (void)
    {
            // same here
        unregisterPlugIn ();
    }

    virtual const char* getName (void) const;
    virtual const char* getDesc (void) const;
};


const char* PVCommRS232PlugIn::getName (void) const
{
	static std::string s;
	s = dgettext ("pvcomm.rs232", "RS232 Interface");
    return s.c_str ();
}
const char* PVCommRS232PlugIn::getDesc (void) const
{
	static std::string s;
	s = dgettext ("pvcomm.rs232", "Implements the RS232 interface for the CASIO "
		   "PV-x50X, PV-Sx50, PV-Sx60 and PV-750(+) models\n"
		   "(c) 2008 by AUDACIA Software");
	return s.c_str ();
}

void ucl_oo_globalRegisterPlugIn (void)
{
    static PVCommRS232PlugIn myPlugIn;
}



