#include "CloseWindow.h"

#include <string>
#include <io.h>
#include <fcntl.h>

#include <Windows.h>

namespace
{
    BOOL CALLBACK enumByClassName(HWND hwnd, LPARAM lParam) {
        const DWORD NAME_SIZE = 1024;
        WCHAR className[NAME_SIZE];
        GetClassNameW(hwnd, className, NAME_SIZE);

        std::wstring wsTargetWindow = *reinterpret_cast<std::wstring*>(lParam);
        if (wsTargetWindow == className)
        {
            SendMessageW(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
            return FALSE;
        }

        return TRUE;
    }
} // unnamed namespace

bool closeWindowByClassName(const std::wstring& wsTargetWindowClassName)
{
    return EnumWindows(enumByClassName, reinterpret_cast<LPARAM>(&wsTargetWindowClassName));
}
