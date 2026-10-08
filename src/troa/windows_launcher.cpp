// SPDX-License-Identifier: GPL-3.0-or-later
// Stable launcher format 1. Installed once; never replace a live MCP host.
// All Qt/runtime files belong to immutable, versioned payload directories.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Shell declarations require Windows types first (keep this include block separate).
#include <shellapi.h>
#include <string>
#include <vector>

namespace {
std::wstring root;
std::wstring target;
HANDLE running = nullptr;
HWND runningWindow = nullptr;

bool equalPath(const std::wstring &left, const std::wstring &right)
{
    return CompareStringOrdinal(left.c_str(), -1, right.c_str(), -1, TRUE) == CSTR_EQUAL;
}

int fail(const wchar_t *message)
{
#ifdef BIFROST_MCP_HOST
    (void)message;
    const HANDLE error = GetStdHandle(STD_ERROR_HANDLE);
    const char text[] = "Bifrost MCP could not start. Repair the Bifrost Controller installation.\n";
    DWORD written = 0;
    WriteFile(error, text, sizeof(text) - 1, &written, nullptr);
#else
    MessageBoxW(nullptr, message, L"Bifrost Controller", MB_OK | MB_ICONERROR);
#endif
    return 1;
}

// Only numeric dotted release names. The marker cannot select arbitrary paths.
bool releaseName(const std::string &value)
{
    if (value.empty() || value.size() > 48 || value.front() == '.' || value.back() == '.')
        return false;
    unsigned dots = 0;
    char previous = 0;
    for (char c : value)
    {
        if (c == '.')
        {
            if (previous == '.')
                return false;
            ++dots;
        } else if (c < '0' || c > '9')
            return false;
        previous = c;
    }
    return dots == 2;
}

std::wstring quote(const wchar_t *value)
{
    std::wstring result = L"\"";
    unsigned slashes = 0;
    for (const wchar_t *p = value; *p; ++p)
    {
        if (*p == L'\\')
            ++slashes;
        else
        {
            result.append(slashes * (*p == L'\"' ? 2 : 1), L'\\');
            slashes = 0;
            if (*p == L'\"')
                result += L'\\';
            result += *p;
        }
    }
    result.append(slashes * 2, L'\\');
    return result + L'\"';
}

BOOL CALLBACK findMapper(HWND window, LPARAM)
{
    if (GetWindow(window, GW_OWNER) != nullptr)
        return TRUE;
    wchar_t className[64] = {};
    GetClassNameW(window, className, 64);
    if (std::wstring(className).find(L"QWindow") == std::wstring::npos)
        return TRUE;
    wchar_t title[128] = {};
    GetWindowTextW(window, title, 128);
    if (std::wstring(title) != L"Bifrost Controller")
        return TRUE;
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);
    const HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, pid);
    if (!process)
        return TRUE;
    wchar_t path[32768] = {};
    DWORD length = 32768;
    if (QueryFullProcessImageNameW(process, 0, path, &length))
    {
        const std::wstring file(path, length);
        const std::wstring prefix = root + L"\\versions\\";
        const std::wstring suffix = L"\\bin\\bifrost-controller.exe";
        const bool legacy = equalPath(file, root + L"\\bin\\bifrost-controller.exe");
        const bool versioned = file.size() > prefix.size() + suffix.size() &&
                               equalPath(file.substr(0, prefix.size()), prefix) &&
                               equalPath(file.substr(file.size() - suffix.size()), suffix);
        if ((legacy || versioned) && !equalPath(file, target))
        {
            running = process;
            runningWindow = window;
            return FALSE;
        }
    }
    CloseHandle(process);
    return TRUE;
}

bool elevated()
{
    HANDLE token = nullptr;
    TOKEN_ELEVATION elevation = {};
    DWORD length = 0;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
        return true;
    const bool ok = GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &length);
    CloseHandle(token);
    return !ok || elevation.TokenIsElevated;
}

bool sameUser(HANDLE a, HANDLE b)
{
    DWORD sizeA = 0, sizeB = 0;
    GetTokenInformation(a, TokenUser, nullptr, 0, &sizeA);
    GetTokenInformation(b, TokenUser, nullptr, 0, &sizeB);
    std::vector<BYTE> userA(sizeA), userB(sizeB);
    if (!sizeA || !sizeB || !GetTokenInformation(a, TokenUser, userA.data(), sizeA, &sizeA) ||
        !GetTokenInformation(b, TokenUser, userB.data(), sizeB, &sizeB))
        return false;
    return EqualSid(reinterpret_cast<TOKEN_USER *>(userA.data())->User.Sid,
                    reinterpret_cast<TOKEN_USER *>(userB.data())->User.Sid);
}

