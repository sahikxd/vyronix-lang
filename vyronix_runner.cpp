/**
 * VYRONIX Runner v2 — Splash + Modern Dark GUI
 * Compile:
 *   g++ vyronix_runner.cpp -o dist/vyronix_runner.exe ^
 *       -mwindows -lgdi32 -lcomctl32 -lcomdlg32 -lshell32 ^
 *       -lgdiplus -static -std=c++17 -O2
 */

#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <string>
#include <filesystem>
#include <sstream>

using namespace Gdiplus;

// ── Colors ───────────────────────────────────────────────────────────────────
#define C_BG        RGB(10,  12,  16)
#define C_SURFACE   RGB(16,  20,  28)
#define C_CARD      RGB(22,  28,  38)
#define C_BORDER    RGB(38,  48,  64)
#define C_ACCENT    RGB(82, 162, 255)
#define C_ACCENT2   RGB(120, 200, 255)
#define C_SUCCESS   RGB(52, 199, 89)
#define C_ERROR     RGB(255, 69,  58)
#define C_WARN      RGB(255, 179,  0)
#define C_TEXT      RGB(220, 230, 242)
#define C_MUTED     RGB(120, 135, 158)
#define C_OUTPUT_BG RGB(8,   10,  14)

// ── GDI+ Color helpers ───────────────────────────────────────────────────────
inline Color GpColor(COLORREF c, BYTE a = 255) {
    return Color(a, GetRValue(c), GetGValue(c), GetBValue(c));
}

// ── Globals ──────────────────────────────────────────────────────────────────
HWND  hMain = nullptr, hSplash = nullptr;
HWND  hOutput = nullptr, hStatus = nullptr, hInput = nullptr;
HWND  hBtnRun = nullptr, hBtnCopy = nullptr, hBtnClear = nullptr;
HFONT hFontMono = nullptr, hFontUI = nullptr, hFontBold = nullptr;
HBRUSH hBrBg = nullptr, hBrCard = nullptr, hBrOutput = nullptr;
WNDPROC g_OldInputProc = nullptr;

std::wstring g_file;
std::wstring g_outputText;
bool g_running = false;
HICON g_hIcon = nullptr;

ULONG_PTR g_gdiplusToken = 0;

// Forward declarations
void AppendOut(const std::wstring& t);
DWORD WINAPI WorkerThread(LPVOID);

// ── Help System ──────────────────────────────────────────────────────────────
void ShowHelp() {
    std::wstring help = 
        L"Welcome to VYRONIX Help\r\n"
        L"──────────────────────────────────────────────\r\n"
        L"Commands:\r\n"
        L"  help    - Show this help message\r\n"
        L"  clear   - Clear the output window\r\n"
        L"  examples- List available example files\r\n"
        L"  run <file> - Run a specific example (e.g., run 01)\r\n"
        L"──────────────────────────────────────────────\r\n"
        L"Drag & Drop any .vx file to run it immediately!\r\n";
    AppendOut(help);
}

void ListExamples() {
    AppendOut(L"Available Examples:\r\n");
    namespace fs = std::filesystem;
    if (fs::exists("examples")) {
        for (const auto& entry : fs::directory_iterator("examples")) {
            if (entry.path().extension() == ".vx") {
                AppendOut(L"  • " + entry.path().filename().wstring() + L"\r\n");
            }
        }
    } else {
        AppendOut(L"  [examples/ folder not found]\r\n");
    }
}

void ProcessCommand(const std::wstring& cmd) {
    std::wstring low = cmd;
    for (auto& c : low) c = towlower(c);
    
    if (low == L"help" || low == L"helo") {
        ShowHelp();
    } else if (low == L"clear") {
        SetWindowTextW(hOutput, L"");
        g_outputText.clear();
    } else if (low == L"examples") {
        ListExamples();
    } else if (low.find(L"run ") == 0) {
        std::wstring target = cmd.substr(4);
        // Try to find matching example
        namespace fs = std::filesystem;
        if (fs::exists("examples")) {
            for (const auto& entry : fs::directory_iterator("examples")) {
                if (entry.path().filename().wstring().find(target) != std::wstring::npos) {
                    g_file = entry.path().wstring();
                    AppendOut(L"\r\n> Running " + entry.path().filename().wstring() + L"...\r\n");
                    CreateThread(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
                    return;
                }
            }
        }
        AppendOut(L"Error: Example '" + target + L"' not found.\r\n");
    } else if (!cmd.empty()) {
        AppendOut(L"Unknown command: " + cmd + L" (Type 'help' for commands)\r\n");
    }
}

LRESULT CALLBACK InputProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN && wp == VK_RETURN) {
        wchar_t buf[256];
        GetWindowTextW(hwnd, buf, 256);
        SetWindowTextW(hwnd, L"");
        ProcessCommand(buf);
        return 0;
    }
    return CallWindowProcW(g_OldInputProc, hwnd, msg, wp, lp);
}

