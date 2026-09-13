#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <uxtheme.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <tlhelp32.h>
#include <unordered_set>
#include <utility>
#include <vector>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "User32.lib")

namespace fs = std::filesystem;

// -----------------------------------------------------------------------------
// WinterStatic yt-dlp Downloader - native Win32 frontend
// -----------------------------------------------------------------------------

constexpr wchar_t APP_NAME[] = L"WinterStatic yt-dlp Downloader";
constexpr wchar_t APP_VERSION[] = L"0.1.39";
constexpr wchar_t MAIN_CLASS[] = L"WinterStaticYtDlpDownloaderWindow";
constexpr wchar_t SETTINGS_FILE[] = L"settings.ini";

constexpr COLORREF C_BG         = RGB(0, 0, 0);
constexpr COLORREF C_PANEL      = RGB(27, 27, 27);
constexpr COLORREF C_PANEL_EDGE = RGB(43, 43, 43);
constexpr COLORREF C_EDIT       = RGB(18, 18, 18);
constexpr COLORREF C_BUTTON     = RGB(51, 65, 85);
constexpr COLORREF C_BUTTON_HOT = RGB(63, 78, 99);
constexpr COLORREF C_BUTTON_EDGE = RGB(19, 24, 31);
constexpr COLORREF C_COMBO       = RGB(42, 42, 42);
constexpr COLORREF C_COMBO_HOT   = RGB(56, 56, 56);
constexpr COLORREF C_COMBO_EDGE  = RGB(18, 18, 18);
constexpr COLORREF C_COMBO_FOCUS = RGB(24, 24, 24);
constexpr COLORREF C_COMBO_INNER = RGB(58, 58, 58);
constexpr COLORREF C_PROGRESS_BG = RGB(38, 38, 38);
constexpr COLORREF C_PROGRESS_BAR = RGB(104, 104, 104);
constexpr COLORREF C_TEXT       = RGB(232, 232, 232);
constexpr COLORREF C_TEXT_DIM   = RGB(160, 160, 160);
constexpr COLORREF C_TEAL       = RGB(0, 106, 109);
constexpr COLORREF C_TEAL_SOFT  = RGB(60, 164, 168);
constexpr COLORREF C_GREEN      = RGB(40, 112, 75);
constexpr COLORREF C_RED        = RGB(126, 42, 42);
constexpr COLORREF C_BLUE       = RGB(25, 84, 100);
constexpr COLORREF C_WARN       = RGB(222, 170, 60);
constexpr COLORREF C_ERROR      = RGB(230, 92, 92);

constexpr UINT WM_APP_LOG            = WM_APP + 1;
constexpr UINT WM_APP_TOOL_STATUS    = WM_APP + 2;
constexpr UINT WM_APP_ACCOUNT_STATUS = WM_APP + 3;
constexpr UINT WM_APP_PROGRESS       = WM_APP + 4;
constexpr UINT WM_APP_TASK_DONE      = WM_APP + 5;
constexpr UINT WM_APP_JOB_LOG        = WM_APP + 6;

// Control IDs
constexpr int IDC_URL = 1001;
constexpr int IDC_PASTE = 1002;
constexpr int IDC_CLEAR = 1003;
constexpr int IDC_OUTPUT = 1004;
constexpr int IDC_BROWSE_OUTPUT = 1005;
constexpr int IDC_OPEN_OUTPUT = 1006;
constexpr int IDC_QUALITY = 1007;
constexpr int IDC_AUTH = 1008;
constexpr int IDC_FORMAT = 1009;

constexpr int IDC_ACCOUNT_STATUS = 1101;
constexpr int IDC_OPEN_BROWSER = 1102;
constexpr int IDC_CHECK_LOGIN = 1103;
constexpr int IDC_RESET_LOGIN = 1104;
constexpr int IDC_OPEN_BROWSER_PROFILE = 1105;

constexpr int IDC_YTDLP = 1201;
constexpr int IDC_BROWSE_YTDLP = 1202;
constexpr int IDC_UPDATE_YTDLP = 1203;
constexpr int IDC_YTDLP_STATUS = 1204;
constexpr int IDC_BROWSER_PATH = 1205;
constexpr int IDC_OPEN_BROWSER_TOOL = 1206;
constexpr int IDC_OPEN_BROWSER_PROFILE_2 = 1207;
constexpr int IDC_BROWSER_STATUS = 1208;
constexpr int IDC_FFMPEG = 1209;
constexpr int IDC_BROWSE_FFMPEG = 1210;
constexpr int IDC_DETECT_FFMPEG = 1211;
constexpr int IDC_FFMPEG_STATUS = 1212;

constexpr int IDC_DOWNLOAD = 1301;
constexpr int IDC_COPY_COMMAND = 1302;
constexpr int IDC_STOP = 1303;
constexpr int IDC_SNAP = 1304;
constexpr int IDC_REFRESH = 1305;
constexpr int IDC_STAGE = 1306;
constexpr int IDC_PROGRESS = 1307;
constexpr int IDC_DETAIL = 1308;
constexpr int IDC_CLOSE_POWERSHELL = 1309;
constexpr int IDC_BROWSER_SWEEP = 1310;
constexpr int IDC_LOG = 1401;
constexpr int IDC_TABS = 1501;

struct RectI { int x{}, y{}, w{}, h{}; };
struct PanelRects {
    RectI download;
    RectI account;
    RectI tools;
    RectI run;
    RectI log;
};

struct ToolStatus {
    std::wstring ytdlp;
    std::wstring ytdlpPath;
    std::wstring accountBrowser;
    std::wstring accountBrowserPath;
    std::wstring ffmpeg;
    std::wstring ffmpegPath;
};

struct AccountStatus {
    std::wstring text;
    COLORREF color{C_TEXT_DIM};
    bool valid{false};
};

struct DownloadJob {
    int id{0};
    std::wstring url;
    std::wstring output;
    std::wstring quality{L"Maximum 1080p"};
    std::wstring formatPreference{L"Automatic"};
    std::wstring auth{L"Automatic"};
    bool closePowerShellOnSuccess{false};
    bool browserSweep{false};

    int progressPercent{0};
    std::wstring stage{L"Ready"};
    std::wstring detail;
    std::wstring logText;
    bool running{false};
    bool finished{false};
    int exitCode{0};

    std::atomic<unsigned long long> generation{0};

    std::mutex progressMutex;
    int progressPassNumber{0};
    std::string currentDownloadKey;
    bool postProcessPassActive{false};

    std::mutex consoleMutex;
    DWORD consolePid{0};
    HWND consoleHwnd{nullptr};
    std::wstring consoleTitle;
    std::wstring logPath;
    std::wstring donePath;
    std::wstring taskDir;
};

struct ProgressUpdate {
    int jobId{0};
    int percent{0};
    std::wstring stage;
    std::wstring detail;
};

struct JobLogUpdate {
    int jobId{0};
    std::wstring text;
};

struct TaskDoneUpdate {
    int jobId{0};
    int code{1};
    unsigned long long generation{0};
};

HWND g_main = nullptr;
HWND g_tooltip = nullptr;
HFONT g_font = nullptr;
HFONT g_smallFont = nullptr;
HFONT g_boldFont = nullptr;
HFONT g_titleFont = nullptr;
HFONT g_logFont = nullptr;
HBRUSH g_bgBrush = nullptr;
HBRUSH g_panelBrush = nullptr;
HBRUSH g_editBrush = nullptr;
HBRUSH g_comboBrush = nullptr;
PanelRects g_panels{};

std::wstring g_exeDir;
std::wstring g_root;
std::wstring g_settingsPath;
std::wstring g_toolsDir;
std::wstring g_accountBrowserExe;
std::wstring g_accountBrowserUserData;
std::wstring g_accountBrowserProfile;

std::atomic<bool> g_accountSessionValid{false};
int g_progressPercent = 0;
bool g_startupLoginPending = true;

HWND g_tabs = nullptr;
std::vector<std::shared_ptr<DownloadJob>> g_jobs;
int g_activeJobIndex = -1;
int g_nextJobId = 1;
int g_hoverCloseTab = -1;
int g_pressedCloseTab = -1;

void SetProgressPercent(int percent);
std::shared_ptr<DownloadJob> ActiveJob();
std::shared_ptr<DownloadJob> FindJobById(int jobId);
void SaveActiveJobUi();
void LoadActiveJobUi();
void UpdateTabLabel(const std::shared_ptr<DownloadJob>& job);
void RefreshActiveJobControls();

// -----------------------------------------------------------------------------
// Utility
// -----------------------------------------------------------------------------

std::wstring JoinPath(const std::wstring& a, const std::wstring& b) {
    if (a.empty()) return b;
    fs::path p(a);
    p /= b;
    return p.wstring();
}

bool FileExists(const std::wstring& path) {
    std::error_code ec;
    return fs::is_regular_file(fs::path(path), ec);
}

bool DirExists(const std::wstring& path) {
    std::error_code ec;
    return fs::is_directory(fs::path(path), ec);
}

bool EnsureDir(const std::wstring& path) {
    std::error_code ec;
    if (DirExists(path)) return true;
    return fs::create_directories(fs::path(path), ec) || DirExists(path);
}

std::wstring GetExeDirectory() {
    std::vector<wchar_t> buffer(32768);
    DWORD n = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (!n || n >= buffer.size()) return L".";
    fs::path p(std::wstring(buffer.data(), n));
    return p.parent_path().wstring();
}

std::wstring Trim(const std::wstring& s) {
    const wchar_t* ws = L" \t\r\n";
    const size_t first = s.find_first_not_of(ws);
    if (first == std::wstring::npos) return L"";
    const size_t last = s.find_last_not_of(ws);
    return s.substr(first, last - first + 1);
}

std::string TrimA(const std::string& s) {
    const char* ws = " \t\r\n";
    const size_t first = s.find_first_not_of(ws);
    if (first == std::string::npos) return "";
    const size_t last = s.find_last_not_of(ws);
    return s.substr(first, last - first + 1);
}

std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int count = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    if (count <= 0) {
        count = MultiByteToWideChar(CP_ACP, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
        if (count <= 0) return L"";
        std::wstring out(count, 0);
        MultiByteToWideChar(CP_ACP, 0, s.data(), static_cast<int>(s.size()), out.data(), count);
        return out;
    }
    std::wstring out(count, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), count);
    return out;
}

std::string WideToUtf8(const std::wstring& s) {
    if (s.empty()) return "";
    int count = WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0, nullptr, nullptr);
    if (count <= 0) return "";
    std::string out(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), count, nullptr, nullptr);
    return out;
}

std::wstring GetControlText(HWND h) {
    int len = GetWindowTextLengthW(h);
    std::wstring value(static_cast<size_t>(len) + 1, L'\0');
    GetWindowTextW(h, value.data(), len + 1);
    value.resize(static_cast<size_t>(len));
    return value;
}

bool UsesPaddedStaticText(int id) {
    switch (id) {
    case 9101: case 9102: case 9103: case 9104: case 9105: case 9106:
    case 9201: case IDC_ACCOUNT_STATUS:
    case 9301: case 9302: case 9303:
    case IDC_YTDLP_STATUS: case IDC_BROWSER_STATUS: case IDC_FFMPEG_STATUS:
    case IDC_STAGE: case IDC_DETAIL:
        return true;
    default:
        return false;
    }
}

void SetText(int id, const std::wstring& text) {
    if (HWND h = GetDlgItem(g_main, id)) {
        if (UsesPaddedStaticText(id)) {
            const std::wstring padded = L" " + text;
            SetWindowTextW(h, padded.c_str());
        } else {
            SetWindowTextW(h, text.c_str());
        }
    }
}

std::wstring GetText(int id) {
    HWND h = GetDlgItem(g_main, id);
    return h ? GetControlText(h) : L"";
}

std::wstring TimeStamp() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    wchar_t buf[32]{};
    swprintf_s(buf, L"%02u:%02u:%02u", st.wHour, st.wMinute, st.wSecond);
    return buf;
}

std::shared_ptr<DownloadJob> ActiveJob() {
    if (g_activeJobIndex < 0 || g_activeJobIndex >= static_cast<int>(g_jobs.size())) return {};
    return g_jobs[static_cast<size_t>(g_activeJobIndex)];
}

std::shared_ptr<DownloadJob> FindJobById(int jobId) {
    for (const auto& job : g_jobs) {
        if (job && job->id == jobId) return job;
    }
    return {};
}

void TrimJobLog(std::wstring& text) {
    constexpr size_t kMaxChars = 1024 * 1024;
    constexpr size_t kTrimChars = 128 * 1024;
    if (text.size() > kMaxChars) text.erase(0, std::min(kTrimChars, text.size()));
}

void AppendLineToVisibleLog(const std::wstring& line) {
    HWND edit = GetDlgItem(g_main, IDC_LOG);
    if (!edit) return;
    const int len = GetWindowTextLengthW(edit);
    SendMessageW(edit, EM_SETSEL, len, len);
    SendMessageW(edit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(line.c_str()));
    SendMessageW(edit, EM_SCROLLCARET, 0, 0);
}

void AppendLogUi(const std::wstring& text) {
    const std::wstring line = L"[" + TimeStamp() + L"] " + text + L"\r\n";
    auto job = ActiveJob();
    if (job) {
        job->logText += line;
        TrimJobLog(job->logText);
    }
    AppendLineToVisibleLog(line);
}

void AppendJobLogUi(int jobId, const std::wstring& text) {
    auto job = FindJobById(jobId);
    if (!job) return;
    const std::wstring line = L"[" + TimeStamp() + L"] " + text + L"\r\n";
    job->logText += line;
    TrimJobLog(job->logText);
    auto active = ActiveJob();
    if (active && active->id == jobId) AppendLineToVisibleLog(line);
}

void PostLog(const std::wstring& text) {
    if (!g_main) return;
    auto* copy = new std::wstring(text);
    if (!PostMessageW(g_main, WM_APP_LOG, 0, reinterpret_cast<LPARAM>(copy))) delete copy;
}

void PostJobLog(const std::shared_ptr<DownloadJob>& job, const std::wstring& text) {
    if (!g_main || !job) return;
    auto* update = new JobLogUpdate{job->id, text};
    if (!PostMessageW(g_main, WM_APP_JOB_LOG, 0, reinterpret_cast<LPARAM>(update))) delete update;
}

std::wstring PsQuote(const std::wstring& s) {
    std::wstring out = L"'";
    for (wchar_t c : s) {
        if (c == L'\'') out += L"''";
        else out += c;
    }
    out += L"'";
    return out;
}

std::wstring WinArgQuote(const std::wstring& s) {
    if (s.find_first_of(L" \t\"") == std::wstring::npos) return s;
    std::wstring out = L"\"";
    size_t slashes = 0;
    for (wchar_t c : s) {
        if (c == L'\\') {
            ++slashes;
        } else if (c == L'\"') {
            out.append(slashes * 2 + 1, L'\\');
            out += L'\"';
            slashes = 0;
        } else {
            out.append(slashes, L'\\');
            slashes = 0;
            out += c;
        }
    }
    out.append(slashes * 2, L'\\');
    out += L'\"';
    return out;
}

std::wstring ReadIni(const wchar_t* section, const wchar_t* key, const std::wstring& fallback) {
    wchar_t buffer[32768]{};
    GetPrivateProfileStringW(section, key, fallback.c_str(), buffer, static_cast<DWORD>(std::size(buffer)), g_settingsPath.c_str());
    return buffer;
}

void WriteIni(const wchar_t* section, const wchar_t* key, const std::wstring& value) {
    WritePrivateProfileStringW(section, key, value.c_str(), g_settingsPath.c_str());
}

void SaveSettings() {
    SaveActiveJobUi();
    WriteIni(L"General", L"OutputDir", GetText(IDC_OUTPUT));
    WriteIni(L"General", L"YtDlpPath", GetText(IDC_YTDLP));
    WriteIni(L"General", L"FfmpegPath", GetText(IDC_FFMPEG));
    WriteIni(L"General", L"Quality", GetText(IDC_QUALITY));
    WriteIni(L"General", L"PreferredFormat", GetText(IDC_FORMAT));
    WriteIni(L"General", L"Authentication", GetText(IDC_AUTH));
    WriteIni(L"General", L"ClosePowerShellOnSuccess",
             IsDlgButtonChecked(g_main, IDC_CLOSE_POWERSHELL) == BST_CHECKED ? L"1" : L"0");
    WriteIni(L"General", L"BrowserAssistedRecovery",
             IsDlgButtonChecked(g_main, IDC_BROWSER_SWEEP) == BST_CHECKED ? L"1" : L"0");
}

std::wstring DefaultDownloads() {
    wchar_t user[MAX_PATH]{};
    DWORD n = GetEnvironmentVariableW(L"USERPROFILE", user, MAX_PATH);
    if (n && n < MAX_PATH) return JoinPath(user, L"Downloads");
    return JoinPath(g_root, L"Downloads");
}

std::wstring ResolveAppRelativePath(const std::wstring& value) {
    std::wstring path = Trim(value);
    if (path.empty()) return L"";
    fs::path p(path);
    if (p.is_relative()) p = fs::path(g_root) / p;
    return p.lexically_normal().wstring();
}

void ApplyDarkTitleBar(HWND hwnd) {
    BOOL enabled = TRUE;
    if (FAILED(DwmSetWindowAttribute(hwnd, 20, &enabled, sizeof(enabled)))) {
        DwmSetWindowAttribute(hwnd, 19, &enabled, sizeof(enabled));
    }
}

void ApplyDarkControl(HWND hwnd) {
    SetWindowTheme(hwnd, L"DarkMode_Explorer", nullptr);
}

bool StartsWithHttp(const std::wstring& s) {
    if (s.size() < 7) return false;
    std::wstring lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
    return lower.rfind(L"http://", 0) == 0 || lower.rfind(L"https://", 0) == 0;
}

std::wstring NormalizeUserUrl(const std::wstring& input) {
    std::wstring s = Trim(input);
    if (s.empty() || StartsWithHttp(s)) return s;

    // Protocol-relative links are already unambiguous web URLs.
    if (s.rfind(L"//", 0) == 0) return L"https:" + s;

    // Do not reinterpret an explicitly supplied non-HTTP scheme.
    if (s.find(L"://") != std::wstring::npos) return s;

    // Accept the common address-bar/clipboard form without a scheme, but only
    // when it still looks like a hostname rather than arbitrary pasted text.
    if (std::any_of(s.begin(), s.end(), [](wchar_t c) { return !!iswspace(c); }) ||
        s.find(L'\\') != std::wstring::npos) {
        return s;
    }

    const size_t hostEnd = s.find_first_of(L"/?#");
    const std::wstring host = s.substr(0, hostEnd);
    if (host.empty() || host.front() == L'.' || host.back() == L'.' ||
        host.find(L'.') == std::wstring::npos || host.find(L'@') != std::wstring::npos) {
        return s;
    }

    return L"https://" + s;
}

// -----------------------------------------------------------------------------
// Process helpers
// -----------------------------------------------------------------------------

std::wstring RunHiddenCapture(const std::wstring& commandLine, DWORD timeoutMs = 12000) {
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE readPipe = nullptr, writePipe = nullptr;
    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) return L"";
    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = writePipe;
    si.hStdError = writePipe;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION pi{};

    std::vector<wchar_t> cmd(commandLine.begin(), commandLine.end());
    cmd.push_back(L'\0');
    BOOL ok = CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE,
                             CREATE_NO_WINDOW, nullptr, g_root.c_str(), &si, &pi);
    CloseHandle(writePipe);
    if (!ok) {
        CloseHandle(readPipe);
        return L"";
    }

    const DWORD waitResult = WaitForSingleObject(pi.hProcess, timeoutMs);
    if (waitResult != WAIT_OBJECT_0) {
        // Do not fall through to a blocking ReadFile after the advertised timeout.
        // These captures are short helper probes (for example yt-dlp --version);
        // if one wedges, terminate that helper and let the caller treat it as no result.
        TerminateProcess(pi.hProcess, 1);
        WaitForSingleObject(pi.hProcess, 1000);
        CloseHandle(readPipe);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return L"";
    }

    std::string bytes;
    char buffer[4096];
    DWORD got = 0;
    while (ReadFile(readPipe, buffer, sizeof(buffer), &got, nullptr) && got) {
        bytes.append(buffer, buffer + got);
    }
    CloseHandle(readPipe);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return Trim(Utf8ToWide(bytes));
}

bool LaunchDetached(const std::wstring& exe, const std::wstring& args, const std::wstring& cwd = L"") {
    std::wstring cmd = WinArgQuote(exe);
    if (!args.empty()) cmd += L" " + args;
    std::vector<wchar_t> mutableCmd(cmd.begin(), cmd.end());
    mutableCmd.push_back(L'\0');

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessW(nullptr, mutableCmd.data(), nullptr, nullptr, FALSE, 0,
                             nullptr, cwd.empty() ? nullptr : cwd.c_str(), &si, &pi);
    if (!ok) return false;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

bool ProcessIdMatchesImage(DWORD pid, const wchar_t* imageName) {
    if (!pid || !imageName) return false;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    bool found = false;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (pe.th32ProcessID == pid && _wcsicmp(pe.szExeFile, imageName) == 0) {
                found = true;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return found;
}

std::wstring GetEnvVar(const wchar_t* name) {
    if (!name || !*name) return L"";
    DWORD needed = GetEnvironmentVariableW(name, nullptr, 0);
    if (!needed) return L"";
    std::vector<wchar_t> buffer(needed);
    DWORD written = GetEnvironmentVariableW(name, buffer.data(), needed);
    if (!written || written >= needed) return L"";
    return std::wstring(buffer.data(), written);
}

std::wstring ProcessCommandLine(DWORD pid) {
    if (!pid) return L"";

    // ProcessCommandLineInformation (60) is exposed by ntdll on supported Windows
    // versions. Resolve it dynamically so the frontend keeps its existing link set.
    using NtQueryInformationProcessFn = LONG (NTAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    static const auto ntQueryInformationProcess = []() -> NtQueryInformationProcessFn {
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll) return nullptr;
        return reinterpret_cast<NtQueryInformationProcessFn>(
            GetProcAddress(ntdll, "NtQueryInformationProcess"));
    }();
    if (!ntQueryInformationProcess) return L"";

    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) return L"";

    // A Windows command line cannot exceed 32K UTF-16 characters. 128 KiB leaves
    // ample room for the returned UNICODE_STRING header plus the command line.
    std::vector<unsigned char> buffer(128 * 1024);
    ULONG returned = 0;
    const LONG status = ntQueryInformationProcess(process, 60, buffer.data(),
                                                   static_cast<ULONG>(buffer.size()), &returned);
    CloseHandle(process);
    if (status < 0) return L"";

    struct NativeUnicodeString {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR Buffer;
    };
    const auto* text = reinterpret_cast<const NativeUnicodeString*>(buffer.data());
    if (!text->Buffer || text->Length == 0 || (text->Length % sizeof(wchar_t)) != 0) return L"";
    return std::wstring(text->Buffer, text->Length / sizeof(wchar_t));
}

bool SameAccountBrowserPath(const std::wstring& a, const std::wstring& b) {
    if (a.empty() || b.empty()) return false;
    const std::wstring left = fs::path(a).lexically_normal().wstring();
    const std::wstring right = fs::path(b).lexically_normal().wstring();
    return _wcsicmp(left.c_str(), right.c_str()) == 0;
}

bool CommandLineUsesUserDataDir(const std::wstring& commandLine, const std::wstring& userDataDir) {
    if (commandLine.empty() || userDataDir.empty()) return false;

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(commandLine.c_str(), &argc);
    if (!argv) return false;

    bool match = false;
    constexpr wchar_t prefix[] = L"--user-data-dir=";
    constexpr size_t prefixLen = (sizeof(prefix) / sizeof(prefix[0])) - 1;
    for (int i = 0; i < argc && !match; ++i) {
        const std::wstring arg = argv[i] ? argv[i] : L"";
        std::wstring candidate;
        if (_wcsicmp(arg.c_str(), L"--user-data-dir") == 0 && i + 1 < argc && argv[i + 1]) {
            candidate = argv[++i];
        } else if (arg.size() >= prefixLen && _wcsnicmp(arg.c_str(), prefix, prefixLen) == 0) {
            candidate = arg.substr(prefixLen);
        }
        if (!candidate.empty() && SameAccountBrowserPath(candidate, userDataDir)) match = true;
    }
    LocalFree(argv);
    return match;
}

bool AccountBrowserProfileInUse() {
    if (g_accountBrowserUserData.empty()) return false;

    // Chromium/Edge does not use the old Firefox-style profile lockfile that the
    // LibreWolf implementation relied on. Instead, identify only Edge processes
    // whose command line points at WinterStatic's dedicated --user-data-dir. This
    // deliberately ignores the user's normal Edge profile, which may remain open.
    std::wstring imageName = L"msedge.exe";
    if (!g_accountBrowserExe.empty()) {
        const std::wstring detectedName = fs::path(g_accountBrowserExe).filename().wstring();
        if (!detectedName.empty()) imageName = detectedName;
    }

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    bool inUse = false;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, imageName.c_str()) != 0) continue;
            const std::wstring commandLine = ProcessCommandLine(pe.th32ProcessID);
            if (CommandLineUsesUserDataDir(commandLine, g_accountBrowserUserData)) {
                inUse = true;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return inUse;
}

bool WaitForAccountBrowserProfileRelease(DWORD timeoutMs = 4000) {
    if (!AccountBrowserProfileInUse()) return true;
    const ULONGLONG deadline = GetTickCount64() + timeoutMs;
    while (GetTickCount64() < deadline) {
        Sleep(250);
        if (!AccountBrowserProfileInUse()) return true;
    }
    return !AccountBrowserProfileInUse();
}

// -----------------------------------------------------------------------------
// Tool detection
// -----------------------------------------------------------------------------

std::wstring PortableYtDlpPath() {
    return JoinPath(g_toolsDir, L"yt-dlp\\yt-dlp.exe");
}

std::wstring PortableAuthYtDlpPath() {
    return JoinPath(g_toolsDir, L"yt-dlp\\yt-dlp-auth.exe");
}

std::wstring PortableFfmpegPath() {
    return JoinPath(g_toolsDir, L"FFmpeg\\bin\\ffmpeg.exe");
}

std::wstring DetectYtDlpPath() {
    const std::vector<std::wstring> candidates = {
        PortableYtDlpPath(),
        JoinPath(g_root, L"yt-dlp.exe"),
        JoinPath(g_exeDir, L"yt-dlp.exe")
    };
    for (const auto& p : candidates) if (FileExists(p)) return p;

    wchar_t found[32768]{};
    DWORD n = SearchPathW(nullptr, L"yt-dlp.exe", nullptr,
                          static_cast<DWORD>(std::size(found)), found, nullptr);
    if (n && n < std::size(found)) return found;
    return L"";
}

std::wstring DetectFfmpegPath() {
    const std::vector<std::wstring> candidates = {
        PortableFfmpegPath(),
        JoinPath(g_toolsDir, L"FFmpeg\\ffmpeg.exe"),
        JoinPath(g_root, L"ffmpeg.exe"),
        JoinPath(g_exeDir, L"ffmpeg.exe")
    };
    for (const auto& p : candidates) if (FileExists(p)) return p;

    wchar_t found[32768]{};
    DWORD n = SearchPathW(nullptr, L"ffmpeg.exe", nullptr,
                          static_cast<DWORD>(std::size(found)), found, nullptr);
    if (n && n < std::size(found)) return found;
    return L"";
}

std::wstring YtDlpVersion(const std::wstring& path) {
    if (!FileExists(path)) return L"Missing";
    std::wstring output = RunHiddenCapture(WinArgQuote(path) + L" --version");
    if (output.empty()) return L"Detected";
    size_t line = output.find_first_of(L"\r\n");
    return line == std::wstring::npos ? output : output.substr(0, line);
}

std::wstring DetectEdgePath() {
    std::vector<std::wstring> candidates;
    auto add = [&](const std::wstring& base, const wchar_t* relative) {
        if (!base.empty()) candidates.push_back(JoinPath(base, relative));
    };
    add(GetEnvVar(L"ProgramFiles(x86)"), L"Microsoft\\Edge\\Application\\msedge.exe");
    add(GetEnvVar(L"ProgramFiles"), L"Microsoft\\Edge\\Application\\msedge.exe");
    add(GetEnvVar(L"LOCALAPPDATA"), L"Microsoft\\Edge\\Application\\msedge.exe");
    for (const auto& candidate : candidates) {
        if (FileExists(candidate)) return candidate;
    }

    wchar_t found[32768]{};
    DWORD n = SearchPathW(nullptr, L"msedge.exe", nullptr,
                          static_cast<DWORD>(std::size(found)), found, nullptr);
    if (n && n < std::size(found)) return found;
    return L"";
}

void RefreshToolStatusAsync() {
    const std::wstring ytdlp = GetText(IDC_YTDLP);
    const std::wstring currentFfmpeg = GetText(IDC_FFMPEG);
    std::thread([ytdlp, currentFfmpeg]() {
        auto* status = new ToolStatus;

        const std::wstring portableYtDlp = PortableYtDlpPath();
        if (FileExists(portableYtDlp)) {
            status->ytdlpPath = portableYtDlp;
        } else {
            status->ytdlpPath = Trim(ytdlp);
            if (status->ytdlpPath.empty() || !FileExists(status->ytdlpPath)) {
                status->ytdlpPath = DetectYtDlpPath();
            }
        }
        std::wstring primaryVersion;
        if (!status->ytdlpPath.empty() && FileExists(status->ytdlpPath)) {
            primaryVersion = YtDlpVersion(status->ytdlpPath);
            if (primaryVersion == L"Detected" || primaryVersion == L"Missing") primaryVersion.clear();
        }
        const std::wstring authYtDlpPath = PortableAuthYtDlpPath();
        std::wstring authVersion = FileExists(authYtDlpPath) ? YtDlpVersion(authYtDlpPath) : L"";
        if (authVersion == L"Detected" || authVersion == L"Missing") authVersion.clear();

        // Both yt-dlp backends are intentionally visible in the existing status field.
        // Keep the complete upstream version strings so screenshots are unambiguous.
        if (status->ytdlpPath.empty() || !FileExists(status->ytdlpPath)) {
            status->ytdlp = L"Missing";
        } else if (primaryVersion.empty()) {
            // The executable exists, but its version could not be read. Preserve
            // the long-standing distinction between "present" and "missing".
            status->ytdlp = L"Detected";
        } else if (!authVersion.empty()) {
            status->ytdlp = primaryVersion + L"+" + authVersion;
        } else {
            status->ytdlp = primaryVersion + L"+--";
        }

        status->accountBrowserPath = DetectEdgePath();
        status->accountBrowser = !status->accountBrowserPath.empty()
            ? L"Detected — dedicated profile" : L"Missing";

        const std::wstring portableFfmpeg = PortableFfmpegPath();
        if (FileExists(portableFfmpeg)) {
            status->ffmpegPath = portableFfmpeg;
        } else {
            status->ffmpegPath = Trim(currentFfmpeg);
            if (status->ffmpegPath.empty() || !FileExists(status->ffmpegPath)) {
                status->ffmpegPath = DetectFfmpegPath();
            }
        }
        status->ffmpeg = (!status->ffmpegPath.empty() && FileExists(status->ffmpegPath))
            ? L"Detected — full quality" : L"Missing — fallback";

        if (!PostMessageW(g_main, WM_APP_TOOL_STATUS, 0,
                          reinterpret_cast<LPARAM>(status))) {
            delete status;
        }
    }).detach();
}

// -----------------------------------------------------------------------------
// Dedicated Microsoft Edge account profile
// -----------------------------------------------------------------------------

void EnsureAccountBrowserProfile() {
    EnsureDir(g_accountBrowserUserData);
    EnsureDir(g_accountBrowserProfile);
}

