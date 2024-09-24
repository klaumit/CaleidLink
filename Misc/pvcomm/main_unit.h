//---------------------------------------------------------------------------

#ifndef main_unitH
#define main_unitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ImgList.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <XPMan.hpp>
#include <Menus.hpp>
#include <ToolWin.hpp>

    // Standard library includes
#include <vector>

    // ucl includes
#include <ucl/win/thread.hpp>
#include <ucl/stl/meta.hpp>
#include <ucl/oo/pluginmgr.hpp>

    // plugin-related includes
#include "plugin/apvfile.hpp"
#include "plugin/pvftp.hpp"
#include "plugin/pvext.hpp"

    // application specific includes
#include "filelist.hpp"
#include "settings_unit.h"
#include "int3OnFileDrop.hpp"

#define WM_COMM_TASK_MESSAGE (WM_USER + 100)



//---------------------------------------------------------------------------
class TFrmMain : public TForm
{
__published:	// Von der IDE verwaltete Komponenten
    TImage *ImgStatus;
    TImageList *IlsLED;
    TLabel *LblFileName;
    TEdit *EdtFileName;
    TButton *BtnFileName;
    TListBox *LbxFiles;
    TLabel *LblFiles;
    TStatusBar *StbStatus;
    TOpenDialog *OpdAddFile;
    TLabel *LblMode;
    TComboBox *CbxMode;
    TLabel *LblSubmode;
    TTimer *TmrCheckSrlState;
    TXPManifest *XPManifest;
    TMainMenu *MainMenu;
    TToolBar *TbrButtons;
    TMenuItem *MniFile;
    TMenuItem *MniAddFiles;
    TMenuItem *MniExportFiles;
    TMenuItem *MniRemoveFiles;
    TMenuItem *N1;
    TMenuItem *MniExit;
    TMenuItem *MniConnection;
    TMenuItem *MniConnectionSettings;
    TMenuItem *N2;
    TMenuItem *MniConnectionActive;
    TMenuItem *MniHelp;
    TMenuItem *MniAbout;
    TToolButton *TbnAddFiles;
    TToolButton *TbnRemoveFiles;
    TToolButton *TbnExportFiles;
    TToolButton *TbnSettings;
    TToolButton *Tbn1;
    TToolButton *TbnConnect;
    TToolButton *TbnAbout;
    TToolButton *Tbn2;
    TImageList *ImlToolbarActive;
    TImageList *ImlToolbarInactive;
    TCheckBox *ChkModeSecret;
    TComboBox *CbxSubmode;
    TPopupMenu *PpmFiles;
    TMenuItem *PmiExportFiles;
    TMenuItem *PmiRemoveFiles;
    void __fastcall BtnConnectClick(TObject *Sender);
    void __fastcall BtnCloseClick(TObject *Sender);
    void __fastcall BtnAboutClick(TObject *Sender);
    void __fastcall RefreshFile(TObject *Sender);
    void __fastcall LbxFilesClick(TObject *Sender);
    void __fastcall BtnFileAddClick(TObject *Sender);
    void __fastcall CbxModeChange(TObject *Sender);
    void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
    void __fastcall BtnFileDeleteClick(TObject *Sender);
    void __fastcall BtnFileExportClick(TObject *Sender);
    void __fastcall TmrCheckSrlStateTimer(TObject *Sender);
    void __fastcall EdtFileNameKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall BtnSettingsClick(TObject *Sender);
    void __fastcall EdtFileNameChange(TObject *Sender);
    void __fastcall CbxSubmodeChange(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);

private:	// Benutzer-Deklarationen
    enum status_t
    {
        stDisconnected = 0,
        stConnecting = 1,
        stConnected = 2,
        stBusy = 3,
    };

    struct commtaskdata_t
    {
        unsigned                    cmd;

            // in
        AnsiString                  filename;
        WORD                        fp1, fp2;
        DWORD                       speed;
        PVCommFileList::listitem_t  item;
        ucl::stl::ArrayPtr <BYTE>   file;

            // out
        apv::PVFTPComm::mode_t      mode;
        unsigned                    filecnt;
        apv::PVFTPComm::namelist_t  namelist;
        apv::PVFTPComm::fplist_t    fplist;
        bool                        conn;

        commtaskdata_t (unsigned command)
         : cmd (command)
        {}
    };

    status_t status;
    bool     conn;
    apv::PVFTPComm* pvftp;
    ucl::win::ThreadPool tp;
    bool closing;
    TOnFileDrop* onFileDrop;

    PVComm_Settings settings;

    PVCommFileList cfl;
    apv::PVFTPComm::mode_t mode;

    void __fastcall FormMove (TWMMove& Msg);

    void __fastcall CommTaskMessage (ucl::win::ThreadPool::Message& Msg);
    void __fastcall SetStatus (status_t stat);
    void __fastcall RefreshList (TObject* Sender);
    void __fastcall applySettings (void);

    void __fastcall addFile (AnsiString filename);

    void PerformCommTask (ucl::win::ThreadPool::Task& task);

    void pluginmgr_errorhandler (const char* appreq, const char* dllreq,
        const char* pluginname, ucl::oo::PlugInManager::PlugInError pe);

    BEGIN_MESSAGE_MAP
        VCL_MESSAGE_HANDLER (WM_COMM_TASK_MESSAGE,
            ucl::win::ThreadPool::Message, CommTaskMessage)
        VCL_MESSAGE_HANDLER (WM_MOVE, TWMMove, FormMove)
    END_MESSAGE_MAP (TForm)

public:		// Benutzer-Deklarationen
    __fastcall TFrmMain(TComponent* Owner);
    __fastcall ~TFrmMain(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TFrmMain *FrmMain;
//---------------------------------------------------------------------------
#endif
