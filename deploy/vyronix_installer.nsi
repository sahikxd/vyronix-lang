; VYRONIX Language Installer Script (NSIS)
; To compile: Install NSIS and run 'makensis vyronix_installer.nsi'

!define APP_NAME "VYRONIX"
!define COMP_NAME "VYRONIX Team"
!define VERSION "1.0.0"
!define EXE_NAME "vyronix.exe"

Name "${APP_NAME}"
OutFile "Vyronix_Setup_v${VERSION}.exe"
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKLM "Software\${APP_NAME}" "Install_Dir"

RequestExecutionLevel admin

Page directory
Page instfiles

Section "Install"
    SetOutPath "$INSTDIR"
    
    ; Add the executable
    File "..\vyronix.exe"
    
    ; Add some standard libraries or includes if they exist
    ; File /r "..\include"
    
    ; Registry keys for the uninstaller
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME} Language"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayIcon" "$INSTDIR\${EXE_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "Publisher" "${COMP_NAME}"
    
    ; Add to PATH (Experimental)
    ; Note: This requires a plugin or manual registry manipulation
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "VYRONIX_PATH" "$INSTDIR"
    
    WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\${EXE_NAME}"
    Delete "$INSTDIR\uninstall.exe"
    RMDir "$INSTDIR"
    
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
    DeleteRegKey HKLM "Software\${APP_NAME}"
SectionEnd