bool BinaryFileContainsAny(const std::wstring& path, const std::vector<std::string>& needles) {
    std::ifstream file(fs::path(path), std::ios::binary);
    if (!file) return false;
    constexpr size_t CHUNK = 1024 * 1024;
    std::string carry;
    std::vector<char> buf(CHUNK);
    while (file) {
        file.read(buf.data(), static_cast<std::streamsize>(buf.size()));
        const std::streamsize got = file.gcount();
        if (got <= 0) break;
        std::string block = carry;
        block.append(buf.data(), static_cast<size_t>(got));
        for (const auto& needle : needles) {
            if (block.find(needle) != std::string::npos) return true;
        }
        const size_t keep = 128;
        carry = block.size() > keep ? block.substr(block.size() - keep) : block;
    }
    return false;
}

std::wstring AccountCookieDatabase() {
    const std::wstring modern = JoinPath(g_accountBrowserProfile, L"Network\\Cookies");
    if (FileExists(modern)) return modern;
    const std::wstring legacy = JoinPath(g_accountBrowserProfile, L"Cookies");
    if (FileExists(legacy)) return legacy;
    return modern;
}

AccountStatus InspectAccountSession() {
    AccountStatus result;
    if (g_accountBrowserExe.empty() || !FileExists(g_accountBrowserExe)) {
        result.text = L"Microsoft Edge was not found";
        result.color = C_WARN;
        return result;
    }
    if (!WaitForAccountBrowserProfileRelease()) {
        result.text = L"WinterStatic Edge profile is open — close it before using saved cookies";
        result.color = C_WARN;
        return result;
    }

    const std::wstring db = AccountCookieDatabase();
    if (!FileExists(db)) {
        result.text = L"No dedicated Edge cookie database yet";
        result.color = C_WARN;
        return result;
    }

    // Chromium keeps cookie names as readable SQLite fields even though cookie
    // values are encrypted. This lightweight check only decides whether the UI
    // should try authenticated mode; yt-dlp remains the final authority.
    const std::vector<std::string> authNames = {
        "__Secure-1PSID", "__Secure-3PSID", "__Secure-1PAPISID", "__Secure-3PAPISID",
        "SAPISID", "APISID", "LOGIN_INFO"
    };
    bool found = BinaryFileContainsAny(db, authNames);
    const std::wstring wal = db + L"-wal";
    if (!found && FileExists(wal)) found = BinaryFileContainsAny(wal, authNames);

    if (found) {
        result.text = L"Dedicated Edge account session found";
        result.color = RGB(74, 222, 128);
        result.valid = true;
    } else {
        result.text = L"Edge cookie database found, but login is not confirmed";
        result.color = C_WARN;
    }
    return result;
}

void CheckLoginAsync(bool startup = false) {
    SetText(IDC_ACCOUNT_STATUS, L"Checking dedicated Edge session…");
    std::thread([startup]() {
        auto* status = new AccountStatus(InspectAccountSession());
        if (!PostMessageW(g_main, WM_APP_ACCOUNT_STATUS, startup ? 1 : 0, reinterpret_cast<LPARAM>(status))) delete status;
    }).detach();
}

std::wstring TimestampForFilename() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    wchar_t b[32]{};
    swprintf_s(b, L"%04u%02u%02u_%02u%02u%02u", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return b;
}

void OpenAccountBrowser() {
    if (g_accountBrowserExe.empty() || !FileExists(g_accountBrowserExe)) {
        g_accountBrowserExe = DetectEdgePath();
    }
    if (g_accountBrowserExe.empty() || !FileExists(g_accountBrowserExe)) {
        MessageBoxW(g_main, L"Microsoft Edge was not found. This experiment uses Edge for the dedicated account profile.",
                    L"Account browser missing", MB_OK | MB_ICONWARNING);
        return;
    }

    EnsureAccountBrowserProfile();
    std::wstring target = NormalizeUserUrl(GetText(IDC_URL));
    if (!StartsWithHttp(target)) target = L"https://www.youtube.com/";

    std::wstring args = L"--user-data-dir=" + WinArgQuote(g_accountBrowserUserData) +
                        L" --profile-directory=Default --no-first-run --no-default-browser-check --disable-background-mode " +
                        WinArgQuote(target);
    const std::wstring cwd = fs::path(g_accountBrowserExe).parent_path().wstring();
    if (!LaunchDetached(g_accountBrowserExe, args, cwd)) {
        MessageBoxW(g_main, L"Could not open the dedicated Microsoft Edge account browser.",
                    L"Account browser", MB_OK | MB_ICONERROR);
        return;
    }
    SetText(IDC_ACCOUNT_STATUS, L"Dedicated Edge profile is open — log in, test the video, then close it");
    PostLog(L"Opened the dedicated Microsoft Edge account profile.");
}

void ResetLoginProfile() {
    if (!WaitForAccountBrowserProfileRelease()) {
        MessageBoxW(g_main, L"Close the WinterStatic Edge profile before resetting it.",
                    L"Account browser is still running", MB_OK | MB_ICONWARNING);
        return;
    }
    if (MessageBoxW(g_main,
        L"Reset the dedicated Microsoft Edge login profile?\n\nThe old profile will be renamed as a backup rather than deleted.",
        L"Reset login profile", MB_YESNO | MB_ICONQUESTION) != IDYES) return;

    std::error_code ec;
    if (DirExists(g_accountBrowserUserData)) {
        fs::path oldPath(g_accountBrowserUserData);
        fs::path backup = oldPath.parent_path() / (L"BrowserProfile_backup_" + TimestampForFilename());
        fs::rename(oldPath, backup, ec);
        if (ec) {
            MessageBoxW(g_main, L"Could not rename the existing dedicated Edge profile.",
                        L"Profile reset failed", MB_OK | MB_ICONERROR);
            return;
        }
        PostLog(L"Old account profile kept as: " + backup.filename().wstring());
    }
    EnsureAccountBrowserProfile();
    g_accountSessionValid = false;
    SetText(IDC_ACCOUNT_STATUS, L"Login profile reset");
}

void OpenAccountProfileFolder() {
    EnsureAccountBrowserProfile();
    ShellExecuteW(g_main, L"open", g_accountBrowserUserData.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

// -----------------------------------------------------------------------------
// Dialog and clipboard helpers
// -----------------------------------------------------------------------------

std::wstring BrowseExe(const wchar_t* title, const wchar_t* initial = nullptr) {
    wchar_t file[32768]{};
    if (initial) wcsncpy_s(file, initial, _TRUNCATE);
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_main;
    ofn.lpstrFile = file;
    ofn.nMaxFile = static_cast<DWORD>(std::size(file));
    ofn.lpstrTitle = title;
    ofn.lpstrFilter = L"Executables (*.exe)\0*.exe\0All files (*.*)\0*.*\0\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    return GetOpenFileNameW(&ofn) ? std::wstring(file) : L"";
}

std::wstring BrowseFolder(const std::wstring& initial) {
    IFileDialog* dialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) return L"";
    DWORD options = 0;
    dialog->GetOptions(&options);
    dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    dialog->SetTitle(L"Choose download folder");

    IShellItem* start = nullptr;
    if (!initial.empty() && SUCCEEDED(SHCreateItemFromParsingName(initial.c_str(), nullptr, IID_PPV_ARGS(&start)))) {
        dialog->SetFolder(start);
        start->Release();
    }

    std::wstring result;
    if (SUCCEEDED(dialog->Show(g_main))) {
        IShellItem* item = nullptr;
        if (SUCCEEDED(dialog->GetResult(&item))) {
            PWSTR path = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                result = path;
                CoTaskMemFree(path);
            }
            item->Release();
        }
    }
    dialog->Release();
    return result;
}

void PasteUrl() {
    if (!OpenClipboard(g_main)) return;
    HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if (data) {
        if (const wchar_t* text = static_cast<const wchar_t*>(GlobalLock(data))) {
            SetText(IDC_URL, NormalizeUserUrl(text));
            GlobalUnlock(data);
        }
    }
    CloseClipboard();
}

bool CopyToClipboard(const std::wstring& text) {
    if (!OpenClipboard(g_main)) return false;
    EmptyClipboard();
    const SIZE_T bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!mem) { CloseClipboard(); return false; }
    void* dst = GlobalLock(mem);
    memcpy(dst, text.c_str(), bytes);
    GlobalUnlock(mem);
    if (!SetClipboardData(CF_UNICODETEXT, mem)) {
        GlobalFree(mem);
        CloseClipboard();
        return false;
    }
    CloseClipboard();
    return true;
}

// -----------------------------------------------------------------------------
// yt-dlp command construction
// -----------------------------------------------------------------------------

std::vector<std::wstring> BuildFormatArgs(bool hasFfmpeg) {
    const std::wstring quality = GetText(IDC_QUALITY);
    const std::wstring preference = GetText(IDC_FORMAT);
    const bool preferMp4 = preference == L"Prefer MP4";
    const bool preferWebm = preference == L"Prefer WebM";

    auto videoSelector = [&](const wchar_t* height) -> std::wstring {
        const std::wstring cap = height ? std::wstring(L"[height<=?") + height + L"]" : L"";
        if (preferMp4) {
            return L"bv*" + cap + L"[ext=mp4]+ba[ext=m4a]/b" + cap + L"[ext=mp4]/bv*" + cap + L"+ba/b" + cap;
        }
        if (preferWebm) {
            return L"bv*" + cap + L"[ext=webm]+ba[ext=webm]/b" + cap + L"[ext=webm]/bv*" + cap + L"+ba/b" + cap;
        }
        return L"bv*" + cap + L"+ba/b" + cap;
    };

    auto singleFileSelector = [&](const wchar_t* height) -> std::wstring {
        const std::wstring cap = height ? std::wstring(L"[height<=?") + height + L"]" : L"";
        if (preferMp4) return L"best" + cap + L"[ext=mp4]/best" + cap + L"/best";
        if (preferWebm) return L"best" + cap + L"[ext=webm]/best" + cap + L"/best";
        return height ? L"best" + cap + L"/best" : L"best";
    };

    if (quality == L"Audio only (source format)") {
        std::wstring selector = L"bestaudio/best";
        if (preferMp4) selector = L"bestaudio[ext=m4a]/bestaudio/best";
        else if (preferWebm) selector = L"bestaudio[ext=webm]/bestaudio/best";
        return hasFfmpeg
            ? std::vector<std::wstring>{L"-f", selector, L"-x"}
            : std::vector<std::wstring>{L"-f", selector};
    }

    const wchar_t* height = nullptr;
    if (quality == L"Maximum 1080p") height = L"1080";
    else if (quality == L"Maximum 720p") height = L"720";

    if (hasFfmpeg) return {L"-f", videoSelector(height)};
    return {L"-f", singleFileSelector(height)};
}

std::vector<std::wstring> BuildBaseArgs(const std::wstring& output, const std::wstring& ffmpeg) {
    std::vector<std::wstring> args = {
        L"--newline", L"--no-color", L"--progress", L"--windows-filenames", L"--no-playlist",
        // Conservative resilience defaults. yt-dlp already retries internally;
        // these modestly extend the fragile cases without turning failures into
        // unbounded retry loops. More invasive recovery options are applied only
        // after a normal attempt actually fails.
        L"--retries", L"12",
        L"--fragment-retries", L"15",
        L"--file-access-retries", L"5",
        L"--extractor-retries", L"5",
        L"--retry-sleep", L"http:1",
        L"--retry-sleep", L"fragment:1",
        L"--retry-sleep", L"extractor:1",
        L"--paths", output,
        L"--output", L"%(title).180B [%(id)s].%(ext)s",
        L"--progress-template", L"download:WINTERSTATIC_PROGRESS:%(progress.filename)s|%(progress.tmpfilename)s|%(progress.downloaded_bytes)s|%(progress.total_bytes)s|%(progress.total_bytes_estimate)s|%(progress.speed)s|%(progress.status)s|%(progress._percent_str)s|%(progress._speed_str)s|%(info.vcodec)s|%(info.acodec)s|%(info.format_id)s",
        L"--progress-template", L"postprocess:WINTERSTATIC_POSTPROCESS:%(progress.postprocessor)s|%(progress.status)s",
        L"--print", L"after_move:WINTERSTATIC_FINAL:%(filepath)s"
    };
    const bool hasFfmpeg = !ffmpeg.empty() && FileExists(ffmpeg);
    if (hasFfmpeg) {
        args.push_back(L"--ffmpeg-location");
        args.push_back(ffmpeg);
    }
    auto fmt = BuildFormatArgs(hasFfmpeg);
    args.insert(args.end(), fmt.begin(), fmt.end());
    return args;
}

std::vector<std::wstring> AccountArgs() {
    // Keep authentication deliberately simple: let yt-dlp choose its normal
    // YouTube client behavior and only supply cookies from WinterStatic's
    // isolated Edge profile. This mirrors the proven standalone cookie flow.
    return {L"--cookies-from-browser", L"edge:" + g_accountBrowserProfile};
}

bool ValidateInputs(std::wstring& ytdlp, std::wstring& output, std::wstring& ffmpeg) {
    const std::wstring originalUrl = Trim(GetText(IDC_URL));
    const std::wstring url = NormalizeUserUrl(originalUrl);
    if (!StartsWithHttp(url)) {
        MessageBoxW(g_main, L"Paste a valid web video URL first.", L"Video URL", MB_OK | MB_ICONWARNING);
        return false;
    }
    if (url != originalUrl) SetText(IDC_URL, url);
    const std::wstring portableYtDlp = PortableYtDlpPath();
    if (FileExists(portableYtDlp)) {
        ytdlp = portableYtDlp;
        SetText(IDC_YTDLP, ytdlp);
    } else {
        ytdlp = Trim(GetText(IDC_YTDLP));
        if (!FileExists(ytdlp)) {
            ytdlp = DetectYtDlpPath();
            if (!ytdlp.empty()) SetText(IDC_YTDLP, ytdlp);
        }
    }
    if (!FileExists(ytdlp)) {
        MessageBoxW(g_main, L"yt-dlp.exe was not found. Check the Tools path or place it on PATH.", L"yt-dlp missing", MB_OK | MB_ICONERROR);
        return false;
    }
    output = Trim(GetText(IDC_OUTPUT));
    if (output.empty()) output = DefaultDownloads();
    if (!EnsureDir(output)) {
        MessageBoxW(g_main, L"Could not create the output folder.", L"Output folder", MB_OK | MB_ICONERROR);
        return false;
    }
    const std::wstring portableFfmpeg = PortableFfmpegPath();
    if (FileExists(portableFfmpeg)) {
        ffmpeg = portableFfmpeg;
    } else {
        ffmpeg = Trim(GetText(IDC_FFMPEG));
        if (ffmpeg.empty() || !FileExists(ffmpeg)) ffmpeg = DetectFfmpegPath();
    }
    if (!ffmpeg.empty()) SetText(IDC_FFMPEG, ffmpeg);

    const std::wstring mode = GetText(IDC_AUTH);
    if (mode == L"Use account browser" && (g_accountBrowserExe.empty() || !FileExists(g_accountBrowserExe))) {
        g_accountBrowserExe = DetectEdgePath();
        if (g_accountBrowserExe.empty() || !FileExists(g_accountBrowserExe)) {
            MessageBoxW(g_main, L"Microsoft Edge was not found, so the dedicated account profile cannot be used.",
                        L"Account browser missing", MB_OK | MB_ICONWARNING);
            return false;
        }
    }
    if (mode == L"Use account browser") {
        if (!WaitForAccountBrowserProfileRelease()) {
            MessageBoxW(g_main,
                L"Close the dedicated WinterStatic Edge profile before an authenticated download. Your normal Edge profile can remain open.",
                L"Account browser is still running", MB_OK | MB_ICONWARNING);
            return false;
        }
    }
    return true;
}

std::wstring DirectPowerShellCommand(bool validate) {
    std::wstring ytdlp, output, ffmpeg;
    if (validate && !ValidateInputs(ytdlp, output, ffmpeg)) return L"";
    if (!validate) {
        ytdlp = Trim(GetText(IDC_YTDLP));
        output = Trim(GetText(IDC_OUTPUT));
        ffmpeg = Trim(GetText(IDC_FFMPEG));
    }
    auto args = BuildBaseArgs(output, ffmpeg);
    const std::wstring mode = GetText(IDC_AUTH);
    // Automatic begins anonymously and only falls back to
    // the saved Edge session from the managed PowerShell task when yt-dlp emits a
    // strong authentication-required signal. A copied one-line command represents
    // that first anonymous attempt; explicit account mode still includes cookies.
    if (mode == L"Use account browser") {
        auto auth = AccountArgs();
        args.insert(args.end(), auth.begin(), auth.end());
    }
    args.push_back(Trim(GetText(IDC_URL)));

    std::wstring cmd = L"& " + PsQuote(ytdlp);
    for (const auto& a : args) cmd += L" " + PsQuote(a);
    return cmd;
}

std::wstring UniqueConsoleTitle(int jobId) {
    const ULONGLONG now = GetTickCount64() % 1000000ULL;
    wchar_t b[160]{};
    swprintf_s(b, L"WinterStatic Downloader - Download %d - LIVE PowerShell #%06llu", jobId, now);
    return b;
}

fs::path TempBaseDirectory() {
    wchar_t temp[32768]{};
    DWORD n = GetTempPathW(static_cast<DWORD>(std::size(temp)), temp);
    if (n && n < std::size(temp)) return fs::path(temp);

    std::error_code ec;
    fs::path fallback = fs::temp_directory_path(ec);
    return ec ? fs::path(L".") : fallback;
}

void RemoveTaskDirectoryBestEffort(const fs::path& taskDir) {
    if (taskDir.empty()) return;
    const std::wstring name = taskDir.filename().wstring();
    if (name.rfind(L"WinterStaticDL_", 0) != 0) return;

    std::error_code ec;
    const fs::path base = TempBaseDirectory().lexically_normal();
    const fs::path parent = taskDir.parent_path().lexically_normal();
    if (parent != base) return;
    fs::remove_all(taskDir, ec);
}

int CleanupFinishedTaskDirectories() {
    const fs::path base = TempBaseDirectory();
    std::error_code ec;
    if (!fs::is_directory(base, ec)) return 0;

    const auto now = fs::file_time_type::clock::now();
    const auto minAge = std::chrono::hours(24);
    int removed = 0;

    fs::directory_iterator it(base, fs::directory_options::skip_permission_denied, ec);
    fs::directory_iterator end;
    for (; !ec && it != end; it.increment(ec)) {
        std::error_code itemEc;
        if (!it->is_directory(itemEc)) continue;

        const fs::path dir = it->path();
        const std::wstring name = dir.filename().wstring();
        if (name.rfind(L"WinterStaticDL_", 0) != 0) continue;

        // Only reap tasks that reached the normal done-marker path. Active or
        // abruptly interrupted jobs are deliberately left alone.
        const fs::path done = dir / L"done.txt";
        if (!fs::is_regular_file(done, itemEc)) continue;

        const auto modified = fs::last_write_time(done, itemEc);
        if (itemEc || now - modified < minAge) continue;

        const auto count = fs::remove_all(dir, itemEc);
        if (!itemEc && count > 0) ++removed;
    }
    return removed;
}

std::wstring TempTaskDirectory() {
    fs::path base = TempBaseDirectory();
    std::wstringstream name;
    name << L"WinterStaticDL_" << GetCurrentProcessId() << L"_" << GetTickCount64();
    fs::path out = base / name.str();
    std::error_code ec;
    fs::create_directories(out, ec);
    return out.wstring();
}

bool WriteUtf8BomFile(const std::wstring& path, const std::wstring& text) {
    std::ofstream f(fs::path(path), std::ios::binary | std::ios::trunc);
    if (!f) return false;
    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    f.write(reinterpret_cast<const char*>(bom), 3);
    const std::string utf8 = WideToUtf8(text);
    f.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
    return !!f;
}

std::wstring PowerShellArray(const std::vector<std::wstring>& values) {
    std::wstring out;
    for (size_t i = 0; i < values.size(); ++i) {
        out += L"    " + PsQuote(values[i]);
        if (i + 1 < values.size()) out += L",";
        out += L"\r\n";
    }
    return out;
}

