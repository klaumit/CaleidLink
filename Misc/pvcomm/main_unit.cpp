//---------------------------------------------------------------------------

#define NO_WIN32_LEAN_AND_MEAN
#include <algorithm>
#include <vcl.h>
#include <shlobj.h>
#include <ole2.h>
#pragma hdrstop

#include "main_unit.h"
#include "aboutbox.h"
#include "gnugettext.hpp"
#include "dxgettext_forms.hpp"


//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFrmMain *FrmMain;

static const unsigned _PVFTP_COM_OPEN = 0;
static const unsigned _PVFTP_COM_CLOSE = 1;
static const unsigned _PVFTP_COM_FILEREAD = 2;
static const unsigned _PVFTP_COM_PING = 3;


//---------------------------------------------------------------------------
__fastcall TFrmMain::TFrmMain(TComponent* Owner)
    : TForm(Owner), status (stDisconnected), conn (false),
      tp (Handle, WM_COMM_TASK_MESSAGE, 1), closing (false), pvftp (NULL)
{
    AnsiString ftmask;
    std::string extstr;

        // initialize Drag&Drop
    onFileDrop = new TOnFileDrop (LbxFiles);
    onFileDrop->OnFileDrop = this->addFile;
    onFileDrop->OnDropEnd = this->RefreshList;

        // register form as translatable component
    ComponentList.registerComponent (this);

        // initialize status
    SetStatus (stDisconnected);

        // initialize controls
    for (unsigned i = 0; i < sizeof (apv::modedesc) / sizeof (apv::modedesc_t); ++i)
        CbxMode->Items->Add (apv::modedesc[i].desc);
    CbxMode->ItemIndex = sizeof (apv::modedesc) / sizeof (apv::modedesc_t) - 1;

    CbxModeChange (NULL);

    applySettings ();

    ucl::oo::PlugInManager::get ().setErrorHandler (
        ucl::oo::PlugInManager::errorfunc_t ().assign <TFrmMain, &TFrmMain::pluginmgr_errorhandler> (this));

    //apv::PVComm_pm.loadPlugIns ();
    ucl::oo::PlugInManager::get ().loadPlugInsFromRegistry (
        HKEY_LOCAL_MACHINE, "SOFTWARE\\AUDACIA\\PVComm\\PlugIns");
    ucl::oo::PlugInManager::get ().loadPlugInsFromRegistry (
        HKEY_CURRENT_USER,  "SOFTWARE\\AUDACIA\\PVComm\\PlugIns");

        // initialize the OpenDialog filter
    ftmask = gettext ("Known APV file types (%s)|%s|All files (*.*)|*.*");
    extstr = apv::PVExtensionReadHandler::getExtensionString ();
    /*OpdAddFile->Filter = AnsiString ()
        .sprintf (ftmask.c_str (),
            apv::PVComm_pm.getRegisteredExtensions ().c_str (),
            apv::PVComm_pm.getRegisteredExtensions ().c_str ());*/
    OpdAddFile->Filter = AnsiString ()
        .sprintf (ftmask.c_str (), extstr.c_str (), extstr.c_str ());
}

__fastcall TFrmMain::~TFrmMain (void)
{
    FormResize (NULL);

    delete pvftp;
}

void TFrmMain::pluginmgr_errorhandler (const char* appreq, const char* dllreq,
        const char* pluginname, ucl::oo::PlugInManager::PlugInError pe)
{
    AnsiString errtype = gettext ("Unknown");

    switch (pe)
    {
    case ucl::oo::PlugInManager::peIncompatibleABI:
        errtype = gettext ("Incompatible Application Binary Interface");
        break;

    case ucl::oo::PlugInManager::peIncompatibleApplicationSignature:
        errtype = gettext ("Incompatible application signature");
        break;

    case ucl::oo::PlugInManager::peIncompatibleApplicationInterfaceVersion:
        errtype = gettext ("Incompatible application interface version");
         break;
    }

    MessageBox (Handle, AnsiString ().sprintf (AnsiString (
            gettext ("ucl::oo::PlugInManager: Error while loading Plug-in\n"
                     "Plug-in DLL: %s\n"
                     "Error: %s\n"
                     "    Application: \"%s\"\n"
                     "    DLL: \"%s\"")).c_str (), pluginname, errtype, appreq, dllreq).c_str (),
                AnsiString (gettext ("Plug-in error")).c_str (), MB_ICONERROR);
}

