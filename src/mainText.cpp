#include "mainText.h"
#include "global.h"

namespace mainText {
    extern "C" {
    uintptr_t g_RenderToScreen_3_RetAddr = 0;
    uintptr_t g_RenderToScreen_3_BypassAddr = 0;

    uintptr_t g_RenderToScreen_1_RetAddr = 0;
    uintptr_t g_RenderToScreen_1_BypassAddr = 0;
    uint32_t g_RenderToScreen_1_CurrentChar = 0;

    uintptr_t g_RenderToScreen_2_RetAddr = 0;
    uintptr_t g_RenderToScreen_2_BypassAddr = 0;

    }

    __attribute__((naked)) void naked_CBitmapFont_RenderToScreen_1() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "movzx edx, byte ptr [rdi+r11] \n"

                "cmp dl, %c[e1] \n"
                "jz 1f \n"
                "cmp dl, %c[e2] \n"
                "jz 2f \n"
                "cmp dl, %c[e3] \n"
                "jz 3f \n"
                "cmp dl, %c[e4] \n"
                "jz 4f \n"

                "mov dword ptr [rip + _g_RenderToScreen_1_CurrentChar], 1\n"
                "jmp qword ptr [rip + _g_RenderToScreen_1_RetAddr] \n"

                "1: \n"
                "movzx edx, word ptr [rdi+r11+1] \n"
                "jmp 5f \n"

                "2: \n"
                "movzx edx, word ptr [rdi+r11+1] \n"
                "sub edx, %c[s2] \n"
                "jmp 5f \n"

                "3: \n"
                "movzx edx, word ptr [rdi+r11+1] \n"
                "add edx, %c[s3] \n"
                "jmp 5f \n"

                "4: \n"
                "movzx edx, word ptr [rdi+r11+1] \n"
                "add edx, %c[s4] \n"

                "5: \n"
                "mov dword ptr [rip + _g_RenderToScreen_1_CurrentChar], edx \n"

                "mov ax, [rdi+r11+1] \n" // 读取标识符后2个字节
                "mov [rsi+rcx+1], ax \n" // 写入缓存区 [rbx+1]

                //        "mov al, [rdi+r11+1]\n" // 读取标识符后第 1 个字节
                //        "mov [rsi+rcx+1], al\n" // 写入缓存区 [rbx+1]
                //        "mov al, [rdi+r11+2]\n" // 读取标识符后第 2 个字节
                //        "mov [rsi+rcx+2], al\n" // 写入缓存区 [rbx+2]

                "add r13d, 2 \n" // 源字符串读取索引 + 2
                "add r12d, 2 \n" // 缓存区索引 + 2

                "cmp edx, 256 \n"
                "jb 7f \n"
                "add edx, 1712 \n"
                "7: \n"
                "jmp qword ptr [rip + _g_RenderToScreen_1_BypassAddr] \n"

                ".att_syntax prefix \n"
                :
                : [e1] "i"(ESCAPE_SEQ_1),
        [e2] "i"(ESCAPE_SEQ_2),
        [e3] "i"(ESCAPE_SEQ_3),
        [e4] "i"(ESCAPE_SEQ_4),
        [s2] "i"(SHIFT_2),
        [s3] "i"(SHIFT_3),
        [s4] "i"(SHIFT_4),
        [nf] "i"(NO_FONT),
        [nd] "i"(NOT_DEF)
        );
    }

/**
 Hook函数：CBitmapFont::RenderToScreen 字符预处理循环
 作用：使其能正确识别双字节文本
 */
    void install_CBitmapFont_RenderToScreen_1() {
        TRACK_FUNCTION();
        std::string pattern = "42 0F B6 14 1F 80 7D 38 00";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_RenderToScreen_1_RetAddr = leaAddress + 5;
        g_RenderToScreen_1_BypassAddr = leaAddress + 0x57;
        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_RenderToScreen_1);

        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx 返回地址2:0x%lx\n",
               __func__,
               matchAddress, leaAddress, g_RenderToScreen_1_RetAddr, g_RenderToScreen_1_BypassAddr);
        SET_SUCCESS();
    }

    __attribute__((naked)) void naked_CBitmapFont_RenderToScreen_2() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "cmp dword ptr [_g_RenderToScreen_1_CurrentChar + rip], 0xFF \n"
                "ja 1f \n"

                "cmp word ptr [r14+6], 0 \n"
                "jmp qword ptr [rip + _g_RenderToScreen_2_RetAddr] \n"

                "1: \n"
                "jmp qword ptr [rip + _g_RenderToScreen_2_BypassAddr] \n"

                ".att_syntax prefix \n"
                :
                : [e1] "i"(ESCAPE_SEQ_1),
        [e2] "i"(ESCAPE_SEQ_2),
        [e3] "i"(ESCAPE_SEQ_3),
        [e4] "i"(ESCAPE_SEQ_4),
        [s2] "i"(SHIFT_2),
        [s3] "i"(SHIFT_3),
        [s4] "i"(SHIFT_4),
        [nf] "i"(NO_FONT),
        [nd] "i"(NOT_DEF)
        );
    }

