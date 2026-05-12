; VYRONIX Language Installer Script
; Requires EnVar plugin: https://nsis.sourceforge.io/EnVar_plug-in

!define APP_NAME "VYRONIX"
!define COMPILER_EXE "vyronixc.exe"
!define VM_EXE "vyronixvm.exe"
!define RUNNER_EXE "vyronix_runner.exe"
!define ICON_FILE "assets\vyronix.ico"
!define VERSION "1.0.0"
!define PUBLISHER "VYRONIX TEAM"

Name "${APP_NAME}"
OutFile "VyronixSetup-${VERSION}.exe"
InstallDir "$PROGRAMFILES64\Vyronix"
RequestExecutionLevel admin

; UI Settings
!include "MUI2.nsh"
!define MUI_ICON "${ICON_FILE}"
!define MUI_UNICON "${ICON_FILE}"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "Install"
    SetOutPath "$INSTDIR"
    
    ; Files to include
    File "dist\${COMPILER_EXE}"
    File "dist\${VM_EXE}"
    File "dist\${RUNNER_EXE}"
    File "assets\vyronix.ico"
    File "README.md"
    File "LICENSE.txt"
    
    SetOutPath "$INSTDIR\examples"
    File "examples\hello.vx"
    
    ; Registry for uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Vyronix" "DisplayName" "VYRONIX Programming Language"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Vyronix" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Vyronix" "DisplayIcon" "$INSTDIR\vyronix.ico"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Vyronix" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Vyronix" "Publisher" "${PUBLISHER}"

    ; File Association (.vx)
    WriteRegStr HKCR ".vx" "" "VyronixFile"
    WriteRegStr HKCR "VyronixFile" "" "VYRONIX Source File"
    WriteRegStr HKCR "VyronixFile\DefaultIcon" "" "$INSTDIR\vyronix.ico,0"
    WriteRegStr HKCR "VyronixFile\shell\open\command" "" '"$INSTDIR\${RUNNER_EXE}" "%1"'
    
    ; Refresh icons
    System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
    
    ; Add to PATH using EnVar plugin
    EnVar::AddValue "Path" "$INSTDIR"
    
    DetailPrint "VYRONIX has been added to PATH."
SectionEnd

Section "Uninstall"
    ; Remove from PATH
    EnVar::DeleteValue "Path" "$INSTDIR"
    
    ; Remove Registry keys
    DeleteRegKey HKCR ".vx"
    DeleteRegKey HKCR "VyronixFile"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Vyronix"
    
    ; Remove files
    Delete "$INSTDIR\${COMPILER_EXE}"
    Delete "$INSTDIR\${VM_EXE}"
    Delete "$INSTDIR\${RUNNER_EXE}"
    Delete "$INSTDIR\vyronix.ico"
    Delete "$INSTDIR\README.md"
    Delete "$INSTDIR\LICENSE.txt"
    Delete "$INSTDIR\examples\hello.vx"
    RMDir "$INSTDIR\examples"
    Delete "$INSTDIR\uninstall.exe"
    RMDir "$INSTDIR"
    
    DetailPrint "VYRONIX has been uninstalled."
SectionEnd
