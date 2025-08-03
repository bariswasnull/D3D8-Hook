#pragma once
// Windows Üst Bilgi Dosyalarý
#include <windows.h>
#include <iostream>
#include <string>
#include "SharedMem.h"
#include "MathObjects.h"



extern FILE* f;
namespace sdk {
    namespace utilities {
        extern HMODULE h_module;
        void set_console_size(int width, int height);
        void setup_console(const char* title = "ADMINMODE");
    }
}
class MemoryFunctions : protected SharedMem
{
protected:
    void EnableVTMode();

    static short read2Bytes(uintptr_t address);
    static uint16_t* readchain(uintptr_t baseAddress, const std::vector<uint32_t>& offsets);
    static std::string readString(const uintptr_t address, size_t maxLength = 32);
    static std::string ReadStringFromMemory(uintptr_t baseAddress, uint32_t offset);
    static std::wstring ReadWideStringFromMemory(uintptr_t baseAddress, uint32_t offset);
    static std::wstring ReadUTF8StringFromMemory(uintptr_t baseAddress, uint32_t offset);
    static std::string ReadANSIStringFromMemory(uintptr_t baseAddress, uint32_t offset);
    static std::wstring UTF8ToWideString(const char* utf8String);
    static void PrintRawMemory(uintptr_t baseAddress, uint32_t offset, size_t length);

    template <typename T>
    static T read(const uintptr_t address) {
        T Novalue = {};
        if (!IsBadReadPtr((const void*)address, sizeof(T))) {
            return *(T*)(address);
        }
        else {
            return Novalue;
        }
    }

    template <typename T>
    static T readvec3(const uintptr_t address) {
        if (address == 0) {
            return T();
        }
        return *(T*)(address);
    }
    template <typename T>
    static void write(const uintptr_t address, T value) {
        if (!IsBadWritePtr((LPVOID)address, sizeof(T))) {
            *(T*)(address) = value;
        }
        else {
            printf("Belleðe yazýlamýyor: Geçersiz adres 0x%p\n", (void*)address);
        }
    }
};