void __fastcall TFrmMain::applySettings (void)
{
    //apv::PVFTPComm::device_t dev;

    TbrButtons->ShowCaptions = settings.ui_showbtntext;
    if (settings.ui_width < 0)
    {
        Width = 640;
        Height = 480;                            
        Position = poScreenCenter;
    }
    else
    {
        settings.changing = true;
        Left = settings.ui_xpos;
        Top = settings.ui_ypos;
        Width = settings.ui_width;
        Height = settings.ui_height;
        settings.changing = false;
    }

    ComponentList.switchLanguage (settings.ui_language);
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::BtnConnectClick(TObject *Sender)
{
    commtaskdata_t* ctd;

    switch (status)
    {
    case stDisconnected: // connect
        ctd = new commtaskdata_t (_PVFTP_COM_OPEN);
        ctd->filename = settings.comm_device;
        ctd->filename.Unique (); // no refcount across threads!
        ctd->speed = PVComm_Settings::speeds[settings.comm_speed_idx];
        tp.addTask <TFrmMain, &TFrmMain::PerformCommTask> (*this, ctd);
        SetStatus (stConnecting);
        break;

    case stConnecting:   // cancel
        if (pvftp)
            pvftp->cancel ();
        break;

    case stConnected:    // disconnect
        tp.cancelPendingTasks ();
        ctd = new commtaskdata_t (_PVFTP_COM_CLOSE);
        tp.addTask <TFrmMain, &TFrmMain::PerformCommTask> (*this, ctd);
        break;
    }
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::BtnCloseClick(TObject *Sender)
{
    Close ();
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::BtnAboutClick(TObject *Sender)
{
    static const TFrmAboutBox::AboutBoxInfo abi = {
        "PVComm Terminal",
        "Moritz Beutel",
        "",
        "www.audacia-software.de",
        0,
    };

    static const char* abcomp[] = {
        "Borland C++Builder 2006 (c) 2005 Borland Software Corporation",
        "ucl v0.2.0 (c) 2008 by AUDACIA Software",
        "TOnFileDrop v1.03 (c) by int 3 Software",
        "TranslateStandardExceptions (c) 2003 Early Ehlinger",
        "dxgettext v1.2.1 (c) Lars Dybdahl",
    };

    TFrmAboutBox::ShowAboutBox (0, abi,
        abcomp, abcomp + ucl::stl::arraysize (abcomp));
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::CommTaskMessage (ucl::win::ThreadPool::Message& Msg)
{
    ucl::stl::SmartPtr <ucl::win::ThreadPool::Task> task = Msg.getTaskAndFree ();

    AnsiString errmsg;
    commtaskdata_t* ctd;
    std::vector <apv::PVFTPComm::name_fp_t>::iterator i, e;
    unsigned idx;


    switch (Msg.getEvent ())
    {
    case ucl::win::ThreadPool::msgBusy:
        if (conn)
            SetStatus (stBusy);
        break;

    case ucl::win::ThreadPool::msgIdle:
        if (conn)
            SetStatus (stConnected);
        break;

    case ucl::win::ThreadPool::msgTaskCompleted:
        ctd = static_cast <commtaskdata_t*> (task->getTaskData ());
        switch (ctd->cmd)
        {
        case _PVFTP_COM_OPEN:
            conn = pvftp &&  pvftp->isConnected ();
            SetStatus (conn ? stConnected : stDisconnected);
            if (!conn) // connection failed
            {
                delete pvftp;
                pvftp = NULL;
            }
            else
                CbxModeChange (NULL);
            break;

        case _PVFTP_COM_CLOSE:
            /*conn = (pvftp != NULL);
            TmrCheckSrlState->Enabled = conn;
            SetStatus (conn ? stConnected : stDisconnected);
            if (conn)
                CbxModeChange (NULL);*/

            conn = false;
            delete pvftp; pvftp = 0;
            SetStatus (stDisconnected);

            if (closing) // avoid AV because form is not available
                Close ();
            break;

        case _PVFTP_COM_PING:
            if (!ctd->conn)
                SetStatus (stDisconnected);

        case apv::PVFTP_COM_NAMEDFILE:
            mode = ctd->mode;
            break;

        case apv::PVFTP_COM_NAMELIST:
            idx = 0;
            cfl.clear ();
            for (i = ctd->namelist->begin (), e = ctd->namelist->end ();
                 i != e; ++i, ++idx)
                cfl.addFile (i->name.c_str (), i->fp, idx);
            LbxFiles->Items = cfl.getStringList ();
            break;

        case _PVFTP_COM_FILEREAD:
        case apv::PVFTP_COM_FILEREAD:
        case apv::PVFTP_COM_FILEREMOVE:
        case apv::PVFTP_COM_FILEWRITE:
            // nothing to do
            break;

        case apv::PVFTP_COM_FPLIST:
        case apv::PVFTP_COM_FILECNT:
        case apv::PVFTP_COM_FILEXCHG:
            // not implemented
            break;
        }
        delete ctd;
        break;

    case ucl::win::ThreadPool::msgTaskTerminated:
        ctd = static_cast <commtaskdata_t*> (task->getTaskData ());
        delete ctd;
        break;
        
    case ucl::win::ThreadPool::msgException:
        errmsg = task->getErrorMessage ().c_str ();
        delete pvftp; pvftp = NULL; 
        SetStatus (stDisconnected);
        throw Exception (errmsg);
    }
}

//---------------------------------------------------------------------------
void TFrmMain::PerformCommTask (ucl::win::ThreadPool::Task& task)
{
    commtaskdata_t* ctd = static_cast <commtaskdata_t*> (task.getTaskData ());
    ucl::win::WindowsHandle <HANDLE> hFile;
    std::vector <PVCommFileList::file_t>::iterator i, e;
    //std::vector <ucl::stl::ArrayPtr <BYTE> > files;
    //std::vector <apv::PVFTPComm::file_t> files;
    apv::APVFile apvfile;
    

    switch (ctd->cmd)
    {
    case _PVFTP_COM_OPEN:
        // TODO: !!!

        delete pvftp;
        pvftp = apv::PVFTPComm::create (ctd->filename.c_str ());
        if (pvftp)
            pvftp->open (ctd->filename.c_str (), ctd->speed);
        else
            throw std::runtime_error (AnsiString (
                gettext ("Connection type not implemented!")).c_str ());
        break;
        
    case _PVFTP_COM_CLOSE:
        delete pvftp; pvftp = NULL;
        break;

    case _PVFTP_COM_PING:
        ctd->conn = pvftp ? (pvftp->isConnected ()) : false;
        break;
        
    case _PVFTP_COM_FILEREAD:
        for (i = ctd->item.files.begin (), e = ctd->item.files.end (); i != e; ++i)
        {
            apv::PVFTPComm::file_t file = pvftp->doFileRead (mode, i->fp);
            apvfile.addFile (apv::APVFile::file_t (file.get (), file.size (),
                static_cast <apv::APVFile::mode_t> (mode.mode), mode.submode));
        }
        apvfile.setFileType (ExtractFileExt (ctd->filename).UpperCase ().c_str ());
        apv::PVExtensionWriteHandler::saveToFile (apvfile, ctd->filename.c_str ());
        /*for (i = ctd->item.files.begin (), e = ctd->item.files.end (); i != e; ++i)
            files.push_back (pvftp->doFileRead (mode, i->fp));
        apv::PVComm_pm.savePVFile (files, ctd->filename.c_str (), mode.mode);*/
        break;

    case apv::PVFTP_COM_NAMEDFILE:
        ctd->mode = pvftp->doNamedFile (ctd->filename.c_str ());
        break;

    case apv::PVFTP_COM_NAMELIST:
        ctd->namelist = pvftp->doNameList (mode);
        break;

    case apv::PVFTP_COM_FILEREMOVE:
        pvftp->doFileRemove (mode, ctd->fp1);
        break;

    case apv::PVFTP_COM_FILEREAD:
        pvftp->doFileRead (mode, ctd->fp1);

        break;

    case apv::PVFTP_COM_FILEWRITE:
        pvftp->doFileWrite (mode, ctd->fp1, ctd->file.get (), ctd->file.size ());
        break;

    case apv::PVFTP_COM_FPLIST:
    case apv::PVFTP_COM_FILECNT:
    case apv::PVFTP_COM_FILEXCHG:
        // not implemented
        break;
    }
}


void __fastcall TFrmMain::RefreshList (TObject* Sender)
{
    commtaskdata_t* ctd;

    ctd = new commtaskdata_t (apv::PVFTP_COM_NAMELIST);
    tp.addTask <TFrmMain, &TFrmMain::PerformCommTask> (*this, ctd);
}

void __fastcall TFrmMain::RefreshFile(TObject *Sender)
{
    commtaskdata_t* ctd;

    if (apv::modedesc[CbxMode->ItemIndex].mode == 11)
    {
        ctd = new commtaskdata_t (apv::PVFTP_COM_NAMEDFILE);
        ctd->filename = EdtFileName->Text;
        ctd->filename.Unique (); // no refcount across threads!
        tp.addTask <TFrmMain, &TFrmMain::PerformCommTask> (*this, ctd);
    }
    else
    {
        mode.mode = apv::modedesc[CbxMode->ItemIndex].mode;
        mode.submode = apv::modedesc[CbxMode->ItemIndex]
            .submodes[CbxSubmode->ItemIndex].submode
            + (ChkModeSecret->Checked ? 16 : 0);
    }

    RefreshList (Sender);
}

//---------------------------------------------------------------------------

void __fastcall TFrmMain::SetStatus (status_t stat)
{
    WideString c_button[4];
        //= {"Connect", "Cancel", "Disconnect", "Disconnect"};
    WideString c_status[4];
    WideString c_busy;
        //= {"Not connected", "Connecting...", "Connected", "Connected"};

    unsigned idx = static_cast <unsigned> (stat);
    bool connected = (stat == stConnected) || (stat == stBusy);
    bool idle      = (stat == stConnected);


    c_button[0] = gettext ("Connect");
    c_button[1] = gettext ("Cancel");
    c_button[2] = gettext ("Disconnect");
    c_button[3] = c_button[2];

    c_status[0] = gettext ("Not connected");
    c_status[1] = gettext ("Connecting...");
    c_status[2] = gettext ("Connected");
    c_status[3] = c_status[2];

    c_busy = gettext ("Busy...");


    status = stat;
    StbStatus->Panels->Items[0]->Text = c_status[idx];
    StbStatus->Panels->Items[1]->Text = (stat == stBusy) ? c_busy : WideString ();
    StbStatus->Panels->Items[2]->Text = pvftp
                                      ? pvftp->getConnectedDeviceName ()
                                      : "";

    if (!connected)
        LbxFiles->Items->Clear ();

    LblSubmode->Enabled = idle;
    CbxSubmode->Enabled = idle;
    ChkModeSecret->Enabled = idle;

    LblFileName->Enabled = idle;
    EdtFileName->Enabled = idle;
    BtnFileName->Enabled = idle;
    LblMode->Enabled = idle;
    CbxMode->Enabled = idle;
    LblFiles->Enabled = idle;
    LbxFiles->Enabled = idle;

    TbnAddFiles->Enabled = idle;
    MniAddFiles->Enabled = idle;

    LbxFilesClick (0);

    TbnConnect->Caption = c_button[idx];
    TbnConnect->Hint = c_button[idx];
    MniConnectionActive->Checked = connected;
    IlsLED->GetIcon (connected ? 1 : 0, ImgStatus->Picture->Icon,
                     dsTransparent, itImage);
}

//---------------------------------------------------------------------------

void __fastcall TFrmMain::LbxFilesClick(TObject *Sender)
{
    bool cond = LbxFiles->Enabled && (LbxFiles->SelCount > 0) && (status != stBusy);
    TbnRemoveFiles->Enabled = cond;
    TbnExportFiles->Enabled = cond;
    MniRemoveFiles->Enabled = cond;
    MniExportFiles->Enabled = cond;
    PmiRemoveFiles->Enabled = cond;
    PmiExportFiles->Enabled = cond;
}
//---------------------------------------------------------------------------


void __fastcall TFrmMain::addFile (AnsiString filename)
{
    std::vector <ucl::stl::ArrayPtr <BYTE> > files;
    commtaskdata_t* ctd;
    apv::APVFile apvfile;

    apv::PVExtensionReadHandler::loadFromFile (apvfile, filename.c_str ());

        // check for modes
    for (apv::APVFile::iterator i = apvfile.begin (), e = apvfile.end ();
         i != e; ++i)
        if (i->mode != static_cast <apv::APVFile::mode_t> (mode.mode))
            throw Exception (AnsiString ().sprintf (AnsiString (
                gettext ("Wrong file mode (current: %d, required: %d)")).c_str (),
                static_cast <int> (i->mode), static_cast <int> (mode.mode)));

    for (apv::APVFile::iterator i = apvfile.begin (), e = apvfile.end ();
         i != e; ++i)
    {
        ctd = new commtaskdata_t (apv::PVFTP_COM_FILEWRITE);
        ctd->mode = mode;
        ctd->fp1 = 0xFFFF;
        ctd->file.alloc (i->file.size ());
        std::memcpy (ctd->file.get (), &(i->file[0]), i->file.size ());
        tp.addTask <TFrmMain, &TFrmMain::PerformCommTask> (*this, ctd);
    }
}

void __fastcall TFrmMain::BtnFileAddClick(TObject *Sender)
{
    if (!OpdAddFile->Execute ())
        return;

    for (unsigned i = 0, l = OpdAddFile->Files->Count; i < l; ++i)
        addFile (OpdAddFile->Files->Strings[i]);

    RefreshList (Sender);
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::CbxModeChange(TObject *Sender)
{
    bool _addin = (apv::modedesc[CbxMode->ItemIndex].mode == 11);
    bool addin = (status == stConnected) && _addin;
    bool not_addin = (status == stConnected) && !_addin;
    bool read_only;

    apv::modedesc_t* md = &apv::modedesc[CbxMode->ItemIndex];


    LblSubmode->Visible = not_addin;
    CbxSubmode->Visible = not_addin;
    ChkModeSecret->Visible = not_addin;

    LblFileName->Visible = addin;
    EdtFileName->Visible = addin;
    BtnFileName->Visible = addin;

    if (addin)
    {
        EdtFileName->Text = "SHARED";
        // TODO: registry setting
    }
    else if (not_addin)
    {
        CbxSubmode->Items->Clear ();
        for (unsigned i = 0; i < md->submodecnt; ++i)
            CbxSubmode->Items->Add (md->submodes[i].desc);
        CbxSubmode->ItemIndex = md->default_submode;

        read_only = md->submodes[md->default_submode].read_only;
        TbnAddFiles->Enabled = !read_only;
        MniAddFiles->Enabled = !read_only;
        TbnRemoveFiles->Enabled = !read_only;
        MniRemoveFiles->Enabled = !read_only;
        PmiRemoveFiles->Enabled = !read_only;
    }

    if (status == stConnected)
        RefreshFile (Sender);
}
//---------------------------------------------------------------------------



void __fastcall TFrmMain::FormCloseQuery(TObject *Sender, bool &CanClose)
{
    commtaskdata_t* ctd;

    switch (status)
    {
    case stDisconnected:
        CanClose = true;
        break;

    case stConnecting:
        tp.terminateAllTasks ();
        CanClose = true;
        break;

    case stConnected:
    case stBusy:
        tp.cancelPendingTasks ();
        ctd = new commtaskdata_t (_PVFTP_COM_CLOSE);
        tp.addTask <TFrmMain, &TFrmMain::PerformCommTask> (*this, ctd);
        CanClose = false;
        break;
    }
    closing = true;
}
//---------------------------------------------------------------------------


void __fastcall TFrmMain::BtnFileDeleteClick(TObject *Sender)
{
    unsigned l = cfl.itemCount ();
    std::vector <unsigned> indices;
    PVCommFileList::PFiles files;
    commtaskdata_t* ctd;


    if (settings.ui_confirmdelete)
    {
        String c_delcapt = gettext ("Confirm File Delete"),
               c_delq = gettext ("Really delete the selected files?");

        if (MessageBox (Handle, c_delq.c_str (), c_delcapt.c_str (),
            MB_YESNO | MB_ICONQUESTION) != IDYES)
            return;
    }
    
    indices.reserve (2 * LbxFiles->SelCount);

    for (int i = l - 1; i >= 0; --i)
    {
        if (LbxFiles->Selected[i])
            indices.push_back (i);
    }

    files = cfl.collectItems (indices.size (), &indices[0]);
    for (std::vector <PVCommFileList::file_t>::iterator i = files->begin (),
         e = files->end (); i != e; ++i)
    {
        ctd = new commtaskdata_t (apv::PVFTP_COM_FILEREMOVE);
        ctd->mode = mode;
        ctd->fp1 = i->fp;
        tp.addTask <TFrmMain, &TFrmMain::PerformCommTask> (*this, ctd);
    }

    RefreshList (Sender);
}
//---------------------------------------------------------------------------

static AnsiString selectDirecory (HWND hOwner, AnsiString text,
        UINT flags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE)
{
    BROWSEINFO bi;
    char buf[MAX_PATH];
    PItemIDList piidl;


    bi.hwndOwner      = hOwner;
    bi.pidlRoot       = NULL;
    bi.pszDisplayName = buf;
    bi.lpszTitle      = text.c_str ();
    bi.ulFlags        = BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
    bi.lpfn           = NULL;

    OleInitialize (NULL);
    CoInitialize (NULL);
    piidl = SHBrowseForFolder (&bi);
    if (piidl)
        if (SHGetPathFromIDList (piidl, buf))
            return AnsiString (buf);
    CoUninitialize ();
    OleUninitialize ();
    
    return AnsiString ();
}

void __fastcall TFrmMain::BtnFileExportClick(TObject *Sender)
{
    unsigned l = cfl.itemCount ();
    commtaskdata_t* ctd;
    AnsiString path, name;
    char c;


    if (LbxFiles->SelCount <= 0)
        return;

    path = selectDirecory (Handle, gettext ("Please select output directory:"));
    if (path.IsEmpty ())
        return;
    
    if (path[path.Length ()] != '\\')
        path += "\\";

    for (int i = l - 1; i >= 0; --i)
    {
        if (LbxFiles->Selected[i])
        {
            ctd = new commtaskdata_t (_PVFTP_COM_FILEREAD);
            ctd->mode = mode;
            name = cfl.getStringList ()->Strings[i];
            c = name[2];
            if ((name[1] == '!') && ((c == ' ')
                                  || (c == '+')
                                  || (c == '*')))
                name = name.SubString (3, name.Length () - 2)
                    + ((c == ' ') ? ".bas"
                                 : ((c == '+') ? ".app" : ".inc"));
            else if (ExtractFileExt (name).IsEmpty ())
                /*name += (apv::filetypes[mode.mode] == apv::ftText)
                      ? ".txt" : ".apv";*/
                name += (apv::APVFile::file_t::getFileType (
                        static_cast <apv::APVFile::mode_t> (mode.mode)) == apv::APVFile::ftText)
                    ? ".txt" : ".apv";
            ctd->filename = path + name;
            ctd->item = cfl.getItem (i);
            tp.addTask <TFrmMain, &TFrmMain::PerformCommTask> (*this, ctd);
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::TmrCheckSrlStateTimer(TObject *Sender)
{
    if (!conn)
        return;
    if (status == stBusy)
        return;

    if (pvftp)
    {
        if (!pvftp->isConnected ())
            if (status != stDisconnected)
                BtnConnectClick (Sender);
    }
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::EdtFileNameKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{                                    
    if (Key == VK_RETURN)
        RefreshFile (Sender);
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::BtnSettingsClick(TObject *Sender)
{
    static TFrmSettings* FrmSettings = NULL;

    if (!FrmSettings)
        Application->CreateForm (__classid (TFrmSettings), &FrmSettings);

    FrmSettings->ShowSettingsDialog (settings, status != stDisconnected);
    applySettings ();
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::EdtFileNameChange(TObject *Sender)
{
    int l = EdtFileName->Text.Length ();

    BtnFileName->Enabled = ((l > 0) && (l < 15));
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::CbxSubmodeChange(TObject *Sender)
{
    RefreshFile (Sender);    
}
//---------------------------------------------------------------------------


void __fastcall TFrmMain::FormResize(TObject *Sender)
{
    if (settings.changing)
        return;
    settings.ui_width = Width;
    settings.ui_height = Height;    
}

void __fastcall TFrmMain::FormMove (TWMMove& Msg)
{
    if (settings.changing)
        return;
    settings.ui_xpos = Left;
    settings.ui_ypos = Top;
}
//---------------------------------------------------------------------------

