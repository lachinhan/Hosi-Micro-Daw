; ==============================================================================
; Inno Setup Script for LiveStream Micro-DAW & LiveStream OBS Receiver
; Auto-installs Standalone DAW, VST2 (.dll), and VST3 (.vst3) into system directories
; ==============================================================================

#define MyAppName "LiveStream Micro-DAW"
#define MyAppVersion "2.0.1"
#define MyAppPublisher "Hosi Studio"
#define MyAppURL "https://github.com/hosistudio/LiveStreamMicroDAW"
#define MyAppExeName "LiveStream Micro-DAW.exe"

[Setup]
AppId={{D814B4E1-995A-4B6A-9154-E826AE2F9F3B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=
OutputDir=..\installer_output
OutputBaseFilename=LiveStream_Micro_DAW_Setup
SetupIconFile=..\assets\icon.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\assets\icon.ico
UninstallDisplayName={#MyAppName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "installvst"; Description: "Tự động cài đặt OBS Receiver Plugin (VST2 / VST3) cho OBS Studio"; GroupDescription: "Cấu hình Plugin Livestream:"; Flags: checkedonce

[Files]
; 1. Standalone Application & Documentation
Source: "..\LiveStream Micro-DAW.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README_EN.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\HUONG_DAN_SU_DUNG.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\assets\icon.ico"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "..\assets\icon.png"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "..\sounds\*"; DestDir: "{app}\sounds"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist

; 2. OBS Receiver VST2 Plugin (.dll) -> System VST2 Directories
Source: "..\build\LiveStreamOBSReceiver_artefacts\Release\VST\LiveStream OBS Receiver.dll"; DestDir: "{commonpf}\VstPlugins"; Tasks: installvst; Flags: ignoreversion uninsrestartdelete
Source: "..\build\LiveStreamOBSReceiver_artefacts\Release\VST\LiveStream OBS Receiver.dll"; DestDir: "{commonpf}\Steinberg\VstPlugins"; Tasks: installvst; Flags: ignoreversion uninsrestartdelete
Source: "..\build\LiveStreamOBSReceiver_artefacts\Release\VST\LiveStream OBS Receiver.dll"; DestDir: "{commoncf}\VST2"; Tasks: installvst; Flags: ignoreversion uninsrestartdelete
Source: "..\build\LiveStreamOBSReceiver_artefacts\Release\VST\LiveStream OBS Receiver.dll"; DestDir: "{app}\plugins"; Flags: ignoreversion

; 3. OBS Receiver VST3 Plugin (.vst3 bundle) -> System VST3 Directory
Source: "..\build\LiveStreamOBSReceiver_artefacts\Release\VST3\LiveStream OBS Receiver.vst3\*"; DestDir: "{commoncf}\VST3\LiveStream OBS Receiver.vst3"; Tasks: installvst; Flags: ignoreversion recursesubdirs createallsubdirs uninsrestartdelete
Source: "..\build\LiveStreamOBSReceiver_artefacts\Release\VST3\LiveStream OBS Receiver.vst3\*"; DestDir: "{app}\plugins\LiveStream OBS Receiver.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\assets\icon.ico"
Name: "{autoprograms}\{#MyAppName}\Hướng dẫn sử dụng"; Filename: "{app}\HUONG_DAN_SU_DUNG.txt"
Name: "{autoprograms}\{#MyAppName}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\assets\icon.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
Filename: "{app}\HUONG_DAN_SU_DUNG.txt"; Description: "Mở tài liệu Hướng dẫn sử dụng"; Flags: postinstall shellexec skipifsilent unchecked

[Code]
// Custom Inno Pascal Script for automatic OBS detection and notification
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    // Create recordings directory if not exists
    if not DirExists(ExpandConstant('{app}\recordings')) then
      CreateDir(ExpandConstant('{app}\recordings'));
  end;
end;
