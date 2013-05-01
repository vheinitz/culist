;InnoSetup installer-builder script for CULIST.
;V.Heinitz, 2012-05-03


[Setup]
AppName=CULIST
AppVerName=CULIST 0.1.0
OutputBaseFilename=CulistSetup_0.1.0
AppPublisher=HeinitzIT
AppPublisherURL=http://heinitz-it.de
AppSupportURL=http://heinitz-it.de
AppUpdatesURL=http://heinitz-it.de
AppCopyright=Copyright © 2013 HeinitzIT
DefaultDirName={pf}\\HeinitzIT\\CULIST
DisableDirPage=no
DefaultGroupName=HeinitzIT\CULIST
DisableProgramGroupPage=yes
AllowNoIcons=yes
OutputDir=..
SetupIconFile=..\culist.ico
Compression=lzma
WizardImageFile=.\res\culist.bmp
WizardSmallImageFile=.\res\culist.bmp
SolidCompression=yes
BackColor=clGreen

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Qt\4.8.4\bin\QtCore4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\4.8.4\bin\QtGui4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\4.8.4\bin\QtNetwork4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\4.8.4\bin\QtXml4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\4.8.4\bin\QtSql4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\CULIST.exe"; DestDir: "{app}"; Flags: ignoreversion; Permissions: admins-full;


[Icons]
Name: "{group}\CULIST"; Filename: "{app}\CULIST.exe"
Name: "{group}\{cm:UninstallProgram,CULIST}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\CULIST"; Filename: "{app}\CULIST.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\CULIST.exe"; Description: "{cm:LaunchProgram,CULIST}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCU; SubKey: Software\HeinitzIT\EXE; ValueType: string; ValueName: CULIST; ValueData: "{app}\CULIST.exe";
Root: HKCU; SubKey: Software\HeinitzIT\DATA; ValueType: string; ValueName: CULIST; ValueData: "{userdocs}\\HeinitzIT\\CULIST";
