
#include "saveFileName.h"
#include "global.h"
#include "strConvert.h"

namespace saveFileName {

/**
 Hook函数：CString::RemoveSpecialCharacters
 作用：阻止其过滤字符串，直接返回
 */
    void install_CString_RemoveSpecialCharacters() {
        TRACK_FUNCTION();
        void *address = findFn("_ZN7CString23RemoveSpecialCharactersEv");
        if (address == nullptr) {
            printf("eu4dll_mac [Error] %s 函数地址查找失败！\n", __func__);
            return;
        }
        uint8_t patch[] = {0xC3, 0x90, 0x90, 0x90};
        WriteMemory((uintptr_t) address, patch, 4);
        printf("eu4dll_mac [Success] %s WriteMemory 写入地址:%p\n", __func__, address);
        SET_SUCCESS();
    }

    extern "C" uintptr_t g_SaveGame_RetAddr = 0;

    __attribute__((naked)) void naked_CIngameSaveMenu_SaveGame() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "call [rip + _escapedStrToUtf8ReplaceAddress] \n"
                "jmp [rip + _g_SaveGame_RetAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CIngameSaveMenu::SaveGame
 作用：将获取到的存档名从带标识符的双字节逃逸文本（UTF16LE）转换为UTF8格式，使其能保持正确的名字存储在硬盘上。
 */
    void install_CIngameSaveMenu_SaveGame() {
        TRACK_FUNCTION();
        std::string pattern = "48 89 DF E8 ? ? ? ? 48 8D BD 60 FF FF FF 48 8D 75 A8";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress += 3;
        g_SaveGame_RetAddr = leaAddress + 5;
        HookJMP(leaAddress, (uintptr_t) naked_CIngameSaveMenu_SaveGame);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_SaveGame_RetAddr);
        SET_SUCCESS();
    }

    extern "C" uintptr_t g_CLocalSavegameItem_RetAddr = 0;


    __attribute__((naked)) void naked_CLocalSavegameItem_CLocalSavegameItem() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                "mov rdi, r12 \n"
                "call [rip + _utf8ToEscapedStrReplaceAddress] \n"
                "mov rdi, r14 \n"
                "mov rsi, r12 \n"
                "jmp [rip + _g_CLocalSavegameItem_RetAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CLocalSavegameItem::CLocalSavegameItem
 作用：HOOK初始化函数，将从硬盘上获取到的UTF8存档文件名转换为逃逸文本，使其能正确显示在存档列表
 */
    void install_CLocalSavegameItem_CLocalSavegameItem() {
        TRACK_FUNCTION();
        std::string pattern = "49 89 C7 4D 85 F6 74 ? 4C 89 F7 4C 89 E6 31 D2 E8";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 8;
        g_CLocalSavegameItem_RetAddr = leaAddress + 6;
        HookJMP(leaAddress, (uintptr_t) naked_CLocalSavegameItem_CLocalSavegameItem);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_CLocalSavegameItem_RetAddr);
        SET_SUCCESS();
    }

    extern "C" uintptr_t g_CConfirmSave_RetAddr = 0;

    __attribute__((naked)) void naked_CConfirmSave_CConfirmSave() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                "mov rdi, r14 \n"
                "call [rip + _utf8ToEscapedStrReplaceAddress] \n"
                "mov rdi, [r12+8] \n"
                "jmp [rip + _g_CConfirmSave_RetAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CConfirmSave::CConfirmSave
 作用：存档覆盖提示的UTF8文件名转换为逃逸文本
 */
    void install_CConfirmSave_CConfirmSave() {
        TRACK_FUNCTION();
        std::string pattern = "49 8B 7C 24 08 48 8B 07 48 8D 35 ? ? ? ? FF 90 C0 00 00 00 48 89 C3 48 8D 35 S S S S 48 8D 15";
        uintptr_t matchAddress = ScanMainModule(pattern, {"CONFIRMSAVETEXT"},
                                                "_ZN12CConfirmSaveC2EP9CEU3IdlerRK7CStringP21CloudFileCLOUDSTORAGEbbRKNSt3__16vectorIS2_NS7_9allocatorIS2_EEEE",
                                                1024);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_CConfirmSave_RetAddr = leaAddress + 5;
        HookJMP(leaAddress, (uintptr_t) naked_CConfirmSave_CConfirmSave);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_CConfirmSave_RetAddr);
        SET_SUCCESS();
    }


    extern "C" uintptr_t g_UpdateHeaderInfo_RetAddr = 0;

    __attribute__((naked)) void naked_CLocalSavegameItem_UpdateHeaderInfo() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                "lea rdi, [rax+0x220] \n"
                "call [rip + _utf8ToEscapedStrAddress] \n"
                "mov rsi, rax \n"
                "jmp [rip + _g_UpdateHeaderInfo_RetAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CLocalSavegameItem::UpdateHeaderInfo
 作用：将存档列表鼠标悬浮提示的UTF8文件名转换为逃逸文本
 */
    void install_CLocalSavegameItem_UpdateHeaderInfo() {
        TRACK_FUNCTION();
        std::string pattern = "48 8D B0 20 02 00 00 48 8D 7D 90 E8";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_UpdateHeaderInfo_RetAddr = leaAddress + 7;
        HookJMP(leaAddress, (uintptr_t) naked_CLocalSavegameItem_UpdateHeaderInfo);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_UpdateHeaderInfo_RetAddr);
        SET_SUCCESS();
    }

    extern "C" uintptr_t g_DoLoadGame_RetAddr = 0;
    extern "C" void *g_EU4LoadGameHelper_Load_Addr = nullptr;

    __attribute__((naked)) void naked_CIngameLoadMenu_DoLoadGame() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                "lea rdi, [rbp-0x58] \n"
                "call [rip + _escapedStrToUtf8ReplaceAddress] \n"
                "lea rdi, [rbp-0x58] \n"
                "call [rip + _g_EU4LoadGameHelper_Load_Addr] \n"
                "jmp [rip + _g_DoLoadGame_RetAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CIngameLoadMenu::DoLoadGame
 作用：确认载入存档时将逃逸文本转换为UTF8真实文件名
 */
    void install_CIngameLoadMenu_DoLoadGame() {
        TRACK_FUNCTION();
        std::string pattern = "48 8D 7D A8 E8 ? ? ? ? 48 8D 7D A8 E8 ? ? ? ? 48 8B 7B 30 E8";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 9;
        g_DoLoadGame_RetAddr = leaAddress + 9;
        HookJMP(leaAddress, (uintptr_t) naked_CIngameLoadMenu_DoLoadGame);
        g_EU4LoadGameHelper_Load_Addr = findFn("_ZN17EU4LoadGameHelper4LoadERNS_19SLoadGameParametersE");
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_DoLoadGame_RetAddr);
        SET_SUCCESS();
    }

    extern "C" uintptr_t g_GetCurrentTooltip_RetAddr = 0;

    __attribute__((naked)) void naked_CFrontEnd_GetCurrentTooltip() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                "lea rdi, [rbp-0x2E0] \n"
                "call [rip + _utf8ToEscapedStrAddress] \n"
                "mov rsi, rax \n"
                "lea rdi, [rbp-0xB0] \n"
                "jmp [rip + _g_GetCurrentTooltip_RetAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CFrontEnd::GetCurrentTooltip
 作用：开始界面继续游戏按钮的悬浮提示，将其中的UTF8存档名转换为逃逸文本，使其能正确显示
 */
    void install_CFrontEnd_GetCurrentTooltip() {
        TRACK_FUNCTION();
        std::string pattern = "48 8D BD 50 FF FF FF 48 8D B5 20 FD FF FF E8";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_GetCurrentTooltip_RetAddr = leaAddress + 0xE;
        HookJMP(leaAddress, (uintptr_t) naked_CFrontEnd_GetCurrentTooltip);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_GetCurrentTooltip_RetAddr);
        SET_SUCCESS();
    }


    extern "C" uintptr_t g_CConfirmLocalDeleteInGame_RetAddr = 0;

    __attribute__((naked)) void naked_CConfirmLocalDeleteInGame_CConfirmLocalDeleteInGame() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                "mov rdi, r12 \n"
                "call [rip + _utf8ToEscapedStrAddress] \n"
                "mov r12, rax \n"

                "mov rdi, [rbx+8] \n"
                "mov rax, [rdi] \n"
                "jmp [rip + _g_CConfirmLocalDeleteInGame_RetAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CConfirmLocalDeleteInGame::CConfirmLocalDeleteInGame
 作用：删除存档确认提示框，将其中的UTF8存档名转换为逃逸文本，使其能正确显示
 */
    void install_CConfirmLocalDeleteInGame_CConfirmLocalDeleteInGame() {
        TRACK_FUNCTION();
        std::string pattern = "48 8B 7B 08 48 8B 07 48 8D 35 ? ? ? ? FF 90 C0 00 00 00 48 89 C3 48 8D 35 S S S S 48 8D 15";
        uintptr_t matchAddress = ScanMainModule(pattern, {"CONFIRMDELETETEXT"},
                                                "_ZN25CConfirmLocalDeleteInGameC2EP9CEU3IdlerRK7CStringS4_RKNSt3__18functionIFvvEEE",
                                                1024);
        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_CConfirmLocalDeleteInGame_RetAddr = leaAddress + 7;
        HookJMP(leaAddress, (uintptr_t) naked_CConfirmLocalDeleteInGame_CConfirmLocalDeleteInGame);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_CConfirmLocalDeleteInGame_RetAddr);
        SET_SUCCESS();
    }


    void install() {
        //阻止过滤字符串
        install_CString_RemoveSpecialCharacters();
        //让存档名使用UTF8编码保存
        install_CIngameSaveMenu_SaveGame();
        //让存档列表正确显示
        install_CLocalSavegameItem_CLocalSavegameItem();
        //存档覆盖提示框
        install_CConfirmSave_CConfirmSave();
        //存档列表鼠标悬浮提示
        install_CLocalSavegameItem_UpdateHeaderInfo();
        //确认载入存档提示框
        install_CIngameLoadMenu_DoLoadGame();
        //开始界面继续游戏按钮的悬浮提示
        install_CFrontEnd_GetCurrentTooltip();
        //删除存档确认提示框
        install_CConfirmLocalDeleteInGame_CConfirmLocalDeleteInGame();
    }
}