// ── Run a process, capture stdout+stderr ─────────────────────────────────────
std::wstring RunProc(const std::wstring& cmd, bool& ok) {
    SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };
    HANDLE hR, hW;
    if (!CreatePipe(&hR, &hW, &sa, 0)) { ok = false; return L"[pipe error]"; }
    SetHandleInformation(hR, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = si.hStdError = hW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};
    std::wstring c = cmd;
    ok = !!CreateProcessW(nullptr, c.data(), nullptr, nullptr, TRUE,
                          CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(hW);

    std::string raw;
    char buf[4096]; DWORD n;
    while (ReadFile(hR, buf, sizeof(buf)-1, &n, nullptr) && n)
        raw.append(buf, n);
    CloseHandle(hR);

    DWORD exit = 0;
    if (ok) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &exit);
        ok = (exit == 0);
        CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    }

    int len = MultiByteToWideChar(CP_UTF8, 0, raw.c_str(), -1, nullptr, 0);
    std::wstring w(len, 0);
    if (MultiByteToWideChar(CP_UTF8, 0, raw.c_str(), -1, w.data(), len) == 0) {
        len = MultiByteToWideChar(CP_ACP, 0, raw.c_str(), -1, nullptr, 0);
        w.resize(len);
        MultiByteToWideChar(CP_ACP, 0, raw.c_str(), -1, w.data(), len);
    }
    return w;
}

// ── Append text to output box ─────────────────────────────────────────────────
void AppendOut(const std::wstring& t) {
    g_outputText += t;
    int n = GetWindowTextLengthW(hOutput);
    SendMessageW(hOutput, EM_SETSEL, n, n);
    SendMessageW(hOutput, EM_REPLACESEL, FALSE, (LPARAM)t.c_str());
    SendMessageW(hOutput, EM_SCROLL, SB_BOTTOM, 0);
}

void SetStatus(const std::wstring& t) { SetWindowTextW(hStatus, t.c_str()); }

// ── Compile + Run thread ──────────────────────────────────────────────────────
DWORD WINAPI WorkerThread(LPVOID) {
    g_running = true;
    EnableWindow(hBtnRun, FALSE);

    SetWindowTextW(hOutput, L""); g_outputText.clear();

    namespace fs = std::filesystem;
    fs::path vx(g_file);
    fs::path dir = vx.parent_path();
    std::wstring stem = vx.stem().wstring();
    std::wstring vxb  = (dir / (stem + L".vxb")).wstring();

    // ── Header ──
    AppendOut(L"--- Compiling " + vx.wstring() + L" ---\r\n");

    // ── Compile ──
    SetStatus(L"  ⚙  Compiling...");

    std::wstring compiler = (dir / L"vyronixc.exe").wstring();
    // fallback: same dir as runner
    if (!std::filesystem::exists(compiler)) {
        wchar_t me[MAX_PATH]; GetModuleFileNameW(nullptr, me, MAX_PATH);
        compiler = (fs::path(me).parent_path() / L"vyronixc.exe").wstring();
    }

    bool cOk = false;
    std::wstring cOut = RunProc(L"\"" + compiler + L"\" \"" + g_file + L"\" -o \"" + vxb + L"\"", cOk);
    if (!cOut.empty()) AppendOut(cOut + L"\r\n");

    if (!cOk) {
        AppendOut(L"\r\n  ✗  COMPILE FAILED\r\n");
        SetStatus(L"  ✗  Compile failed — see output for details");
        EnableWindow(hBtnRun, TRUE); g_running = false; return 0;
    }

    // ── Run ──
    SetStatus(L"  ▶  Running program...");
    AppendOut(L"--- Running ---\r\n");

    std::wstring vm = (dir / L"vyronixvm.exe").wstring();
    if (!std::filesystem::exists(vm)) {
        wchar_t me[MAX_PATH]; GetModuleFileNameW(nullptr, me, MAX_PATH);
        vm = (fs::path(me).parent_path() / L"vyronixvm.exe").wstring();
    }

    bool rOk = false;
    std::wstring rOut = RunProc(L"\"" + vm + L"\" \"" + vxb + L"\"", rOk);
    if (!rOut.empty()) AppendOut(rOut);

    if (rOk) {
        SetStatus(L"  ✓  Done — program exited successfully");
    } else {
        SetStatus(L"  ✗  Runtime error — check output above");
    }

    EnableWindow(hBtnRun, TRUE); g_running = false;
    return 0;
}

