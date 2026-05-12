!include "MUI2.nsh"
!include "WinMessages.nsh"

; ──────────────────────────────────────────────────────────────
; BASIC INFO
; ──────────────────────────────────────────────────────────────
Name "VYRONIX Programming Language"
OutFile "VYRONIX_Setup.exe"
InstallDir "$LOCALAPPDATA\VYRONIX"
InstallDirRegKey HKCU "Software\VYRONIX" ""
RequestExecutionLevel user ; Admin লাগবে না, User-level ইনস্টল সেফ

; ──────────────────────────────────────────────────────────────
; UI SETUP
; ──────────────────────────────────────────────────────────────
!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\vyronix.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Open VYRONIX REPL"
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

; ──────────────────────────────────────────────────────────────
; INSTALLATION
; ──────────────────────────────────────────────────────────────
Section "VYRONIX Core"
  SetOutPath "$INSTDIR"
  
  ; ফাইল কপি করুন (আপনার বিল্ড ফোল্ডার থেকে)
  File "vyronix.exe"
  File "vyx.exe"
  File "README.md"
  File "LICENSE"
  
  ; আনইনস্টলার লেখা
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
  ; রেজিস্ট্রি: Add/Remove Programs এ দেখাবে
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\VYRONIX" "DisplayName" "VYRONIX Programming Language"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\VYRONIX" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\VYRONIX" "DisplayIcon" '"$INSTDIR\vyronix.exe"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\VYRONIX" "DisplayVersion" "0.1.0"
  WriteRegStr HKCU "Software\VYRONIX" "" "$INSTDIR"
  
  ; 🔑 PATH এ অটো অ্যাড করবে (User Environment)
  Call AddToPath
  Call BroadcastEnvChange
  
  ; শর্টকাট তৈরি
  CreateDirectory "$SMPROGRAMS\VYRONIX"
  CreateShortCut "$SMPROGRAMS\VYRONIX\VYRONIX REPL.lnk" "$INSTDIR\vyronix.exe" "repl"
  CreateShortCut "$SMPROGRAMS\VYRONIX\VYRONIX Docs.lnk" "https://sahikxd.github.io"
  CreateShortCut "$DESKTOP\VYRONIX.lnk" "$INSTDIR\vyronix.exe"
SectionEnd

; ──────────────────────────────────────────────────────────────
; UNINSTALLATION
; ──────────────────────────────────────────────────────────────
Section "Uninstall"
  ; PATH থেকে রিমুভ
  Call RemoveFromPath
  Call BroadcastEnvChange
  
  ; শর্টকাট ডিলিট
  Delete "$DESKTOP\VYRONIX.lnk"
  RMDir /r "$SMPROGRAMS\VYRONIX"
  
  ; ফাইল ডিলিট
  Delete "$INSTDIR\vyronix.exe"
  Delete "$INSTDIR\vyx.exe"
  Delete "$INSTDIR\README.md"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\uninstall.exe"
  RMDir "$INSTDIR"
  
  ; রেজিস্ট্রি ক্লিনআপ
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\VYRONIX"
  DeleteRegKey HKCU "Software\VYRONIX"
SectionEnd

; ──────────────────────────────────────────────────────────────
; PATH MANAGEMENT FUNCTIONS (USER ENVIRONMENT)
; ──────────────────────────────────────────────────────────────
Function AddToPath
  Push $0
  ReadRegStr $0 HKCU "Environment" "Path"
  StrCmp $0 "" AddPath_Empty
  ; ডুপ্লিকেট চেক
  StrCmp $0 "$INSTDIR" AddPath_Exit
  StrCmp $0 "$INSTDIR;" AddPath_Exit
  StrCmp $0 ";$INSTDIR" AddPath_Exit
  StrCmp $0 ";$INSTDIR;" AddPath_Exit
  StrCpy $0 "$0;$INSTDIR"
  Goto AddPath_Write
AddPath_Empty:
  StrCpy $0 "$INSTDIR"
AddPath_Write:
  WriteRegExpandStr HKCU "Environment" "Path" "$0"
AddPath_Exit:
  Pop $0
FunctionEnd

Function RemoveFromPath
  Push $0
  Push $1
  ReadRegStr $0 HKCU "Environment" "Path"
  StrCmp $0 "" RemovePath_Exit
  
  ; ";$INSTDIR" বা "$INSTDIR;" বা "$INSTDIR" রিপ্লেস করে ফাঁকা বা সঠিক সেমিকোলন রাখা
  ; NSIS-এ সরাসরি StrReplace নেই, তাই লুপ ব্যবহার করে স্ট্রিং মডিফাই
  StrCpy $1 0
RemovePath_Loop:
  StrCpy $2 $0 1 $1
  StrCmp $2 "" RemovePath_Done
  StrCmp $2 ";" +2
  Goto RemovePath_Next
  StrCpy $3 $0 1 $1
  StrCmp $3 "$INSTDIR" +2
  Goto RemovePath_Next
  StrCpy $0 "$0" $1
  StrCpy $0 "$0" -1 $1
  StrCpy $0 "$0$3"
RemovePath_Next:
  IntOp $1 $1 + 1
  Goto RemovePath_Loop
RemovePath_Done:
  WriteRegExpandStr HKCU "Environment" "Path" "$0"
RemovePath_Exit:
  Pop $1
  Pop $0
FunctionEnd

Function BroadcastEnvChange
  SendMessage ${HWND_BROADCAST} ${WM_SETTINGCHANGE} 0 "STR:Environment" /TIMEOUT=5000
FunctionEnd
