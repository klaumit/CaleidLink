//---------------------------------------------------------------------------

#ifndef settings_unitH
#define settings_unitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

struct PVComm_Settings
{
private:
    void _writeToRegistry (void);

public:
    static const DWORD speeds[5];

    bool changing;

    DWORD       comm_speed_idx;
    AnsiString  comm_device;
    AnsiString  ui_language;
    bool        ui_showbtntext;
    bool        ui_confirmdelete;
    int         ui_xpos;
    int         ui_ypos;
    int         ui_width;
    int         ui_height;

    PVComm_Settings (void);  // loads settings from registry
    ~PVComm_Settings (void); // writes settings to registry
};

//---------------------------------------------------------------------------
class TFrmSettings : public TForm
{
__published:	// Von der IDE verwaltete Komponenten
    TLabel *LblDevice;
    TLabel *LblSpeed;
    TComboBox *CbxDevice;
    TComboBox *CbxSpeed;
    TButton *BtnClose;
    TButton *BtnOK;
    TGroupBox *GpbConnection;
    TGroupBox *GpbUI;
    TCheckBox *ChkShowBtnText;
    TCheckBox *ChkConfirmDelete;
    TButton *BtnResetScreenPosition;
    TComboBox *CbxLanguage;
    TLabel *LblLanguage;
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall BtnCloseClick(TObject *Sender);
    void __fastcall BtnOKClick(TObject *Sender);
    void __fastcall BtnResetScreenPositionClick(TObject *Sender);
private:	// Benutzer-Deklarationen
    bool save;
    PVComm_Settings* pps;

public:		// Benutzer-Deklarationen
    __fastcall TFrmSettings(TComponent* Owner);

    void __fastcall ShowSettingsDialog (PVComm_Settings& settings,
        bool connected);
};
//---------------------------------------------------------------------------
#endif
