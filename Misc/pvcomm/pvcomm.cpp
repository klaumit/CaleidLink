//---------------------------------------------------------------------------

#include <vcl.h>
#include <gnugettext.hpp>
#include "plugin/pvcomm.hpp"
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("main_unit.cpp", FrmMain);
USEFORM("settings_unit.cpp", FrmSettings);
USEFORM("..\..\repository\AUDACIA\aboutbox\aboutbox.cpp", FrmAboutBox);
//---------------------------------------------------------------------------
UCL_OO_DEFINE_PLUGINMGR (plugintypedescriptor, ucl::oo::PlugInBase)
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        TP_GlobalIgnoreClassProperty (__classid (TAction), "Category");
        TP_GlobalIgnoreClassProperty (__classid (TControl), "HelpKeyword");
        TP_GlobalIgnoreClassProperty (__classid (TNotebook), "Pages");
        TP_GlobalIgnoreClass (__classid (TFont));
        DefaultInstance->textdomain ("pvcomm");

         Application->Initialize();
         Application->Title = "PVComm Terminal";
         Application->CreateForm(__classid(TFrmMain), &FrmMain);
         Application->Run();
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    catch (...)
    {
         try
         {
             throw Exception("");
         }
         catch (Exception &exception)
         {
             Application->ShowException(&exception);
         }
    }
    return 0;
}
//---------------------------------------------------------------------------
