#include "MemoryFunctions.h"






FILE* f;
bool running = true;
short MemoryFunctions::read2Bytes(uintptr_t address) {
    return *(short*)address;
}
namespace sdk {
    namespace utilities {
        HMODULE h_module;

        void set_console_size(int width, int height) {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            COORD newSize;
            newSize.X = width;
            newSize.Y = height;
            SetConsoleScreenBufferSize(hConsole, newSize);
            SMALL_RECT rect;
            rect.Left = 0;
            rect.Top = 0;
            rect.Right = width - 1;
            rect.Bottom = height - 1;
            SetConsoleWindowInfo(hConsole, TRUE, &rect);
        }

        void setup_console(const char* title) {
            AllocConsole();
            freopen_s(&f, "CONOUT$", "w", stdout);
        }
    }
}

void MemoryFunctions::EnableVTMode() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}


uint16_t* MemoryFunctions::readchain(uintptr_t baseAddress, const std::vector<uint32_t>& offsets) {
    uintptr_t currentAddress = baseAddress;
    for (size_t i = 0; i < offsets.size(); ++i) {
        currentAddress = *(uintptr_t*)currentAddress;
        currentAddress += offsets[i];
    }
    return (uint16_t*)currentAddress;
}
std::string MemoryFunctions::readString(const uintptr_t address, size_t maxLength) {
    std::string result;
    if (IsBadReadPtr((const void*)address, maxLength)) {
        return result;
    }
    char currentChar = 0;
    for (size_t i = 0; i < maxLength; ++i) {
        currentChar = read<char>(address + i);
        if (currentChar == '\0') {
            break;
        }
        result += currentChar;
    }
    return result;
}
std::string MemoryFunctions::ReadStringFromMemory(uintptr_t baseAddress, uint32_t offset) {
    char* stringPtr = (char*)(baseAddress + offset);
    if (stringPtr) {
        return std::string(stringPtr);
    }
    return "";
}
std::wstring MemoryFunctions::ReadWideStringFromMemory(uintptr_t baseAddress, uint32_t offset) {
    wchar_t* stringPtr = (wchar_t*)(baseAddress + offset);
    if (stringPtr) {
        return std::wstring(stringPtr);
    }
    return L"";
}
std::wstring MemoryFunctions::UTF8ToWideString(const char* utf8String) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, &wstrTo[0], size_needed);
    return wstrTo;
}
std::wstring MemoryFunctions::ReadUTF8StringFromMemory(uintptr_t baseAddress, uint32_t offset) {
    const char* stringPtr = (const char*)(baseAddress + offset);
    if (stringPtr) {
        return UTF8ToWideString(stringPtr);
    }
    return L"";
}
std::string MemoryFunctions::ReadANSIStringFromMemory(uintptr_t baseAddress, uint32_t offset) {
    const char* stringPtr = (const char*)(baseAddress + offset);
    if (stringPtr) {
        return std::string(stringPtr);
    }
    return "";
}
void MemoryFunctions::PrintRawMemory(uintptr_t baseAddress, uint32_t offset, size_t length) {
    unsigned char* ptr = (unsigned char*)(baseAddress + offset);
    for (size_t i = 0; i < length; ++i) {
        printf("%02X ", ptr[i]);
    }
    printf("\n");
}