std::wstring BuildDownloadScript(const std::wstring& ytdlp,
                                 const std::wstring& output,
                                 const std::wstring& ffmpeg,
                                 const std::wstring& logPath,
                                 const std::wstring& donePath,
                                 const std::wstring& title) {
    auto baseArgs = BuildBaseArgs(output, ffmpeg);
    auto accountArgs = AccountArgs();
    const bool hasFfmpeg = !ffmpeg.empty() && FileExists(ffmpeg);
    auto formatArgs = BuildFormatArgs(hasFfmpeg);
    const std::wstring url = Trim(GetText(IDC_URL));
    const std::wstring mode = GetText(IDC_AUTH);
    const bool autoUse = g_accountSessionValid.load();
    const bool closeOnSuccess = IsDlgButtonChecked(g_main, IDC_CLOSE_POWERSHELL) == BST_CHECKED;
    const bool browserAssistedRecovery = IsDlgButtonChecked(g_main, IDC_BROWSER_SWEEP) == BST_CHECKED;
    const bool audioOnlyMode = GetText(IDC_QUALITY) == L"Audio only (source format)";
    const std::wstring authYtdlp = PortableAuthYtDlpPath();

    std::wstringstream s;
    s << L"$YtDlp = " << PsQuote(ytdlp) << L"\r\n";
    s << L"$AuthYtDlp = " << PsQuote(authYtdlp) << L"\r\n";
    s << L"$script:CurrentYtDlp = $YtDlp\r\n";
    s << L"$Ffmpeg = " << PsQuote(ffmpeg) << L"\r\n";
    s << L"$Url = " << PsQuote(url) << L"\r\n";
    s << L"$LogPath = " << PsQuote(logPath) << L"\r\n";
    s << L"$DonePath = " << PsQuote(donePath) << L"\r\n";
    s << L"$TaskDir = [System.IO.Path]::GetDirectoryName($DonePath)\r\n";
    s << L"if ([string]::IsNullOrWhiteSpace($TaskDir)) { $TaskDir = [System.IO.Path]::GetTempPath() }\r\n";
    s << L"$AccountBrowserPath = " << PsQuote(g_accountBrowserExe) << L"\r\n";
    s << L"$Mode = " << PsQuote(mode) << L"\r\n";
    s << L"$AutoUseAccount = $" << (autoUse ? L"true" : L"false") << L"\r\n";
    s << L"$CloseOnSuccess = $" << (closeOnSuccess ? L"true" : L"false") << L"\r\n";
    s << L"$BrowserAssistedRecovery = $" << (browserAssistedRecovery ? L"true" : L"false") << L"\r\n";
    s << L"$host.UI.RawUI.WindowTitle = " << PsQuote(title) << L"\r\n";
    s << L"$ProgressPreference = 'Continue'\r\n$ErrorActionPreference = 'Continue'\r\n";
    s << L"[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new()\r\n";
    s << L"$BaseArgs = @(\r\n" << PowerShellArray(baseArgs) << L")\r\n";
    s << L"$AccountArgs = @(\r\n" << PowerShellArray(accountArgs) << L")\r\n";
    s << L"$FormatArgs = @(\r\n" << PowerShellArray(formatArgs) << L")\r\n";
    s << L"$FormatRecoveryArgs = @('-f', " << PsQuote(audioOnlyMode ? L"bestaudio/best" : L"bv*+ba/b") << L")\r\n";
    s << L"$MwebClientArgs = @('--extractor-args', 'youtube:player_client=mweb')\r\n";
    s << L"$AudioOnlyMode = $" << (audioOnlyMode ? L"true" : L"false") << L"\r\n";
    s << L"$script:IsYouTubeUrl = $false\r\n";
    s << L"$script:AuthMwebFallbackUsed = $false\r\n";
    s << L"try { $u = [System.Uri]$Url; $h = $u.Host.ToLowerInvariant(); $script:IsYouTubeUrl = ($h -eq 'youtu.be' -or $h -eq 'youtube.com' -or $h.EndsWith('.youtube.com')) } catch {}\r\n";
    s << L"$Utf8NoBom = [System.Text.UTF8Encoding]::new($false)\r\n";
    s << L"$LogStream = $null\r\n$LogWriter = $null\r\n";
    s << L"try {\r\n";
    s << L"    $LogStream = [System.IO.FileStream]::new($LogPath, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write, [System.IO.FileShare]::ReadWrite)\r\n";
    s << L"    $LogWriter = [System.IO.StreamWriter]::new($LogStream, $Utf8NoBom)\r\n";
    s << L"    $LogWriter.AutoFlush = $true\r\n";
    s << L"} catch { $LogStream = $null; $LogWriter = $null }\r\n";
    s << L"function Write-WinterStaticLog([string]$Text) {\r\n";
    s << L"    if ($null -eq $script:LogWriter) { return }\r\n";
    s << L"    try { $script:LogWriter.WriteLine($Text) } catch {}\r\n";
    s << L"}\r\n";
    s << L"$script:RecoverySawImpersonationSignal = $false\r\n";
    s << L"$script:RecoverySawTransportSignal = $false\r\n";
    s << L"$script:RecoverySawUnsupportedUrl = $false\r\n";
    s << L"$script:RecoverySawHttp404 = $false\r\n";
    s << L"$script:RecoverySawDownloadProgress = $false\r\n";
    s << L"$script:RecoverySawFormatUnavailable = $false\r\n";
    s << L"$script:ImpersonationChecked = $false\r\n";
    s << L"$script:ImpersonationArgs = @()\r\n";
    s << L"function Get-WinterStaticImpersonationArgs {\r\n";
    s << L"    if ($script:ImpersonationChecked) { return @($script:ImpersonationArgs) }\r\n";
    s << L"    $script:ImpersonationChecked = $true\r\n";
    s << L"    try {\r\n";
    s << L"        $rows = @(& $script:CurrentYtDlp --ignore-config --list-impersonate-targets 2>$null)\r\n";
    s << L"        foreach ($row in $rows) {\r\n";
    s << L"            $text = $row.ToString().Trim()\r\n";
    s << L"            if ($text -match '^Chrome' -and $text -match 'curl_cffi' -and $text -notmatch 'not available') {\r\n";
    s << L"                $candidate = (($text -split '\\s+')[0]).ToLowerInvariant()\r\n";
    s << L"                if ($candidate) { $script:ImpersonationArgs = @('--impersonate', $candidate); break }\r\n";
    s << L"            }\r\n";
    s << L"        }\r\n";
    s << L"    } catch {}\r\n";
    s << L"    return @($script:ImpersonationArgs)\r\n";
    s << L"}\r\n";
    s << L"$script:DiscoveryUserAgent = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 Chrome/131.0 Safari/537.36'\r\n";
    s << L"$script:DiscoveryDirectFetchFailed = $false\r\n";
    s << L"function Resolve-WinterStaticMediaUrl([string]$Value, [string]$BaseUrl) {\r\n";
    s << L"    if ([string]::IsNullOrWhiteSpace($Value)) { return '' }\r\n";
    s << L"    try {\r\n";
    s << L"        $v = [System.Net.WebUtility]::HtmlDecode($Value.Trim())\r\n";
    s << L"        $v = $v.Replace('\\/', '/').Replace('\\u0026', '&').Replace('\\u003d', '=').Replace('\\u002F', '/')\r\n";
    s << L"        if ($v -match '(?i)^(data|blob|javascript):') { return '' }\r\n";
    s << L"        return ([System.Uri]::new([System.Uri]$BaseUrl, $v)).AbsoluteUri\r\n";
    s << L"    } catch { return '' }\r\n";
    s << L"}\r\n";
    s << L"function New-WinterStaticMediaCandidate([string]$Kind, [string]$Value, [string]$BaseUrl) {\r\n";
    s << L"    $resolved = Resolve-WinterStaticMediaUrl $Value $BaseUrl\r\n";
    s << L"    if (-not $resolved) { return $null }\r\n";
    s << L"    return [PSCustomObject]@{ Kind = $Kind; Url = $resolved; Referer = $BaseUrl }\r\n";
    s << L"}\r\n";
    s << L"function Test-WinterStaticMediaCandidate($Candidate, [string]$RefererUrl) {\r\n";
    s << L"    if ($null -eq $Candidate -or [string]::IsNullOrWhiteSpace([string]$Candidate.Url)) { return $false }\r\n";
    s << L"    $candidateUrl = [string]$Candidate.Url\r\n";
    s << L"    $knownImageExt = $candidateUrl -match '(?i)\\.(?:jpe?g|png|gif|webp|avif|bmp|svg|ico)(?:[?#]|$)'\r\n";
    s << L"    if ($knownImageExt) { Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_PROBE_REJECT_IMAGE_EXT:' + $Candidate.Kind); return $false }\r\n";
    s << L"    $knownExt = $candidateUrl -match '(?i)\\.(?:m3u8|mpd|mp4|webm|m4v|mov|m4a|mp3|aac|ogg|opus)(?:[?#]|$)'\r\n";
    s << L"    try {\r\n";
    s << L"        $request = [System.Net.HttpWebRequest]::Create($candidateUrl)\r\n";
    s << L"        $request.Method = 'GET'\r\n";
    s << L"        $request.UserAgent = $script:DiscoveryUserAgent\r\n";
    s << L"        $request.Accept = '*/*'\r\n";
    s << L"        if (-not [string]::IsNullOrWhiteSpace($RefererUrl)) { $request.Referer = $RefererUrl }\r\n";
    s << L"        $request.AllowAutoRedirect = $true\r\n";
    s << L"        $request.Timeout = 12000\r\n";
    s << L"        $request.ReadWriteTimeout = 12000\r\n";
    s << L"        $request.AddRange(0, 65535)\r\n";
    s << L"        $response = [System.Net.HttpWebResponse]$request.GetResponse()\r\n";
    s << L"        $contentType = [string]$response.ContentType\r\n";
    s << L"        $totalLength = [long]0\r\n";
    s << L"        $contentRange = [string]$response.Headers['Content-Range']\r\n";
    s << L"        if ($contentRange -match '/(?<n>\\d+)$') {\r\n";
    s << L"            try { $totalLength = [long]$Matches['n'] } catch { $totalLength = [long]0 }\r\n";
    s << L"        } elseif ([int]$response.StatusCode -eq 200 -and $response.ContentLength -gt 0) {\r\n";
    s << L"            $totalLength = [long]$response.ContentLength\r\n";
    s << L"        }\r\n";
    s << L"        $stream = $response.GetResponseStream()\r\n";
    s << L"        $buffer = New-Object byte[] 4096\r\n";
    s << L"        $read = $stream.Read($buffer, 0, $buffer.Length)\r\n";
    s << L"        $prefix = if ($read -gt 0) { [System.Text.Encoding]::UTF8.GetString($buffer, 0, $read) } else { '' }\r\n";
    s << L"        $stream.Close()\r\n";
    s << L"        $response.Close()\r\n";
    s << L"        $isMediaType = $contentType -match '(?i)^(?:video/|audio/|application/(?:vnd\\.apple\\.mpegurl|x-mpegurl|mpegurl|dash\\+xml))'\r\n";
    s << L"        $isManifestText = $prefix -match '(?is)^\\s*#EXTM3U|<MPD(?:\\s|>)'\r\n";
    s << L"        try {\r\n";
    s << L"            $Candidate | Add-Member -NotePropertyName ProbeContentType -NotePropertyValue $contentType -Force\r\n";
    s << L"            $Candidate | Add-Member -NotePropertyName ProbeTotalLength -NotePropertyValue $totalLength -Force\r\n";
    s << L"            $Candidate | Add-Member -NotePropertyName ProbeIsManifest -NotePropertyValue ([bool]$isManifestText) -Force\r\n";
    s << L"        } catch {}\r\n";
    s << L"        $isImageType = $contentType -match '(?i)^image/'\r\n";
    s << L"        $isImageMagic = ($read -ge 3 -and $buffer[0] -eq 0xFF -and $buffer[1] -eq 0xD8 -and $buffer[2] -eq 0xFF) -or ($read -ge 8 -and $buffer[0] -eq 0x89 -and $buffer[1] -eq 0x50 -and $buffer[2] -eq 0x4E -and $buffer[3] -eq 0x47 -and $buffer[4] -eq 0x0D -and $buffer[5] -eq 0x0A -and $buffer[6] -eq 0x1A -and $buffer[7] -eq 0x0A) -or ($read -ge 6 -and (($prefix.StartsWith('GIF87a')) -or ($prefix.StartsWith('GIF89a')))) -or ($read -ge 12 -and $prefix.StartsWith('RIFF') -and $prefix.Substring(8,4) -eq 'WEBP') -or ($read -ge 2 -and $buffer[0] -eq 0x42 -and $buffer[1] -eq 0x4D)\r\n";
    s << L"        if ($isImageType -or $isImageMagic) {\r\n";
    s << L"            Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_PROBE_REJECT_IMAGE:' + $Candidate.Kind + ':' + $contentType)\r\n";
    s << L"            return $false\r\n";
    s << L"        }\r\n";
    s << L"        $looksHtml = ($contentType -match '(?i)^(?:text/html|application/xhtml\\+xml)') -or ($prefix -match '(?is)^\\s*(?:<!doctype\\s+html|<html\\b)')\r\n";
    s << L"        if ($looksHtml -and -not $isManifestText) {\r\n";
    s << L"            Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_PROBE_REJECT_HTML:' + $Candidate.Kind)\r\n";
    s << L"            return $false\r\n";
    s << L"        }\r\n";
    s << L"        if ($isMediaType -or $isManifestText -or $knownExt) {\r\n";
    s << L"            Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_PROBE_OK:' + $Candidate.Kind + ':' + $contentType)\r\n";
    s << L"            return $true\r\n";
    s << L"        }\r\n";
    s << L"        Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_PROBE_INCONCLUSIVE:' + $Candidate.Kind + ':' + $contentType)\r\n";
    s << L"        return $true\r\n";
    s << L"    } catch [System.Net.WebException] {\r\n";
    s << L"        $status = 0\r\n";
    s << L"        try {\r\n";
    s << L"            if ($_.Exception.Response -is [System.Net.HttpWebResponse]) {\r\n";
    s << L"                $status = [int]$_.Exception.Response.StatusCode\r\n";
    s << L"                $_.Exception.Response.Close()\r\n";
    s << L"            }\r\n";
    s << L"        } catch {}\r\n";
    s << L"        if ($status -eq 404 -or $status -eq 410) {\r\n";
    s << L"            Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_PROBE_REJECT_HTTP:' + $Candidate.Kind + ':' + $status)\r\n";
    s << L"            return $false\r\n";
    s << L"        }\r\n";
    s << L"        Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_PROBE_HTTP_INCONCLUSIVE:' + $Candidate.Kind + ':' + $status)\r\n";
    s << L"        return $true\r\n";
    s << L"    } catch {\r\n";
    s << L"        Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_PROBE_ERROR_INCONCLUSIVE:' + $Candidate.Kind)\r\n";
    s << L"        return $true\r\n";
    s << L"    }\r\n";
    s << L"}\r\n";
    s << L"\r\n";
    s << L"function Add-WinterStaticRankedCandidate($List, $Candidate, [int]$BaseScore, [bool]$NeedsProbe = $true) {\r\n";
    s << L"    if ($null -eq $List -or $null -eq $Candidate -or [string]::IsNullOrWhiteSpace([string]$Candidate.Url)) { return }\r\n";
    s << L"    $url = [string]$Candidate.Url\r\n";
    s << L"    if ($url -match '(?i)\\.(?:jpe?g|png|gif|webp|avif|bmp|svg|ico)(?:[?#]|$)') { return }\r\n";
    s << L"    $score = $BaseScore\r\n";
    s << L"    if ($url -match '(?i)\\.(?:m3u8|mpd)(?:[?#]|$)') { $score += 35 }\r\n";
    s << L"    elseif ($url -match '(?i)\\.(?:mp4|webm|m4v|mov)(?:[?#]|$)') { $score += 5 }\r\n";
    s << L"    $previewHint = $false\r\n";
    s << L"    try { if ($Candidate.PSObject.Properties['PreviewHint']) { $previewHint = [bool]$Candidate.PreviewHint } } catch {}\r\n";
    s << L"    if ($url -match '(?i)(?:^|[/_.?=&%+\\-])(?:preview|previews|teaser|teasers|hover|thumbnail|thumbnails|thumb|sprite|storyboard|poster|loop|sample)(?:[/_.?=&%+\\-]|$)') { $previewHint = $true }\r\n";
    s << L"    if ($previewHint) { $score -= 180 }\r\n";
    s << L"    elseif ($url -match '(?i)(?:^|[/_.?=&%+\\-])(?:promo|autoplay)(?:[/_.?=&%+\\-]|$)') { $score -= 35 }\r\n";
    s << L"    foreach ($existing in @($List)) {\r\n";
    s << L"        if ([string]$existing.Url -ieq $url) {\r\n";
    s << L"            $existingPreview = $false\r\n";
    s << L"            try { if ($existing.PSObject.Properties['PreviewHint']) { $existingPreview = [bool]$existing.PreviewHint } } catch {}\r\n";
    s << L"            if ($existingPreview -and -not $previewHint) { $score -= 180; $previewHint = $true }\r\n";
    s << L"            if ($previewHint -and -not $existingPreview) { $existing.Score = [int]$existing.Score - 180; $existing | Add-Member -NotePropertyName PreviewHint -NotePropertyValue $true -Force }\r\n";
    s << L"            if ($score -gt [int]$existing.Score) {\r\n";
    s << L"                $existing.Kind = $Candidate.Kind\r\n";
    s << L"                $existing.Referer = $Candidate.Referer\r\n";
    s << L"                $existing.Score = $score\r\n";
    s << L"                $existing.NeedsProbe = $NeedsProbe\r\n";
    s << L"            }\r\n";
    s << L"            if ($previewHint) { $existing | Add-Member -NotePropertyName PreviewHint -NotePropertyValue $true -Force }\r\n";
    s << L"            return\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"    if ($List.Count -ge 64) { return }\r\n";
    s << L"    $script:DiscoveryCandidateOrder++\r\n";
    s << L"    $Candidate | Add-Member -NotePropertyName Score -NotePropertyValue $score -Force\r\n";
    s << L"    $Candidate | Add-Member -NotePropertyName Order -NotePropertyValue $script:DiscoveryCandidateOrder -Force\r\n";
    s << L"    $Candidate | Add-Member -NotePropertyName NeedsProbe -NotePropertyValue $NeedsProbe -Force\r\n";
    s << L"    $Candidate | Add-Member -NotePropertyName PreviewHint -NotePropertyValue ([bool]$previewHint) -Force\r\n";
    s << L"    [void]$List.Add($Candidate)\r\n";
    s << L"}\r\n";
    s << L"\r\n";
    s << L"function Select-WinterStaticRankedCandidate($List) {\r\n";
    s << L"    if ($null -eq $List -or $List.Count -eq 0) { return $null }\r\n";
    s << L"    $ordered = @($List | Sort-Object @{Expression='Score';Descending=$true}, @{Expression='Order';Descending=$false})\r\n";
    s << L"    $valid = New-Object System.Collections.ArrayList\r\n";
    s << L"    $probeCount = 0\r\n";
    s << L"    foreach ($candidate in $ordered) {\r\n";
    s << L"        if ([bool]$candidate.NeedsProbe) {\r\n";
    s << L"            if ($probeCount -ge 10) { continue }\r\n";
    s << L"            $probeCount++\r\n";
    s << L"            $referer = if ($candidate.PSObject.Properties['Referer'] -and $candidate.Referer) { [string]$candidate.Referer } else { '' }\r\n";
    s << L"            if (-not (Test-WinterStaticMediaCandidate $candidate $referer)) { continue }\r\n";
    s << L"            $score = [int]$candidate.Score\r\n";
    s << L"            $isManifest = ([string]$candidate.Url -match '(?i)\\.(?:m3u8|mpd)(?:[?#]|$)') -or ([bool]$candidate.ProbeIsManifest) -or ([string]$candidate.ProbeContentType -match '(?i)application/(?:vnd\\.apple\\.mpegurl|x-mpegurl|mpegurl|dash\\+xml)')\r\n";
    s << L"            $isDirectVideo = [string]$candidate.Url -match '(?i)\\.(?:mp4|webm|m4v|mov)(?:[?#]|$)'\r\n";
    s << L"            $length = [long]0\r\n";
    s << L"            if ($candidate.PSObject.Properties['ProbeTotalLength']) { try { $length = [long]$candidate.ProbeTotalLength } catch {} }\r\n";
    s << L"            if ($isManifest) { $score += 20 }\r\n";
    s << L"            elseif ($isDirectVideo -and $length -gt 0) {\r\n";
    s << L"                if ($length -lt 1572864) { $score -= 90 }\r\n";
    s << L"                elseif ($length -lt 6291456) { $score -= 45 }\r\n";
    s << L"                elseif ($length -lt 15728640) { $score -= 20 }\r\n";
    s << L"                elseif ($length -ge 157286400) { $score += 35 }\r\n";
    s << L"                elseif ($length -ge 52428800) { $score += 25 }\r\n";
    s << L"                elseif ($length -ge 20971520) { $score += 15 }\r\n";
    s << L"            }\r\n";
    s << L"            $candidate.Score = $score\r\n";
    s << L"        }\r\n";
    s << L"        [void]$valid.Add($candidate)\r\n";
    s << L"    }\r\n";
    s << L"    if ($valid.Count -eq 0) { return $null }\r\n";
    s << L"    $ranked = @($valid | Sort-Object @{Expression='Score';Descending=$true}, @{Expression='Order';Descending=$false})\r\n";
    s << L"    Write-Host 'Discovery candidate summary (URLs hidden):' -ForegroundColor DarkGray\r\n";
    s << L"    foreach ($item in @($ranked | Select-Object -First 5)) {\r\n";
    s << L"        $mime = if ($item.PSObject.Properties['ProbeContentType'] -and $item.ProbeContentType) { [string]$item.ProbeContentType } else { 'not probed' }\r\n";
    s << L"        $bytes = [long]0; if ($item.PSObject.Properties['ProbeTotalLength']) { try { $bytes = [long]$item.ProbeTotalLength } catch {} }\r\n";
    s << L"        $size = if ($bytes -gt 0) { ('{0:N1} MB' -f ($bytes / 1MB)) } else { 'size unknown' }\r\n";
    s << L"        $preview = $false; try { if ($item.PSObject.Properties['PreviewHint']) { $preview = [bool]$item.PreviewHint } } catch {}\r\n";
    s << L"        Write-Host ('  {0} | score {1} | {2} | {3} | preview-like={4}' -f $item.Kind, $item.Score, $mime, $size, $preview) -ForegroundColor DarkGray\r\n";
    s << L"        Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_CANDIDATE:' + $item.Kind + ':score=' + $item.Score + ':mime=' + $mime + ':bytes=' + $bytes + ':preview=' + $preview)\r\n";
    s << L"    }\r\n";
    s << L"    $selectable = @($ranked | Where-Object { -not ($_.PSObject.Properties['PreviewHint'] -and [bool]$_.PreviewHint) })\r\n";
    s << L"    if ($selectable.Count -eq 0) {\r\n";
    s << L"        Write-Host 'Only preview-like video candidates were found; refusing to download a likely hover/teaser clip.' -ForegroundColor Yellow\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_DISCOVERY_PREVIEW_ONLY'\r\n";
    s << L"        return $null\r\n";
    s << L"    }\r\n";
    s << L"    $best = $selectable[0]\r\n";
    s << L"    Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_SELECTED:' + $best.Kind + ':score=' + $best.Score)\r\n";
    s << L"    return $best\r\n";
    s << L"}\r\n";
    s << L"function Join-WinterStaticPathSafe([string]$Base, [string]$Child) {\r\n";
    s << L"    if ([string]::IsNullOrWhiteSpace($Base)) { return '' }\r\n";
    s << L"    try { return (Join-Path $Base $Child) } catch { return '' }\r\n";
    s << L"}\r\n";
    s << L"function Get-WinterStaticRuntimeBrowser {\r\n";
    s << L"    # Edge-first runtime discovery with scalar CDP target handling and safer fallback selection.\r\n";
    s << L"    $rows = @(\r\n";
    s << L"        [PSCustomObject]@{ Name = 'Edge'; Path = $AccountBrowserPath },\r\n";
    s << L"        [PSCustomObject]@{ Name = 'Edge'; Path = (Join-WinterStaticPathSafe ${env:ProgramFiles(x86)} 'Microsoft\\Edge\\Application\\msedge.exe') },\r\n";
    s << L"        [PSCustomObject]@{ Name = 'Edge'; Path = (Join-WinterStaticPathSafe $env:ProgramFiles 'Microsoft\\Edge\\Application\\msedge.exe') },\r\n";
    s << L"        [PSCustomObject]@{ Name = 'Chrome'; Path = (Join-WinterStaticPathSafe $env:LOCALAPPDATA 'Google\\Chrome\\Application\\chrome.exe') },\r\n";
    s << L"        [PSCustomObject]@{ Name = 'Chrome'; Path = (Join-WinterStaticPathSafe $env:ProgramFiles 'Google\\Chrome\\Application\\chrome.exe') },\r\n";
    s << L"        [PSCustomObject]@{ Name = 'Chrome'; Path = (Join-WinterStaticPathSafe ${env:ProgramFiles(x86)} 'Google\\Chrome\\Application\\chrome.exe') },\r\n";
    s << L"        [PSCustomObject]@{ Name = 'Brave'; Path = (Join-WinterStaticPathSafe $env:LOCALAPPDATA 'BraveSoftware\\Brave-Browser\\Application\\brave.exe') },\r\n";
    s << L"        [PSCustomObject]@{ Name = 'Brave'; Path = (Join-WinterStaticPathSafe $env:ProgramFiles 'BraveSoftware\\Brave-Browser\\Application\\brave.exe') },\r\n";
    s << L"        [PSCustomObject]@{ Name = 'Brave'; Path = (Join-WinterStaticPathSafe ${env:ProgramFiles(x86)} 'BraveSoftware\\Brave-Browser\\Application\\brave.exe') }\r\n";
    s << L"    )\r\n";
    s << L"    foreach ($row in $rows) {\r\n";
    s << L"        try { if ($row.Path -and (Test-Path -LiteralPath $row.Path)) { return $row } } catch {}\r\n";
    s << L"    }\r\n";
    s << L"    return $null\r\n";
    s << L"}\r\n";
    s << L"function Send-WinterStaticCdpMessage($Socket, [int]$Id, [string]$Method, $Params = $null) {\r\n";
    s << L"    try {\r\n";
    s << L"        $payload = if ($null -eq $Params) {\r\n";
    s << L"            @{ id = $Id; method = $Method }\r\n";
    s << L"        } else {\r\n";
    s << L"            @{ id = $Id; method = $Method; params = $Params }\r\n";
    s << L"        }\r\n";
    s << L"        $json = $payload | ConvertTo-Json -Compress -Depth 8\r\n";
    s << L"        $bytes = [System.Text.Encoding]::UTF8.GetBytes($json)\r\n";
    s << L"        $segment = [System.ArraySegment[byte]]::new($bytes, 0, $bytes.Length)\r\n";
    s << L"        $Socket.SendAsync($segment, [System.Net.WebSockets.WebSocketMessageType]::Text, $true, [System.Threading.CancellationToken]::None).GetAwaiter().GetResult()\r\n";
    s << L"        return $true\r\n";
    s << L"    } catch { return $false }\r\n";
    s << L"}\r\n";
    s << L"function Receive-WinterStaticCdpText($Socket, [datetime]$Deadline) {\r\n";
    s << L"    $buffer = New-Object byte[] 65536\r\n";
    s << L"    $stream = New-Object System.IO.MemoryStream\r\n";
    s << L"    try {\r\n";
    s << L"        do {\r\n";
    s << L"            $segment = [System.ArraySegment[byte]]::new($buffer, 0, $buffer.Length)\r\n";
    s << L"            $task = $Socket.ReceiveAsync($segment, [System.Threading.CancellationToken]::None)\r\n";
    s << L"            while (-not $task.IsCompleted) {\r\n";
    s << L"                if ((Get-Date) -ge $Deadline) { return $null }\r\n";
    s << L"                Start-Sleep -Milliseconds 75\r\n";
    s << L"            }\r\n";
    s << L"            $result = $task.GetAwaiter().GetResult()\r\n";
    s << L"            if ($result.MessageType -eq [System.Net.WebSockets.WebSocketMessageType]::Close) { return $null }\r\n";
    s << L"            if ($result.Count -gt 0) { $stream.Write($buffer, 0, $result.Count) }\r\n";
    s << L"        } while (-not $result.EndOfMessage)\r\n";
    s << L"        if ($stream.Length -eq 0) { return '' }\r\n";
    s << L"        return [System.Text.Encoding]::UTF8.GetString($stream.ToArray())\r\n";
    s << L"    } catch { return $null } finally { $stream.Dispose() }\r\n";
    s << L"}\r\n";
    s << L"function Add-WinterStaticRuntimeCandidate($List, $Candidate) {\r\n";
    s << L"    if ($null -eq $Candidate -or [string]::IsNullOrWhiteSpace([string]$Candidate.Url)) { return }\r\n";
    s << L"    foreach ($old in @($List)) {\r\n";
    s << L"        if ([string]$old.Url -ieq [string]$Candidate.Url) {\r\n";
    s << L"            if ([int]$Candidate.Score -gt [int]$old.Score) {\r\n";
    s << L"                $old.Score = [int]$Candidate.Score\r\n";
    s << L"                $old.Kind = [string]$Candidate.Kind\r\n";
    s << L"                $old.Referer = [string]$Candidate.Referer\r\n";
    s << L"                $old.UserAgent = [string]$Candidate.UserAgent\r\n";
    s << L"                $old.Origin = [string]$Candidate.Origin\r\n";
    s << L"                $old.Mime = [string]$Candidate.Mime\r\n";
    s << L"                $old.TotalLength = [long]$Candidate.TotalLength\r\n";
    s << L"                $old.PreviewHint = [bool]$Candidate.PreviewHint\r\n";
    s << L"            }\r\n";
    s << L"            return\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"    if ($List.Count -lt 96) { [void]$List.Add($Candidate) }\r\n";
    s << L"}\r\n";
    s << L"function Invoke-WinterStaticRuntimeDiscovery([string]$PageUrl) {\r\n";
    s << L"    $browser = Get-WinterStaticRuntimeBrowser\r\n";
    s << L"    if ($null -eq $browser) {\r\n";
    s << L"        Write-Host 'Runtime network discovery needs Edge, Chrome, or Brave; none was found.' -ForegroundColor DarkYellow\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_RUNTIME_BROWSER_UNAVAILABLE'\r\n";
    s << L"        return $null\r\n";
    s << L"    }\r\n";
    s << L"    $listener = $null\r\n";
    s << L"    $port = 0\r\n";
    s << L"    try {\r\n";
    s << L"        $listener = [System.Net.Sockets.TcpListener]::new([System.Net.IPAddress]::Loopback, 0)\r\n";
    s << L"        $listener.Start()\r\n";
    s << L"        $port = ([System.Net.IPEndPoint]$listener.LocalEndpoint).Port\r\n";
    s << L"    } catch {\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_RUNTIME_PORT_FAILED'\r\n";
    s << L"        return $null\r\n";
    s << L"    } finally {\r\n";
    s << L"        if ($null -ne $listener) { try { $listener.Stop() } catch {} }\r\n";
    s << L"    }\r\n";
    s << L"    $profile = Join-Path $TaskDir ('runtime-browser-' + [Guid]::NewGuid().ToString('N'))\r\n";
    s << L"    try { [void](New-Item -ItemType Directory -Path $profile -Force -ErrorAction Stop) } catch { return $null }\r\n";
    s << L"    $browserArgs = @(\r\n";
    s << L"        ('--remote-debugging-port=' + $port),\r\n";
    s << L"        ('--user-data-dir=\"' + $profile.Replace('\"','') + '\"'),\r\n";
    s << L"        '--remote-allow-origins=*\',\r\n";
    s << L"        '--no-first-run',\r\n";
    s << L"        '--no-default-browser-check',\r\n";
    s << L"        '--disable-background-mode',\r\n";
    s << L"        $PageUrl\r\n";
    s << L"    )\r\n";
    s << L"    Write-Host ('Static discovery exhausted. Opening an isolated visible {0} detector window for runtime network discovery...' -f $browser.Name) -ForegroundColor Yellow\r\n";
    s << L"    Write-Host 'Start/play the main video in that window if needed. Listening for up to 30 seconds; detected URLs are not printed.' -ForegroundColor Yellow\r\n";
    s << L"    Write-Host 'If the video was already playing before the observer attached, refresh that detector tab once.' -ForegroundColor DarkYellow\r\n";
    s << L"    Write-WinterStaticLog ('WINTERSTATIC_RUNTIME_START:' + $browser.Name)\r\n";
    s << L"    try { $browserVersion = (Get-Item -LiteralPath $browser.Path -ErrorAction Stop).VersionInfo.FileVersion } catch { $browserVersion = '' }\r\n";
    s << L"    Write-Host ('Runtime detector browser: {0} {1}' -f $browser.Name, $browserVersion) -ForegroundColor DarkGray\r\n";
    s << L"    Write-WinterStaticLog ('WINTERSTATIC_RUNTIME_BROWSER:' + $browser.Name + ':version=' + $browserVersion)\r\n";
    s << L"    try {\r\n";
    s << L"        $browserProcess = Start-Process -FilePath $browser.Path -ArgumentList $browserArgs -PassThru -ErrorAction Stop\r\n";
    s << L"    } catch {\r\n";
    s << L"        Write-Host 'Could not start the runtime detector browser.' -ForegroundColor DarkYellow\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_RUNTIME_LAUNCH_FAILED'\r\n";
    s << L"        return $null\r\n";
    s << L"    }\r\n";
    s << L"    $endpoint = 'http://127.0.0.1:' + $port + '/json/list'\r\n";
    s << L"    $targets = @()\r\n";
    s << L"    $target = $null\r\n";
    s << L"    $fallbackTarget = $null\r\n";
    s << L"    $pageHost = ''\r\n";
    s << L"    try { $pageHost = ([System.Uri]$PageUrl).Host.ToLowerInvariant() } catch {}\r\n";
    s << L"    $startupDeadline = (Get-Date).AddSeconds(60)\r\n";
    s << L"    $fallbackDeadline = (Get-Date).AddSeconds(8)\r\n";
    s << L"    $socket = $null\r\n";
    s << L"    $attachAttempts = 0\r\n";
    s << L"    $firstAttachError = ''\r\n";
    s << L"    Write-Host 'Waiting up to 60 seconds for the Edge-first observer endpoint/page target and read-only attachment to become ready...' -ForegroundColor DarkYellow\r\n";
    s << L"    while ((Get-Date) -lt $startupDeadline -and $null -eq $socket) {\r\n";
    s << L"        $target = $null\r\n";
    s << L"        try {\r\n";
    s << L"            # Normalize the DevTools target response explicitly before selecting a page target.\r\n";
    s << L"            # This prevents a multi-value webSocketDebuggerUrl from reaching System.Uri/ConnectAsync.\r\n";
    s << L"            $rawTargets = Invoke-RestMethod -Uri $endpoint -Method Get -TimeoutSec 2 -ErrorAction Stop\r\n";
    s << L"            $targets = @()\r\n";
    s << L"            foreach ($rawTarget in $rawTargets) { if ($null -ne $rawTarget) { $targets += $rawTarget } }\r\n";
    s << L"            # Keep a current HTTP(S) page fallback. The isolated profile may redirect to a different host\r\n";
    s << L"            # (consent/login/canonical URL), so host matching must not be the only attach path.\r\n";
    s << L"            $fallbackMatches = @($targets | Where-Object { $_.type -eq 'page' -and $_.webSocketDebuggerUrl -and ([string]$_.url -match '(?i)^https?://') })\r\n";
    s << L"            if ($fallbackMatches.Count -eq 0) { $fallbackMatches = @($targets | Where-Object { $_.type -eq 'page' -and $_.webSocketDebuggerUrl }) }\r\n";
    s << L"            $fallbackTarget = if ($fallbackMatches.Count -gt 0) { $fallbackMatches[0] } else { $null }\r\n";
    s << L"            $matchingTargets = @()\r\n";
    s << L"            if (-not [string]::IsNullOrWhiteSpace($pageHost)) {\r\n";
    s << L"                $matchingTargets = @($targets | Where-Object { $_.type -eq 'page' -and $_.webSocketDebuggerUrl -and ([string]$_.url -like ('*' + $pageHost + '*')) })\r\n";
    s << L"            }\r\n";
    s << L"            if ($matchingTargets.Count -gt 0) {\r\n";
    s << L"                $target = $matchingTargets[0]\r\n";
    s << L"            } elseif ($null -ne $fallbackTarget -and ($fallbackMatches.Count -eq 1 -or [string]::IsNullOrWhiteSpace($pageHost) -or (Get-Date) -ge $fallbackDeadline)) {\r\n";
    s << L"                $target = $fallbackTarget\r\n";
    s << L"            }\r\n";
    s << L"        } catch {}\r\n";
    s << L"        $wsUrl = ''\r\n";
    s << L"        if ($null -ne $target) {\r\n";
    s << L"            try {\r\n";
    s << L"                $wsValues = @($target.webSocketDebuggerUrl)\r\n";
    s << L"                if ($wsValues.Count -gt 0) { $wsUrl = [string]$wsValues[0] }\r\n";
    s << L"            } catch {}\r\n";
    s << L"        }\r\n";
    s << L"        if (-not [string]::IsNullOrWhiteSpace($wsUrl)) {\r\n";
    s << L"            $candidateSocket = [System.Net.WebSockets.ClientWebSocket]::new()\r\n";
    s << L"            try {\r\n";
    s << L"                $attachAttempts++\r\n";
    s << L"                $wsUri = [System.Uri]::new($wsUrl)\r\n";
    s << L"                $candidateSocket.ConnectAsync($wsUri, [System.Threading.CancellationToken]::None).GetAwaiter().GetResult()\r\n";
    s << L"                if (-not (Send-WinterStaticCdpMessage $candidateSocket 1 'Network.enable' @{ maxTotalBufferSize = 10485760; maxResourceBufferSize = 5242880 })) { throw 'CDP Network.enable could not be sent.' }\r\n";
    s << L"                $socket = $candidateSocket\r\n";
    s << L"                $candidateSocket = $null\r\n";
    s << L"                Write-WinterStaticLog ('WINTERSTATIC_RUNTIME_ATTACH_OK:attempt=' + $attachAttempts)\r\n";
    s << L"                break\r\n";
    s << L"            } catch {\r\n";
    s << L"                if ([string]::IsNullOrWhiteSpace($firstAttachError)) {\r\n";
    s << L"                    try {\r\n";
    s << L"                        $firstAttachError = $_.Exception.GetType().FullName + ': ' + $_.Exception.Message\r\n";
    s << L"                        if ($_.Exception.InnerException) { $firstAttachError += ' | Inner: ' + $_.Exception.InnerException.GetType().FullName + ': ' + $_.Exception.InnerException.Message }\r\n";
    s << L"                    } catch { $firstAttachError = 'Unknown WebSocket attachment error' }\r\n";
    s << L"                    try { $firstAttachError = [regex]::Replace($firstAttachError, '(?i)(?:wss?|https?)://[^\\s]+', '[URL hidden]') } catch {}\r\n";
    s << L"                    if ($firstAttachError.Length -gt 600) { $firstAttachError = $firstAttachError.Substring(0, 600) }\r\n";
    s << L"                    Write-Host ('First runtime-browser WebSocket attachment error: ' + $firstAttachError) -ForegroundColor DarkYellow\r\n";
    s << L"                    Write-WinterStaticLog ('WINTERSTATIC_RUNTIME_ATTACH_ERROR:' + $firstAttachError)\r\n";
    s << L"                }\r\n";
    s << L"                try { $candidateSocket.Dispose() } catch {}\r\n";
    s << L"            }\r\n";
    s << L"        }\r\n";
    s << L"        Start-Sleep -Milliseconds 350\r\n";
    s << L"    }\r\n";
    s << L"    if ($null -eq $socket) {\r\n";
    s << L"        if ($null -eq $targets -or $targets.Count -eq 0) {\r\n";
    s << L"            Write-Host 'The browser opened, but its network-observer endpoint did not become ready within 60 seconds.' -ForegroundColor DarkYellow\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_RUNTIME_ENDPOINT_UNAVAILABLE'\r\n";
    s << L"        } elseif ($null -eq $target -and $null -eq $fallbackTarget) {\r\n";
    s << L"            Write-Host 'No observable page target became available within 60 seconds.' -ForegroundColor DarkYellow\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_RUNTIME_TARGET_UNAVAILABLE'\r\n";
    s << L"        } else {\r\n";
    s << L"            Write-Host ('Could not attach the read-only network observer after waiting 60 seconds ({0} attach attempt(s)).' -f $attachAttempts) -ForegroundColor DarkYellow\r\n";
    s << L"            Write-WinterStaticLog ('WINTERSTATIC_RUNTIME_ATTACH_FAILED:attempts=' + $attachAttempts)\r\n";
    s << L"        }\r\n";
    s << L"        return $null\r\n";
    s << L"    }\r\n";
    s << L"    $requests = @{}\r\n";
    s << L"    $extraHeaders = @{}\r\n";
    s << L"    $candidates = New-Object System.Collections.ArrayList\r\n";
    s << L"    $captureDeadline = (Get-Date).AddSeconds(30)\r\n";
    s << L"    $strongSeenAt = $null\r\n";
    s << L"    try {\r\n";
    s << L"        while ((Get-Date) -lt $captureDeadline) {\r\n";
    s << L"            if ($null -ne $strongSeenAt -and ((Get-Date) - $strongSeenAt).TotalSeconds -ge 2.0) { break }\r\n";
    s << L"            $messageText = Receive-WinterStaticCdpText $socket $captureDeadline\r\n";
    s << L"            if ([string]::IsNullOrWhiteSpace($messageText)) { continue }\r\n";
    s << L"            try { $message = $messageText | ConvertFrom-Json -ErrorAction Stop } catch { continue }\r\n";
    s << L"            $method = [string]$message.method\r\n";
    s << L"            if ($method -eq 'Network.requestWillBeSent') {\r\n";
    s << L"                $p = $message.params\r\n";
    s << L"                $headers = $p.request.headers\r\n";
    s << L"                $referer = ''\r\n";
    s << L"                $ua = ''\r\n";
    s << L"                $origin = ''\r\n";
    s << L"                try { if ($headers.Referer) { $referer = [string]$headers.Referer } elseif ($headers.referer) { $referer = [string]$headers.referer } } catch {}\r\n";
    s << L"                try { if ($headers.'User-Agent') { $ua = [string]$headers.'User-Agent' } elseif ($headers.'user-agent') { $ua = [string]$headers.'user-agent' } } catch {}\r\n";
    s << L"                try { if ($headers.Origin) { $origin = [string]$headers.Origin } elseif ($headers.origin) { $origin = [string]$headers.origin } } catch {}\r\n";
    s << L"                if (-not $referer) { try { $referer = [string]$p.documentURL } catch {} }\r\n";
    s << L"                $requests[[string]$p.requestId] = [PSCustomObject]@{ Url = [string]$p.request.url; Referer = $referer; UserAgent = $ua; Origin = $origin }\r\n";
    s << L"                continue\r\n";
    s << L"            }\r\n";
    s << L"            if ($method -eq 'Network.requestWillBeSentExtraInfo') {\r\n";
    s << L"                try { $extraHeaders[[string]$message.params.requestId] = $message.params.headers } catch {}\r\n";
    s << L"                continue\r\n";
    s << L"            }\r\n";
    s << L"            if ($method -ne 'Network.responseReceived') { continue }\r\n";
    s << L"            $p = $message.params\r\n";
    s << L"            $response = $p.response\r\n";
    s << L"            $requestId = [string]$p.requestId\r\n";
    s << L"            $url = [string]$response.url\r\n";
    s << L"            $mime = ([string]$response.mimeType).ToLowerInvariant()\r\n";
    s << L"            $status = 0\r\n";
    s << L"            try { $status = [int]$response.status } catch {}\r\n";
    s << L"            if (-not $url -or $status -ge 400 -or $url -match '(?i)^(?:data|blob|chrome|devtools):') { continue }\r\n";
    s << L"            if ($url -match '(?i)\\.(?:jpe?g|png|gif|webp|avif|bmp|svg|ico)(?:[?#]|$)' -or $mime -match '^image/') { continue }\r\n";
    s << L"            if ($url -match '(?i)\\.(?:ts|m4s|m2ts)(?:[?#]|$)') { continue }\r\n";
    s << L"            $isManifest = ($url -match '(?i)\\.(?:m3u8|mpd)(?:[?#]|$)') -or ($mime -match '(?:mpegurl|dash\\+xml)')\r\n";
    s << L"            $isDirectVideo = ($url -match '(?i)\\.(?:mp4|webm|m4v|mov|ogv)(?:[?#]|$)') -or ($mime -match '^video/')\r\n";
    s << L"            $isMediaResource = ([string]$p.type -eq 'Media')\r\n";
    s << L"            if (-not $isManifest -and -not $isDirectVideo -and -not $isMediaResource) { continue }\r\n";
    s << L"            if ($mime -match '^audio/' -and -not $isManifest -and $url -notmatch '(?i)\\.(?:mp4|webm|m4v|mov|ogv)(?:[?#]|$)') { continue }\r\n";
    s << L"            $info = $requests[$requestId]\r\n";
    s << L"            $referer = if ($null -ne $info -and $info.Referer) { [string]$info.Referer } else { $PageUrl }\r\n";
    s << L"            $ua = if ($null -ne $info -and $info.UserAgent) { [string]$info.UserAgent } else { $script:DiscoveryUserAgent }\r\n";
    s << L"            $origin = if ($null -ne $info -and $info.Origin) { [string]$info.Origin } else { '' }\r\n";
    s << L"            if ($extraHeaders.ContainsKey($requestId)) {\r\n";
    s << L"                $eh = $extraHeaders[$requestId]\r\n";
    s << L"                try { if (-not $referer -and $eh.Referer) { $referer = [string]$eh.Referer } } catch {}\r\n";
    s << L"                try { if (-not $ua -and $eh.'User-Agent') { $ua = [string]$eh.'User-Agent' } } catch {}\r\n";
    s << L"                try { if (-not $origin -and $eh.Origin) { $origin = [string]$eh.Origin } } catch {}\r\n";
    s << L"            }\r\n";
    s << L"            $totalLength = [long]0\r\n";
    s << L"            try {\r\n";
    s << L"                $headers = $response.headers\r\n";
    s << L"                $range = [string]$headers.'content-range'\r\n";
    s << L"                if (-not $range) { $range = [string]$headers.'Content-Range' }\r\n";
    s << L"                if ($range -match '/(?<n>\\d+)$') { $totalLength = [long]$Matches['n'] }\r\n";
    s << L"                if ($totalLength -le 0) {\r\n";
    s << L"                    $cl = [string]$headers.'content-length'\r\n";
    s << L"                    if (-not $cl) { $cl = [string]$headers.'Content-Length' }\r\n";
    s << L"                    if ($cl -match '^\\d+$') { $totalLength = [long]$cl }\r\n";
    s << L"                }\r\n";
    s << L"            } catch {}\r\n";
    s << L"            $previewHint = $url -match '(?i)(?:^|[/_.?=&%+\\-])(?:preview|previews|teaser|teasers|hover|thumbnail|thumbnails|thumb|sprite|storyboard|poster|loop|sample)(?:[/_.?=&%+\\-]|$)'\r\n";
    s << L"            $segmentHint = $url -match '(?i)(?:^|[/_.?=&%+\\-])(?:segment|segments|seg|chunk|chunks|fragment|fragments|frag|part|init)(?:[/_.?=&%+\\-]|$)'\r\n";
    s << L"            $score = 0\r\n";
    s << L"            $kind = 'runtime media response'\r\n";
    s << L"            if ($isManifest) { $score = 360; $kind = 'runtime streaming manifest' }\r\n";
    s << L"            elseif ($mime -match '^video/') { $score = 250; $kind = 'runtime video response' }\r\n";
    s << L"            elseif ($url -match '(?i)\\.(?:mp4|webm|m4v|mov|ogv)(?:[?#]|$)') { $score = 220; $kind = 'runtime direct video' }\r\n";
    s << L"            elseif ($isMediaResource) { $score = 180; $kind = 'runtime media response' }\r\n";
    s << L"            if ($url -match '(?i)(?:master|manifest|playlist)') { $score += 25 }\r\n";
    s << L"            if ($previewHint) { $score -= 220 }\r\n";
    s << L"            if ($segmentHint -and -not $isManifest) { $score -= 140 }\r\n";
    s << L"            if (-not $isManifest -and $totalLength -gt 0) {\r\n";
    s << L"                if ($totalLength -lt 1572864) { $score -= 130; $previewHint = $true }\r\n";
    s << L"                elseif ($totalLength -lt 6291456) { $score -= 70 }\r\n";
    s << L"                elseif ($totalLength -lt 15728640) { $score -= 25 }\r\n";
    s << L"                elseif ($totalLength -ge 52428800) { $score += 25 }\r\n";
    s << L"            }\r\n";
    s << L"            $candidate = [PSCustomObject]@{\r\n";
    s << L"                Kind = $kind; Url = $url; Referer = $referer; UserAgent = $ua; Origin = $origin\r\n";
    s << L"                Mime = $mime; TotalLength = $totalLength; Score = $score; PreviewHint = [bool]$previewHint\r\n";
    s << L"            }\r\n";
    s << L"            Add-WinterStaticRuntimeCandidate $candidates $candidate\r\n";
    s << L"            if ($score -ge 340 -and -not $previewHint) { if ($null -eq $strongSeenAt) { $strongSeenAt = Get-Date } }\r\n";
    s << L"        }\r\n";
    s << L"    } finally {\r\n";
    s << L"        try {\r\n";
    s << L"            if ($socket.State -eq [System.Net.WebSockets.WebSocketState]::Open) {\r\n";
    s << L"                $socket.CloseAsync([System.Net.WebSockets.WebSocketCloseStatus]::NormalClosure, 'done', [System.Threading.CancellationToken]::None).GetAwaiter().GetResult()\r\n";
    s << L"            }\r\n";
    s << L"        } catch {}\r\n";
    s << L"        try { $socket.Dispose() } catch {}\r\n";
    s << L"    }\r\n";
    s << L"    if ($candidates.Count -eq 0) {\r\n";
    s << L"        Write-Host 'Runtime observer saw no plausible full-video or manifest response.' -ForegroundColor DarkYellow\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_RUNTIME_NONE'\r\n";
    s << L"        Write-Host 'The detector/account browser may be closed now.' -ForegroundColor DarkGray\r\n";
    s << L"        return $null\r\n";
    s << L"    }\r\n";
    s << L"    $ranked = @($candidates | Sort-Object @{Expression='Score';Descending=$true})\r\n";
    s << L"    Write-Host 'Runtime candidate summary (URLs hidden):' -ForegroundColor DarkGray\r\n";
    s << L"    foreach ($item in @($ranked | Select-Object -First 5)) {\r\n";
    s << L"        $size = if ([long]$item.TotalLength -gt 0) { ('{0:N1} MB' -f ([long]$item.TotalLength / 1MB)) } else { 'size unknown' }\r\n";
    s << L"        Write-Host ('  {0} | score {1} | {2} | {3} | preview-like={4}' -f $item.Kind, $item.Score, $item.Mime, $size, $item.PreviewHint) -ForegroundColor DarkGray\r\n";
    s << L"        Write-WinterStaticLog ('WINTERSTATIC_RUNTIME_CANDIDATE:' + $item.Kind + ':score=' + $item.Score + ':mime=' + $item.Mime + ':bytes=' + $item.TotalLength + ':preview=' + $item.PreviewHint)\r\n";
    s << L"    }\r\n";
    s << L"    $usable = @($ranked | Where-Object { -not [bool]$_.PreviewHint -and [int]$_.Score -ge 120 })\r\n";
    s << L"    if ($usable.Count -eq 0) {\r\n";
    s << L"        Write-Host 'Runtime observer found only preview/segment-like video responses; refusing them.' -ForegroundColor Yellow\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_RUNTIME_PREVIEW_ONLY'\r\n";
    s << L"        Write-Host 'The detector/account browser may be closed now.' -ForegroundColor DarkGray\r\n";
    s << L"        return $null\r\n";
    s << L"    }\r\n";
    s << L"    Write-WinterStaticLog ('WINTERSTATIC_RUNTIME_SELECTED:' + $usable[0].Kind + ':score=' + $usable[0].Score)\r\n";
    s << L"    Write-Host 'Runtime network discovery found a stronger media candidate.' -ForegroundColor Green\r\n";
    s << L"    Write-Host 'The detector/account browser may be closed now.' -ForegroundColor DarkGray\r\n";
    s << L"    return $usable[0]\r\n";
    s << L"}\r\n";
    s << L"function Get-WinterStaticPageHtmlViaYtDlp([string]$PageUrl, [string[]]$FetchArgs) {\r\n";
    s << L"    $scratch = Join-Path ([System.IO.Path]::GetTempPath()) ('WinterStatic-page-' + [Guid]::NewGuid().ToString('N'))\r\n";
    s << L"    try {\r\n";
    s << L"        [void](New-Item -ItemType Directory -Path $scratch -Force -ErrorAction Stop)\r\n";
    s << L"        Push-Location $scratch\r\n";
    s << L"        try {\r\n";
    s << L"            $captureArgs = @('--ignore-config', '--skip-download', '--no-playlist', '--write-pages', '--socket-timeout', '20', '--retries', '0', '--no-warnings')\r\n";
    s << L"            if ($null -ne $FetchArgs) { $captureArgs += @($FetchArgs) }\r\n";
    s << L"            $captureArgs += @($PageUrl)\r\n";
    s << L"            & $script:CurrentYtDlp @captureArgs *> $null\r\n";
    s << L"        } finally { Pop-Location }\r\n";
    s << L"        $dump = Get-ChildItem -LiteralPath $scratch -File -Filter '*.dump' -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1\r\n";
    s << L"        if ($null -eq $dump) {\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_DISCOVERY_YTDLP_PAGE_NONE'\r\n";
    s << L"            return ''\r\n";
    s << L"        }\r\n";
    s << L"        $bytes = [System.IO.File]::ReadAllBytes($dump.FullName)\r\n";
    s << L"        if ($bytes.Length -eq 0) { return '' }\r\n";
    s << L"        $html = [System.Text.Encoding]::UTF8.GetString($bytes)\r\n";
    s << L"        if ($html.Length -gt 8388608) { $html = $html.Substring(0, 8388608) }\r\n";
    s << L"        Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_YTDLP_PAGE_OK:' + $bytes.Length)\r\n";
    s << L"        return $html\r\n";
    s << L"    } catch {\r\n";
    s << L"        Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_YTDLP_PAGE_ERROR:' + $_.Exception.Message)\r\n";
    s << L"        return ''\r\n";
    s << L"    } finally {\r\n";
    s << L"        try { if (Test-Path -LiteralPath $scratch) { Remove-Item -LiteralPath $scratch -Recurse -Force -ErrorAction SilentlyContinue } } catch {}\r\n";
    s << L"    }\r\n";
    s << L"}\r\n";
    s << L"function New-WinterStaticStructuredPlayerCandidate([string]$Scan, [string]$PageUrl) {\r\n";
    s << L"    if ([string]::IsNullOrWhiteSpace($Scan)) { return $null }\r\n";
    s << L"    function New-HostedPlayerUrl([string]$Account, [string]$Player, [string]$Embed, [string]$Video) {\r\n";
    s << L"        if ([string]::IsNullOrWhiteSpace($Account) -or [string]::IsNullOrWhiteSpace($Player) -or [string]::IsNullOrWhiteSpace($Video)) { return '' }\r\n";
    s << L"        if ($Account -notmatch '^\\d{4,}$' -or $Player -notmatch '^[A-Za-z0-9_-]{2,}$' -or $Video -notmatch '^(?:ref:)?[A-Za-z0-9_-]{4,}$') { return '' }\r\n";
    s << L"        if ([string]::IsNullOrWhiteSpace($Embed)) { $Embed = 'default' }\r\n";
    s << L"        if ($Embed -notmatch '^[A-Za-z0-9_-]+$') { $Embed = 'default' }\r\n";
    s << L"        return ('https://players.brightcove.net/{0}/{1}_{2}/index.html?videoId={3}' -f $Account, $Player, $Embed, $Video)\r\n";
    s << L"    }\r\n";
    s << L"    foreach ($m in [regex]::Matches($Scan, '(?is)<[^>]+\\bdata-video-id\\s*=\\s*[\"''](?<video>[^\"'']+)[\"''][^>]*>')) {\r\n";
    s << L"        $tag = $m.Value\r\n";
    s << L"        $video = $m.Groups['video'].Value\r\n";
    s << L"        $account = [regex]::Match($tag, '(?i)\\bdata-account\\s*=\\s*[\"''](?<v>[^\"'']+)[\"'']').Groups['v'].Value\r\n";
    s << L"        $player = [regex]::Match($tag, '(?i)\\bdata-player\\s*=\\s*[\"''](?<v>[^\"'']+)[\"'']').Groups['v'].Value\r\n";
    s << L"        $embed = [regex]::Match($tag, '(?i)\\bdata-embed\\s*=\\s*[\"''](?<v>[^\"'']+)[\"'']').Groups['v'].Value\r\n";
    s << L"        $url = New-HostedPlayerUrl $account $player $embed $video\r\n";
    s << L"        if ($url) { return [PSCustomObject]@{ Kind = 'structured hosted-player metadata'; Url = $url; Referer = $PageUrl } }\r\n";
    s << L"    }\r\n";
    s << L"    $bootstrap = [regex]::Match($Scan, '(?i)(?:https?:)?//players\\.brightcove\\.net/(?<account>\\d{4,})/(?<player>[A-Za-z0-9_-]+)_(?<embed>[A-Za-z0-9_-]+)/index(?:\\.min)?\\.js')\r\n";
    s << L"    if ($bootstrap.Success) {\r\n";
    s << L"        $videoMatches = @([regex]::Matches($Scan, '(?i)(?:data-video-id\\s*=\\s*[\"'']|[\"''](?:videoId|video_id)[\"'']\\s*[:=]\\s*[\"'']?)(?<v>(?:ref:)?[A-Za-z0-9_-]{4,})'))\r\n";
    s << L"        $videoIds = @($videoMatches | ForEach-Object { $_.Groups['v'].Value } | Select-Object -Unique)\r\n";
    s << L"        if ($videoIds.Count -eq 1) {\r\n";
    s << L"            $url = New-HostedPlayerUrl $bootstrap.Groups['account'].Value $bootstrap.Groups['player'].Value $bootstrap.Groups['embed'].Value $videoIds[0]\r\n";
    s << L"            if ($url) { return [PSCustomObject]@{ Kind = 'hosted-player bootstrap plus video ID'; Url = $url; Referer = $PageUrl } }\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"    foreach ($m in [regex]::Matches($Scan, '(?i)[\"'']video[\"'']\\s*:')) {\r\n";
    s << L"        $length = [Math]::Min(8192, $Scan.Length - $m.Index)\r\n";
    s << L"        $body = $Scan.Substring($m.Index, $length)\r\n";
    s << L"        $account = [regex]::Match($body, '(?i)[\"''](?:accountId|account_id)[\"'']\\s*:\\s*[\"'']?(?<v>\\d{4,})').Groups['v'].Value\r\n";
    s << L"        $player = [regex]::Match($body, '(?i)[\"''](?:playerId|player_id)[\"'']\\s*:\\s*[\"''](?<v>[A-Za-z0-9_-]{2,})').Groups['v'].Value\r\n";
    s << L"        $embed = [regex]::Match($body, '(?i)[\"''](?:embedId|embed_id|embed)[\"'']\\s*:\\s*[\"''](?<v>[A-Za-z0-9_-]+)').Groups['v'].Value\r\n";
    s << L"        $video = [regex]::Match($body, '(?i)[\"''](?:videoId|video_id|id)[\"'']\\s*:\\s*[\"'']?(?<v>(?:ref:)?[A-Za-z0-9_-]{4,})').Groups['v'].Value\r\n";
    s << L"        $url = New-HostedPlayerUrl $account $player $embed $video\r\n";
    s << L"        if ($url) { return [PSCustomObject]@{ Kind = 'serialized hosted-player metadata'; Url = $url; Referer = $PageUrl } }\r\n";
    s << L"    }\r\n";
    s << L"    foreach ($m in [regex]::Matches($Scan, '(?i)[\"''](?:videoId|video_id)[\"'']\\s*:\\s*[\"'']?(?<video>(?:ref:)?[A-Za-z0-9_-]{4,})')) {\r\n";
    s << L"        $start = [Math]::Max(0, $m.Index - 4096)\r\n";
    s << L"        $length = [Math]::Min(8192, $Scan.Length - $start)\r\n";
    s << L"        $context = $Scan.Substring($start, $length)\r\n";
    s << L"        $account = [regex]::Match($context, '(?i)[\"''](?:accountId|account_id)[\"'']\\s*:\\s*[\"'']?(?<v>\\d{4,})').Groups['v'].Value\r\n";
    s << L"        $player = [regex]::Match($context, '(?i)[\"''](?:playerId|player_id)[\"'']\\s*:\\s*[\"''](?<v>[A-Za-z0-9_-]{2,})').Groups['v'].Value\r\n";
    s << L"        $embed = [regex]::Match($context, '(?i)[\"''](?:embedId|embed_id|embed)[\"'']\\s*:\\s*[\"''](?<v>[A-Za-z0-9_-]+)').Groups['v'].Value\r\n";
    s << L"        $url = New-HostedPlayerUrl $account $player $embed $m.Groups['video'].Value\r\n";
    s << L"        if ($url) { return [PSCustomObject]@{ Kind = 'serialized player component IDs'; Url = $url; Referer = $PageUrl } }\r\n";
    s << L"    }\r\n";
    s << L"    return $null\r\n";
    s << L"}\r\n";
    s << L"function Test-WinterStaticPreviewVideoTag([string]$Tag) {\r\n";
    s << L"    if ([string]::IsNullOrWhiteSpace($Tag)) { return $false }\r\n";
    s << L"    $hasLoop = $Tag -match '(?i)\\bloop(?:\\s|=|>|/)'; $hasMuted = $Tag -match '(?i)\\bmuted(?:\\s|=|>|/)'; $hasAutoplay = $Tag -match '(?i)\\bautoplay(?:\\s|=|>|/)'; $hasInline = $Tag -match '(?i)\\bplaysinline(?:\\s|=|>|/)'\r\n";
    s << L"    return (($hasLoop -and ($hasMuted -or $hasAutoplay)) -or ($hasAutoplay -and $hasMuted -and $hasInline))\r\n";
    s << L"}\r\n";
    s << L"function Find-WinterStaticMediaCandidate([string]$PageUrl, [int]$Depth = 0, [string]$HtmlOverride = '') {\r\n";
    s << L"    $html = $HtmlOverride\r\n";
    s << L"    if ([string]::IsNullOrWhiteSpace($html)) {\r\n";
    s << L"        try {\r\n";
    s << L"            $headers = @{ 'User-Agent' = $script:DiscoveryUserAgent; 'Accept' = 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8' }\r\n";
    s << L"            $response = Invoke-WebRequest -Uri $PageUrl -UseBasicParsing -Headers $headers -MaximumRedirection 5 -TimeoutSec 20 -ErrorAction Stop\r\n";
    s << L"            $html = [string]$response.Content\r\n";
    s << L"        } catch {\r\n";
    s << L"            if ($Depth -eq 0) { $script:DiscoveryDirectFetchFailed = $true }\r\n";
    s << L"            Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_FETCH_FAIL:' + $_.Exception.Message)\r\n";
    s << L"            return $null\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"    if ([string]::IsNullOrWhiteSpace($html)) { return $null }\r\n";
    s << L"    if ($html.Length -gt 8388608) { $html = $html.Substring(0, 8388608) }\r\n";
    s << L"    $scan = [System.Net.WebUtility]::HtmlDecode($html).Replace('\\/', '/').Replace('\\u0026', '&').Replace('\\u003d', '=').Replace('\\u002F', '/')\r\n";
    s << L"    try { $scan = [regex]::Replace($scan, '\\\\u(?<h>[0-9A-Fa-f]{4})', { param($m) [char][Convert]::ToInt32($m.Groups['h'].Value, 16) }) } catch {}\r\n";
    s << L"    try { $scan = [regex]::Replace($scan, '\\\\x(?<h>[0-9A-Fa-f]{2})', { param($m) [char][Convert]::ToInt32($m.Groups['h'].Value, 16) }) } catch {}\r\n";
    s << L"    $scan = $scan.Replace(([string][char]92 + [char]34), [string][char]34).Replace(([string][char]92 + [char]39), [string][char]39)\r\n";
    s << L"    $candidates = New-Object System.Collections.ArrayList\r\n";
    s << L"    if ($Depth -eq 0) { $script:DiscoveryCandidateOrder = 0 }\r\n";
    s << L"\r\n";
    s << L"    foreach ($m in [regex]::Matches($scan, '(?is)<meta\\b[^>]*>')) {\r\n";
    s << L"        $tag = $m.Value\r\n";
    s << L"        $key = [regex]::Match($tag, '(?i)\\b(?:property|name)\\s*=\\s*[\\\"''](?<v>og:video(?::secure_url|:url)?|twitter:player(?::stream)?)[\\\"'']')\r\n";
    s << L"        $val = [regex]::Match($tag, '(?i)\\bcontent\\s*=\\s*[\\\"''](?<v>[^\\\"'']+)[\\\"'']')\r\n";
    s << L"        if ($key.Success -and $val.Success) {\r\n";
    s << L"            if ($key.Groups['v'].Value -ieq 'twitter:player') {\r\n";
    s << L"                $playerUrl = Resolve-WinterStaticMediaUrl $val.Groups['v'].Value $PageUrl\r\n";
    s << L"                if ($playerUrl -and $playerUrl -ne $PageUrl) {\r\n";
    s << L"                    Add-WinterStaticRankedCandidate $candidates ([PSCustomObject]@{ Kind = 'page player metadata'; Url = $playerUrl; Referer = $PageUrl }) 185 $false\r\n";
    s << L"                }\r\n";
    s << L"            } else {\r\n";
    s << L"                $c = New-WinterStaticMediaCandidate 'page video metadata' $val.Groups['v'].Value $PageUrl\r\n";
    s << L"                Add-WinterStaticRankedCandidate $candidates $c 125 $true\r\n";
    s << L"            }\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    foreach ($m in [regex]::Matches($scan, '(?is)<video\\b[^>]*>')) {\r\n";
    s << L"        $tag = $m.Value\r\n";
    s << L"        $previewHint = Test-WinterStaticPreviewVideoTag $tag\r\n";
    s << L"        $src = [regex]::Match($tag, '(?i)\\b(?<n>src|data-src|data-video-src)\\s*=\\s*[\\\"''](?<v>[^\\\"'']+)[\\\"'']')\r\n";
    s << L"        if ($src.Success) {\r\n";
    s << L"            $sourceKind = if ($previewHint) { 'preview-like HTML5 media source' } elseif ($src.Groups['n'].Value -ieq 'src') { 'HTML5 media source' } else { 'lazy HTML5 media source' }\r\n";
    s << L"            $c = New-WinterStaticMediaCandidate $sourceKind $src.Groups['v'].Value $PageUrl\r\n";
    s << L"            if ($null -ne $c -and $previewHint) { $c | Add-Member -NotePropertyName PreviewHint -NotePropertyValue $true -Force }\r\n";
    s << L"            Add-WinterStaticRankedCandidate $candidates $c 120 $true\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"    foreach ($block in [regex]::Matches($scan, '(?is)<video\\b(?<open>[^>]*)>(?<body>.*?)</video\\s*>')) {\r\n";
    s << L"        $previewHint = Test-WinterStaticPreviewVideoTag ('<video ' + $block.Groups['open'].Value + '>')\r\n";
    s << L"        foreach ($sm in [regex]::Matches($block.Groups['body'].Value, '(?is)<source\\b[^>]*>')) {\r\n";
    s << L"            $src = [regex]::Match($sm.Value, '(?i)\\b(?<n>src|data-src|data-video-src)\\s*=\\s*[\\\"''](?<v>[^\\\"'']+)[\\\"'']')\r\n";
    s << L"            if (-not $src.Success) { continue }\r\n";
    s << L"            $sourceKind = if ($previewHint) { 'preview-like HTML5 source' } elseif ($src.Groups['n'].Value -ieq 'src') { 'HTML5 media source' } else { 'lazy HTML5 media source' }\r\n";
    s << L"            $c = New-WinterStaticMediaCandidate $sourceKind $src.Groups['v'].Value $PageUrl\r\n";
    s << L"            if ($null -ne $c -and $previewHint) { $c | Add-Member -NotePropertyName PreviewHint -NotePropertyValue $true -Force }\r\n";
    s << L"            Add-WinterStaticRankedCandidate $candidates $c 120 $true\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"    foreach ($m in [regex]::Matches($scan, '(?is)<source\\b[^>]*>')) {\r\n";
    s << L"        $src = [regex]::Match($m.Value, '(?i)\\b(?<n>src|data-src|data-video-src)\\s*=\\s*[\\\"''](?<v>[^\\\"'']+)[\\\"'']')\r\n";
    s << L"        if (-not $src.Success) { continue }\r\n";
    s << L"        $sourceKind = if ($src.Groups['n'].Value -ieq 'src') { 'HTML5 media source' } else { 'lazy HTML5 media source' }\r\n";
    s << L"        $c = New-WinterStaticMediaCandidate $sourceKind $src.Groups['v'].Value $PageUrl\r\n";
    s << L"        Add-WinterStaticRankedCandidate $candidates $c 120 $true\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    foreach ($m in [regex]::Matches($scan, '(?is)[\\\"'']contentUrl[\\\"'']\\s*:\\s*[\\\"''](?<v>[^\\\"'']+)[\\\"'']')) {\r\n";
    s << L"        $c = New-WinterStaticMediaCandidate 'JSON-LD content URL' $m.Groups['v'].Value $PageUrl\r\n";
    s << L"        Add-WinterStaticRankedCandidate $candidates $c 75 $true\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    foreach ($m in [regex]::Matches($scan, '(?is)[\\x22''](?:embedUrl|embed_url|playerUrl|player_url)[\\x22'']\\s*:\\s*[\\x22''](?<v>[^\\x22'']+)[\\x22'']')) {\r\n";
    s << L"        $playerUrl = Resolve-WinterStaticMediaUrl $m.Groups['v'].Value $PageUrl\r\n";
    s << L"        if ($playerUrl -and $playerUrl -ne $PageUrl) {\r\n";
    s << L"            Add-WinterStaticRankedCandidate $candidates ([PSCustomObject]@{ Kind = 'serialized player URL'; Url = $playerUrl; Referer = $PageUrl }) 180 $false\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    foreach ($m in [regex]::Matches($scan, '(?is)<(?:meta|link)\\b[^>]*>')) {\r\n";
    s << L"        $tag = $m.Value\r\n";
    s << L"        $item = [regex]::Match($tag, '(?i)\\bitemprop\\s*=\\s*[\\x22''](?<v>embedUrl|contentUrl)[\\x22'']')\r\n";
    s << L"        if (-not $item.Success) { continue }\r\n";
    s << L"        $val = [regex]::Match($tag, '(?i)\\b(?:content|href)\\s*=\\s*[\\x22''](?<v>[^\\x22'']+)[\\x22'']')\r\n";
    s << L"        if (-not $val.Success) { continue }\r\n";
    s << L"        if ($item.Groups['v'].Value -ieq 'embedUrl') {\r\n";
    s << L"            $playerUrl = Resolve-WinterStaticMediaUrl $val.Groups['v'].Value $PageUrl\r\n";
    s << L"            if ($playerUrl -and $playerUrl -ne $PageUrl) {\r\n";
    s << L"                Add-WinterStaticRankedCandidate $candidates ([PSCustomObject]@{ Kind = 'structured-data player URL'; Url = $playerUrl; Referer = $PageUrl }) 180 $false\r\n";
    s << L"            }\r\n";
    s << L"        } else {\r\n";
    s << L"            $c = New-WinterStaticMediaCandidate 'structured-data content URL' $val.Groups['v'].Value $PageUrl\r\n";
    s << L"            Add-WinterStaticRankedCandidate $candidates $c 80 $true\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    $structuredPlayer = New-WinterStaticStructuredPlayerCandidate $scan $PageUrl\r\n";
    s << L"    if ($null -ne $structuredPlayer) { Add-WinterStaticRankedCandidate $candidates $structuredPlayer 195 $false }\r\n";
    s << L"\r\n";
    s << L"    if ($Depth -lt 1) {\r\n";
    s << L"        foreach ($m in [regex]::Matches($scan, '(?is)<link\\b[^>]*>')) {\r\n";
    s << L"            $tag = $m.Value\r\n";
    s << L"            $rel = [regex]::Match($tag, '(?i)\\brel\\s*=\\s*[\"''](?<v>[^\"'']+)[\"'']')\r\n";
    s << L"            if (-not $rel.Success -or $rel.Groups['v'].Value -notmatch '(?i)(?:^|\\s)amphtml(?:\\s|$)') { continue }\r\n";
    s << L"            $href = [regex]::Match($tag, '(?i)\\bhref\\s*=\\s*[\"''](?<v>[^\"'']+)[\"'']')\r\n";
    s << L"            if (-not $href.Success) { continue }\r\n";
    s << L"            $alternateUrl = Resolve-WinterStaticMediaUrl $href.Groups['v'].Value $PageUrl\r\n";
    s << L"            if (-not $alternateUrl -or $alternateUrl -eq $PageUrl) { continue }\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_DISCOVERY_ALTERNATE_PAGE'\r\n";
    s << L"            Write-Host 'Declared alternate page found. Scanning that representation for media...' -ForegroundColor DarkYellow\r\n";
    s << L"            $alternate = Find-WinterStaticMediaCandidate $alternateUrl ($Depth + 1)\r\n";
    s << L"            if ($null -ne $alternate) {\r\n";
    s << L"                $alternateScore = if ($alternate.PSObject.Properties['Score']) { [int]$alternate.Score + 5 } else { 150 }\r\n";
    s << L"                Add-WinterStaticRankedCandidate $candidates $alternate $alternateScore $false\r\n";
    s << L"            }\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    foreach ($m in [regex]::Matches($scan, '(?is)<a\\b[^>]*>')) {\r\n";
    s << L"        $href = [regex]::Match($m.Value, '(?i)\\bhref\\s*=\\s*[\\\"''](?<v>[^\\\"'']+)[\\\"'']')\r\n";
    s << L"        if (-not $href.Success) { continue }\r\n";
    s << L"        $resolved = Resolve-WinterStaticMediaUrl $href.Groups['v'].Value $PageUrl\r\n";
    s << L"        if (-not $resolved -or $resolved -notmatch '(?i)\\.(?:m3u8|mpd|mp4|webm|m4v|mov|m4a|mp3|aac|ogg|opus)(?:[?#]|$)') { continue }\r\n";
    s << L"        $kind = if ($resolved -match '(?i)\\.m3u8(?:[?#]|$)') { 'HLS playlist link' } elseif ($resolved -match '(?i)\\.mpd(?:[?#]|$)') { 'DASH manifest link' } else { 'direct media link' }\r\n";
    s << L"        $baseScore = if ($kind -match 'playlist|manifest') { 150 } else { 70 }\r\n";
    s << L"        $c = New-WinterStaticMediaCandidate $kind $resolved $PageUrl\r\n";
    s << L"        Add-WinterStaticRankedCandidate $candidates $c $baseScore $true\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    foreach ($m in [regex]::Matches($scan, '(?is)[\\\"''](?<v>(?:/|\\./|\\.\\./)[^\\\"''<>\\s]+?\\.(?:m3u8|mpd|mp4|webm|m4v|mov|m4a|mp3|aac|ogg|opus)(?:[?#][^\\\"''<>\\s]*)?)[\\\"'']')) {\r\n";
    s << L"        $raw = $m.Groups['v'].Value\r\n";
    s << L"        $baseScore = if ($raw -match '(?i)\\.(?:m3u8|mpd)(?:[?#]|$)') { 145 } else { 65 }\r\n";
    s << L"        $c = New-WinterStaticMediaCandidate 'relative media reference' $raw $PageUrl\r\n";
    s << L"        Add-WinterStaticRankedCandidate $candidates $c $baseScore $true\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    foreach ($m in [regex]::Matches($scan, '(?i)https?://[^\\s\\\"''<>]+?\\.(?:m3u8|mpd|mp4|webm|m4v|mov|m4a|mp3|aac|ogg|opus)(?:\\?[^\\s\\\"''<>]*)?')) {\r\n";
    s << L"        $raw = $m.Value\r\n";
    s << L"        $kind = if ($raw -match '(?i)\\.m3u8(?:\\?|$)') { 'HLS playlist' } elseif ($raw -match '(?i)\\.mpd(?:\\?|$)') { 'DASH manifest' } else { 'direct media URL' }\r\n";
    s << L"        $baseScore = if ($kind -match 'playlist|manifest') { 145 } else { 60 }\r\n";
    s << L"        $c = New-WinterStaticMediaCandidate $kind $raw $PageUrl\r\n";
    s << L"        Add-WinterStaticRankedCandidate $candidates $c $baseScore $true\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    foreach ($m in [regex]::Matches($scan, '(?is)<script\\b[^>]*>')) {\r\n";
    s << L"        $src = [regex]::Match($m.Value, '(?i)\\b(?:src|data-src)\\s*=\\s*[\\\"''](?<v>[^\\\"'']+)[\\\"'']')\r\n";
    s << L"        if (-not $src.Success) { continue }\r\n";
    s << L"        $playerUrl = Resolve-WinterStaticMediaUrl $src.Groups['v'].Value $PageUrl\r\n";
    s << L"        if ($playerUrl -and $playerUrl -match '(?i)/(?:players|embed)/[^/?#]{4,}') {\r\n";
    s << L"            Add-WinterStaticRankedCandidate $candidates ([PSCustomObject]@{ Kind = 'embedded player bootstrap'; Url = $playerUrl; Referer = $PageUrl }) 170 $false\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    foreach ($m in [regex]::Matches($scan, '(?is)[\\\"''](?<v>https?://[^\\\"''<>\\s]+/(?:players|embed)/[^\\\"''<>\\s]{4,})[\\\"'']')) {\r\n";
    s << L"        $playerUrl = Resolve-WinterStaticMediaUrl $m.Groups['v'].Value $PageUrl\r\n";
    s << L"        if ($playerUrl) { Add-WinterStaticRankedCandidate $candidates ([PSCustomObject]@{ Kind = 'embedded player reference'; Url = $playerUrl; Referer = $PageUrl }) 175 $false }\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    if ($Depth -lt 1) {\r\n";
    s << L"        $frames = @([regex]::Matches($scan, '(?is)<iframe\\b[^>]*>'))\r\n";
    s << L"        $limit = [Math]::Min($frames.Count, 4)\r\n";
    s << L"        for ($i = 0; $i -lt $limit; $i++) {\r\n";
    s << L"            $src = [regex]::Match($frames[$i].Value, '(?i)\\b(?:src|data-src|data-lazy-src)\\s*=\\s*[\\\"''](?<v>[^\\\"'']+)[\\\"'']')\r\n";
    s << L"            if (-not $src.Success) { continue }\r\n";
    s << L"            $frameUrl = Resolve-WinterStaticMediaUrl $src.Groups['v'].Value $PageUrl\r\n";
    s << L"            if (-not $frameUrl -or $frameUrl -eq $PageUrl) { continue }\r\n";
    s << L"            if ($frameUrl -match '(?i)\\.(?:m3u8|mpd|mp4|webm|m4v|mov|m4a|mp3|aac|ogg|opus)(?:\\?|$)') {\r\n";
    s << L"                $kind = if ($frameUrl -match '(?i)\\.m3u8(?:\\?|$)') { 'HLS playlist' } elseif ($frameUrl -match '(?i)\\.mpd(?:\\?|$)') { 'DASH manifest' } else { 'direct media URL' }\r\n";
    s << L"                $baseScore = if ($kind -match 'playlist|manifest') { 155 } else { 105 }\r\n";
    s << L"                $c = [PSCustomObject]@{ Kind = $kind; Url = $frameUrl; Referer = $PageUrl }\r\n";
    s << L"                Add-WinterStaticRankedCandidate $candidates $c $baseScore $true\r\n";
    s << L"            }\r\n";
    s << L"            $nested = Find-WinterStaticMediaCandidate $frameUrl ($Depth + 1)\r\n";
    s << L"            if ($null -ne $nested) {\r\n";
    s << L"                $nestedScore = if ($nested.PSObject.Properties['Score']) { [int]$nested.Score + 10 } else { 150 }\r\n";
    s << L"                Add-WinterStaticRankedCandidate $candidates $nested $nestedScore $false\r\n";
    s << L"            }\r\n";
    s << L"            if ($frameUrl -match '(?i)(youtube|youtu\\.be|vimeo|dailymotion|brightcove|wistia|streamable|/embed/|/player/|/players/)') {\r\n";
    s << L"                Add-WinterStaticRankedCandidate $candidates ([PSCustomObject]@{ Kind = 'embedded player'; Url = $frameUrl; Referer = $PageUrl }) 165 $false\r\n";
    s << L"            }\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"\r\n";
    s << L"    return Select-WinterStaticRankedCandidate $candidates\r\n";
    s << L"}\r\n";
    s << L"Write-Host 'WinterStatic yt-dlp Downloader - LIVE PowerShell' -ForegroundColor Cyan\r\n";
    s << L"Write-Host 'This window owns the real yt-dlp task. Closing the GUI will not stop it.' -ForegroundColor Yellow\r\n";
    if (closeOnSuccess) {
        s << L"Write-Host 'Ctrl+C stops yt-dlp. This window will close automatically after a successful task.' -ForegroundColor Yellow\r\n";
    } else {
        s << L"Write-Host 'Ctrl+C stops yt-dlp. When the task finishes, this window stays open.' -ForegroundColor Yellow\r\n";
    }
    s << L"Write-Host ''\r\n";
    s << L"Write-Host ('yt-dlp primary executable: ' + $YtDlp) -ForegroundColor DarkGray\r\n";
    s << L"try { $ytVersion = (& $YtDlp --version 2>$null | Select-Object -First 1); if ($ytVersion) { Write-Host ('yt-dlp primary version: ' + $ytVersion) -ForegroundColor DarkGray } } catch {}\r\n";
    s << L"if (Test-Path -LiteralPath $AuthYtDlp) { Write-Host ('Authenticated YouTube executable: ' + $AuthYtDlp) -ForegroundColor DarkGray; try { $authVersion = (& $AuthYtDlp --version 2>$null | Select-Object -First 1); if ($authVersion) { Write-Host ('Authenticated YouTube version: ' + $authVersion) -ForegroundColor DarkGray } } catch {} } else { Write-Host 'Authenticated YouTube backend is missing.' -ForegroundColor DarkYellow }\r\n";
    s << L"if ($Ffmpeg) { Write-Host ('FFmpeg executable: ' + $Ffmpeg) -ForegroundColor DarkGray }\r\n";
    s << L"Write-Host ''\r\n";
    s << L"function Get-WinterStaticSelectedHeight([string[]]$ExtraArgs) {\r\n";
    s << L"    if (-not $script:IsYouTubeUrl -or $AudioOnlyMode) { return $null }\r\n";
    s << L"    $probeArgs = @('--no-playlist', '--skip-download', '--no-warnings', '--dump-single-json') + $FormatArgs + $ExtraArgs\r\n";
    s << L"    if ($Ffmpeg) { $probeArgs += @('--ffmpeg-location', $Ffmpeg) }\r\n";
    s << L"    $probeArgs += @($Url)\r\n";
    s << L"    try {\r\n";
    s << L"        $raw = @(& $script:CurrentYtDlp @probeArgs 2>$null)\r\n";
    s << L"        $probeCode = $LASTEXITCODE\r\n";
    s << L"        if ($probeCode -ne 0 -or $raw.Count -eq 0) { return $null }\r\n";
    s << L"        $info = (($raw -join [Environment]::NewLine) | ConvertFrom-Json)\r\n";
    s << L"        $heights = @()\r\n";
    s << L"        if ($null -ne $info.requested_formats) {\r\n";
    s << L"            foreach ($f in @($info.requested_formats)) { if ($null -ne $f.height) { try { $heights += [int]$f.height } catch {} } }\r\n";
    s << L"        }\r\n";
    s << L"        if ($heights.Count -eq 0 -and $null -ne $info.height) { try { $heights += [int]$info.height } catch {} }\r\n";
    s << L"        if ($heights.Count -eq 0) { return $null }\r\n";
    s << L"        return [int](($heights | Measure-Object -Maximum).Maximum)\r\n";
    s << L"    } catch { return $null }\r\n";
    s << L"}\r\n";
    s << L"function Get-WinterStaticAuthenticatedYouTubeArgs() {\r\n";
    s << L"    $script:AuthMwebFallbackUsed = $false\r\n";
    s << L"    $normalArgs = @($AccountArgs)\r\n";
    s << L"    if (-not $script:IsYouTubeUrl -or $AudioOnlyMode) { return $normalArgs }\r\n";
    s << L"    $normalHeight = Get-WinterStaticSelectedHeight $normalArgs\r\n";
    s << L"    if ($null -eq $normalHeight) {\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_AUTH_PREFLIGHT_UNKNOWN'\r\n";
    s << L"        return $normalArgs\r\n";
    s << L"    }\r\n";
    s << L"    Write-Host ('Authenticated YouTube preflight: default client selects {0}p.' -f $normalHeight) -ForegroundColor DarkGray\r\n";
    s << L"    Write-WinterStaticLog ('WINTERSTATIC_AUTH_PREFLIGHT_DEFAULT_HEIGHT:' + $normalHeight)\r\n";
    s << L"    if ($normalHeight -gt 360) { return $normalArgs }\r\n";
    s << L"    Write-Host 'Only 360p (or lower) was exposed by the default authenticated YouTube client. Testing the mweb fallback...' -ForegroundColor Yellow\r\n";
    s << L"    Write-WinterStaticLog 'WINTERSTATIC_AUTH_MWEB_PROBE'\r\n";
    s << L"    $mwebArgs = @($AccountArgs + $MwebClientArgs)\r\n";
    s << L"    $mwebHeight = Get-WinterStaticSelectedHeight $mwebArgs\r\n";
    s << L"    if ($null -ne $mwebHeight) {\r\n";
    s << L"        Write-Host ('Authenticated YouTube preflight: mweb client selects {0}p.' -f $mwebHeight) -ForegroundColor DarkGray\r\n";
    s << L"        Write-WinterStaticLog ('WINTERSTATIC_AUTH_PREFLIGHT_MWEB_HEIGHT:' + $mwebHeight)\r\n";
    s << L"    }\r\n";
    s << L"    if ($null -ne $mwebHeight -and $mwebHeight -gt $normalHeight) {\r\n";
    s << L"        Write-Host 'mweb exposes a better authenticated format. Using it for this download.' -ForegroundColor Green\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_AUTH_MWEB_FALLBACK_USED'\r\n";
    s << L"        $script:AuthMwebFallbackUsed = $true\r\n";
    s << L"        return $mwebArgs\r\n";
    s << L"    }\r\n";
    s << L"    Write-Host 'mweb did not expose a better format. Keeping yt-dlp normal authenticated client selection.' -ForegroundColor DarkYellow\r\n";
    s << L"    Write-WinterStaticLog 'WINTERSTATIC_AUTH_MWEB_FALLBACK_NOT_BETTER'\r\n";
    s << L"    return $normalArgs\r\n";
    s << L"}\r\n";
    s << L"function Reset-WinterStaticRecoverySignals {\r\n";
    s << L"    $script:RecoverySawImpersonationSignal = $false\r\n";
    s << L"    $script:RecoverySawTransportSignal = $false\r\n";
    s << L"    $script:RecoverySawUnsupportedUrl = $false\r\n";
    s << L"    $script:RecoverySawHttp404 = $false\r\n";
    s << L"    $script:RecoverySawDownloadProgress = $false\r\n";
    s << L"    $script:RecoverySawFormatUnavailable = $false\r\n";
    s << L"    $script:RecoverySawAuthenticationRequired = $false\r\n";
    s << L"}\r\n";
    s << L"function Invoke-WinterStaticAttempt([string]$Label, [string[]]$ExtraArgs, [string]$TargetUrl = '') {\r\n";
    s << L"    Reset-WinterStaticRecoverySignals\r\n";
    s << L"    if ([string]::IsNullOrWhiteSpace($TargetUrl)) { $TargetUrl = $Url }\r\n";
    s << L"    Write-Host ('=== {0} ===' -f $Label) -ForegroundColor Cyan\r\n";
    s << L"    Write-WinterStaticLog ('=== {0} ===' -f $Label)\r\n";
    s << L"    $RunArgs = @($BaseArgs + $ExtraArgs + @($TargetUrl))\r\n";
    s << L"    & $script:CurrentYtDlp @RunArgs 2>&1 | ForEach-Object {\r\n";
    s << L"        $line = $_.ToString()\r\n";
    s << L"        $probe = $line.ToLowerInvariant()\r\n";
    s << L"        if ($probe -match 'http error 403|forbidden|http error 429|too many requests|cloudflare|captcha|challenge|bot protection|bot detection') {\r\n";
    s << L"            $script:RecoverySawImpersonationSignal = $true\r\n";
    s << L"        }\r\n";
    s << L"        if ($probe -match 'throttl|incompleteread|connection reset|remote end closed|timed out|timeout|broken pipe|connection aborted|http error 502|http error 503|http error 504') {\r\n";
    s << L"            $script:RecoverySawTransportSignal = $true\r\n";
    s << L"        }\r\n";
    s << L"        if ($probe -match 'unsupported url') {\r\n";
    s << L"            $script:RecoverySawUnsupportedUrl = $true\r\n";
    s << L"        }\r\n";
    s << L"        if ($probe -match 'http error 404|\\b404\\s+(?:not found|error)\\b|\\b404:\\s*not found\\b') {\r\n";
    s << L"            $script:RecoverySawHttp404 = $true\r\n";
    s << L"        }\r\n";
    s << L"        if ($probe -match 'requested format is not available|no video formats found') {\r\n";
    s << L"            $script:RecoverySawFormatUnavailable = $true\r\n";
    s << L"        }\r\n";
    s << L"        # Automatic mode must not treat every YouTube hiccup as a reason to use account cookies.\r\n";
    s << L"        # Escalate only on strong yt-dlp wording that explicitly asks for login/cookies or describes gated content.\r\n";
    s << L"        if ($script:IsYouTubeUrl -and $probe -match 'sign in to confirm|sign in to view|login required|log in to|authentication required|use --cookies-from-browser|use --cookies|this video is private|private video|join this channel to get access|members[- ]only|available to this channel.?s members|only available to registered users') {\r\n";
    s << L"            $script:RecoverySawAuthenticationRequired = $true\r\n";
    s << L"        }\r\n";
    s << L"        if ($line.StartsWith('WINTERSTATIC_PROGRESS:')) {\r\n";
    s << L"            $script:RecoverySawDownloadProgress = $true\r\n";
    s << L"            $bits = $line.Substring('WINTERSTATIC_PROGRESS:'.Length).Split('|')\r\n";
    s << L"            $pct = if ($bits.Count -gt 7 -and $bits[7]) { $bits[7] } else { '?' }\r\n";
    s << L"            $spd = if ($bits.Count -gt 8 -and $bits[8] -and $bits[8] -ne 'NA') { $bits[8] } else { '' }\r\n";
    s << L"            if ($spd) { Write-Host ('[download] {0} at {1}' -f $pct, $spd) } else { Write-Host ('[download] {0}' -f $pct) }\r\n";
    s << L"        } elseif ($line.StartsWith('WINTERSTATIC_POSTPROCESS:')) {\r\n";
    s << L"            # Structured post-processing lines are for the GUI only.\r\n";
    s << L"        } else { Write-Host $line }\r\n";
    s << L"        Write-WinterStaticLog $line\r\n";
    s << L"    }\r\n";
    s << L"    return [int]$LASTEXITCODE\r\n}\r\n";
    s << L"function Invoke-WinterStaticRecovery([int]$Code, [string[]]$ExtraArgs) {\r\n";
    s << L"    if ($Code -eq 0) { return 0 }\r\n";
    s << L"    if ($script:RecoverySawFormatUnavailable) {\r\n";
    s << L"        Write-Host 'Requested format was unavailable. Retrying with yt-dlp best-available format selection...' -ForegroundColor Yellow\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_FORMAT_RECOVERY'\r\n";
    s << L"        Start-Sleep -Milliseconds 350\r\n";
    s << L"        $Code = Invoke-WinterStaticAttempt 'Format recovery: best available' @($ExtraArgs + $FormatRecoveryArgs)\r\n";
    s << L"        if ($Code -eq 0) { return 0 }\r\n";
    s << L"    }\r\n";
    s << L"    $preDownload404 = $script:RecoverySawHttp404 -and -not $script:RecoverySawDownloadProgress\r\n";
    s << L"    if ($script:RecoverySawUnsupportedUrl -or $preDownload404) {\r\n";
    s << L"        if ($preDownload404 -and -not $script:RecoverySawUnsupportedUrl) {\r\n";
    s << L"            Write-Host 'HTTP 404 occurred before media download began. Trying page and runtime media recovery...' -ForegroundColor Yellow\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_DISCOVERY_START_PRE_DOWNLOAD_404'\r\n";
    s << L"        } else {\r\n";
    s << L"            Write-Host 'Unsupported URL detected. Looking for a direct media stream in the page...' -ForegroundColor Yellow\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_DISCOVERY_START'\r\n";
    s << L"        }\r\n";
    s << L"        $script:DiscoveryDirectFetchFailed = $false\r\n";
    s << L"        $candidate = Find-WinterStaticMediaCandidate $Url 0\r\n";
    s << L"        if ($null -eq $candidate -or -not $candidate.Url) {\r\n";
    s << L"            if ($script:DiscoveryDirectFetchFailed) {\r\n";
    s << L"                Write-Host 'Direct recovery page fetch was blocked or failed. Reusing yt-dlp to capture the page source...' -ForegroundColor Yellow\r\n";
    s << L"            } else {\r\n";
    s << L"                Write-Host 'Static scan found no usable media URL. Reusing yt-dlp to capture the page source...' -ForegroundColor Yellow\r\n";
    s << L"            }\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_DISCOVERY_YTDLP_PAGE_START'\r\n";
    s << L"            $capturedHtml = Get-WinterStaticPageHtmlViaYtDlp $Url $ExtraArgs\r\n";
    s << L"            if (-not [string]::IsNullOrWhiteSpace($capturedHtml)) {\r\n";
    s << L"                $candidate = Find-WinterStaticMediaCandidate $Url 0 $capturedHtml\r\n";
    s << L"            }\r\n";
    s << L"        }\r\n";
    s << L"        if ($null -eq $candidate -or -not $candidate.Url) {\r\n";
    s << L"            if ($BrowserAssistedRecovery) {\r\n";
    s << L"                Write-WinterStaticLog 'WINTERSTATIC_RUNTIME_FALLBACK_START'\r\n";
    s << L"                $candidate = Invoke-WinterStaticRuntimeDiscovery $Url\r\n";
    s << L"            } else {\r\n";
    s << L"                Write-Host 'Browser-assisted recovery is disabled. Enable Browser sweep in the main window to try runtime network detection.' -ForegroundColor DarkYellow\r\n";
    s << L"                Write-WinterStaticLog 'WINTERSTATIC_RUNTIME_FALLBACK_SKIPPED_DISABLED'\r\n";
    s << L"            }\r\n";
    s << L"        }\r\n";
    s << L"        if ($null -ne $candidate -and $candidate.Url) {\r\n";
    s << L"            Write-Host ('Media discovery found {0}. Retrying that resource through yt-dlp...' -f $candidate.Kind) -ForegroundColor Yellow\r\n";
    s << L"            Write-WinterStaticLog ('WINTERSTATIC_DISCOVERY_FOUND:' + $candidate.Kind)\r\n";
    s << L"            $candidateReferer = if ($candidate.PSObject.Properties['Referer'] -and $candidate.Referer) { [string]$candidate.Referer } else { $Url }\r\n";
    s << L"            $candidateUserAgent = if ($candidate.PSObject.Properties['UserAgent'] -and $candidate.UserAgent) { [string]$candidate.UserAgent } else { $script:DiscoveryUserAgent }\r\n";
    s << L"            # Recovered player/manifest URLs can occasionally be exposed by yt-dlp as multiple entries or streams.\r\n";
    s << L"            # This recovery-only guard preserves the normal quality selector while ensuring one requested media item.\r\n";
    s << L"            $discoveryArgs = @($ExtraArgs + @('--referer', $candidateReferer, '--user-agent', $candidateUserAgent, '--playlist-items', '1', '--no-video-multistreams', '--no-audio-multistreams'))\r\n";
    s << L"            Write-Host 'Single-output recovery guard enabled.' -ForegroundColor DarkGray\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_DISCOVERY_SINGLE_OUTPUT_GUARD'\r\n";
    s << L"            if ($candidate.PSObject.Properties['Origin'] -and $candidate.Origin) { $discoveryArgs += @('--add-header', ('Origin:' + [string]$candidate.Origin)) }\r\n";
    s << L"            Start-Sleep -Milliseconds 500\r\n";
    s << L"            $Code = Invoke-WinterStaticAttempt ('Discovered media retry: ' + $candidate.Kind) $discoveryArgs $candidate.Url\r\n";
    s << L"            if ($Code -eq 0) { return 0 }\r\n";
    s << L"        } else {\r\n";
    s << L"            if ($BrowserAssistedRecovery) {\r\n";
    s << L"                Write-Host 'No usable static or runtime media candidate was found.' -ForegroundColor DarkYellow\r\n";
    s << L"            } else {\r\n";
    s << L"                Write-Host 'No usable static media candidate was found; browser sweep was not enabled.' -ForegroundColor DarkYellow\r\n";
    s << L"            }\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_DISCOVERY_NONE'\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"    $impArgs = @()\r\n";
    s << L"    if ($script:RecoverySawImpersonationSignal) {\r\n";
    s << L"        $impArgs = @(Get-WinterStaticImpersonationArgs)\r\n";
    s << L"        if ($impArgs.Count -gt 0) {\r\n";
    s << L"            $target = $impArgs[1]\r\n";
    s << L"            Write-Host ('HTTP rejection detected. Retrying with browser impersonation ({0})...' -f $target) -ForegroundColor Yellow\r\n";
    s << L"            Write-WinterStaticLog ('WINTERSTATIC_RESILIENCE_IMPERSONATE:' + $target)\r\n";
    s << L"            Start-Sleep -Milliseconds 900\r\n";
    s << L"            $Code = Invoke-WinterStaticAttempt 'Resilience retry: browser impersonation' @($ExtraArgs + $impArgs)\r\n";
    s << L"            if ($Code -eq 0) { return 0 }\r\n";
    s << L"        } else {\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_RESILIENCE_IMPERSONATE_UNAVAILABLE'\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"    if ($script:RecoverySawTransportSignal) {\r\n";
    s << L"        $chunkArgs = @($ExtraArgs)\r\n";
    s << L"        if ($impArgs.Count -gt 0) { $chunkArgs += $impArgs }\r\n";
    s << L"        $chunkArgs += @('--http-chunk-size', '5M')\r\n";
    s << L"        Write-Host 'Transport/throttling trouble detected. Retrying with conservative HTTP chunks...' -ForegroundColor Yellow\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_RESILIENCE_CHUNK'\r\n";
    s << L"        Start-Sleep -Milliseconds 1200\r\n";
    s << L"        $Code = Invoke-WinterStaticAttempt 'Resilience retry: chunked HTTP' $chunkArgs\r\n";
    s << L"    }\r\n";
    s << L"    return [int]$Code\r\n}\r\n";
    s << L"function Invoke-WinterStaticAuthenticated([string]$Label) {\r\n";
    s << L"    $previousYtDlp = $script:CurrentYtDlp\r\n";
    s << L"    $usingAuthBackend = $false\r\n";
    s << L"    if ($script:IsYouTubeUrl) {\r\n";
    s << L"        if (-not (Test-Path -LiteralPath $AuthYtDlp)) {\r\n";
    s << L"            Write-Host 'Authenticated YouTube needs the dedicated WinterStatic auth backend, but yt-dlp-auth.exe is missing.' -ForegroundColor Red\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_AUTH_BACKEND_MISSING'\r\n";
    s << L"            return 1\r\n";
    s << L"        }\r\n";
    s << L"        $script:CurrentYtDlp = $AuthYtDlp\r\n";
    s << L"        $usingAuthBackend = $true\r\n";
    s << L"        $script:ImpersonationChecked = $false\r\n";
    s << L"        $script:ImpersonationArgs = @()\r\n";
    s << L"        Write-Host 'Authenticated YouTube: switching to the dedicated auth-compatible stable yt-dlp backend.' -ForegroundColor DarkGray\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_AUTH_YTDLP_BACKEND'\r\n";
    s << L"    }\r\n";
    s << L"    try {\r\n";
    s << L"        Start-Sleep -Milliseconds 500\r\n";
    s << L"        $authArgs = @(Get-WinterStaticAuthenticatedYouTubeArgs)\r\n";
    s << L"        $code = Invoke-WinterStaticAttempt $Label $authArgs\r\n";
    s << L"        if ($code -ne 0 -and $script:AuthMwebFallbackUsed) {\r\n";
    s << L"            Write-Host 'mweb fallback failed during the real download. Returning to yt-dlp normal authenticated client selection...' -ForegroundColor Yellow\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_AUTH_MWEB_DOWNLOAD_FAILED_NORMAL_RETRY'\r\n";
    s << L"            $script:AuthMwebFallbackUsed = $false\r\n";
    s << L"            $authArgs = @($AccountArgs)\r\n";
    s << L"            Start-Sleep -Milliseconds 650\r\n";
    s << L"            $code = Invoke-WinterStaticAttempt 'Authenticated retry after mweb fallback' $authArgs\r\n";
    s << L"        }\r\n";
    s << L"        if ($code -ne 0 -and -not $script:RecoverySawUnsupportedUrl -and -not $script:RecoverySawHttp404 -and -not $script:RecoverySawFormatUnavailable) {\r\n";
    s << L"            Write-Host 'Authenticated attempt failed. Trying the saved dedicated Edge session once more...' -ForegroundColor Yellow\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_AUTH_SECOND_RETRY'\r\n";
    s << L"            Start-Sleep -Milliseconds 1200\r\n";
    s << L"            $code = Invoke-WinterStaticAttempt 'Authenticated retry 2 of 2' $authArgs\r\n";
    s << L"        }\r\n";
    s << L"        $code = Invoke-WinterStaticRecovery $code $authArgs\r\n";
    s << L"        return [int]$code\r\n";
    s << L"    } finally {\r\n";
    s << L"        if ($usingAuthBackend) {\r\n";
    s << L"            $script:CurrentYtDlp = $previousYtDlp\r\n";
    s << L"            $script:ImpersonationChecked = $false\r\n";
    s << L"            $script:ImpersonationArgs = @()\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"}\r\n";
    s << L"$ExitCode = 1\r\n";
    s << L"try {\r\n";
    s << L"    if ($Mode -eq 'Use account browser') {\r\n";
    s << L"        $ExitCode = Invoke-WinterStaticAuthenticated 'Authenticated download using dedicated Edge profile'\r\n";
    s << L"    } elseif ($Mode -eq 'Anonymous') {\r\n";
    s << L"        $ExitCode = Invoke-WinterStaticAttempt 'Anonymous download' @()\r\n";
    s << L"        $ExitCode = Invoke-WinterStaticRecovery $ExitCode @()\r\n";
    s << L"    } else {\r\n";
    s << L"        Write-WinterStaticLog 'WINTERSTATIC_AUTO_ANON_FIRST'\r\n";
    s << L"        $ExitCode = Invoke-WinterStaticAttempt 'Automatic mode: anonymous first attempt' @()\r\n";
    s << L"        if ($ExitCode -ne 0 -and $script:RecoverySawAuthenticationRequired -and $AutoUseAccount) {\r\n";
    s << L"            Write-Host 'YouTube explicitly requested authentication. Retrying with the saved dedicated Edge session...' -ForegroundColor Yellow\r\n";
    s << L"            Write-WinterStaticLog 'WINTERSTATIC_AUTO_AUTH_REQUIRED_FALLBACK'\r\n";
    s << L"            $ExitCode = Invoke-WinterStaticAuthenticated 'Automatic mode: authentication-required fallback'\r\n";
    s << L"        } else {\r\n";
    s << L"            if ($ExitCode -ne 0 -and $script:RecoverySawAuthenticationRequired -and -not $AutoUseAccount) {\r\n";
    s << L"                Write-Host 'YouTube requested authentication, but no confirmed saved account session is available.' -ForegroundColor DarkYellow\r\n";
    s << L"                Write-WinterStaticLog 'WINTERSTATIC_AUTO_AUTH_REQUIRED_NO_SESSION'\r\n";
    s << L"            }\r\n";
    s << L"            $ExitCode = Invoke-WinterStaticRecovery $ExitCode @()\r\n";
    s << L"        }\r\n";
    s << L"    }\r\n";
    s << L"} catch {\r\n";
    s << L"    Write-Host ($_ | Out-String -Width 240) -ForegroundColor Red\r\n    $ExitCode = 1\r\n";
    s << L"} finally {\r\n";
    s << L"    try { if ($null -ne $LogWriter) { $LogWriter.Flush(); $LogWriter.Dispose(); $LogWriter = $null } } catch {}\r\n";
    s << L"    try { if ($null -ne $LogStream) { $LogStream.Dispose(); $LogStream = $null } } catch {}\r\n";
    s << L"    try { Set-Content -Path $DonePath -Value ([int]$ExitCode) -Encoding ASCII -Force } catch {}\r\n";
    s << L"}\r\n";
    // The completion marker is written before this point, so the GUI can report
    // success even if the PowerShell host exits immediately afterward.  Failed
    // tasks deliberately stay open so yt-dlp's diagnostics remain visible.
    // -NoExit intentionally keeps PowerShell interactive for the normal workflow.
    // A script-level `exit` is not sufficient to close that host, so successful
    // auto-close uses Environment.Exit to terminate the actual PowerShell process.
    // The done marker and log streams have already been finalized above.
    s << L"if ($CloseOnSuccess -and $ExitCode -eq 0) { [System.Environment]::Exit(0) }\r\n";
    s << L"try { Set-Location $env:USERPROFILE } catch {}\r\n";
    s << L"Write-Host ''\r\nWrite-Host ('Task finished. Exit code: {0}' -f $ExitCode) -ForegroundColor Cyan\r\n";
    s << L"if ($CloseOnSuccess -and $ExitCode -ne 0) {\r\n";
    s << L"    Write-Host 'Close on success is enabled, but the task failed, so this window will stay open.' -ForegroundColor Yellow\r\n";
    s << L"} else {\r\n";
    s << L"    Write-Host 'This PowerShell window is now yours. You can close it normally.' -ForegroundColor Green\r\n";
    s << L"}\r\n";
    return s.str();
}

struct VisibleWindowInfo {
    HWND hwnd{};
    DWORD pid{};
    std::wstring title;
    std::wstring className;
};

BOOL CALLBACK EnumVisibleWindowsProc(HWND hwnd, LPARAM lp) {
    auto* out = reinterpret_cast<std::vector<VisibleWindowInfo>*>(lp);
    if (!out || !IsWindowVisible(hwnd)) return TRUE;

    wchar_t title[1024]{};
    GetWindowTextW(hwnd, title, static_cast<int>(std::size(title)));
    wchar_t cls[256]{};
    GetClassNameW(hwnd, cls, static_cast<int>(std::size(cls)));
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);

    out->push_back({hwnd, pid, title, cls});
    return TRUE;
}

std::vector<VisibleWindowInfo> VisibleWindowsSnapshot() {
    std::vector<VisibleWindowInfo> windows;
    EnumWindows(EnumVisibleWindowsProc, reinterpret_cast<LPARAM>(&windows));
    return windows;
}

std::unordered_set<HWND> VisibleWindowHandles() {
    std::unordered_set<HWND> handles;
    for (const auto& w : VisibleWindowsSnapshot()) handles.insert(w.hwnd);
    return handles;
}

std::wstring LowerCopy(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
    return value;
}

bool TitleContains(const std::wstring& title, const std::wstring& needle) {
    if (needle.empty()) return false;
    return LowerCopy(title).find(LowerCopy(needle)) != std::wstring::npos;
}

bool LooksLikeConsoleOrTerminal(const VisibleWindowInfo& w) {
    const std::wstring cls = LowerCopy(w.className);
    const std::wstring title = LowerCopy(w.title);
    return cls == L"consolewindowclass"
        || cls == L"cascadia_hosting_window_class"
        || title.find(L"powershell") != std::wstring::npos
        || title.find(L"windows terminal") != std::wstring::npos
        || title.find(L"winterstatic yt-dlp downloader") != std::wstring::npos;
}

bool RectNearlyEqual(const RECT& a, const RECT& b, int tolerance = 2) {
    return std::abs(static_cast<int>(a.left - b.left)) <= tolerance
        && std::abs(static_cast<int>(a.top - b.top)) <= tolerance
        && std::abs(static_cast<int>(a.right - b.right)) <= tolerance
        && std::abs(static_cast<int>(a.bottom - b.bottom)) <= tolerance;
}

bool WaitForStableTopLevelWindow(HWND hwnd, DWORD stableMs = 700, DWORD maxWaitMs = 3500) {
    if (!hwnd || !IsWindow(hwnd)) return false;

    const ULONGLONG end = GetTickCount64() + maxWaitMs;
    ULONGLONG stableSince = 0;
    RECT previous{};
    bool havePrevious = false;

    while (GetTickCount64() < end) {
        if (!IsWindow(hwnd) || !IsWindowVisible(hwnd)) return false;

        // A just-created Windows Terminal host can briefly expose a top-level
        // HWND while its non-client/title-bar layout is still being rebuilt.
        // Do not resize/move it during that phase.  Wait until its rectangle has
        // stopped changing for a continuous interval first.
        if (IsIconic(hwnd)) {
            stableSince = 0;
            havePrevious = false;
            Sleep(100);
            continue;
        }

        RECT current{};
        if (!GetWindowRect(hwnd, &current)) return false;
        const int width = static_cast<int>(current.right - current.left);
        const int height = static_cast<int>(current.bottom - current.top);
        if (width < 320 || height < 180) {
            stableSince = 0;
            previous = current;
            havePrevious = true;
            Sleep(100);
            continue;
        }

        if (havePrevious && RectNearlyEqual(previous, current)) {
            if (!stableSince) stableSince = GetTickCount64();
            if (GetTickCount64() - stableSince >= stableMs) return true;
        } else {
            stableSince = 0;
        }

        previous = current;
        havePrevious = true;
        Sleep(100);
    }
    return false;
}

bool MoveWindowToRect(HWND hwnd, const RectI& r) {
    if (!hwnd || !IsWindow(hwnd)) return false;
    if (IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);

    // Windows Terminal can replace/re-layout its visible host shortly after
    // powershell.exe starts.  Moving during that startup race can leave the
    // title bar's drawn controls and hit-test regions out of sync.  Make one
    // placement only after the HWND has settled; if it never settles, skip the
    // automatic snap rather than risk leaving a malformed terminal window.
    if (!WaitForStableTopLevelWindow(hwnd)) return false;

    return SetWindowPos(hwnd, nullptr, r.x, r.y, r.w, r.h,
                        SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW) != FALSE;
}

void CloseWindowHandle(HWND hwnd) {
    if (hwnd && IsWindow(hwnd)) PostMessageW(hwnd, WM_CLOSE, 0, 0);
}

void CloseTrackedWindows(DWORD pid, const std::wstring& title) {
    for (const auto& w : VisibleWindowsSnapshot()) {
        if ((pid && w.pid == pid) || (!title.empty() && TitleContains(w.title, title))) {
            CloseWindowHandle(w.hwnd);
        }
    }
}

RectI GetRightPaneRect() {
    RECT work{};
    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    HMONITOR monitor = MonitorFromWindow(g_main, MONITOR_DEFAULTTONEAREST);
    if (monitor && GetMonitorInfoW(monitor, &mi)) {
        work = mi.rcWork;
    } else {
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    }
    RECT mainRect{};
    GetWindowRect(g_main, &mainRect);

    const int gap = 8;
    int x = std::clamp(static_cast<int>(mainRect.right) + gap, static_cast<int>(work.left), static_cast<int>(work.right));
    int y = std::max(static_cast<int>(work.top), static_cast<int>(mainRect.top));
    int w = static_cast<int>(work.right) - x;
    int h = std::min(static_cast<int>(mainRect.bottom), static_cast<int>(work.bottom)) - y;

    if (w < 520) {
        // Fall back to the right side of the whole work area. This also handles
        // a manually moved/maximized GUI without trying to create a 30px console.
        const int workW = static_cast<int>(work.right - work.left);
        w = std::max(520, std::min(860, workW / 2));
        x = static_cast<int>(work.right) - w;
        y = static_cast<int>(work.top);
        h = static_cast<int>(work.bottom - work.top);
    }
    return {x, y, w, std::max(420, h)};
}

HWND MoveConsoleRight(DWORD pid, const std::wstring& title, DWORD timeoutMs = 12000,
                      const std::unordered_set<HWND>& existingWindows = {},
                      HWND foregroundBefore = nullptr) {
    const RectI r = GetRightPaneRect();
    const ULONGLONG end = GetTickCount64() + timeoutMs;

    while (GetTickCount64() < end) {
        const auto windows = VisibleWindowsSnapshot();
        const HWND foregroundNow = GetForegroundWindow();
        HWND best = nullptr;
        int bestScore = -1;

        for (const auto& w : windows) {
            if (!w.hwnd || w.hwnd == g_main) continue;

            const bool titleMatch = !title.empty() && TitleContains(w.title, title);
            const bool pidMatch = pid && w.pid == pid;
            const bool terminalLike = LooksLikeConsoleOrTerminal(w);
            const bool isNew = !existingWindows.empty() && existingWindows.find(w.hwnd) == existingWindows.end();
            const bool newlyForeground = foregroundBefore && foregroundNow && foregroundNow != foregroundBefore && w.hwnd == foregroundNow;

            // Require either a hard identity match or a window that really looks
            // like a console/Windows Terminal host.  This keeps the foreground
            // fallback from ever grabbing an unrelated application window.
            if (!titleMatch && !pidMatch && !terminalLike) continue;

            int score = 0;
            if (titleMatch) score += 1200;
            if (pidMatch) score += 1000;
            if (isNew) score += 650;
            if (newlyForeground) score += 600;
            if (terminalLike) score += 220;
            if (w.hwnd == foregroundNow) score += 80;

            if (score > bestScore) {
                bestScore = score;
                best = w.hwnd;
            }
        }

        // Hard PID/title matches qualify immediately.  A new or newly-foreground
        // terminal also qualifies, which covers Windows Terminal reusing a host
        // whose top-level PID/title is not the PowerShell child's.
        if (best && bestScore >= 800 && MoveWindowToRect(best, r)) return best;

        Sleep(120);
    }
    return nullptr;
}

void PostProgress(const std::shared_ptr<DownloadJob>& job, int percent,
                  const std::wstring& stage, const std::wstring& detail) {
    if (!g_main || !job) return;
    auto* p = new ProgressUpdate{job->id, percent, stage, detail};
    if (!PostMessageW(g_main, WM_APP_PROGRESS, 0, reinterpret_cast<LPARAM>(p))) delete p;
}

void ResetDownloadProgressState(const std::shared_ptr<DownloadJob>& job) {
    if (!job) return;
    std::lock_guard<std::mutex> lock(job->progressMutex);
    job->progressPassNumber = 0;
    job->currentDownloadKey.clear();
    job->postProcessPassActive = false;
}

unsigned long long ParseUnsignedA(const std::string& text) {
    const std::string value = TrimA(text);
    if (value.empty() || value == "NA" || value == "None" || value == "null") return 0;
    try {
        size_t used = 0;
        const double number = std::stod(value, &used);
        if (used == 0 || number <= 0.0) return 0;
        return static_cast<unsigned long long>(number + 0.5);
    } catch (...) {
        return 0;
    }
}

double ParseDoubleA(const std::string& text) {
    const std::string value = TrimA(text);
    if (value.empty() || value == "NA" || value == "None" || value == "null") return 0.0;
    try { return std::stod(value); } catch (...) { return 0.0; }
}

std::vector<std::string> SplitPipeFields(const std::string& text) {
    std::vector<std::string> fields;
    size_t start = 0;
    while (true) {
        const size_t bar = text.find('|', start);
        if (bar == std::string::npos) {
            fields.push_back(text.substr(start));
            break;
        }
        fields.push_back(text.substr(start, bar - start));
        start = bar + 1;
    }
    return fields;
}

bool CodecPresent(const std::string& codec) {
    std::string value = TrimA(codec);
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return !value.empty() && value != "none" && value != "na" && value != "null";
}

int EnsureDownloadPass(const std::shared_ptr<DownloadJob>& job, const std::string& key) {
    if (!job) return 0;
    std::lock_guard<std::mutex> lock(job->progressMutex);
    // Once post-processing starts, buffered download progress must not move this
    // job backwards into another download pass.
    if (job->postProcessPassActive) return 0;
    if (job->progressPassNumber == 0 || key != job->currentDownloadKey) {
        ++job->progressPassNumber;
        job->currentDownloadKey = key;
    }
    return job->progressPassNumber;
}

int EnsurePostProcessPass(const std::shared_ptr<DownloadJob>& job) {
    if (!job) return 0;
    std::lock_guard<std::mutex> lock(job->progressMutex);
    if (!job->postProcessPassActive) {
        job->postProcessPassActive = true;
        ++job->progressPassNumber;
    }
    return job->progressPassNumber;
}

std::wstring PassLabel(int pass, const std::wstring& activity) {
    return L"Pass " + std::to_wstring(pass) + L" — " + activity;
}

void ParseTaskLine(const std::string& raw, const std::shared_ptr<DownloadJob>& job) {
    if (!job) return;
    std::string line = TrimA(raw);
    if (line.empty()) return;

    if (line.rfind("=== ", 0) == 0) {
        ResetDownloadProgressState(job);
        return;
    }
    if (line.rfind("WINTERSTATIC_PROGRESS:", 0) == 0) {
        const auto fields = SplitPipeFields(line.substr(strlen("WINTERSTATIC_PROGRESS:")));
        if (fields.size() < 12) return;

        std::string key = TrimA(fields[11]); // format_id is the most stable stream identity
        if (key.empty() || key == "NA") key = TrimA(fields[0]);
        if (key.empty() || key == "NA") key = TrimA(fields[1]);
        if (key.empty() || key == "NA") key = "stream";

        const int pass = EnsureDownloadPass(job, key);
        if (pass <= 0) return;

        const unsigned long long downloaded = ParseUnsignedA(fields[2]);
        unsigned long long total = ParseUnsignedA(fields[3]);
        if (!total) total = ParseUnsignedA(fields[4]);
        const std::string status = TrimA(fields[6]);

        std::string pct = fields[7];
        pct.erase(std::remove(pct.begin(), pct.end(), '%'), pct.end());
        double percent = ParseDoubleA(pct);
        if (status == "finished") percent = 100.0;
        else if (percent <= 0.0 && total > 0) {
            percent = (static_cast<double>(downloaded) / static_cast<double>(total)) * 100.0;
        }
        // Follow yt-dlp's current estimate directly. Fragmented downloads can
        // legitimately revise that estimate in either direction, so the GUI no
        // longer keeps a monotonic high-water mark. yt-dlp can briefly report 100%
        // before a stream is actually finished; discard that one misleading update
        // and wait for either the next live percentage or the real finished event.
        if (status != "finished" && percent >= 100.0) return;
        if (status == "finished") percent = 100.0;
        else percent = std::max(0.0, std::min(99.9, percent));

        const bool hasVideo = CodecPresent(fields[9]);
        const bool hasAudio = CodecPresent(fields[10]);
        std::wstring activity = L"Downloading";
        if (hasVideo && !hasAudio) activity = L"Downloading video";
        else if (!hasVideo && hasAudio) activity = L"Downloading audio";

        std::wstringstream detail;
        detail << std::fixed << std::setprecision(1) << percent << L"%";
        const std::string speedText = TrimA(fields[8]);
        if (!speedText.empty() && speedText != "NA") detail << L"  •  " << Utf8ToWide(speedText);
        const int barPercent = status == "finished"
            ? 100
            : std::clamp(static_cast<int>(percent), 0, 99);
        PostProgress(job, barPercent, PassLabel(pass, activity), detail.str());
        return;
    }
    if (line.rfind("WINTERSTATIC_POSTPROCESS:", 0) == 0) {
        const auto fields = SplitPipeFields(line.substr(strlen("WINTERSTATIC_POSTPROCESS:")));
        const std::string postprocessor = fields.empty() ? "" : TrimA(fields[0]);
        const std::string status = fields.size() > 1 ? TrimA(fields[1]) : "";
        if (status != "finished") {
            const int pass = EnsurePostProcessPass(job);
            const bool audio = postprocessor.find("ExtractAudio") != std::string::npos
                            || postprocessor.find("FFmpegExtractAudio") != std::string::npos;
            const bool merger = postprocessor.find("Merger") != std::string::npos;
            const std::wstring activity = audio ? L"Processing audio"
                                        : merger ? L"Assembling"
                                                 : L"Processing";
            const std::wstring detail = audio ? L"Processing downloaded audio…"
                                      : merger ? L"Combining video and audio…"
                                               : L"Processing downloaded media…";
            PostProgress(job, 0, PassLabel(pass, activity), detail);
        }
        return;
    }
    if (line.rfind("WINTERSTATIC_FINAL:", 0) == 0) {
        PostJobLog(job, L"Saved: " + Utf8ToWide(line.substr(strlen("WINTERSTATIC_FINAL:"))));
        return;
    }
    if (line.rfind("[Merger]", 0) == 0) {
        const int pass = EnsurePostProcessPass(job);
        PostProgress(job, 0, PassLabel(pass, L"Assembling"), L"Combining video and audio…");
        return;
    }
    if (line.rfind("[ExtractAudio]", 0) == 0) {
        const int pass = EnsurePostProcessPass(job);
        PostProgress(job, 0, PassLabel(pass, L"Processing audio"), L"Processing downloaded audio…");
        return;
    }
    if (line == "WINTERSTATIC_AUTO_ANON_FIRST") {
        PostJobLog(job, L"Automatic mode: trying the normal anonymous path first.");
        return;
    }
    if (line == "WINTERSTATIC_AUTO_AUTH_REQUIRED_FALLBACK") {
        PostJobLog(job, L"Automatic mode: YouTube explicitly requested authentication; retrying with the saved dedicated Edge session.");
        return;
    }
    if (line == "WINTERSTATIC_AUTO_AUTH_REQUIRED_NO_SESSION") {
        PostJobLog(job, L"Automatic mode: YouTube requested authentication, but no confirmed saved Edge session is available.");
        return;
    }
    if (line == "WINTERSTATIC_AUTH_SECOND_RETRY") {
        PostJobLog(job, L"Authenticated attempt failed; retrying the saved dedicated Edge session once more.");
        return;
    }
    if (line.rfind("WINTERSTATIC_RESILIENCE_IMPERSONATE:", 0) == 0) {
        const std::wstring target = Utf8ToWide(line.substr(strlen("WINTERSTATIC_RESILIENCE_IMPERSONATE:")));
        PostJobLog(job, L"Resilience recovery: retrying with yt-dlp browser impersonation (" + target + L").");
        return;
    }
    if (line == "WINTERSTATIC_RESILIENCE_IMPERSONATE_UNAVAILABLE") {
        PostJobLog(job, L"Resilience recovery: browser impersonation was suggested, but this yt-dlp build exposes no compatible Chrome target.");
        return;
    }
    if (line == "WINTERSTATIC_RESILIENCE_CHUNK") {
        PostJobLog(job, L"Resilience recovery: retrying with conservative chunked HTTP transfer.");
        return;
    }
    if (line == "WINTERSTATIC_FORMAT_RECOVERY") {
        PostJobLog(job, L"Format recovery: retrying with yt-dlp best-available format selection.");
        return;
    }
    if (line == "WINTERSTATIC_DISCOVERY_START") {
        PostJobLog(job, L"Unsupported URL recovery: inspecting the webpage for a direct media stream.");
        return;
    }
    if (line.rfind("WINTERSTATIC_DISCOVERY_FOUND:", 0) == 0) {
        const std::wstring kind = Utf8ToWide(line.substr(strlen("WINTERSTATIC_DISCOVERY_FOUND:")));
        PostJobLog(job, L"Unsupported URL recovery: found " + kind + L"; retrying it through yt-dlp.");
        return;
    }
    if (line == "WINTERSTATIC_DISCOVERY_NONE") {
        PostJobLog(job, L"Unsupported URL recovery: no usable static media reference was found.");
        return;
    }
    if (line.find("ERROR:") != std::string::npos) PostJobLog(job, Utf8ToWide(line));
}

void MonitorTaskFiles(std::wstring logPath, std::wstring donePath,
                      const std::shared_ptr<DownloadJob>& job,
                      unsigned long long generation) {
    if (!job) return;

    std::uintmax_t offset = 0;
    std::string pending;

    auto consumeChunk = [&](std::string chunk) {
        if (offset == chunk.size() && chunk.size() >= 3 &&
            static_cast<unsigned char>(chunk[0]) == 0xEF &&
            static_cast<unsigned char>(chunk[1]) == 0xBB &&
            static_cast<unsigned char>(chunk[2]) == 0xBF) {
            chunk.erase(0, 3);
        }

        pending += chunk;
        size_t start = 0;
        while (true) {
            const size_t nl = pending.find('\n', start);
            if (nl == std::string::npos) break;
            std::string line = pending.substr(start, nl - start);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            ParseTaskLine(line, job);
            start = nl + 1;
        }
        if (start > 0) pending.erase(0, start);
    };

    while (generation == job->generation.load()) {
        std::error_code ec;
        const std::uintmax_t size = fs::is_regular_file(fs::path(logPath), ec)
            ? fs::file_size(fs::path(logPath), ec)
            : 0;

        if (!ec) {
            if (size < offset) {
                offset = 0;
                pending.clear();
            }
            if (size > offset) {
                std::ifstream f(fs::path(logPath), std::ios::binary);
                if (f) {
                    f.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
                    std::string chunk((std::istreambuf_iterator<char>(f)),
                                      std::istreambuf_iterator<char>());
                    offset += static_cast<std::uintmax_t>(chunk.size());
                    consumeChunk(std::move(chunk));
                }
            }
        }

        if (FileExists(donePath)) {
            if (!pending.empty()) {
                std::string line = pending;
                if (!line.empty() && line.back() == '\r') line.pop_back();
                ParseTaskLine(line, job);
                pending.clear();
            }

            std::ifstream done{fs::path(donePath)};
            int code = 1;
            if (done) done >> code;
            auto* update = new TaskDoneUpdate{job->id, code, generation};
            HWND main = g_main;
            if (!main || !PostMessageW(main, WM_APP_TASK_DONE, 0, reinterpret_cast<LPARAM>(update))) {
                delete update;
            }
            return;
        }
        Sleep(200);
    }
}

bool LaunchPowerShellTask(const std::wstring& scriptPath, const std::wstring& title, DWORD& pidOut) {
    std::wstring cmd = L"powershell.exe -NoProfile -NoExit -ExecutionPolicy Bypass -File " + WinArgQuote(scriptPath);
    std::vector<wchar_t> mutableCmd(cmd.begin(), cmd.end());
    mutableCmd.push_back(L'\0');
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    // Do not give Windows Terminal an early STARTF_USEPOSITION hint.  On some
    // Windows 11 Terminal startup paths that races its own title-bar/layout
    // initialization.  Let the host create normally, then place its stable HWND.
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;
    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessW(nullptr, mutableCmd.data(), nullptr, nullptr, FALSE,
                             CREATE_NEW_CONSOLE, nullptr, fs::path(scriptPath).parent_path().c_str(), &si, &pi);
    if (!ok) return false;
    pidOut = pi.dwProcessId;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

void StartDownload() {
    auto job = ActiveJob();
    if (!job) return;

    SaveActiveJobUi();
    if (job->running) {
        MessageBoxW(g_main, L"This tab already has a running task.", L"Download already running",
                    MB_OK | MB_ICONINFORMATION);
        return;
    }

    std::wstring ytdlp, output, ffmpeg;
    if (!ValidateInputs(ytdlp, output, ffmpeg)) return;

    if (GetText(IDC_AUTH) == L"Automatic") {
        const AccountStatus status = InspectAccountSession();
        g_accountSessionValid = status.valid;
        SetText(IDC_ACCOUNT_STATUS, status.text);
    }

    SaveSettings();
    SaveActiveJobUi();
    ResetDownloadProgressState(job);

    const std::wstring taskDir = TempTaskDirectory();
    const std::wstring logPath = JoinPath(taskDir, L"download.log");
    const std::wstring donePath = JoinPath(taskDir, L"done.txt");
    const std::wstring scriptPath = JoinPath(taskDir, L"task.ps1");
    const std::wstring title = UniqueConsoleTitle(job->id);
    const std::wstring script = BuildDownloadScript(ytdlp, output, ffmpeg, logPath, donePath, title);
    if (!WriteUtf8BomFile(scriptPath, script)) {
        RemoveTaskDirectoryBestEffort(fs::path(taskDir));
        MessageBoxW(g_main, L"Could not create the temporary PowerShell task script.", L"Download",
                    MB_OK | MB_ICONERROR);
        return;
    }

    const auto existingWindows = VisibleWindowHandles();
    const HWND foregroundBefore = GetForegroundWindow();
    DWORD pid = 0;
    if (!LaunchPowerShellTask(scriptPath, title, pid)) {
        RemoveTaskDirectoryBestEffort(fs::path(taskDir));
        MessageBoxW(g_main, L"Could not open PowerShell.", L"Download", MB_OK | MB_ICONERROR);
        return;
    }

    const unsigned long long generation = ++job->generation;
    {
        std::lock_guard<std::mutex> lock(job->consoleMutex);
        job->consolePid = pid;
        job->consoleHwnd = nullptr;
        job->consoleTitle = title;
        job->logPath = logPath;
        job->donePath = donePath;
        job->taskDir = taskDir;
    }

    job->running = true;
    job->finished = false;
    job->exitCode = 0;
    job->progressPercent = 0;
    job->stage = L"Opening PowerShell";
    job->detail.clear();

    RefreshActiveJobControls();
    UpdateTabLabel(job);
    AppendJobLogUi(job->id, L"Opening a live PowerShell window for yt-dlp...");

    std::thread([job, pid, title, existingWindows, foregroundBefore]() {
        HWND moved = MoveConsoleRight(pid, title, 12000, existingWindows, foregroundBefore);
        if (moved) {
            std::lock_guard<std::mutex> lock(job->consoleMutex);
            if (job->consolePid == pid && job->consoleTitle == title) job->consoleHwnd = moved;
        }
        PostJobLog(job, moved
            ? L"PowerShell snapped to the right pane."
            : L"PowerShell opened; automatic snap skipped because no stable tracked terminal window was available.");
    }).detach();

    std::thread(MonitorTaskFiles, logPath, donePath, job, generation).detach();
}

void TerminateJobTask(const std::shared_ptr<DownloadJob>& job) {
    if (!job) return;

    DWORD pid = 0;
    HWND hwnd = nullptr;
    std::wstring title;
    fs::path currentTaskDir;
    {
        std::lock_guard<std::mutex> lock(job->consoleMutex);
        pid = job->consolePid;
        hwnd = job->consoleHwnd;
        title = job->consoleTitle;
        if (!job->taskDir.empty()) currentTaskDir = fs::path(job->taskDir);
    }

    // Invalidate this tab's monitor before touching its process tree. Other tabs
    // keep their own generation counters and continue independently.
    ++job->generation;
    job->running = false;

    bool trackedProcessStillRunning = false;
    if (pid && ProcessIdMatchesImage(pid, L"powershell.exe")) {
        const std::wstring cmd = L"taskkill /PID " + std::to_wstring(pid) + L" /T /F";
        RunHiddenCapture(cmd, 8000);

        // Give taskkill a short bounded window to finish the process tree before
        // deleting the temporary task folder.
        const ULONGLONG waitUntil = GetTickCount64() + 1500;
        while (GetTickCount64() < waitUntil && ProcessIdMatchesImage(pid, L"powershell.exe")) {
            Sleep(50);
        }
        trackedProcessStillRunning = ProcessIdMatchesImage(pid, L"powershell.exe");
    }

    // Windows Terminal can host several PowerShell tabs in one top-level window.
    // Never close a shared host just because one download tab is being stopped.
    bool sharedTerminalWindow = false;
    if (hwnd && IsWindow(hwnd)) {
        for (const auto& other : g_jobs) {
            if (!other || other == job) continue;
            std::lock_guard<std::mutex> otherLock(other->consoleMutex);
            if (other->consoleHwnd == hwnd) {
                sharedTerminalWindow = true;
                break;
            }
        }
        if (!sharedTerminalWindow) CloseWindowHandle(hwnd);
    } else if (g_jobs.size() == 1) {
        CloseTrackedWindows(pid, title);
    }

    {
        std::lock_guard<std::mutex> lock(job->consoleMutex);
        job->consolePid = 0;
        job->consoleHwnd = nullptr;
        job->consoleTitle.clear();
        job->logPath.clear();
        job->donePath.clear();
        job->taskDir.clear();
    }

    if (!trackedProcessStillRunning && !currentTaskDir.empty()) {
        RemoveTaskDirectoryBestEffort(currentTaskDir);
    }
}

void ResetJobForReuse(const std::shared_ptr<DownloadJob>& job) {
    if (!job) return;
    ResetDownloadProgressState(job);
    job->url.clear();
    job->progressPercent = 0;
    job->stage = L"Ready";
    job->detail.clear();
    job->logText.clear();
    job->running = false;
    job->finished = false;
    job->exitCode = 0;
}

void StopManagedConsoles() {
    auto job = ActiveJob();
    if (!job) return;

    // Nuke is the clean-slate action for one download tab. Keep the user's output,
    // quality, format preference, authentication, and checkbox choices, but stop the task and clear
    // the URL, progress, status, and log so the tab is ready to reuse.
    SaveActiveJobUi();
    TerminateJobTask(job);
    ResetJobForReuse(job);
    UpdateTabLabel(job);
    LoadActiveJobUi();
}

void SnapCurrentConsole() {
    auto job = ActiveJob();
    if (!job) return;

    DWORD pid = 0;
    HWND hwnd = nullptr;
    std::wstring title;
    {
        std::lock_guard<std::mutex> lock(job->consoleMutex);
        pid = job->consolePid;
        hwnd = job->consoleHwnd;
        title = job->consoleTitle;
    }

    if (!pid && title.empty() && (!hwnd || !IsWindow(hwnd))) {
        AppendJobLogUi(job->id, L"No tracked PowerShell window is currently open for this tab.");
        return;
    }

    std::thread([job, pid, hwnd, title]() {
        HWND moved = nullptr;
        if (hwnd && IsWindow(hwnd) && MoveWindowToRect(hwnd, GetRightPaneRect())) {
            moved = hwnd;
        } else {
            HWND foregroundBefore = g_main;
            moved = MoveConsoleRight(pid, title, 5000, {}, foregroundBefore);

            // The final one-terminal fallback is only safe when there is exactly
            // one terminal-like top-level window. With multiple jobs this will
            // normally be skipped in favor of PID/title matching.
            if (!moved) {
                HWND onlyTerminal = nullptr;
                int terminalCount = 0;
                for (const auto& w : VisibleWindowsSnapshot()) {
                    if (w.hwnd == g_main || !LooksLikeConsoleOrTerminal(w)) continue;
                    onlyTerminal = w.hwnd;
                    ++terminalCount;
                    if (terminalCount > 1) break;
                }
                if (terminalCount == 1 && MoveWindowToRect(onlyTerminal, GetRightPaneRect())) moved = onlyTerminal;
            }
        }

        if (moved) {
            std::lock_guard<std::mutex> lock(job->consoleMutex);
            if (job->consolePid == pid && job->consoleTitle == title) job->consoleHwnd = moved;
        }

        PostJobLog(job, moved
            ? L"PowerShell snapped to the right pane."
            : L"Could not find a stable tracked PowerShell window to move safely.");
    }).detach();
}

void UpdateYtDlp() {
    const std::wstring path = Trim(GetText(IDC_YTDLP));
    if (!FileExists(path)) {
        MessageBoxW(g_main, L"yt-dlp.exe was not found.", L"yt-dlp missing", MB_OK | MB_ICONERROR);
        return;
    }

    // Keep both backends on yt-dlp's official stable channel.
    // The primary always advances to the latest stable. The authenticated
    // YouTube slot follows that same stable unless its version is explicitly
    // blacklisted for authenticated-quality regressions. Nightlies are never
    // selected by this updater. 2026.08.19 is currently the only auth blacklist
    // entry, with 2026.07.04 retained as the known-good fallback while needed.
    const std::wstring authPath = PortableAuthYtDlpPath();
    std::wstring script =
        L"$ErrorActionPreference='Stop'; "
        L"$ProgressPreference='SilentlyContinue'; "
        L"[Net.ServicePointManager]::SecurityProtocol=[Net.SecurityProtocolType]::Tls12; "
        L"$dest=" + PsQuote(path) + L"; "
        L"$authDest=" + PsQuote(authPath) + L"; "
        L"$stableUrl='https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe'; "
        L"$authFallbackVersion='2026.07.04'; "
        L"$authBlacklist=@('2026.08.19'); "
        L"$tmp=$dest + '.winterstatic-new.exe'; "
        L"$authTmp=$authDest + '.winterstatic-new.exe'; "
        L"$min=[version]'2026.8.19'; "
        L"$authDir=Split-Path -Parent $authDest; "
        L"if ($authDir -and -not (Test-Path -LiteralPath $authDir)) { [void](New-Item -ItemType Directory -Path $authDir -Force) }; "
        L"if (Test-Path -LiteralPath $tmp) { Remove-Item -LiteralPath $tmp -Force }; "
        L"if (Test-Path -LiteralPath $authTmp) { Remove-Item -LiteralPath $authTmp -Force }; "
        L"try { "
        L"  Invoke-WebRequest -UseBasicParsing -Headers @{'User-Agent'='WinterStatic-yt-dlp-Downloader'} -Uri $stableUrl -OutFile $tmp; "
        L"  $version=([string](& $tmp --version 2>$null | Select-Object -First 1)).Trim(); "
        L"  $v=[version]$version; "
        L"  if (-not $version -or -not $v -or $v -lt $min -or $version -notmatch '^\\d{4}\\.\\d{2}\\.\\d{2}$') { throw ('Downloaded primary stable is not acceptable: ' + $version) }; "
        L"  $authBlocked=$authBlacklist -contains $version; "
        L"  if ($authBlocked) { "
        L"    $authUrl='https://github.com/yt-dlp/yt-dlp/releases/download/' + $authFallbackVersion + '/yt-dlp.exe'; "
        L"    Invoke-WebRequest -UseBasicParsing -Headers @{'User-Agent'='WinterStatic-yt-dlp-Downloader'} -Uri $authUrl -OutFile $authTmp; "
        L"    $authVersion=([string](& $authTmp --version 2>$null | Select-Object -First 1)).Trim(); "
        L"    if ($authVersion -ne $authFallbackVersion) { throw ('Authenticated fallback did not report ' + $authFallbackVersion + ': ' + $authVersion) }; "
        L"    Write-Host ('Authenticated backend: stable ' + $version + ' is blacklisted; using ' + $authVersion + '.') -ForegroundColor Yellow; "
        L"  } else { "
        L"    Copy-Item -LiteralPath $tmp -Destination $authTmp -Force; "
        L"    $authVersion=([string](& $authTmp --version 2>$null | Select-Object -First 1)).Trim(); "
        L"    if ($authVersion -ne $version -or $authBlacklist -contains $authVersion -or $authVersion -notmatch '^\\d{4}\\.\\d{2}\\.\\d{2}$') { throw ('Authenticated stable is not acceptable: ' + $authVersion) }; "
        L"    Write-Host ('Authenticated backend follows latest stable ' + $authVersion + '.') -ForegroundColor Green; "
        L"  }; "
        L"  Move-Item -LiteralPath $tmp -Destination $dest -Force; "
        L"  Move-Item -LiteralPath $authTmp -Destination $authDest -Force; "
        L"  Write-Host ('Installed primary yt-dlp latest stable ' + $version) -ForegroundColor Green; "
        L"  Write-Host ('Primary path: ' + $dest) -ForegroundColor DarkGray; "
        L"  Write-Host ('Authenticated path: ' + $authDest) -ForegroundColor DarkGray; "
        L"} finally { "
        L"  if (Test-Path -LiteralPath $tmp) { Remove-Item -LiteralPath $tmp -Force -ErrorAction SilentlyContinue }; "
        L"  if (Test-Path -LiteralPath $authTmp) { Remove-Item -LiteralPath $authTmp -Force -ErrorAction SilentlyContinue }; "
        L"}";

    std::wstring cmd = L"powershell.exe -NoProfile -NoExit -Command \"" + script + L"\"";
    std::vector<wchar_t> mutableCmd(cmd.begin(), cmd.end());
    mutableCmd.push_back(L'\0');
    STARTUPINFOW si{}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (CreateProcessW(nullptr, mutableCmd.data(), nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, g_root.c_str(), &si, &pi)) {
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
        AppendLogUi(L"Opened WinterStatic dual yt-dlp updater in PowerShell.");
    } else {
        MessageBoxW(g_main, L"Could not start the yt-dlp updater.", L"yt-dlp update", MB_OK | MB_ICONERROR);
    }
}

// -----------------------------------------------------------------------------
// UI
// -----------------------------------------------------------------------------

HWND AddStatic(const wchar_t* text, int id = -1, DWORD extraStyle = 0) {
    std::wstring initial = text ? text : L"";
    if (id >= 0 && UsesPaddedStaticText(id)) initial = L" " + initial;
    HWND h = CreateWindowExW(0, L"STATIC", initial.c_str(), WS_CHILD | WS_VISIBLE | extraStyle,
                             0, 0, 10, 10, g_main, id >= 0 ? reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)) : nullptr,
                             GetModuleHandleW(nullptr), nullptr);
    SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    return h;
}

HWND AddEdit(int id, DWORD style = ES_AUTOHSCROLL) {
    HWND h = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | style,
                             0, 0, 10, 10, g_main, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    ApplyDarkControl(h);
    return h;
}

void SetEditMargins(HWND h, int left = 3, int right = 2) {
    if (!h) return;
    SendMessageW(h, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(left, right));
}

HWND AddButton(const wchar_t* text, int id) {
    HWND h = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                             0, 0, 10, 10, g_main, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(g_boldFont), TRUE);
    return h;
}

