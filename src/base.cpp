#include "base.h"
#include "global.h"

namespace base {
    extern "C" {
    uintptr_t g_ParseFontFile_RetAddr;
    }


/**
 作用：在给CEU3BitmapFont分配内存时替换operator new函数调用，将其分配空间为0x3538扩充至0x86AC8，以包容双字节文本。同时清零分配到的内存空间，
 否则会有奇奇怪怪的问题（分配合适小内存就崩溃，分配超大了就没事，最后发现是分配大空间时系统会自动清零）。
 */
    void *proxy_CEU3Graphics_ReadGameSpecific_Operator_new(unsigned long size) {
        //安全大小计算：原始大小0x3538，双字节最大值(0xFFFF + (0x3538 / 8)) * 8 + 0x3538
        void *address = operator new(0x86AC8);
        memset(address, 0, 0x86AC8);
        return address;
    }

/**
 修改函数：CEU3Graphics::ReadGameSpecific
 作用：CEU3BitmapFont原始大小为0x3538，本补丁将其扩充至0x86AC8以支持双字节文本。其E8偏移处指向CBitmapCharacterSet类，该类第一个成员是大小为256的数组，每个数组成员占8字节指向CBitmapCharacter类。
 */
    void install_CEU3Graphics_ReadGameSpecific() {
        TRACK_FUNCTION();
        std::string pattern = "BF 38 35 00 00";
        uintptr_t matchAddress = ScanMainModule(pattern);
        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t address = matchAddress;
        address = address + 5;
        ReplaceCall(address, (uintptr_t) proxy_CEU3Graphics_ReadGameSpecific_Operator_new);
        printf("eu4dll_mac [Success] %s ReplaceCall 匹配地址:0x%lx 写入地址:0x%lx\n", __func__, matchAddress, address);
        SET_SUCCESS();

    }

/**
 修改函数：CBitmapFont::ParseFontFile
 作用：原函数在加载字体文件.fnt时会过滤大于0xFF(255)的字体信息，此补丁将其上限修改为0xFFFF(65535)以支持双字节文本
 */
    void install_CBitmapFont_ParseFontFile_1() {
        TRACK_FUNCTION();
        std::string pattern = "41 81 FE FF 00 00 00 0F 87 F8 01 00 00";
        uintptr_t matchAddress = ScanMainModule(pattern);
        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }

        uint8_t patchByte = 0xFF;
        uintptr_t writeAddress = matchAddress + 4;
        WriteMemory(writeAddress, &patchByte, 1);
        printf("eu4dll_mac [Success] %s WriteMemory 匹配地址:0x%lx 写入地址:0x%lx\n", __func__, matchAddress,
               writeAddress);
        SET_SUCCESS();
    }

    __attribute__((naked)) void naked_CBitmapFont_ParseFontFile_2() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "cmp r14d, 256\n"
                "jb 1f \n"
                "add r14d, 1712\n"

                "1: \n"
                "mov ecx, r14d \n"
                "mov rax, [rbp - 0xD10] \n"
                "jmp qword ptr [rip + _g_ParseFontFile_RetAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CBitmapFont::ParseFontFile
 作用：CEU3BitmapFont原始大小为0x3538，扩充其内存大小后为避免查询字体编码信息时访问到0x3538以内的地址，为编码大于256的字体信息添加安全距离(0x3538 / 8)
 */
    void install_CBitmapFont_ParseFontFile_2() {
        TRACK_FUNCTION();
        uintptr_t matchAddress = ScanMainModule("44 89 F1 48 8B 85 F0 F2 FF FF");
        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t address = matchAddress;
        g_ParseFontFile_RetAddr = address + 10;
        HookJMP(address, (uintptr_t) naked_CBitmapFont_ParseFontFile_2);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__, matchAddress,
               address, g_ParseFontFile_RetAddr);
        OptimizeNakedHook((uintptr_t) naked_CBitmapFont_ParseFontFile_2);
        SET_SUCCESS();
    }


/**
 修改函数：CTextureHandler::LoadTexture
 作用：原函数在加载字体文件.dds时设置了上限16MB，此补丁将其修改为64MB。FF FF FF 00 → FF FF FF 03
 */
    void install_CTextureHandler_LoadTexture() {
        TRACK_FUNCTION();
        uint8_t patch = 0x03;
        uintptr_t matchAddress = ScanMainModule("89 4D CC 81 FB FF FF FF 00");
        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t address = matchAddress;
        address = address + 8;
        WriteMemory(address, &patch, 1);
        printf("eu4dll_mac [Success] %s WriteMemory#1 匹配地址:0x%lx 写入地址:0x%lx\n", __func__, matchAddress,
               address);

        matchAddress = ScanMainModule("41 81 FC FF FF FF 00 76");
        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        address = matchAddress;
        address = address + 6;
        WriteMemory(address, &patch, 1);
        printf("eu4dll_mac [Success] %s WriteMemory#2 匹配地址:0x%lx 写入地址:0x%lx\n", __func__, matchAddress,
               address);
        SET_SUCCESS();
    }


    void install() {
        //修改CEU3BitmapFont类初始大小，使其能容纳双字节文本
        install_CEU3Graphics_ReadGameSpecific();
        //允许录入编码大于255字符信息
        install_CBitmapFont_ParseFontFile_1();
        //为字体编码添加安全距离
        install_CBitmapFont_ParseFontFile_2();
        //扩充字体文件大小上限至64MB
        install_CTextureHandler_LoadTexture();

        g_CString_AppendCharAddress = findFn("_ZN7CStringpLEc");
        g_CString_AppendCharConstAddress = findFn("_ZN7CStringpLEPKc");

    }
}