// ════════════════════════════════════════════════════════════════════════════
//  SPLASH SCREEN
// ════════════════════════════════════════════════════════════════════════════
LRESULT CALLBACK SplashProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static int frame = 0;
    static UINT_PTR timer = 0;

    switch (msg) {
    case WM_CREATE:
        timer = SetTimer(hwnd, 1, 16, nullptr); // ~60fps
        break;

    case WM_TIMER: {
        frame++;
        InvalidateRect(hwnd, nullptr, FALSE);

        // After ~2.5 seconds, close splash and show main
        if (frame > 155) {
            KillTimer(hwnd, timer);
            ShowWindow(hMain, SW_SHOW);
            UpdateWindow(hMain);
            if (!g_file.empty())
                CreateThread(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
            DestroyWindow(hwnd);
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        int W = rc.right, H = rc.bottom;

        // Double buffer
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBmp = CreateCompatibleBitmap(hdc, W, H);
        SelectObject(memDC, memBmp);

        Graphics g(memDC);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

        // Background gradient
        LinearGradientBrush bgGrad(
            Point(0, 0), Point(0, H),
            Color(255, 8, 10, 16),
            Color(255, 16, 22, 36));
        g.FillRectangle(&bgGrad, 0, 0, W, H);

        // Animated glow circle behind logo
        float progress = std::min(1.0f, (float)frame / 80.0f);
        float glowAlpha = (float)(sin(frame * 0.04) * 0.3 + 0.7) * 255 * progress;
        int glowR = (int)(120 * progress);

        // Outer glow rings
        for (int i = 3; i >= 0; i--) {
            int r = glowR + i * 18;
            BYTE a = (BYTE)(glowAlpha * (0.15f - i * 0.03f));
            SolidBrush gb(Color(a, 82, 162, 255));
            g.FillEllipse(&gb, W/2 - r, H/2 - 60 - r, r*2, r*2);
        }

        // ── Logo Square (Removed as requested) ──

        // ── VYRONIX text ──
        if (progress > 0.5f) {
            float tAlpha = std::min(1.0f, (progress - 0.5f) / 0.5f) * 255;
            FontFamily ff(L"Segoe UI");
            Font titleFont(&ff, 28, FontStyleBold, UnitPixel);
            Font subFont(&ff, 11, FontStyleRegular, UnitPixel);

            StringFormat sf;
            sf.SetAlignment(StringAlignmentCenter);

            SolidBrush titleBr(Color((BYTE)tAlpha, 220, 235, 255));
            SolidBrush mutedBr(Color((BYTE)tAlpha * 0.6f, 120, 140, 170));

            // Centered text positioning
            float textBaseY = H / 2.0f - 40;
            RectF titleRect((float)(W/2 - 160), textBaseY, 320, 40);
            g.DrawString(L"VYRONIX", -1, &titleFont, titleRect, &sf, &titleBr);

            RectF subRect((float)(W/2 - 160), textBaseY + 40, 320, 20);
            g.DrawString(L"PROGRAMMING LANGUAGE", -1, &subFont, subRect, &sf, &mutedBr);
        }

        // ── Loading bar ──
        {
            float barW = 200.0f;
            float barX = (W - barW) / 2.0f;
            float barY = H - 60.0f;
            float barH = 3.0f;

            // Track
            SolidBrush trackBr(Color(60, 82, 162, 255));
            g.FillRectangle(&trackBr, barX, barY, barW, barH);

            // Fill — animated
            float fill = std::min(1.0f, (float)frame / 150.0f) * barW;
            LinearGradientBrush fillGrad(
                PointF(barX, barY), PointF(barX + fill, barY),
                Color(255, 82, 162, 255),
                Color(255, 120, 200, 255));
            if (fill > 0)
                g.FillRectangle(&fillGrad, barX, barY, fill, barH);

            // Loading text
            FontFamily ff(L"Segoe UI");
            Font loadFont(&ff, 10, FontStyleRegular, UnitPixel);
            StringFormat sf; sf.SetAlignment(StringAlignmentCenter);
            SolidBrush loadBr(Color(140, 120, 140, 170));

            const wchar_t* msgs[] = {
                L"Initializing runtime...",
                L"Loading compiler...",
                L"Ready."
            };
            int msgIdx = std::min(2, (int)(frame / 55));
            RectF loadRect(barX - 50, barY + 10, barW + 100, 20);
            g.DrawString(msgs[msgIdx], -1, &loadFont, loadRect, &sf, &loadBr);
        }

        // Blit
        BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
        DeleteObject(memBmp);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ════════════════════════════════════════════════════════════════════════════
//  MAIN WINDOW
// ════════════════════════════════════════════════════════════════════════════
LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {

    case WM_CREATE: {
        hFontMono = CreateFontW(15,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,
                                CLEARTYPE_QUALITY,FIXED_PITCH|FF_MODERN,L"Consolas");
        hFontUI   = CreateFontW(13,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,
                                CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        hFontBold = CreateFontW(13,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,
                                CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");

        hBrBg     = CreateSolidBrush(C_BG);
        hBrCard   = CreateSolidBrush(C_CARD);
        hBrOutput = CreateSolidBrush(C_OUTPUT_BG);

        // Output
        hOutput = CreateWindowExW(0, L"EDIT", L"",
            WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL,
            10, 54, 760, 420, hwnd, (HMENU)101, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(hOutput, WM_SETFONT, (WPARAM)hFontMono, TRUE);

        // Input
        hInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
            10, 480, 760, 26, hwnd, (HMENU)102, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(hInput, WM_SETFONT, (WPARAM)hFontUI, TRUE);
        SendMessageW(hInput, EM_SETCUEBANNER, FALSE, (LPARAM)L"Type 'help' and press Enter...");
        g_OldInputProc = (WNDPROC)SetWindowLongPtrW(hInput, GWLP_WNDPROC, (LONG_PTR)InputProc);

        // Buttons
        hBtnRun = CreateWindowW(L"BUTTON", L"▶  Run File",
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            10, 514, 110, 32, hwnd, (HMENU)201, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(hBtnRun, WM_SETFONT, (WPARAM)hFontBold, TRUE);

        hBtnCopy = CreateWindowW(L"BUTTON", L"⎘  Copy",
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            128, 514, 90, 32, hwnd, (HMENU)202, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(hBtnCopy, WM_SETFONT, (WPARAM)hFontUI, TRUE);

        hBtnClear = CreateWindowW(L"BUTTON", L"✕  Clear",
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            226, 514, 90, 32, hwnd, (HMENU)203, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(hBtnClear, WM_SETFONT, (WPARAM)hFontUI, TRUE);

        // Status
        hStatus = CreateWindowW(L"STATIC", L"  Ready — open a .vx file to run",
            WS_CHILD|WS_VISIBLE|SS_LEFT,
            10, 556, 760, 20, hwnd, (HMENU)301, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(hStatus, WM_SETFONT, (WPARAM)hFontUI, TRUE);

        DragAcceptFiles(hwnd, TRUE);
        break;
    }

    case WM_COMMAND: {
        int id = LOWORD(wp);
        if (id == 201 && !g_running) { // Run
            if (g_file.empty()) {
                OPENFILENAMEW ofn{}; wchar_t buf[MAX_PATH]{};
                ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = hwnd;
                ofn.lpstrFilter = L"VYRONIX Source (*.vx)\0*.vx\0All Files\0*.*\0";
                ofn.lpstrFile = buf; ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_FILEMUSTEXIST;
                if (GetOpenFileNameW(&ofn)) {
                    g_file = buf;
                    std::wstring t = L"VYRONIX Runtime — " +
                        std::filesystem::path(g_file).filename().wstring();
                    SetWindowTextW(hwnd, t.c_str());
                    CreateThread(nullptr,0,WorkerThread,nullptr,0,nullptr);
                }
            } else {
                CreateThread(nullptr,0,WorkerThread,nullptr,0,nullptr);
            }
        }
        if (id == 202) { // Copy
            if (OpenClipboard(hwnd)) {
                EmptyClipboard();
                size_t bytes = (g_outputText.size()+1)*sizeof(wchar_t);
                HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, bytes);
                if (hg) {
                    memcpy(GlobalLock(hg), g_outputText.c_str(), bytes);
                    GlobalUnlock(hg);
                    SetClipboardData(CF_UNICODETEXT, hg);
                }
                CloseClipboard();
                SetStatus(L"  ✓  Output copied to clipboard!");
            }
        }
        if (id == 203) { // Clear
            SetWindowTextW(hOutput, L"");
            g_outputText.clear();
            SetStatus(L"  Cleared");
        }
        break;
    }

    case WM_DROPFILES: {
        HDROP hd = (HDROP)wp;
        wchar_t buf[MAX_PATH]; DragQueryFileW(hd, 0, buf, MAX_PATH);
        DragFinish(hd);
        g_file = buf;
        std::wstring t = L"VYRONIX Runtime — " +
            std::filesystem::path(g_file).filename().wstring();
        SetWindowTextW(hwnd, t.c_str());
        if (!g_running) CreateThread(nullptr,0,WorkerThread,nullptr,0,nullptr);
        break;
    }

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wp;
        RECT rc; GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, hBrBg);

        // ── Top header bar ──
        Graphics g(hdc);
        g.SetSmoothingMode(SmoothingModeAntiAlias);

        // Header bg
        LinearGradientBrush hdrGrad(
            Point(0, 0), Point(rc.right, 50),
            Color(255, 18, 24, 38), Color(255, 14, 18, 28));
        g.FillRectangle(&hdrGrad, 0, 0, rc.right, 50);

        // Accent line at bottom of header
        LinearGradientBrush accentLine(
            Point(0, 49), Point(rc.right, 49),
            Color(180, 82, 162, 255), Color(0, 82, 162, 255));
        g.FillRectangle(&accentLine, 0, 49, rc.right, 1);

        // Logo area
        if (g_hIcon) {
            DrawIconEx(hdc, 10, 9, g_hIcon, 32, 32, 0, nullptr, DI_NORMAL);
        } else {
            // Fallback: Logo square (mini)
            SolidBrush sqBr(Color(255, 26, 34, 50));
            g.FillRectangle(&sqBr, 10, 9, 32, 32);
            Pen sqPen(Color(180, 82, 162, 255), 1.0f);
            g.DrawRectangle(&sqPen, 10, 9, 32, 32);

            // VX in mini square
            Pen vPen(Color(220, 220, 235, 255), 2.0f);
            vPen.SetStartCap(LineCapRound); vPen.SetEndCap(LineCapRound);
            PointF vPts[] = { PointF(16,14), PointF(26,36), PointF(36,14) };
            g.DrawLines(&vPen, vPts, 3);
        }

        // Title text
        FontFamily ff(L"Segoe UI");
        Font titleF(&ff, 15, FontStyleBold, UnitPixel);
        Font fileF(&ff, 10, FontStyleRegular, UnitPixel);
        SolidBrush wBr(Color(255, 220, 235, 255));
        SolidBrush mBr(Color(180, 100, 130, 170));

        g.DrawString(L"VYRONIX", -1, &titleF, PointF(50, 10), &wBr);

        if (!g_file.empty()) {
            std::wstring fn = std::filesystem::path(g_file).filename().wstring();
            g.DrawString(fn.c_str(), -1, &fileF, PointF(52, 30), &mBr);
        } else {
            g.DrawString(L"No file selected", -1, &fileF, PointF(52, 30), &mBr);
        }

        // Button area bg
        RECT btnRc = {0, 504, rc.right, 548};
        FillRect(hdc, &btnRc, hBrCard);

        // Status bar bg
        RECT stRc = {0, 548, rc.right, rc.bottom};
        FillRect(hdc, &stRc, hBrBg);

        return 1;
    }

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wp;
        SetTextColor(hdc, C_TEXT);
        SetBkColor(hdc, C_OUTPUT_BG);
        return (LRESULT)hBrOutput;
    }

    case WM_CTLCOLORBTN:
        return (LRESULT)hBrCard;

    case WM_SIZE: {
        int W = LOWORD(lp), H = HIWORD(lp);
        if (hOutput)   SetWindowPos(hOutput,   nullptr, 10, 54, W-20, H-180, SWP_NOZORDER);
        if (hInput)    SetWindowPos(hInput,    nullptr, 10, H-120, W-20, 26, SWP_NOZORDER);
        if (hBtnRun)   SetWindowPos(hBtnRun,   nullptr, 10,  H-84, 110, 32, SWP_NOZORDER);
        if (hBtnCopy)  SetWindowPos(hBtnCopy,  nullptr, 128, H-84,  90, 32, SWP_NOZORDER);
        if (hBtnClear) SetWindowPos(hBtnClear, nullptr, 226, H-84,  90, 32, SWP_NOZORDER);
        if (hStatus)   SetWindowPos(hStatus,   nullptr, 10,  H-22, W-20, 20, SWP_NOZORDER);
        InvalidateRect(hwnd, nullptr, TRUE);
        break;
    }

    case WM_DESTROY:
        DeleteObject(hFontMono); DeleteObject(hFontUI); DeleteObject(hFontBold);
        DeleteObject(hBrBg); DeleteObject(hBrCard); DeleteObject(hBrOutput);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ════════════════════════════════════════════════════════════════════════════
//  WinMain
// ════════════════════════════════════════════════════════════════════════════
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    // GDI+
    GdiplusStartupInput gsi;
    GdiplusStartup(&g_gdiplusToken, &gsi, nullptr);

    // Args
    int argc; LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc > 1) g_file = argv[1];
    LocalFree(argv);

    // Common Controls
    INITCOMMONCONTROLSEX icx{ sizeof(icx), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&icx);

    // ── Register Splash class ──
    WNDCLASSEXW wcs{};
    wcs.cbSize = sizeof(wcs); wcs.lpfnWndProc = SplashProc;
    wcs.hInstance = hInst; wcs.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcs.lpszClassName = L"VxSplash";
    wcs.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcs.hIcon = g_hIcon ? g_hIcon : LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wcs);

    // ── Register Main class ──
    g_hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(101));
    if (!g_hIcon) {
        g_hIcon = (HICON)LoadImageW(nullptr, L"vyronix.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
        if (!g_hIcon) g_hIcon = (HICON)LoadImageW(nullptr, L"assets/vyronix.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    }

    WNDCLASSEXW wcm{};
    wcm.cbSize = sizeof(wcm); wcm.lpfnWndProc = MainProc;
    wcm.hInstance = hInst; wcm.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcm.lpszClassName = L"VxMain";
    wcm.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcm.hIcon = g_hIcon ? g_hIcon : LoadIconW(nullptr, IDI_APPLICATION);
    wcm.hIconSm = g_hIcon ? g_hIcon : LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wcm);

    // ── Create Main (hidden) ──
    std::wstring mainTitle = L"VYRONIX Output";
    if (!g_file.empty())
        mainTitle += L" — " + std::filesystem::path(g_file).filename().wstring();

    hMain = CreateWindowExW(
        WS_EX_ACCEPTFILES | WS_EX_APPWINDOW,
        L"VxMain", mainTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 820, 640,
        nullptr, nullptr, hInst, nullptr);

    // ── Create Splash (centered) ──
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int splW = 380, splH = 420;
    hSplash = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"VxSplash", L"VYRONIX",
        WS_POPUP,
        (sw - splW)/2, (sh - splH)/2, splW, splH,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(hSplash, SW_SHOW);
    UpdateWindow(hSplash);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    GdiplusShutdown(g_gdiplusToken);
    return (int)msg.wParam;
}