HWND AddCheckbox(const wchar_t* text, int id) {
    HWND h = CreateWindowExW(0, L"BUTTON", text,
                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                             0, 0, 10, 10, g_main,
                             reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                             GetModuleHandleW(nullptr), nullptr);
    SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(g_smallFont), TRUE);
    ApplyDarkControl(h);
    return h;
}

void AddTooltip(HWND target, const wchar_t* text) {
    if (!g_tooltip || !target || !text || !*text) return;
    TOOLINFOW ti{};
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = g_main;
    ti.uId = reinterpret_cast<UINT_PTR>(target);
    ti.lpszText = const_cast<LPWSTR>(text);
    SendMessageW(g_tooltip, TTM_ADDTOOLW, 0, reinterpret_cast<LPARAM>(&ti));
}

HWND AddCombo(int id, const std::vector<std::wstring>& items) {
    HWND h = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS | WS_VSCROLL,
                             0, 0, 10, 200, g_main, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    for (const auto& item : items) SendMessageW(h, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
    SetWindowTheme(h, L"", L"");
    SetWindowSubclass(h, [](HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR) -> LRESULT {
        LRESULT result = DefSubclassProc(hwnd, msg, wp, lp);
        if (msg == WM_PAINT || msg == WM_NCPAINT || msg == WM_SETFOCUS || msg == WM_KILLFOCUS) {
            // Let the native ComboBox handle behaviour, keyboard input and the
            // popup list, then cover only the pieces of classic Windows chrome
            // that do not fit the dark UI.  This keeps the control native and
            // avoids adding a custom widget implementation or dependency.
            HDC dc = GetDC(hwnd);
            if (dc) {
                RECT cr{}; GetClientRect(hwnd, &cr);
                const int buttonW = GetSystemMetrics(SM_CXVSCROLL);
                RECT br{std::max(cr.left, cr.right - buttonW - 2), cr.top, cr.right, cr.bottom};
                HBRUSH bb = CreateSolidBrush(C_COMBO);
                FillRect(dc, &br, bb);
                DeleteObject(bb);

                const int cx = (br.left + br.right) / 2;
                const int cy = (br.top + br.bottom) / 2 + 1;
                POINT arrow[3]{{cx - 4, cy - 2}, {cx + 4, cy - 2}, {cx, cy + 3}};
                HBRUSH ab = CreateSolidBrush(C_TEXT_DIM);
                HPEN ap = CreatePen(PS_SOLID, 1, C_TEXT_DIM);
                HGDIOBJ oldB = SelectObject(dc, ab);
                HGDIOBJ oldP = SelectObject(dc, ap);
                Polygon(dc, arrow, 3);
                SelectObject(dc, oldP); SelectObject(dc, oldB);
                DeleteObject(ap); DeleteObject(ab);
                ReleaseDC(hwnd, dc);
            }

            // Paint one deliberate frame around the complete ComboBox.  The
            // grey belongs inside the frame; the outer black edge must line up
            // continuously with the drop-arrow section on the right.
            HDC windowDc = GetWindowDC(hwnd);
            if (windowDc) {
                RECT wr{}; GetWindowRect(hwnd, &wr);
                const int w = std::max(1, static_cast<int>(wr.right - wr.left));
                const int hgt = std::max(1, static_cast<int>(wr.bottom - wr.top));
                constexpr int frame = 2;
                const COLORREF edgeColor = (GetFocus() == hwnd) ? C_COMBO_FOCUS : C_COMBO_EDGE;
                HBRUSH edge = CreateSolidBrush(edgeColor);

                RECT top{0, 0, w, std::min(frame, hgt)};
                RECT bottom{0, std::max(0, hgt - frame), w, hgt};
                RECT left{0, 0, std::min(frame, w), hgt};
                RECT right{std::max(0, w - frame), 0, w, hgt};
                FillRect(windowDc, &top, edge);
                FillRect(windowDc, &bottom, edge);
                FillRect(windowDc, &left, edge);
                FillRect(windowDc, &right, edge);

                // Add one subtle grey pixel immediately inside the black frame
                // around the text portion.  This gives the left/top/bottom edge
                // the same visual weight as the arrow side without brightening
                // the outer dark-mode border.
                const int buttonW = GetSystemMetrics(SM_CXVSCROLL);
                const int separatorX = std::clamp(w - buttonW - frame, frame, std::max(frame, w - frame));
                RECT innerText{frame, frame, separatorX, std::max(frame, hgt - frame)};
                if (innerText.right > innerText.left && innerText.bottom > innerText.top) {
                    HBRUSH innerEdge = CreateSolidBrush(C_COMBO_INNER);
                    FrameRect(windowDc, &innerText, innerEdge);
                    DeleteObject(innerEdge);
                }

                // Draw exactly one separator for the arrow button.  Keep it
                // inside the outer frame so all black lines terminate cleanly
                // on the same top/bottom edges.
                RECT separator{separatorX, frame, std::min(separatorX + 1, w - frame), std::max(frame, hgt - frame)};
                if (separator.right > separator.left && separator.bottom > separator.top) {
                    FillRect(windowDc, &separator, edge);
                }

                DeleteObject(edge);
                ReleaseDC(hwnd, windowDc);
            }
        }
        return result;
    }, 1, 0);
    return h;
}

void SetComboText(HWND combo, const std::wstring& text, int fallback = 0) {
    const LRESULT found = SendMessageW(combo, CB_FINDSTRINGEXACT, -1, reinterpret_cast<LPARAM>(text.c_str()));
    SendMessageW(combo, CB_SETCURSEL, found == CB_ERR ? fallback : found, 0);
}

void StyleTabOverflowControl();

std::wstring JobTabText(const std::shared_ptr<DownloadJob>& job) {
    if (!job) return L"Download";
    std::wstring text = L"Download " + std::to_wstring(job->id);
    if (job->running) {
        if (job->progressPercent > 0) text += L" (" + std::to_wstring(job->progressPercent) + L"%)";
        else text += L" (running)";
    } else if (job->finished) {
        text += job->exitCode == 0 ? L" (done)" : L" (failed)";
    }
    return text;
}

// The native tab control sizes items from their stored text. Keep a fixed sizing
// label in each job tab and paint the live status ourselves. This prevents progress
// text changes from moving every tab to the right while preserving a compact '+' tab.
const wchar_t* JobTabSizingText() {
    return L"Download 12 (running)  x";
}

RECT TabCloseRectFromItemRect(RECT r) {
    RECT close = r;
    close.left = std::max(r.left + 20, r.right - 22);
    close.right = r.right - 5;
    close.top += 5;
    close.bottom -= 5;
    return close;
}

bool GetTabCloseRect(int index, RECT& close) {
    if (!g_tabs || index < 0 || index >= static_cast<int>(g_jobs.size())) return false;
    RECT item{};
    if (!TabCtrl_GetItemRect(g_tabs, index, &item)) return false;
    close = TabCloseRectFromItemRect(item);
    return close.right > close.left && close.bottom > close.top;
}

int HitTestTabClose(POINT pt) {
    if (!g_tabs) return -1;
    for (int i = 0; i < static_cast<int>(g_jobs.size()); ++i) {
        RECT close{};
        if (GetTabCloseRect(i, close) && PtInRect(&close, pt)) return i;
    }
    return -1;
}

void UpdateTabLabel(const std::shared_ptr<DownloadJob>& job) {
    if (!g_tabs || !job) return;
    for (size_t i = 0; i < g_jobs.size(); ++i) {
        if (g_jobs[i] != job) continue;
        RECT r{};
        if (TabCtrl_GetItemRect(g_tabs, static_cast<int>(i), &r))
            InvalidateRect(g_tabs, &r, FALSE);
        else
            InvalidateRect(g_tabs, nullptr, FALSE);
        return;
    }
}

void SaveActiveJobUi() {
    auto job = ActiveJob();
    if (!job || !g_main) return;
    job->url = GetText(IDC_URL);
    job->output = GetText(IDC_OUTPUT);
    job->quality = GetText(IDC_QUALITY);
    job->formatPreference = GetText(IDC_FORMAT);
    job->auth = GetText(IDC_AUTH);
    job->closePowerShellOnSuccess =
        IsDlgButtonChecked(g_main, IDC_CLOSE_POWERSHELL) == BST_CHECKED;
    job->browserSweep =
        IsDlgButtonChecked(g_main, IDC_BROWSER_SWEEP) == BST_CHECKED;
}

void RefreshActiveJobControls() {
    auto job = ActiveJob();
    if (!job || !g_main) return;

    SetProgressPercent(job->progressPercent);
    SetText(IDC_STAGE, job->stage);
    SetText(IDC_DETAIL, job->detail);

    HWND log = GetDlgItem(g_main, IDC_LOG);
    if (log) {
        SetWindowTextW(log, job->logText.c_str());
        const int len = GetWindowTextLengthW(log);
        SendMessageW(log, EM_SETSEL, len, len);
        SendMessageW(log, EM_SCROLLCARET, 0, 0);
    }

    EnableWindow(GetDlgItem(g_main, IDC_DOWNLOAD), job->running ? FALSE : TRUE);
}

void LoadActiveJobUi() {
    auto job = ActiveJob();
    if (!job || !g_main) return;

    SetText(IDC_URL, job->url);
    SetText(IDC_OUTPUT, job->output);

    if (HWND quality = GetDlgItem(g_main, IDC_QUALITY)) {
        SetComboText(quality, job->quality, 1);
    }
    if (HWND format = GetDlgItem(g_main, IDC_FORMAT)) {
        SetComboText(format, job->formatPreference, 0);
    }
    if (HWND auth = GetDlgItem(g_main, IDC_AUTH)) {
        SetComboText(auth, job->auth, 0);
    }

    CheckDlgButton(g_main, IDC_CLOSE_POWERSHELL,
                   job->closePowerShellOnSuccess ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(g_main, IDC_BROWSER_SWEEP,
                   job->browserSweep ? BST_CHECKED : BST_UNCHECKED);

    RefreshActiveJobControls();
}

LRESULT CALLBACK TabOverflowSubclassProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
                                                  UINT_PTR subclassId, DWORD_PTR) {
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd, &ps);
        RECT client{};
        GetClientRect(hwnd, &client);

        POINT cursor{};
        GetCursorPos(&cursor);
        ScreenToClient(hwnd, &cursor);
        const bool inside = PtInRect(&client, cursor) != FALSE;
        const bool down = (GetKeyState(VK_LBUTTON) & 0x8000) != 0;

        const int split = client.left + (client.right - client.left) / 2;
        RECT left = client;
        RECT right = client;
        left.right = split;
        right.left = split;

        auto drawHalf = [&](const RECT& r, bool hovered, bool pointsRight) {
            COLORREF fill = C_EDIT;
            if (hovered) fill = down ? C_BUTTON_HOT : C_PANEL;
            HBRUSH bg = CreateSolidBrush(fill);
            FillRect(dc, &r, bg);
            DeleteObject(bg);

            const int cx = (r.left + r.right) / 2;
            const int cy = (r.top + r.bottom) / 2;
            POINT arrow[3]{};
            if (pointsRight) {
                arrow[0] = {cx - 2, cy - 4};
                arrow[1] = {cx - 2, cy + 4};
                arrow[2] = {cx + 3, cy};
            } else {
                arrow[0] = {cx + 2, cy - 4};
                arrow[1] = {cx + 2, cy + 4};
                arrow[2] = {cx - 3, cy};
            }

            HBRUSH arrowBrush = CreateSolidBrush(hovered ? C_TEXT : C_TEXT_DIM);
            HPEN arrowPen = CreatePen(PS_SOLID, 1, hovered ? C_TEXT : C_TEXT_DIM);
            HGDIOBJ oldBrush = SelectObject(dc, arrowBrush);
            HGDIOBJ oldPen = SelectObject(dc, arrowPen);
            Polygon(dc, arrow, 3);
            SelectObject(dc, oldPen);
            SelectObject(dc, oldBrush);
            DeleteObject(arrowPen);
            DeleteObject(arrowBrush);
        };

        const bool leftHover = inside && cursor.x < split;
        const bool rightHover = inside && cursor.x >= split;
        drawHalf(left, leftHover, false);
        drawHalf(right, rightHover, true);

        HBRUSH edge = CreateSolidBrush(C_PANEL_EDGE);
        FrameRect(dc, &client, edge);
        DeleteObject(edge);

        HPEN divider = CreatePen(PS_SOLID, 1, C_PANEL_EDGE);
        HGDIOBJ oldPen = SelectObject(dc, divider);
        MoveToEx(dc, split, client.top + 1, nullptr);
        LineTo(dc, split, client.bottom - 1);
        SelectObject(dc, oldPen);
        DeleteObject(divider);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_MOUSEMOVE: {
        TRACKMOUSEEVENT tme{};
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd;
        TrackMouseEvent(&tme);
        LRESULT result = DefSubclassProc(hwnd, msg, wp, lp);
        InvalidateRect(hwnd, nullptr, FALSE);
        return result;
    }

    case WM_MOUSELEAVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP: {
        LRESULT result = DefSubclassProc(hwnd, msg, wp, lp);
        InvalidateRect(hwnd, nullptr, FALSE);
        return result;
    }

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, TabOverflowSubclassProc, subclassId);
        break;
    }

    return DefSubclassProc(hwnd, msg, wp, lp);
}

