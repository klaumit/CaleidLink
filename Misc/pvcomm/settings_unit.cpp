//---------------------------------------------------------------------------

#include <vcl.h>
#include <winnt.h>
#pragma hdrstop

#include "settings_unit.h"
#include "dxgettext_forms.hpp"
#include "languagecodes.hpp"
#include "plugin/pvftp.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TFrmSettings::TFrmSettings(TComponent* Owner)
    : TForm(Owner), pps (NULL)
{
    ComponentList.registerComponent (this);
}
//---------------------------------------------------------------------------
void __fastcall TFrmSettings::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    if (Key == VK_ESCAPE)
        Close ();    
}
//---------------------------------------------------------------------------

void __fastcall TFrmSettings::BtnCloseClick(TObject *Sender)
{
    Close ();    
}
//---------------------------------------------------------------------------
void __fastcall TFrmSettings::ShowSettingsDialog (PVComm_Settings& settings,
    bool connected)
{
    unsigned idx, max;


    LblDevice->Enabled = !connected;
    CbxDevice->Enabled = !connected;
    LblSpeed->Enabled  = !connected;
    CbxSpeed->Enabled  = !connected;

        // copy settings to UI
    CbxSpeed->ItemIndex = settings.comm_speed_idx;
    ChkShowBtnText->Checked = settings.ui_showbtntext;

        // insert languages
    TStrings* csl = CbxLanguage->Items;
    TStringList* sl = new TStringList;

    csl->Clear ();
    DefaultInstance->GetListOfLanguages ("pvcomm", sl);
    max = sl->Count;
    for (unsigned i = 0; i < max; ++i)
    {
        csl->Add (getlanguagename (sl->Strings[i]));
        if (sl->Strings[i] == settings.ui_language)
            CbxLanguage->ItemIndex = i;
    }

        // insert communication protocols
    CbxDevice->Items->Clear ();
        // you see: the C++0x interpretation of 'auto' rocks!
    for (apv::PVFTPComm::FactoryType::const_iterator
            i = apv::PVFTPComm::getFactory ().begin (),
            e = apv::PVFTPComm::getFactory ().end ();
         i != e; ++i)
        for (apv::PVFTPComm::FactoryType::FactoryImplementation::const_iterator
                ii = i->begin (),
                ie = i->end ();
             ii != ie; ++ii)
            CbxDevice->Items->Add (*ii);
    CbxDevice->ItemIndex = CbxDevice->Items->IndexOf (settings.comm_device);

        // initialize and show form
    save = false;
    pps = &settings;
    Position = poMainFormCenter;
    ShowModal ();
    pps = NULL;

        // if OK, copy settings to structure
    if (save)
    {
        settings.comm_speed_idx = CbxSpeed->ItemIndex;
        settings.comm_device = CbxDevice->Text;
        settings.ui_showbtntext = ChkShowBtnText->Checked;
        settings.ui_language = sl->Strings[CbxLanguage->ItemIndex];
    }

    delete sl;
}

void __fastcall TFrmSettings::BtnOKClick(TObject *Sender)
{
    save = true;
    Close ();    
}
//---------------------------------------------------------------------------