/**
 Hook函数：CBitmapFont::RenderToScreen 字符预处理循环
 作用：双字节文本时强制检测是否换行
 */
    void install_CBitmapFont_RenderToScreen_2() {
        TRACK_FUNCTION();
        std::string pattern = "66 41 83 7E 06 00 0F 84";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_RenderToScreen_2_RetAddr = leaAddress + 6;
        g_RenderToScreen_2_BypassAddr = leaAddress + 0x297;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_RenderToScreen_2);

        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx 返回地址2:0x%lx\n",
               __func__,
               matchAddress, leaAddress, g_RenderToScreen_2_RetAddr, g_RenderToScreen_2_BypassAddr);
        SET_SUCCESS();
    }

    __attribute__((naked)) void naked_CBitmapFont_RenderToScreen_3() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "lea rbx, [r12+rax] \n"
                "movzx eax, byte ptr [rbx] \n"

                "cmp al, %c[e1] \n"
                "jz 1f \n"
                "cmp al, %c[e2] \n"
                "jz 2f \n"
                "cmp al, %c[e3] \n"
                "jz 3f \n"
                "cmp al, %c[e4] \n"
                "jz 4f \n"

                "jmp qword ptr [rip + _g_RenderToScreen_3_RetAddr] \n"

                "1: \n"
                "movzx eax, word ptr [rbx+1] \n"
                "jmp 5f \n"

                "2: \n"
                "movzx eax, word ptr [rbx+1] \n"
                "sub eax, %c[s2] \n"
                "jmp 5f \n"

                "3: \n"
                "movzx eax, word ptr [rbx+1] \n"
                "add eax, %c[s3] \n"
                "jmp 5f \n"

                "4: \n"
                "movzx eax, word ptr [rbx+1] \n"
                "add eax, %c[s4] \n"

                // 通用收尾
                "5: \n"
                "add r15d, 2 \n"         // 增加循环总计数
                "add r12, 2 \n"         // 同步增加当前迭代的索引

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"
                "7: \n"
                "jmp qword ptr [rip + _g_RenderToScreen_3_BypassAddr] \n"

                ".att_syntax prefix \n"
                :
                : [e1] "i"(ESCAPE_SEQ_1),
        [e2] "i"(ESCAPE_SEQ_2),
        [e3] "i"(ESCAPE_SEQ_3),
        [e4] "i"(ESCAPE_SEQ_4),
        [s2] "i"(SHIFT_2),
        [s3] "i"(SHIFT_3),
        [s4] "i"(SHIFT_4),
        [nf] "i"(NO_FONT),
        [nd] "i"(NOT_DEF)
        );
    }

/**
 Hook函数：CBitmapFont::RenderToScreen 渲染循环
 作用：使其能正确识别双字节文本
 */
    void install_CBitmapFont_RenderToScreen_3() {
        TRACK_FUNCTION();
        std::string pattern = "41 0F B6 04 04 80 7D 38 00";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_RenderToScreen_3_RetAddr = leaAddress + 5;
        g_RenderToScreen_3_BypassAddr = leaAddress + 0x1B4;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_RenderToScreen_3);

        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx 返回地址2:0x%lx\n",
               __func__,
               matchAddress, leaAddress, g_RenderToScreen_3_RetAddr, g_RenderToScreen_3_BypassAddr);
        SET_SUCCESS();
    }

    void install() {
        //文本预处理
        install_CBitmapFont_RenderToScreen_1();
        //双字节文本时强制检查是否换行
        install_CBitmapFont_RenderToScreen_2();
        //渲染
        install_CBitmapFont_RenderToScreen_3();
    }

}