void StyleTabOverflowControl() {
    if (!g_tabs) return;
    HWND upDown = FindWindowExW(g_tabs, nullptr, L"msctls_updown32", nullptr);
    if (!upDown) return;
    SetWindowTheme(upDown, L"", L"");
    SetWindowSubclass(upDown, TabOverflowSubclassProc, 3, 0);
    InvalidateRect(upDown, nullptr, TRUE);
}

void InsertPlusTab() {
    if (!g_tabs) return;
    TCITEMW plus{};
    plus.mask = TCIF_TEXT;
    wchar_t text[] = L"+";
    plus.pszText = text;
    TabCtrl_InsertItem(g_tabs, static_cast<int>(g_jobs.size()), &plus);
    StyleTabOverflowControl();
}

void InitializeJobTabsFromCurrentUi() {
    if (!g_tabs || !g_jobs.empty()) return;

    auto job = std::make_shared<DownloadJob>();
    job->id = g_nextJobId++;
    job->url = GetText(IDC_URL);
    job->output = GetText(IDC_OUTPUT);
    job->quality = GetText(IDC_QUALITY);
    job->formatPreference = GetText(IDC_FORMAT);
    job->auth = GetText(IDC_AUTH);
    job->closePowerShellOnSuccess =
        IsDlgButtonChecked(g_main, IDC_CLOSE_POWERSHELL) == BST_CHECKED;
    job->browserSweep =
        IsDlgButtonChecked(g_main, IDC_BROWSER_SWEEP) == BST_CHECKED;

    g_jobs.push_back(job);

    TCITEMW item{};
    item.mask = TCIF_TEXT;
    item.pszText = const_cast<wchar_t*>(JobTabSizingText());
    TabCtrl_InsertItem(g_tabs, 0, &item);
    InsertPlusTab();

    g_activeJobIndex = 0;
    TabCtrl_SetCurSel(g_tabs, 0);
    InvalidateRect(g_tabs, nullptr, TRUE);
    LoadActiveJobUi();
}