PVComm_Settings::PVComm_Settings (void)
 : comm_speed_idx (2),
   comm_device (""),
   ui_language ("en"),
   ui_showbtntext (true),
   ui_confirmdelete (true),
   ui_xpos (0),
   ui_ypos (0),
   ui_width (-1),
   ui_height (-1),
   changing (false)
{
    HKEY hKey;
    DWORD size;
    DWORD val;


    RegOpenKeyEx (HKEY_CURRENT_USER, "SOFTWARE\\AUDACIA\\PVComm\\Terminal",
        0, KEY_READ, &hKey);

    if (!hKey) // create standard values
        _writeToRegistry ();
    else
    { // load entries
        size = sizeof (comm_speed_idx);
        RegQueryValueEx (hKey, "CommSpeedIdx", NULL, NULL,
            (BYTE*) &comm_speed_idx, &size);

        size = sizeof (ui_xpos);
        RegQueryValueEx (hKey, "UiXPos", NULL, NULL,
            (BYTE*) &ui_xpos, &size);
        size = sizeof (ui_ypos);
        RegQueryValueEx (hKey, "UiYPos", NULL, NULL,
            (BYTE*) &ui_ypos, &size);
        size = sizeof (ui_width);
        RegQueryValueEx (hKey, "UiWidth", NULL, NULL,
            (BYTE*) &ui_width, &size);
        size = sizeof (ui_height);
        RegQueryValueEx (hKey, "UiHeight", NULL, NULL,
            (BYTE*) &ui_height, &size);

        size = sizeof (val);
        if (RegQueryValueEx (hKey, "UiShowButtonsText", NULL, NULL,
            (BYTE*) &val, &size) == ERROR_SUCCESS)
            ui_showbtntext = val;

        size = sizeof (val);
        if (RegQueryValueEx (hKey, "UiConfirmDelete", NULL, NULL,
            (BYTE*) &val, &size) == ERROR_SUCCESS)
            ui_confirmdelete = val;

        size = 0;
        RegQueryValueEx (hKey, "UiLanguage", NULL, NULL, NULL, &size);
        if (size)
        {
            ui_language.SetLength (size - 1); // size contains '\0' character
            if (size > 1)
                RegQueryValueEx (hKey, "UiLanguage", NULL, NULL,
                    &ui_language[1], &size);
        }

        size = 0;
        RegQueryValueEx (hKey, "CommDevice", NULL, NULL, NULL, &size);
        if (size)
        {
            comm_device.SetLength (size - 1); // size contains '\0' character
            if (size > 1)
                RegQueryValueEx (hKey, "CommDevice", NULL, NULL,
                    &comm_device[1], &size);
        }

        RegCloseKey (hKey);
    }
}
PVComm_Settings::~PVComm_Settings (void)
{
    _writeToRegistry ();
}

void PVComm_Settings::_writeToRegistry (void)
{
    HKEY hKey;
    DWORD val;

    if (RegCreateKeyEx (HKEY_CURRENT_USER, "SOFTWARE\\AUDACIA\\PVComm\\Terminal",
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
        return; // dann halt nicht


        // TODO: to be solved using reflection

    RegSetValueEx (hKey, "UiXPos", 0, REG_DWORD,
        (const BYTE*) &ui_xpos, sizeof (ui_xpos));
    RegSetValueEx (hKey, "UiYPos", 0, REG_DWORD,
        (const BYTE*) &ui_ypos, sizeof (ui_ypos));
    RegSetValueEx (hKey, "UiWidth", 0, REG_DWORD,
        (const BYTE*) &ui_width, sizeof (ui_width));
    RegSetValueEx (hKey, "UiHeight", 0, REG_DWORD,
        (const BYTE*) &ui_height, sizeof (ui_height));

    RegSetValueEx (hKey, "CommSpeedIdx", 0, REG_DWORD,
        (const BYTE*) &comm_speed_idx, sizeof (comm_speed_idx));
    RegSetValueEx (hKey, "CommDevice", 0, REG_SZ,
        (const BYTE*) (comm_device.c_str ()), comm_device.Length () + 1);
    RegSetValueEx (hKey, "UiLanguage", 0, REG_SZ,
        (const BYTE*) (ui_language.c_str ()), ui_language.Length () + 1);
    val = ui_showbtntext;
    RegSetValueEx (hKey, "UiShowButtonsText", 0, REG_DWORD,
        (const BYTE*) &val, sizeof (val));
    val = ui_confirmdelete;
    RegSetValueEx (hKey, "UiConfirmDelete", 0, REG_DWORD,
        (const BYTE*) &val, sizeof (val));


    RegCloseKey (hKey);
}

const DWORD PVComm_Settings::speeds[5] = {
    CBR_9600,
    CBR_19200,
    CBR_38400,
    CBR_57600,
    CBR_115200,
};


void __fastcall TFrmSettings::BtnResetScreenPositionClick(TObject *Sender)
{
    pps->ui_xpos = 0;
    pps->ui_ypos = 0;
    pps->ui_width = -1;
    pps->ui_height = -1;
}
//---------------------------------------------------------------------------


