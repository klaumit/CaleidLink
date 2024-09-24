
#include "plugin/pvcomm.hpp"
#include <ucl/io/locale.hpp>


class PVCommUSBPlugIn : public ucl::oo::PlugInBase
{
public:
    PVCommUSBPlugIn (void)
     : ucl::oo::PlugInBase ("PVComm_usb.dll")
    {
            // must be called here to make virtual functions available
        registerPlugIn (plugintypedescriptor);
    }
    virtual ~PVCommUSBPlugIn (void)
    {
            // same here
        unregisterPlugIn ();
    }

    virtual const char* getName (void) const;
    virtual const char* getDesc (void) const;
};


const char* PVCommUSBPlugIn::getName (void) const
{
	static std::string s;
	s = dgettext ("pvcomm.usb", "USB Interface");
	return s.c_str ();
}
const char* PVCommUSBPlugIn::getDesc (void) const
{
	static std::string s;
	s = dgettext ("pvcomm.usb", "Implements the USB interface for the CASIO PV-S1600 model\n"
		   "(c) 2008 by AUDACIA Software");
	return s.c_str ();
}

void ucl_oo_globalRegisterPlugIn (void)
{
    static PVCommUSBPlugIn myPlugIn;
}