void CreateNewJobTab() {
    if (!g_tabs) return;
    if (g_jobs.size() >= 12) {
        MessageBoxW(g_main, L"This build supports up to 12 download tabs.",
                    L"Download tabs", MB_OK | MB_ICONINFORMATION);
        TabCtrl_SetCurSel(g_tabs, g_activeJobIndex);
        return;
    }

    SaveActiveJobUi();
    auto source = ActiveJob();

    auto job = std::make_shared<DownloadJob>();
    job->id = g_nextJobId++;
    if (source) {
        job->output = source->output;
        job->quality = source->quality;
        job->formatPreference = source->formatPreference;
        job->auth = source->auth;
        job->closePowerShellOnSuccess = source->closePowerShellOnSuccess;
        job->browserSweep = source->browserSweep;
    } else {
        job->output = GetText(IDC_OUTPUT);
        job->quality = GetText(IDC_QUALITY);
        job->formatPreference = GetText(IDC_FORMAT);
        job->auth = GetText(IDC_AUTH);
        job->closePowerShellOnSuccess =
            IsDlgButtonChecked(g_main, IDC_CLOSE_POWERSHELL) == BST_CHECKED;
        job->browserSweep =
            IsDlgButtonChecked(g_main, IDC_BROWSER_SWEEP) == BST_CHECKED;
    }

    const int newIndex = static_cast<int>(g_jobs.size());
    g_jobs.push_back(job);

    TCITEMW item{};
    item.mask = TCIF_TEXT;
    item.pszText = const_cast<wchar_t*>(JobTabSizingText());
    TabCtrl_InsertItem(g_tabs, newIndex, &item);
    StyleTabOverflowControl();

    g_activeJobIndex = newIndex;
    TabCtrl_SetCurSel(g_tabs, newIndex);
    InvalidateRect(g_tabs, nullptr, TRUE);
    LoadActiveJobUi();
}

