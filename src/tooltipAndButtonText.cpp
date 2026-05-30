#include "tooltipAndButtonText.h"
#include "global.h"

namespace tooltipAndButtonText {
    extern "C" {

    uintptr_t g_RenderToTexture_1_BypassAddr = 0;
    uint32_t g_RenderToTexture_1_CurrentChar = 0;

    uintptr_t g_RenderToTexture_3_BypassAddr = 0;

    uintptr_t g_RenderToTexture_2_RetAddr = 0;
    uintptr_t g_RenderToTexture_2_BypassAddr = 0;
    }

    __attribute__((naked)) void naked_CBitmapFont_RenderToTexture_1() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "mov rbx,rax \n"

                "cmp byte ptr [rax], %c[e1] \n"
                "jz 1f \n"
                "cmp byte ptr [rax], %c[e2] \n"
                "jz 2f \n"
                "cmp byte ptr [rax], %c[e3] \n"
                "jz 3f \n"
                "cmp byte ptr [rax], %c[e4] \n"
                "jz 4f \n"

                "movzx eax, byte ptr [rax] \n"
                "jmp 7f \n"

                "1: \n"
                "movzx eax, word ptr [rax+1] \n"
                "jmp 5f \n"

                "2: \n"
                "movzx eax, word ptr [rax+1] \n"
                "sub eax, %c[s2] \n"
                "jmp 5f \n"

                "3: \n"
                "movzx eax, word ptr [rax+1] \n"
                "add eax, %c[s3] \n"
                "jmp 5f \n"

                "4: \n"
                "movzx eax, word ptr [rax+1] \n"
                "add eax, %c[s4] \n"

                "5: \n"

                "push rax \n"
                "lea rdi, [rbp - 0x2740] \n"
                "movzx rsi, byte ptr [rbx+1] \n"
                "call [rip + _g_CString_AppendCharAddress] \n"

                "lea rdi, [rbp - 0x2740] \n"
                "movzx rsi, byte ptr [rbx+2] \n"
                "call [rip + _g_CString_AppendCharAddress] \n"

                "pop rax \n"
                "add r12d, 2 \n" // 增加循环总计数


                "cmp eax, 256\n"
                "jb 7f \n"
                "add eax, 1712\n"
                "7: \n"
                "mov dword ptr [rip + _g_RenderToTexture_1_CurrentChar], eax\n"
                "mov rbx, qword ptr [r15+rax*8+0xE8]\n"

                "jmp qword ptr [rip + _g_RenderToTexture_1_BypassAddr] \n"

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
 Hook函数：CBitmapFont::RenderToTexture 字符预处理循环
 作用：使其正确识别双字节
 */
    void install_CBitmapFont_RenderToTexture_1() {
        TRACK_FUNCTION();
        std::string pattern = "0F B6 00 49 8B 9C C7 E8 00 00 00";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_RenderToTexture_1_BypassAddr = leaAddress + 0xB;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_RenderToTexture_1);

        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_RenderToTexture_1_BypassAddr);
        SET_SUCCESS();
    }

    __attribute__((naked)) void naked_CBitmapFont_RenderToTexture_2() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "cmp dword ptr [_g_RenderToTexture_1_CurrentChar + rip], 0xFF \n"
                "ja 1f \n"
                "cmp word ptr [rbx+6], 0 \n"
                "jmp qword ptr [rip + _g_RenderToTexture_2_RetAddr] \n"

                "1: \n"
                "jmp qword ptr [rip + _g_RenderToTexture_2_BypassAddr] \n"

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
 Hook函数：CBitmapFont::RenderToTexture 字符预处理循环
 作用：强制每个双字节字符都检查是否换行
 */
    void install_CBitmapFont_RenderToTexture_2() {
        TRACK_FUNCTION();
        std::string pattern = "66 83 7B 06 00 74 08 4D 89 EE";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_RenderToTexture_2_RetAddr = leaAddress + 5;
        g_RenderToTexture_2_BypassAddr = leaAddress + 0xF;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_RenderToTexture_2);

        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx 返回地址2:0x%lx\n",
               __func__,
               matchAddress, leaAddress, g_RenderToTexture_2_RetAddr, g_RenderToTexture_2_BypassAddr);
        SET_SUCCESS();
    }


    __attribute__((naked)) void naked_CBitmapFont_RenderToTexture_3() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "cmp byte ptr [rax], %c[e1] \n"
                "jz 1f \n"
                "cmp byte ptr [rax], %c[e2] \n"
                "jz 2f \n"
                "cmp byte ptr [rax], %c[e3] \n"
                "jz 3f \n"
                "cmp byte ptr [rax], %c[e4] \n"
                "jz 4f \n"

                "movzx eax, byte ptr [rax] \n"
                "jmp 7f \n"

                "1: \n"
                "movzx eax, word ptr [rax+1] \n"
                "jmp 5f \n"

                "2: \n"
                "movzx eax, word ptr [rax+1] \n"
                "sub eax, %c[s2] \n"
                "jmp 5f \n"

                "3: \n"
                "movzx eax, word ptr [rax+1] \n"
                "add eax, %c[s3] \n"
                "jmp 5f \n"

                "4: \n"
                "movzx eax, word ptr [rax+1] \n"
                "add eax, %c[s4] \n"

                "5: \n"

                "add r12d, 2 \n" // 增加循环总计数

                "cmp eax, 256\n"
                "jb 7f \n"
                "add eax, 1712\n"

                "7: \n"
                "mov r10, qword ptr [r14+rax*8+0xE8]\n"
                "jmp qword ptr [rip + _g_RenderToTexture_3_BypassAddr] \n"

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
 Hook函数：CBitmapFont::RenderToTexture 渲染循环
 作用：部分UI组件使用这个渲染函数
 */
    void install_CBitmapFont_RenderToTexture_3() {
        TRACK_FUNCTION();
        std::string pattern = "0F B6 00 4D 8B 94 C6 E8 00 00 00";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_RenderToTexture_3_BypassAddr = leaAddress + 0xB;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_RenderToTexture_3);

        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_RenderToTexture_3_BypassAddr);
        SET_SUCCESS();
    }

    void install() {
        //预处理
        install_CBitmapFont_RenderToTexture_1();
        //双字节文本时强制检查是否换行
        install_CBitmapFont_RenderToTexture_2();
        //渲染
        install_CBitmapFont_RenderToTexture_3();
    }
}
