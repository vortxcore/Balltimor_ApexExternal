#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <TlHelp32.h>
#include <intrin.h>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "skcrypter.h"

inline HWND hwnd = NULL;
inline uint64_t get_cr3 = 0;

inline uintptr_t GameBase;
inline INT32 get_process_id;

struct HandleDisposer
{
    using pointer = HANDLE;
    void operator()(HANDLE handle) const
    {
        if (handle != NULL && handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle);
        }
    }
};

using unique_handle = std::unique_ptr<HANDLE, HandleDisposer>;

namespace mem {

    inline std::uint32_t GetPID(std::string_view process_name)
    {
        PROCESSENTRY32 processentry;
        processentry.dwSize = sizeof(PROCESSENTRY32);

        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (hSnap == INVALID_HANDLE_VALUE) return 0;

        unique_handle snapshot_handle(&hSnap);

        while (Process32Next(hSnap, &processentry) == TRUE)
        {
            if (process_name == processentry.szExeFile)
            {
                return processentry.th32ProcessID;
            }
        }
        return 0;
    }

    inline bool initdriver() {
        return false;
    }

    inline bool find_base_address() {
        return false;
    }

    inline bool get_cr3() {
        return false;
    }

    template <typename T>
    T Read1(uintptr_t address) {
        return T{};
    }

    inline std::string ReadString(uintptr_t address, size_t max_size) {
        return "Unknown";
    }
}

enum ConsoleColor {
    WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    RED = FOREGROUND_RED | FOREGROUND_INTENSITY,
    GREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
};

void SetColor(HANDLE hConsole, ConsoleColor color) {
    SetConsoleTextAttribute(hConsole, color);
}