void CloseJobTab(int index) {
    if (!g_tabs || index < 0 || index >= static_cast<int>(g_jobs.size())) return;

    // Save edits in the currently selected tab before reindexing the job vector.
    SaveActiveJobUi();

    const int oldActive = g_activeJobIndex;
    auto job = g_jobs[static_cast<size_t>(index)];

    // A closed tab must not leave an unmanaged PowerShell/yt-dlp process behind.
    // Closing the whole GUI is different and still leaves active downloads running.
    TerminateJobTask(job);

    g_hoverCloseTab = -1;
    g_pressedCloseTab = -1;
    g_jobs.erase(g_jobs.begin() + index);
    TabCtrl_DeleteItem(g_tabs, index);
    StyleTabOverflowControl();

    if (g_jobs.empty()) {
        g_activeJobIndex = -1;
        CreateNewJobTab();
        return;
    }

    if (oldActive > index) {
        g_activeJobIndex = oldActive - 1;
    } else if (oldActive == index) {
        g_activeJobIndex = std::min(index, static_cast<int>(g_jobs.size()) - 1);
    } else {
        g_activeJobIndex = oldActive;
    }

    g_activeJobIndex = std::clamp(g_activeJobIndex, 0, static_cast<int>(g_jobs.size()) - 1);
    TabCtrl_SetCurSel(g_tabs, g_activeJobIndex);
    InvalidateRect(g_tabs, nullptr, TRUE);
    LoadActiveJobUi();
}

void HandleTabSelectionChanged() {
    if (!g_tabs) return;
    const int selected = TabCtrl_GetCurSel(g_tabs);
    if (selected < 0) return;

    SaveActiveJobUi();

    if (selected == static_cast<int>(g_jobs.size())) {
        CreateNewJobTab();
        return;
    }

    if (selected >= 0 && selected < static_cast<int>(g_jobs.size())) {
        g_activeJobIndex = selected;
        InvalidateRect(g_tabs, nullptr, TRUE);
        LoadActiveJobUi();
    }
}

COLORREF ButtonColor(int id) {
    switch (id) {
    case IDC_DOWNLOAD: case IDC_CHECK_LOGIN: return C_GREEN;
    case IDC_STOP: case IDC_RESET_LOGIN: return C_RED;
    case IDC_PASTE: case IDC_BROWSE_OUTPUT: case IDC_OPEN_BROWSER: case IDC_COPY_COMMAND: return C_BLUE;
    default: return C_BUTTON;
    }
}

void DrawOwnerButton(const DRAWITEMSTRUCT* dis) {
    RECT r = dis->rcItem;
    int id = static_cast<int>(dis->CtlID);
    COLORREF bg = ButtonColor(id);
    if (dis->itemState & ODS_SELECTED) {
        bg = RGB(std::max(0, GetRValue(bg) - 12), std::max(0, GetGValue(bg) - 12), std::max(0, GetBValue(bg) - 12));
    }
    if (dis->itemState & ODS_DISABLED) bg = RGB(48, 48, 48);
    HBRUSH b = CreateSolidBrush(bg);
    FillRect(dis->hDC, &r, b);
    DeleteObject(b);
    HBRUSH edgeBrush = CreateSolidBrush(C_BUTTON_EDGE);
    FrameRect(dis->hDC, &r, edgeBrush);
    DeleteObject(edgeBrush);

    wchar_t text[256]{};
    GetWindowTextW(dis->hwndItem, text, static_cast<int>(std::size(text)));
    SetBkMode(dis->hDC, TRANSPARENT);
    SetTextColor(dis->hDC, (dis->itemState & ODS_DISABLED) ? C_TEXT_DIM : C_TEXT);
    HFONT old = static_cast<HFONT>(SelectObject(dis->hDC, g_boldFont));
    DrawTextW(dis->hDC, text, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    SelectObject(dis->hDC, old);

    if (dis->itemState & ODS_FOCUS) {
        RECT focus = r;
        InflateRect(&focus, -3, -3);
        DrawFocusRect(dis->hDC, &focus);
    }
}

void PaintTabItem(HDC dc, int index, const RECT& itemRect, bool selected) {
    RECT r = itemRect;
    HBRUSH bg = CreateSolidBrush(selected ? C_PANEL : C_EDIT);
    FillRect(dc, &r, bg);
    DeleteObject(bg);

    HBRUSH edge = CreateSolidBrush(selected ? C_TEAL : C_PANEL_EDGE);
    FrameRect(dc, &r, edge);
    DeleteObject(edge);

    const bool isJobTab = index >= 0 && index < static_cast<int>(g_jobs.size());
    const std::wstring text = isJobTab ? JobTabText(g_jobs[static_cast<size_t>(index)]) : L"+";

    RECT textRect = r;
    if (isJobTab) {
        textRect.left += 4;
        textRect.right -= 23;
    }

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, selected ? C_TEXT : C_TEXT_DIM);
    HFONT old = static_cast<HFONT>(SelectObject(dc, g_smallFont));
    DrawTextW(dc, text.c_str(), -1, &textRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    SelectObject(dc, old);

    if (!isJobTab) return;

    const RECT close = TabCloseRectFromItemRect(r);
    const bool hot = index == g_hoverCloseTab;
    const bool pressed = index == g_pressedCloseTab;
    const COLORREF closeColor = pressed ? C_TEXT : (hot ? C_ERROR : C_TEXT_DIM);
    HPEN pen = CreatePen(PS_SOLID, hot ? 2 : 1, closeColor);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    const int cx = (close.left + close.right) / 2;
    const int cy = (close.top + close.bottom) / 2;
    const int d = 3;
    MoveToEx(dc, cx - d, cy - d, nullptr);
    LineTo(dc, cx + d + 1, cy + d + 1);
    MoveToEx(dc, cx + d, cy - d, nullptr);
    LineTo(dc, cx - d - 1, cy + d + 1);
    SelectObject(dc, oldPen);
    DeleteObject(pen);
}

void DrawTabItem(const DRAWITEMSTRUCT* dis) {
    if (!dis || dis->CtlID != IDC_TABS) return;
    const int index = static_cast<int>(dis->itemID);
    const bool selected = g_tabs && index == TabCtrl_GetCurSel(g_tabs);
    PaintTabItem(dis->hDC, index, dis->rcItem, selected);
}

void DrawComboItem(const DRAWITEMSTRUCT* dis) {
    if (dis->CtlID != IDC_QUALITY && dis->CtlID != IDC_FORMAT && dis->CtlID != IDC_AUTH) return;
    RECT r = dis->rcItem;
    bool selected = (dis->itemState & ODS_SELECTED) != 0;
    COLORREF bg = selected ? C_COMBO_HOT : C_COMBO;
    HBRUSH brush = CreateSolidBrush(bg);
    FillRect(dis->hDC, &r, brush);
    DeleteObject(brush);

    wchar_t text[256]{};
    if (dis->itemID != static_cast<UINT>(-1)) {
        SendMessageW(dis->hwndItem, CB_GETLBTEXT, dis->itemID, reinterpret_cast<LPARAM>(text));
    } else {
        GetWindowTextW(dis->hwndItem, text, static_cast<int>(std::size(text)));
    }

    SetBkMode(dis->hDC, TRANSPARENT);
    SetTextColor(dis->hDC, C_TEXT);
    HFONT old = static_cast<HFONT>(SelectObject(dis->hDC, g_font));
    RECT tr = r;
    tr.left += 8;
    tr.right -= 26;
    DrawTextW(dis->hDC, text, -1, &tr, DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS);
    SelectObject(dis->hDC, old);

    // Do not frame the owner-drawn text item itself. The ComboBox subclass
    // paints one outer frame around the complete control after native drawing.
    // Framing this item as well creates a second vertical line beside the
    // drop-arrow button on Windows 11.
}

void SetProgressPercent(int percent) {
    g_progressPercent = std::clamp(percent, 0, 100);
    if (g_main) {
        HWND progress = GetDlgItem(g_main, IDC_PROGRESS);
        if (progress) InvalidateRect(progress, nullptr, TRUE);
    }
}

void DrawProgress(const DRAWITEMSTRUCT* dis) {
    RECT r = dis->rcItem;
    HBRUSH bg = CreateSolidBrush(C_PROGRESS_BG);
    FillRect(dis->hDC, &r, bg);
    DeleteObject(bg);

    RECT inner = r;
    InflateRect(&inner, -1, -1);
    if (g_progressPercent > 0 && inner.right > inner.left) {
        RECT fill = inner;
        fill.right = fill.left + MulDiv(inner.right - inner.left, g_progressPercent, 100);
        HBRUSH bar = CreateSolidBrush(C_PROGRESS_BAR);
        FillRect(dis->hDC, &fill, bar);
        DeleteObject(bar);
    }

    HBRUSH edge = CreateSolidBrush(RGB(12, 12, 12));
    FrameRect(dis->hDC, &r, edge);
    DeleteObject(edge);
}

void DrawPanel(HDC dc, const RectI& p, const wchar_t* title) {
    RECT r{p.x, p.y, p.x + p.w, p.y + p.h};
    HBRUSH panel = CreateSolidBrush(C_PANEL);
    FillRect(dc, &r, panel);
    DeleteObject(panel);
    HBRUSH edge = CreateSolidBrush(C_PANEL_EDGE);
    FrameRect(dc, &r, edge);
    DeleteObject(edge);

    RECT tr{p.x + 14, p.y + 8, p.x + p.w - 14, p.y + 30};
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, C_TEAL_SOFT);
    HFONT old = static_cast<HFONT>(SelectObject(dc, g_boldFont));
    DrawTextW(dc, title, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old);
}

void PaintBackground(HWND hwnd) {
    PAINTSTRUCT ps{};
    HDC dc = BeginPaint(hwnd, &ps);
    RECT client{};
    GetClientRect(hwnd, &client);
    FillRect(dc, &client, g_bgBrush);
    DrawPanel(dc, g_panels.download, L"Download");
    DrawPanel(dc, g_panels.account, L"YouTube account browser");
    DrawPanel(dc, g_panels.tools, L"Tools");
    DrawPanel(dc, g_panels.run, L"Run");
    DrawPanel(dc, g_panels.log, L"GUI log");
    EndPaint(hwnd, &ps);
}

void MoveCtl(int id, int x, int y, int w, int h) {
    if (HWND ctl = GetDlgItem(g_main, id)) MoveWindow(ctl, x, y, std::max(1, w), std::max(1, h), TRUE);
}