bool launch(std::wstring command, PROCESS_INFORMATION &child)
{
#ifdef BIFROST_MCP_HOST
    STARTUPINFOEXW startup = {};
    startup.StartupInfo.cb = sizeof(startup);
    startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    HANDLE handles[3] = {};
    const DWORD streams[3] = {STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
    bool ok = true;
    for (unsigned i = 0; i < 3 && ok; ++i)
        ok = DuplicateHandle(GetCurrentProcess(), GetStdHandle(streams[i]), GetCurrentProcess(), &handles[i], 0, TRUE,
                             DUPLICATE_SAME_ACCESS);
    startup.StartupInfo.hStdInput = handles[0];
    startup.StartupInfo.hStdOutput = handles[1];
    startup.StartupInfo.hStdError = handles[2];
    SIZE_T size = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
    std::vector<BYTE> attributes(size);
    startup.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributes.data());
    const bool initialized = InitializeProcThreadAttributeList(startup.lpAttributeList, 1, 0, &size);
    // No Qt is loaded into the stable host; stdio goes directly to its worker.
    ok = ok && initialized &&
         UpdateProcThreadAttribute(startup.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handles, sizeof(handles),
                                   nullptr, nullptr);
    if (ok)
        ok = CreateProcessW(target.c_str(), command.data(), nullptr, nullptr, TRUE,
                            CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT, nullptr, root.c_str(), &startup.StartupInfo,
                            &child);
    if (initialized)
        DeleteProcThreadAttributeList(startup.lpAttributeList);
    for (HANDLE handle : handles)
        if (handle)
            CloseHandle(handle);
    return ok;
#else
    STARTUPINFOW startup = {};
    startup.cb = sizeof(startup);
    if (!elevated())
        return CreateProcessW(target.c_str(), command.data(), nullptr, nullptr, FALSE, 0, nullptr, root.c_str(), &startup,
                              &child);
    // Setup is elevated. Launch as the same desktop user, never as the installer.
    DWORD pid = 0;
    GetWindowThreadProcessId(GetShellWindow(), &pid);
    HANDLE shell = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    HANDLE shellToken = nullptr, currentToken = nullptr;
    bool ok = shell && OpenProcessToken(shell, TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, &shellToken) &&
              OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &currentToken) && sameUser(shellToken, currentToken);
    if (ok)
        ok = CreateProcessWithTokenW(shellToken, 0, target.c_str(), command.data(), 0, nullptr, root.c_str(), &startup,
                                     &child);
    if (currentToken)
        CloseHandle(currentToken);
    if (shellToken)
        CloseHandle(shellToken);
    if (shell)
        CloseHandle(shell);
    return ok;
#endif
}

int run()
{
    wchar_t executable[32768] = {};
    const DWORD length = GetModuleFileNameW(nullptr, executable, 32768);
    if (!length || length >= 32768)
        return fail(L"The installation path could not be read.");
    wchar_t absolute[32768] = {};
    const DWORD absoluteLength = GetFullPathNameW(executable, 32768, absolute, nullptr);
    if (!absoluteLength || absoluteLength >= 32768)
        return fail(L"The installation path could not be resolved.");
    // NSIS shortcuts may contain \\.\\; process image paths are normalized.
    root = std::wstring(absolute, absoluteLength);
    root.resize(root.find_last_of(L"\\/"));
    const HANDLE marker =
        CreateFileW((root + L"\\current.txt").c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (marker == INVALID_HANDLE_VALUE)
        return fail(L"No installed release was selected. Run the Bifrost installer again.");
    char bytes[64] = {};
    DWORD read = 0;
    const bool ok = ReadFile(marker, bytes, sizeof(bytes), &read, nullptr);
    CloseHandle(marker);
    std::string version(bytes, read);
    while (!version.empty() && (version.back() == '\r' || version.back() == '\n'))
        version.pop_back();
    if (!ok || !releaseName(version))
        return fail(L"The installed release marker is invalid. Run the installer again.");
    target = root + L"\\versions\\" + std::wstring(version.begin(), version.end()) + L"\\bin\\";
#ifdef BIFROST_MCP_HOST
    target += L"bifrost-controller-mcp.exe";
#else
    target += L"bifrost-controller.exe";
#endif
    if (GetFileAttributesW(target.c_str()) == INVALID_FILE_ATTRIBUTES)
        return fail(L"The selected release is incomplete. Run the installer again.");

    std::wstring command = quote(target.c_str());
    int argc = 0;
    wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv)
        return fail(L"The launch arguments could not be read.");
    for (int i = 1; i < argc; ++i)
        command += L" " + quote(argv[i]);
    LocalFree(argv);
#ifndef BIFROST_MCP_HOST
    EnumWindows(findMapper, 0);
    if (running)
    {
        const int choice = MessageBoxW(nullptr,
                                       L"The update is installed. Restart Bifrost Controller now to use it?\n\n"
                                       L"You can save or cancel any unsaved mapping changes. Codex can stay open.",
                                       L"Bifrost Controller updated", MB_YESNO | MB_ICONINFORMATION | MB_DEFBUTTON1);
        if (choice != IDYES)
        {
            CloseHandle(running);
            return 0;
        }
        // New releases bypass Close to tray. Legacy releases use their safe close flow.
        const UINT restartMessage = RegisterWindowMessageW(L"Bifrost.Controller.RestartForUpdate.v1");
        DWORD_PTR supported = 0;
        const bool newRelease =
            SendMessageTimeoutW(runningWindow, restartMessage, 0, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 2000, &supported) &&
            supported == 1;
        if (!newRelease)
            PostMessageW(runningWindow, WM_CLOSE, 0, 0);
        const DWORD result = WaitForSingleObject(running, 60000);
        CloseHandle(running);
        if (result != WAIT_OBJECT_0)
        {
            MessageBoxW(nullptr,
                        L"The update is installed and your running app was kept open.\n\n"
                        L"If you cancelled saving, keep working. Otherwise choose Bifrost > Quit, then open "
                        L"Bifrost from its updated Start menu shortcut. Codex can stay open.",
                        L"Restart when ready", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
    }
#endif
    PROCESS_INFORMATION child = {};
    if (!launch(command, child))
        return fail(L"The update is installed, but could not be opened from setup. Open Bifrost Controller "
                    L"from its Start menu shortcut.");
    CloseHandle(child.hThread);
#ifdef BIFROST_MCP_HOST
    WaitForSingleObject(child.hProcess, INFINITE);
    DWORD result = 1;
    GetExitCodeProcess(child.hProcess, &result);
    CloseHandle(child.hProcess);
    return static_cast<int>(result);
#else
    CloseHandle(child.hProcess);
    return 0;
#endif
}
} // namespace

#ifdef BIFROST_MCP_HOST
int main() { return run(); }
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return run(); }
#endif
