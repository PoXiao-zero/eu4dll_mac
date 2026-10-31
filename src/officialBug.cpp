#include "officialBug.h"
#include "global.h"

namespace officialBug {

    extern "C" uintptr_t g_CDependency_OnDisable_Ret = 0;

    __attribute__((naked)) void naked_CDependency_OnDisable() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                "mov rdx, [rbp-0x30] \n"
                "mov rbx, [rdx+0x1908] \n"

                "test rbx, rbx \n"

                "jmp [rip + _g_CDependency_OnDisable_Ret] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CDependency::OnDisable 转移附庸时，移除支持独立的外交关系
 作用：解决游戏官方BUG。如果吞并一个国家，且其有附庸国而且这个附庸国有外交支持独立就会触发崩溃。
 */

    void install_CDependency_OnDisable() {
        TRACK_FUNCTION();
        uintptr_t matchAddress = ScanMainModule(
                "48 8B 5B 10 48 85 DB 74 14 49 8B 04 24 48 8B 88 B8 00 00 00");
        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t address = matchAddress;
        g_CDependency_OnDisable_Ret = address + 7;
        HookJMP(address, (uintptr_t) naked_CDependency_OnDisable);
        OptimizeNakedHook((uintptr_t) naked_CDependency_OnDisable);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx 写入地址:0x%lx 返回地址:0x%lx\n",
               __func__,
               matchAddress, address, g_CDependency_OnDisable_Ret);
        SET_SUCCESS();
    }

    void install() {
        install_CDependency_OnDisable();
    }
}