void LayoutControls(int cw, int ch) {
    const int margin = 8;
    const int gap = 9;
    const int panelW = std::max(720, cw - margin * 2);
    MoveCtl(IDC_TABS, margin + 4, 56, panelW - 8, 30);
    int y = 92;
    g_panels.download = {margin, y, panelW, 220}; y += 220 + gap;
    g_panels.account = {margin, y, panelW, 132}; y += 132 + gap;
    g_panels.tools = {margin, y, panelW, 178}; y += 178 + gap;
    g_panels.run = {margin, y, panelW, 164}; y += 164 + gap;
    g_panels.log = {margin, y, panelW, std::max(150, ch - y - margin)};

    // Header
    MoveCtl(9001, 18, 14, 350, 34);
    MoveCtl(9002, 370, 23, 170, 20);
    MoveCtl(9003, std::max(500, cw - 185), 20, 175, 20);

    auto body = [](const RectI& p) { return RectI{p.x + 14, p.y + 38, p.w - 28, p.h - 48}; };

    // Download panel
    RectI d = body(g_panels.download);
    const int labelW = 108;
    const int btnW = 70;
    const int btnGap = 8;
    const int rightButtons = btnW * 2 + btnGap;
    const int fieldX = d.x + labelW;
    const int fieldW = std::max(260, d.w - labelW - rightButtons - 10);
    const int rowH = 31;
    const int labelH = 24;
    const int editH = 30;
    const int buttonH = 30;
    const int r0 = d.y;
    MoveCtl(9101, d.x, r0 + 3, labelW - 8, labelH); MoveCtl(IDC_URL, fieldX, r0 + 3, fieldW, 24);
    MoveCtl(IDC_PASTE, fieldX + fieldW + btnGap, r0, btnW, buttonH); MoveCtl(IDC_CLEAR, fieldX + fieldW + btnGap + btnW + btnGap, r0, btnW, buttonH);
    const int r1 = r0 + rowH + 6;
    MoveCtl(9102, d.x, r1 + 3, labelW - 8, labelH); MoveCtl(IDC_OUTPUT, fieldX, r1 + 3, fieldW, 24);
    MoveCtl(IDC_BROWSE_OUTPUT, fieldX + fieldW + btnGap, r1, btnW, buttonH); MoveCtl(IDC_OPEN_OUTPUT, fieldX + fieldW + btnGap + btnW + btnGap, r1, btnW, buttonH);
    const int r2 = r1 + rowH + 6;
    MoveCtl(9103, d.x, r2 + 3, labelW - 8, labelH);
    const int qualityW = std::max(210, (d.w - labelW) * 46 / 100);
    const int formatGap = 10;
    const int formatLabelW = 62;
    const int formatX = fieldX + qualityW + formatGap;
    const int formatW = std::max(120, d.x + d.w - formatX - formatLabelW);
    MoveCtl(IDC_QUALITY, fieldX, r2, qualityW, 220);
    MoveCtl(9106, formatX, r2 + 3, formatLabelW - 4, labelH);
    MoveCtl(IDC_FORMAT, formatX + formatLabelW, r2, formatW, 180);
    const int r3 = r2 + rowH + 6;
    MoveCtl(9104, d.x, r3 + 3, labelW - 8, labelH); MoveCtl(IDC_AUTH, fieldX, r3, fieldW, 180);
    MoveCtl(9105, fieldX + 2, r3 + 35, std::max(300, d.w - labelW - 5), 22);

    // Account panel
    RectI a = body(g_panels.account);
    MoveCtl(9201, a.x, a.y + 3, 70, 24);
    MoveCtl(IDC_ACCOUNT_STATUS, a.x + 72, a.y, a.w - 72, 30);
    int ay = a.y + 40;
    MoveCtl(IDC_OPEN_BROWSER, a.x, ay, 178, 32);
    MoveCtl(IDC_CHECK_LOGIN, a.x + 187, ay, 108, 32);
    MoveCtl(IDC_RESET_LOGIN, a.x + 304, ay, 108, 32);
    MoveCtl(IDC_OPEN_BROWSER_PROFILE, a.x + 421, ay, 148, 32);

    // Tools panel
    RectI t = body(g_panels.tools);
    const int toolLabelW = 68;
    const int toolLayoutReserve = 80; // preserve prior field width; saved label space shifts the remaining row left
    const int toolBtnW = 72;
    const int statusW = 170;
    const int toolFieldX = t.x + toolLabelW;
    const int toolFieldW = std::max(200, t.w - toolLayoutReserve - toolBtnW * 2 - statusW - 26);
    auto toolRow = [&](int yrow, int labelId, int fieldId, int b1, int b2, int statusId) {
        MoveCtl(labelId, t.x, yrow + 3, toolLabelW - 5, 24);
        MoveCtl(fieldId, toolFieldX, yrow + 2, toolFieldW, 25);
        MoveCtl(b1, toolFieldX + toolFieldW + 7, yrow, toolBtnW, 29);
        MoveCtl(b2, toolFieldX + toolFieldW + 7 + toolBtnW + 7, yrow, toolBtnW, 29);
        MoveCtl(statusId, toolFieldX + toolFieldW + 7 + toolBtnW * 2 + 14, yrow + 2, statusW, 24);
    };
    toolRow(t.y, 9301, IDC_YTDLP, IDC_BROWSE_YTDLP, IDC_UPDATE_YTDLP, IDC_YTDLP_STATUS);
    toolRow(t.y + 38, 9302, IDC_BROWSER_PATH, IDC_OPEN_BROWSER_TOOL, IDC_OPEN_BROWSER_PROFILE_2, IDC_BROWSER_STATUS);
    toolRow(t.y + 76, 9303, IDC_FFMPEG, IDC_BROWSE_FFMPEG, IDC_DETECT_FFMPEG, IDC_FFMPEG_STATUS);

    // Run panel: frequent action (Download) lives at the far right.
    // From right to left: Download, Refresh tools, Nuke task.
    RectI r = body(g_panels.run);
    int bx = r.x;
    MoveCtl(IDC_COPY_COMMAND, bx, r.y, 126, 34); bx += 134;
    MoveCtl(IDC_SNAP, bx, r.y, 174, 34);
    const int downloadX = r.x + r.w - 128;
    const int refreshX = downloadX - 8 - 112;
    const int nukeX = refreshX - 8 - 112;
    MoveCtl(IDC_STOP, nukeX, r.y, 112, 34);
    MoveCtl(IDC_REFRESH, refreshX, r.y, 112, 34);
    MoveCtl(IDC_DOWNLOAD, downloadX, r.y, 128, 34);
    const int closeOptionW = 210;
    const int browserSweepW = 126;
    const int optionGap = 10;
    const int browserSweepX = r.x + r.w - closeOptionW - optionGap - browserSweepW;
    MoveCtl(IDC_STAGE, r.x, r.y + 44, std::max(180, browserSweepX - r.x - optionGap), 22);
    MoveCtl(IDC_BROWSER_SWEEP, browserSweepX, r.y + 42, browserSweepW, 24);
    MoveCtl(IDC_CLOSE_POWERSHELL, r.x + r.w - closeOptionW, r.y + 42, closeOptionW, 24);
    MoveCtl(IDC_PROGRESS, r.x, r.y + 70, r.w, 18);
    MoveCtl(IDC_DETAIL, r.x, r.y + 93, r.w, 22);

    // Log
    RectI l = body(g_panels.log);
    MoveCtl(IDC_LOG, l.x, l.y, l.w, std::max(80, l.h));
    InvalidateRect(g_main, nullptr, TRUE);
}

void CreateUi(HWND hwnd) {
    g_main = hwnd;
    g_bgBrush = CreateSolidBrush(C_BG);
    g_panelBrush = CreateSolidBrush(C_PANEL);
    g_editBrush = CreateSolidBrush(C_EDIT);
    g_comboBrush = CreateSolidBrush(C_COMBO);

    g_font = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                         OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    g_smallFont = CreateFontW(-14, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    g_boldFont = CreateFontW(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    g_titleFont = CreateFontW(-23, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

    g_tooltip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr,
                                  WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
                                  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                  hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (g_tooltip) {
        SetWindowPos(g_tooltip, HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        SendMessageW(g_tooltip, TTM_SETMAXTIPWIDTH, 0, 460);
        SendMessageW(g_tooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 18000);
    }

    HWND title = AddStatic(APP_NAME, 9001); SendMessageW(title, WM_SETFONT, reinterpret_cast<WPARAM>(g_titleFont), TRUE);
    HWND ver = AddStatic((std::wstring(L"v") + APP_VERSION).c_str(), 9002); SendMessageW(ver, WM_SETFONT, reinterpret_cast<WPARAM>(g_smallFont), TRUE);
    HWND native = AddStatic(L"Native Win32 frontend", 9003, SS_RIGHT); SendMessageW(native, WM_SETFONT, reinterpret_cast<WPARAM>(g_smallFont), TRUE);

    g_tabs = CreateWindowExW(0, WC_TABCONTROLW, L"",
                              WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,
                              0, 0, 10, 10, hwnd,
                              reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_TABS)),
                              GetModuleHandleW(nullptr), nullptr);
    SendMessageW(g_tabs, WM_SETFONT, reinterpret_cast<WPARAM>(g_smallFont), TRUE);
    SetWindowTheme(g_tabs, L"", L"");
    SetWindowSubclass(g_tabs, [](HWND tab, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR) -> LRESULT {
        if (msg == WM_MOUSEMOVE) {
            POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            const int hover = HitTestTabClose(pt);
            if (hover != g_hoverCloseTab) {
                g_hoverCloseTab = hover;
                InvalidateRect(tab, nullptr, FALSE);
            }
            TRACKMOUSEEVENT tme{};
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = tab;
            TrackMouseEvent(&tme);
        } else if (msg == WM_MOUSELEAVE) {
            if (g_hoverCloseTab != -1) {
                g_hoverCloseTab = -1;
                InvalidateRect(tab, nullptr, FALSE);
            }
        } else if (msg == WM_LBUTTONDOWN) {
            POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            const int closeIndex = HitTestTabClose(pt);
            if (closeIndex >= 0) {
                g_pressedCloseTab = closeIndex;
                SetCapture(tab);
                InvalidateRect(tab, nullptr, FALSE);
                return 0;
            }
        } else if (msg == WM_LBUTTONUP && g_pressedCloseTab >= 0) {
            POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            const int pressed = g_pressedCloseTab;
            const int releasedOver = HitTestTabClose(pt);
            g_pressedCloseTab = -1;
            if (GetCapture() == tab) ReleaseCapture();
            InvalidateRect(tab, nullptr, FALSE);
            if (releasedOver == pressed) CloseJobTab(pressed);
            return 0;
        } else if (msg == WM_CAPTURECHANGED && g_pressedCloseTab >= 0) {
            g_pressedCloseTab = -1;
            InvalidateRect(tab, nullptr, FALSE);
        }

        if (msg == WM_ERASEBKGND) return 1;
        if (msg == WM_SIZE || msg == WM_WINDOWPOSCHANGED) {
            LRESULT result = DefSubclassProc(tab, msg, wp, lp);
            StyleTabOverflowControl();
            return result;
        }
        if (msg == WM_PAINT) {
            PAINTSTRUCT ps{};
            HDC dc = BeginPaint(tab, &ps);
            RECT client{};
            GetClientRect(tab, &client);

            HBRUSH strip = CreateSolidBrush(C_BG);
            FillRect(dc, &client, strip);
            DeleteObject(strip);

            const int selectedIndex = TabCtrl_GetCurSel(tab);
            const int count = TabCtrl_GetItemCount(tab);
            for (int i = 0; i < count; ++i) {
                RECT r{};
                if (!TabCtrl_GetItemRect(tab, i, &r)) continue;
                PaintTabItem(dc, i, r, i == selectedIndex);
            }

            EndPaint(tab, &ps);
            return 0;
        }
        return DefSubclassProc(tab, msg, wp, lp);
    }, 2, 0);

    AddStatic(L"Video URL", 9101); AddEdit(IDC_URL);
    AddButton(L"Paste", IDC_PASTE); AddButton(L"Clear", IDC_CLEAR);
    AddStatic(L"Save to", 9102); AddEdit(IDC_OUTPUT);
    AddButton(L"Browse", IDC_BROWSE_OUTPUT); AddButton(L"Open", IDC_OPEN_OUTPUT);
    AddStatic(L"Quality", 9103);
    HWND quality = AddCombo(IDC_QUALITY, {L"Best quality", L"Maximum 1080p", L"Maximum 720p", L"Audio only (source format)"});
    AddStatic(L"Format", 9106);
    HWND format = AddCombo(IDC_FORMAT, {L"Automatic", L"Prefer MP4", L"Prefer WebM"});
    HWND authLabel = AddStatic(L"Authentication", 9104, SS_LEFT | SS_CENTERIMAGE | SS_NOPREFIX);
    HWND auth = AddCombo(IDC_AUTH, {L"Automatic", L"Anonymous", L"Use account browser"});
    HWND authHint = AddStatic(L"Automatic tries anonymously first and uses the saved Edge login only when YouTube explicitly requires authentication.", 9105, SS_LEFT | SS_CENTERIMAGE);
    SendMessageW(authHint, WM_SETFONT, reinterpret_cast<WPARAM>(g_smallFont), TRUE);

    AddStatic(L"Status", 9201);
    HWND accountStatus = AddStatic(L"Not checked", IDC_ACCOUNT_STATUS, SS_LEFT | SS_CENTERIMAGE);
    SendMessageW(accountStatus, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    AddButton(L"Open Account Browser", IDC_OPEN_BROWSER);
    AddButton(L"Check Login", IDC_CHECK_LOGIN);
    AddButton(L"Reset Login", IDC_RESET_LOGIN);
    AddButton(L"Open Profile", IDC_OPEN_BROWSER_PROFILE);

    AddStatic(L"yt-dlp", 9301); HWND ytPath = AddEdit(IDC_YTDLP); AddButton(L"Browse", IDC_BROWSE_YTDLP); AddButton(L"Update", IDC_UPDATE_YTDLP); HWND ytStatus = AddStatic(L"Checking…", IDC_YTDLP_STATUS, SS_LEFT | SS_CENTERIMAGE | SS_NOTIFY);
    AddStatic(L"Edge", 9302);
    HWND browserPath = AddEdit(IDC_BROWSER_PATH, ES_AUTOHSCROLL | ES_READONLY);
    AddButton(L"Open", IDC_OPEN_BROWSER_TOOL); AddButton(L"Profile", IDC_OPEN_BROWSER_PROFILE_2); AddStatic(L"Checking…", IDC_BROWSER_STATUS, SS_LEFT | SS_CENTERIMAGE);
    AddStatic(L"FFmpeg", 9303); HWND ffmpegPath = AddEdit(IDC_FFMPEG); AddButton(L"Browse", IDC_BROWSE_FFMPEG); AddButton(L"Detect", IDC_DETECT_FFMPEG); AddStatic(L"Checking…", IDC_FFMPEG_STATUS, SS_LEFT | SS_CENTERIMAGE);

    AddButton(L"Download", IDC_DOWNLOAD); AddButton(L"Copy command", IDC_COPY_COMMAND);
    AddButton(L"Nuke task", IDC_STOP); AddButton(L"Snap PowerShell right", IDC_SNAP); AddButton(L"Refresh tools", IDC_REFRESH);
    HWND browserSweep = AddCheckbox(L"Browser sweep", IDC_BROWSER_SWEEP);
    AddCheckbox(L"Close PowerShell on success", IDC_CLOSE_POWERSHELL);
    HWND stage = AddStatic(L"Ready", IDC_STAGE, SS_LEFT | SS_CENTERIMAGE);
    HWND progress = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
                                    0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_PROGRESS)), GetModuleHandleW(nullptr), nullptr);
    (void)progress;
    HWND detail = AddStatic(L"", IDC_DETAIL, SS_LEFT | SS_CENTERIMAGE); SendMessageW(detail, WM_SETFONT, reinterpret_cast<WPARAM>(g_smallFont), TRUE);

    AddTooltip(ytStatus,
        L"yt-dlp versions: primary+authenticated.\n"
        L"Left: primary backend, always the latest official stable release (never nightly).\n"
        L"Right: authenticated YouTube backend, normally the latest stable unless that release is on the auth blacklist.\n"
        L"Current auth blacklist: 2026.08.19. '--' means the authenticated helper is missing or unreadable.");
    AddTooltip(format,
        L"Container preference only. WinterStatic tries the selected family first, then falls back to the best available format rather than failing.\n"
        L"Prefer MP4 pairs MP4 video with M4A audio where possible. Prefer WebM pairs WebM video and audio where possible. Audio-only maps MP4 preference to M4A.");
    AddTooltip(browserSweep,
        L"Optional last-resort browser-assisted recovery.\n"
        L"If normal yt-dlp attempts, resilience retries, and static discovery fail, WinterStatic may open an isolated visible Edge/Chrome/Brave window and observe its network requests for up to 30 seconds.\n"
        L"You control playback; detected media URLs are not displayed. Off by default.");

    HWND log = AddEdit(IDC_LOG, ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL);
    g_logFont = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");
    SendMessageW(log, WM_SETFONT, reinterpret_cast<WPARAM>(g_logFont), TRUE);
    SendMessageW(log, EM_SETLIMITTEXT, 2 * 1024 * 1024, 0);
    (void)stage;
    SetEditMargins(GetDlgItem(hwnd, IDC_URL), 3, 2);
    SetEditMargins(GetDlgItem(hwnd, IDC_OUTPUT), 3, 2);
    SetEditMargins(ytPath, 3, 2);
    SetEditMargins(browserPath, 3, 2);
    SetEditMargins(ffmpegPath, 3, 2);


    const std::wstring defaultYtdlp = JoinPath(g_toolsDir, L"yt-dlp\\yt-dlp.exe");
    std::wstring savedOutput = Trim(ReadIni(L"General", L"OutputDir", DefaultDownloads()));
    if (savedOutput.empty()) savedOutput = DefaultDownloads();
    std::wstring savedYtdlp = ResolveAppRelativePath(ReadIni(L"General", L"YtDlpPath", defaultYtdlp));
    if (FileExists(defaultYtdlp)) {
        savedYtdlp = defaultYtdlp;
    } else if (!FileExists(savedYtdlp)) {
        const std::wstring detected = DetectYtDlpPath();
        if (!detected.empty()) savedYtdlp = detected;
        else if (savedYtdlp.empty()) savedYtdlp = defaultYtdlp;
    }
    const std::wstring defaultFfmpeg = PortableFfmpegPath();
    std::wstring savedFfmpeg = ResolveAppRelativePath(ReadIni(L"General", L"FfmpegPath", L""));
    if (FileExists(defaultFfmpeg)) {
        savedFfmpeg = defaultFfmpeg;
    } else if (!FileExists(savedFfmpeg)) {
        const std::wstring detected = DetectFfmpegPath();
        if (!detected.empty()) savedFfmpeg = detected;
    }
    SetText(IDC_OUTPUT, savedOutput);
    SetText(IDC_YTDLP, savedYtdlp);
    SetText(IDC_FFMPEG, savedFfmpeg);
    SetText(IDC_BROWSER_PATH, g_accountBrowserExe);
    std::wstring savedQuality = ReadIni(L"General", L"Quality", L"Maximum 1080p");
    std::wstring savedFormat = ReadIni(L"General", L"PreferredFormat", L"");
    // Migrate the old combined quality/container preset without changing the
    // user's effective preference on first launch after upgrading from 0.1.38.
    if (savedQuality == L"Best MP4-compatible") {
        savedQuality = L"Best quality";
        if (savedFormat.empty()) savedFormat = L"Prefer MP4";
    }
    if (savedFormat.empty()) savedFormat = L"Automatic";
    SetComboText(quality, savedQuality, 1);
    SetComboText(format, savedFormat, 0);
    SetComboText(auth, ReadIni(L"General", L"Authentication", L"Automatic"), 0);
    const bool closePowerShellOnSuccess = ReadIni(L"General", L"ClosePowerShellOnSuccess", L"0") == L"1";
    const bool browserAssistedRecovery = ReadIni(L"General", L"BrowserAssistedRecovery", L"0") == L"1";
    CheckDlgButton(hwnd, IDC_BROWSER_SWEEP, browserAssistedRecovery ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CLOSE_POWERSHELL, closePowerShellOnSuccess ? BST_CHECKED : BST_UNCHECKED);
    SetText(9104, L"Authentication");
    InvalidateRect(authLabel, nullptr, TRUE);

    InitializeJobTabsFromCurrentUi();

    RECT r{}; GetClientRect(hwnd, &r); LayoutControls(r.right, r.bottom);
    AppendLogUi(L"Ready. Native Win32 frontend initialized.");
    AppendLogUi(L"Application folder: " + g_root);
    SetFocus(GetDlgItem(hwnd, IDC_URL));
}

LRESULT HandleCtlColor(UINT msg, WPARAM wParam, LPARAM lParam) {
    HDC dc = reinterpret_cast<HDC>(wParam);
    HWND ctl = reinterpret_cast<HWND>(lParam);
    int id = GetDlgCtrlID(ctl);
    SetBkMode(dc, OPAQUE);

    if (msg == WM_CTLCOLORLISTBOX) {
        SetTextColor(dc, C_TEXT);
        SetBkColor(dc, C_COMBO);
        return reinterpret_cast<LRESULT>(g_comboBrush);
    }

    if (msg == WM_CTLCOLOREDIT || id == IDC_ACCOUNT_STATUS || id == IDC_BROWSER_PATH || id == IDC_LOG) {
        SetTextColor(dc, id == IDC_ACCOUNT_STATUS
            ? (g_accountSessionValid.load() ? RGB(74, 222, 128) : C_TEXT_DIM)
            : C_TEXT);
        SetBkColor(dc, C_EDIT);
        return reinterpret_cast<LRESULT>(g_editBrush);
    }

    SetBkMode(dc, TRANSPARENT);
    if (id == 9002 || id == 9003 || id == 9105 || id == IDC_DETAIL || id == IDC_YTDLP_STATUS || id == IDC_BROWSER_STATUS || id == IDC_FFMPEG_STATUS) {
        SetTextColor(dc, C_TEXT_DIM);
    } else if (id == IDC_ACCOUNT_STATUS) {
        SetTextColor(dc, g_accountSessionValid.load() ? RGB(74, 222, 128) : C_TEXT_DIM);
    } else {
        SetTextColor(dc, C_TEXT);
    }
    return reinterpret_cast<LRESULT>(g_bgBrush);
}

void HandleCommand(int id, int code) {
    if (code != BN_CLICKED && code != CBN_SELCHANGE) return;
    switch (id) {
    case IDC_PASTE: PasteUrl(); break;
    case IDC_CLEAR: SetText(IDC_URL, L""); break;
    case IDC_BROWSE_OUTPUT: {
        std::wstring p = BrowseFolder(GetText(IDC_OUTPUT));
        if (!p.empty()) SetText(IDC_OUTPUT, p);
        break;
    }
    case IDC_OPEN_OUTPUT: {
        std::wstring p = Trim(GetText(IDC_OUTPUT));
        if (p.empty()) p = DefaultDownloads();
        EnsureDir(p);
        ShellExecuteW(g_main, L"open", p.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        break;
    }
    case IDC_OPEN_BROWSER: case IDC_OPEN_BROWSER_TOOL: OpenAccountBrowser(); break;
    case IDC_CHECK_LOGIN: CheckLoginAsync(false); break;
    case IDC_RESET_LOGIN: ResetLoginProfile(); break;
    case IDC_OPEN_BROWSER_PROFILE: case IDC_OPEN_BROWSER_PROFILE_2: OpenAccountProfileFolder(); break;
    case IDC_BROWSE_YTDLP: {
        std::wstring p = BrowseExe(L"Select yt-dlp.exe", GetText(IDC_YTDLP).c_str());
        if (!p.empty()) { SetText(IDC_YTDLP, p); RefreshToolStatusAsync(); }
        break;
    }
    case IDC_UPDATE_YTDLP: UpdateYtDlp(); break;
    case IDC_BROWSE_FFMPEG: {
        std::wstring p = BrowseExe(L"Select ffmpeg.exe", GetText(IDC_FFMPEG).c_str());
        if (!p.empty()) { SetText(IDC_FFMPEG, p); RefreshToolStatusAsync(); }
        break;
    }
    case IDC_DETECT_FFMPEG: {
        std::wstring p = DetectFfmpegPath();
        if (!p.empty()) { SetText(IDC_FFMPEG, p); AppendLogUi(L"Detected FFmpeg: " + p); }
        else AppendLogUi(L"FFmpeg was not found. Single-file fallback will be used where possible.");
        RefreshToolStatusAsync();
        break;
    }
    case IDC_DOWNLOAD: StartDownload(); break;
    case IDC_COPY_COMMAND: {
        std::wstring cmd = DirectPowerShellCommand(true);
        if (!cmd.empty() && CopyToClipboard(cmd)) AppendLogUi(L"Copied the current PowerShell command.");
        break;
    }
    case IDC_STOP: StopManagedConsoles(); break;
    case IDC_SNAP: SnapCurrentConsole(); break;
    case IDC_REFRESH: RefreshToolStatusAsync(); CheckLoginAsync(false); break;
    case IDC_CLOSE_POWERSHELL: SaveSettings(); break;
    case IDC_BROWSER_SWEEP: SaveSettings(); break;
    case IDC_AUTH: SaveSettings(); break;
    case IDC_FORMAT: SaveSettings(); break;
    case IDC_QUALITY: SaveSettings(); break;
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        ApplyDarkTitleBar(hwnd);
        CreateUi(hwnd);
        {
            const int cleaned = CleanupFinishedTaskDirectories();
            if (cleaned > 0) {
                AppendLogUi(L"Housekeeping: removed " + std::to_wstring(cleaned) +
                            (cleaned == 1 ? L" finished temporary task folder." : L" finished temporary task folders."));
            }
        }
        RefreshToolStatusAsync();
        return 0;
    case WM_SIZE:
        LayoutControls(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_COMMAND:
        HandleCommand(LOWORD(wParam), HIWORD(wParam));
        return 0;
    case WM_NOTIFY: {
        const auto* hdr = reinterpret_cast<const NMHDR*>(lParam);
        if (hdr && hdr->idFrom == IDC_TABS && hdr->code == TCN_SELCHANGE) {
            HandleTabSelectionChanged();
            return 0;
        }
        break;
    }
    case WM_DRAWITEM: {
        const auto* dis = reinterpret_cast<const DRAWITEMSTRUCT*>(lParam);
        if (dis->CtlType == ODT_BUTTON) { DrawOwnerButton(dis); return TRUE; }
        if (dis->CtlType == ODT_TAB && dis->CtlID == IDC_TABS) { DrawTabItem(dis); return TRUE; }
        if (dis->CtlType == ODT_COMBOBOX) { DrawComboItem(dis); return TRUE; }
        if (dis->CtlType == ODT_STATIC && dis->CtlID == IDC_PROGRESS) { DrawProgress(dis); return TRUE; }
        return FALSE;
    }
    case WM_MEASUREITEM: {
        auto* mi = reinterpret_cast<MEASUREITEMSTRUCT*>(lParam);
        if (mi && mi->CtlType == ODT_COMBOBOX) { mi->itemHeight = 24; return TRUE; }
        return FALSE;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
        return HandleCtlColor(msg, wParam, lParam);
    case WM_CTLCOLORBTN: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        HWND ctl = reinterpret_cast<HWND>(lParam);
        const int id = GetDlgCtrlID(ctl);
        if (id == IDC_CLOSE_POWERSHELL || id == IDC_BROWSER_SWEEP) {
            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, C_TEXT);
            return reinterpret_cast<LRESULT>(g_panelBrush);
        }
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
        PaintBackground(hwnd);
        return 0;
    case WM_GETMINMAXINFO: {
        auto* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
        mmi->ptMinTrackSize.x = 780;
        mmi->ptMinTrackSize.y = 790;
        return 0;
    }
    case WM_APP_LOG: {
        auto* text = reinterpret_cast<std::wstring*>(lParam);
        if (text) { AppendLogUi(*text); delete text; }
        return 0;
    }
    case WM_APP_TOOL_STATUS: {
        auto* status = reinterpret_cast<ToolStatus*>(lParam);
        if (status) {
            SetText(IDC_YTDLP_STATUS, status->ytdlp);
            SetText(IDC_BROWSER_STATUS, status->accountBrowser);
            if (!status->accountBrowserPath.empty()) { g_accountBrowserExe = status->accountBrowserPath; SetText(IDC_BROWSER_PATH, g_accountBrowserExe); }
            SetText(IDC_FFMPEG_STATUS, status->ffmpeg);
            if (!status->ytdlpPath.empty() && (GetText(IDC_YTDLP).empty() || !FileExists(GetText(IDC_YTDLP)))) SetText(IDC_YTDLP, status->ytdlpPath);
            if (!status->ffmpegPath.empty() && (GetText(IDC_FFMPEG).empty() || !FileExists(GetText(IDC_FFMPEG)))) SetText(IDC_FFMPEG, status->ffmpegPath);
            if (g_startupLoginPending) {
                g_startupLoginPending = false;
                EnsureAccountBrowserProfile();
                CheckLoginAsync(true);
            }
            delete status;
        }
        return 0;
    }
    case WM_APP_ACCOUNT_STATUS: {
        auto* status = reinterpret_cast<AccountStatus*>(lParam);
        if (status) {
            g_accountSessionValid = status->valid;
            SetText(IDC_ACCOUNT_STATUS, status->text);
            InvalidateRect(GetDlgItem(hwnd, IDC_ACCOUNT_STATUS), nullptr, TRUE);
            AppendLogUi((wParam ? L"Startup login check: " : L"") + status->text);
            delete status;
        }
        return 0;
    }
    case WM_APP_PROGRESS: {
        auto* p = reinterpret_cast<ProgressUpdate*>(lParam);
        if (p) {
            auto job = FindJobById(p->jobId);
            if (job) {
                job->progressPercent = std::clamp(p->percent, 0, 100);
                job->stage = p->stage;
                job->detail = p->detail;
                UpdateTabLabel(job);

                auto active = ActiveJob();
                if (active && active->id == job->id) {
                    SetProgressPercent(job->progressPercent);
                    SetText(IDC_STAGE, job->stage);
                    SetText(IDC_DETAIL, job->detail);
                }
            }
            delete p;
        }
        return 0;
    }
    case WM_APP_JOB_LOG: {
        auto* update = reinterpret_cast<JobLogUpdate*>(lParam);
        if (update) {
            AppendJobLogUi(update->jobId, update->text);
            delete update;
        }
        return 0;
    }
    case WM_APP_TASK_DONE: {
        auto* update = reinterpret_cast<TaskDoneUpdate*>(lParam);
        if (!update) return 0;

        auto job = FindJobById(update->jobId);
        if (job && update->generation == job->generation.load()) {
            job->running = false;
            job->finished = true;
            job->exitCode = update->code;

            if (update->code == 0) {
                job->progressPercent = 100;
                job->stage = L"Complete";
                job->detail = L"Download and assembly completed successfully";
                AppendJobLogUi(job->id, L"Download finished successfully.");
            } else {
                job->stage = L"Failed - exit code " + std::to_wstring(update->code);
                job->detail = L"See the visible PowerShell window for yt-dlp's full output";
                AppendJobLogUi(job->id, L"Download failed with exit code " +
                                        std::to_wstring(update->code) + L".");
            }

            UpdateTabLabel(job);
            auto active = ActiveJob();
            if (active && active->id == job->id) RefreshActiveJobControls();
        }

        delete update;
        return 0;
    }
    case WM_CLOSE:
        SaveSettings();
        // Console-first behavior is intentional: do not kill a live download merely
        // because the frontend window closes.
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        if (g_tooltip && IsWindow(g_tooltip)) DestroyWindow(g_tooltip);
        g_tooltip = nullptr;
        g_main = nullptr;
        if (g_font) DeleteObject(g_font);
        if (g_smallFont) DeleteObject(g_smallFont);
        if (g_boldFont) DeleteObject(g_boldFont);
        if (g_titleFont) DeleteObject(g_titleFont);
        if (g_logFont) DeleteObject(g_logFont);
        if (g_bgBrush) DeleteObject(g_bgBrush);
        if (g_panelBrush) DeleteObject(g_panelBrush);
        if (g_editBrush) DeleteObject(g_editBrush);
        if (g_comboBrush) DeleteObject(g_comboBrush);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_STANDARD_CLASSES | ICC_PROGRESS_CLASS | ICC_WIN95_CLASSES};
    InitCommonControlsEx(&icc);

    g_exeDir = GetExeDirectory();
    g_root = g_exeDir;
    g_settingsPath = JoinPath(g_root, SETTINGS_FILE);
    g_toolsDir = JoinPath(g_root, L"Tools");
    g_accountBrowserExe = DetectEdgePath();
    g_accountBrowserUserData = JoinPath(g_toolsDir, L"BrowserProfile");
    g_accountBrowserProfile = JoinPath(g_accountBrowserUserData, L"Default");

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(101));
    wc.hIconSm = LoadIconW(instance, MAKEINTRESOURCEW(101));
    wc.hbrBackground = nullptr;
    wc.lpszClassName = MAIN_CLASS;
    RegisterClassExW(&wc);

    RECT work{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    const int workW = work.right - work.left;
    const int workH = work.bottom - work.top;
    int width = workW >= 1700 ? 860 : std::max(780, static_cast<int>(workW * 0.48));
    width = std::min(width, workW - 40);
    int height = std::min(1008, workH);

    std::wstring windowTitle = std::wstring(APP_NAME) + L" v" + APP_VERSION;
    HWND hwnd = CreateWindowExW(0, MAIN_CLASS, windowTitle.c_str(), WS_OVERLAPPEDWINDOW,
                                work.left, work.top, width, height,
                                nullptr, nullptr, instance, nullptr);
    if (!hwnd) {
        CoUninitialize();
        return 1;
    }
    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    CoUninitialize();
    return static_cast<int>(msg.wParam);
}
