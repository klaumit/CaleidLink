; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=PVComm
AppVerName=PVComm v1.50

AppPublisher=AUDACIA Software
AppPublisherURL=http://www.audacia-software.de/
AppSupportURL=http://www.audacia-software.de/
AppUpdatesURL=http://www.audacia-software.de/
AppVersion=1.00
AppId={{93B794B1-13C7-4160-8134-5812E4DB0D07}
AppCopyright=(c) 2008 by AUDACIA Software

DefaultDirName={pf}\AUDACIA\PVComm
DefaultGroupName=AUDACIA Software\PVComm
AllowNoIcons=true
OutputDir=out
OutputBaseFilename=pvcomm150setup
Compression=lzma/ultra
SolidCompression=true
InternalCompressLevel=ultra
VersionInfoVersion=1.5
VersionInfoCompany=AUDACIA Software
VersionInfoDescription=PVComm Setup
VersionInfoCopyright=(c) 2008 by AUDACIA Software
PrivilegesRequired=admin
ShowLanguageDialog=yes
AppContact=support@audacia-software.de
SetupIconFile=setup.ico
InfoAfterFile=changes.rtf

[Languages]
Name: en; MessagesFile: compiler:Default.isl; LicenseFile: license.txt; InfoBeforeFile: info.rtf
Name: de; MessagesFile: compiler:Languages\german.isl; LicenseFile: license_d.txt; InfoBeforeFile: info_d.rtf

[CustomMessages]
en.DriverNotFound=Unable to locate PV-S1600 USB driver (CESG502.SYS). Please install the USB driver before installing the USB Interface Plug-in!
en.CDesc_app=PVComm Terminal
en.CDesc_client=PVComm Client for CASIO PV
en.CDesc_client_twf=As PCsync TWF file
en.CDesc_client_bas=As OWBasic Emulator Toolkit files
en.CDesc_intf=Device Interface Plug-ins
en.CDesc_intf_rs232=RS232 Interface
en.CDesc_intf_usb=USB Interface
en.CDesc_ext=File Format Plug-ins
en.CDesc_ext_txt=Text file handler (*.txt)
en.CDesc_source=Source Code
en.CDesc_source_plugin=Plug-in SDK for C++Builder

de.DriverNotFound=Der USB-Treiber f�r den PV-S1600 (CESG502.SYS) kann nicht gefunden werden. Bitte installieren Sie den USB-Treiber vor dem USB-Interface-Plugin!
de.CDesc_app=PVComm Terminal
de.CDesc_client=PVComm Client f�r CASIO PV
de.CDesc_client_twf=Als PCsync-TWF-Datei
de.CDesc_client_bas=Als OWBasic Emulator Toolkit-Dateien
de.CDesc_intf=Ger�teinterface-Plugins
de.CDesc_intf_rs232=RS232-Interface
de.CDesc_intf_usb=USB-Interface
de.CDesc_ext=Dateiformat-Plugins
de.CDesc_ext_txt=Textdatei-Handler (*.txt)
de.CDesc_source=Quelltext
de.CDesc_source_plugin=Plugin-SDK f�r C++Builder

[Components]
Name: app; Description: {cm:CDesc_app}; Flags: fixed; Types: custom compact full
Name: client; Description: {cm:CDesc_client}
Name: client\twf; Description: {cm:CDesc_client_twf}; Types: custom full
Name: client\bas; Description: {cm:CDesc_client_bas}; Types: full
Name: intf; Description: {cm:CDesc_intf}
Name: intf\rs232; Description: {cm:CDesc_intf_rs232}; Types: custom compact full
Name: intf\usb; Description: {cm:CDesc_intf_usb}; Types: custom full
Name: ext; Description: {cm:CDesc_ext}
Name: ext\txt; Description: {cm:CDesc_ext_txt}; Types: custom full
Name: source; Description: {cm:CDesc_source}
Name: source\plugin; Description: {cm:CDesc_source_plugin}; Types: full


[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}


[Dirs]
Name: {app}\plugin; Components: ext intf
Name: {app}\client; Components: client
Name: {app}\client\5; Components: client\bas
Name: {app}\sdk; Components: source\plugin
Name: {app}\locale
Name: {app}\locale\de
Name: {app}\locale\en
Name: {app}\locale\de\LC_MESSAGES
Name: {app}\locale\en\LC_MESSAGES


[Files]
Source: files\pvcomm.exe; DestDir: {app}; Flags: ignoreversion; Components: app
Source: files\locale\de\LC_MESSAGES\*.mo; DestDir: {app}\locale\de\LC_MESSAGES; Flags: ignoreversion; Components: app
Source: files\locale\en\LC_MESSAGES\*.mo; DestDir: {app}\locale\en\LC_MESSAGES; Flags: ignoreversion; Components: app

Source: files\client\*.twf; DestDir: {app}\client; Components: client\twf; Flags: ignoreversion
Source: files\client\PVComm Client.obp; DestDir: {app}\client; Components: client\bas; Flags: ignoreversion
Source: files\client\5\*.bas; DestDir: {app}\client\5; Components: client\bas; Flags: ignoreversion
Source: files\client\5\*.inc; DestDir: {app}\client\5; Components: client\bas; Flags: ignoreversion
Source: files\client\5\*.app; DestDir: {app}\client\5; Components: client\bas; Flags: ignoreversion

Source: files\sdk\*.*; DestDir: {app}\sdk; Flags: recursesubdirs createallsubdirs; Components: source\plugin

Source: files\PVComm_txt.dll; DestDir: {app}\plugin; Flags: ignoreversion; Components: ext\txt

Source: files\PVComm_rs232.dll; DestDir: {app}\plugin; Flags: ignoreversion; Components: intf\rs232

Source: files\PVComm_usb.dll; DestDir: {app}\plugin; Flags: ignoreversion; Components: intf\usb
Source: files\PVUSBAPI.dll; DestDir: {app}; Flags: ignoreversion; Components: intf\usb
Source: files\EnumDev.dll; DestDir: {app}; Flags: ignoreversion; Components: intf\usb


[Icons]
Name: {group}\PVComm Terminal; Filename: {app}\pvcomm.exe; IconIndex: 0
Name: {group}\{cm:UninstallProgram,PVComm}; Filename: {uninstallexe}
Name: {userdesktop}\PVComm Terminal; Filename: {app}\pvcomm.exe; Tasks: desktopicon; IconIndex: 0

[Run]
Filename: {app}\pvcomm.exe; Description: {cm:LaunchProgram,PVComm}; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKLM; Subkey: SOFTWARE\AUDACIA
Root: HKLM; Subkey: SOFTWARE\AUDACIA\PVComm; Flags: uninsdeletekey
Root: HKLM; Subkey: SOFTWARE\AUDACIA\PVComm\PlugIns; Components: ext intf; ValueType: none
Root: HKLM; Subkey: SOFTWARE\AUDACIA\PVComm\PlugIns; Components: ext\txt; ValueType: string; ValueName: PVComm_txt.dll; ValueData: {app}\plugin\PVComm_txt.dll; Tasks: 
Root: HKLM; Subkey: SOFTWARE\AUDACIA\PVComm\PlugIns; ValueType: string; ValueName: PVComm_usb.dll; ValueData: {app}\plugin\PVComm_usb.dll; Tasks: ; Components: intf\usb
Root: HKLM; Subkey: SOFTWARE\AUDACIA\PVComm\PlugIns; ValueType: string; ValueName: PVComm_rs232.dll; ValueData: {app}\plugin\PVComm_rs232.dll; Tasks: ; Components: intf\rs232

Root: HKCU; Subkey: Software\AUDACIA
Root: HKCU; Subkey: Software\AUDACIA\PVComm; Flags: uninsdeletekey
Root: HKCU; Subkey: Software\AUDACIA\PVComm\Terminal
Root: HKCU; Subkey: Software\AUDACIA\PVComm\Terminal; ValueType: string; ValueName: CommDevice; ValueData: {code:getCommDevice}
Root: HKCU; Subkey: Software\AUDACIA\PVComm\Terminal; ValueType: string; ValueName: UiLanguage; ValueData: {language}

[Code]
function getCommDevice (Param: String) : String;
begin
	if IsComponentSelected ('intf\rs232') then
		result := 'COM1'
	else if IsComponentSelected ('intf\usb') then
		result := 'USB'
	else
		result := '';
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
	result := true;
	if CurPageID <> 7 then
		exit;
	if IsComponentSelected ('intf\usb') then
	begin
		if not FileExists (ExpandConstant ('{sys}\drivers\CESG502.SYS')) then
			if not FileExists (ExpandConstant ('{win}\system32\drivers\CESG502.SYS')) then
			begin
				MsgBox (CustomMessage ('DriverNotFound'), mbError, MB_OK);
				result := false;
			end;
	end;
end;
