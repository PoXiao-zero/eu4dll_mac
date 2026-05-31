#include "text3D.h"
#include "global.h"

namespace text3D {

    extern "C" {
    uintptr_t g_Render3d_1_BypassAddr = 0;

    uintptr_t g_Render3d_2_BypassAddr = 0;
    }

    __attribute__((naked)) void naked_CBitmapFont_Render3d_1() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "mov rdi, rax \n"
                "movzx eax, byte ptr [rax] \n"

                "cmp al, %c[e1] \n"
                "jz 1f \n"
                "cmp al, %c[e2] \n"
                "jz 2f \n"
                "cmp al, %c[e3] \n"
                "jz 3f \n"
                "cmp al, %c[e4] \n"
                "jz 4f \n"

                "jmp 7f \n"

                "1: \n"
                "movzx eax, word ptr [rdi+1] \n"
                "jmp 5f \n"

                "2: \n"
                "movzx eax, word ptr [rdi+1] \n"
                "sub eax, %c[s2] \n"
                "jmp 5f \n"

                "3: \n"
                "movzx eax, word ptr [rdi+1] \n"
                "add eax, %c[s3] \n"
                "jmp 5f \n"

                "4: \n"
                "movzx eax, word ptr [rdi+1] \n"
                "add eax, %c[s4] \n"

                // 通用收尾
                "5: \n"

                "add r12d, 2 \n" // 增加循环总计数

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"
                "7: \n"
                "mov rax, [r15+rax*8+0xE8] \n"
                "jmp qword ptr [rip + _g_Render3d_1_BypassAddr] \n"

                ".att_syntax prefix \n"
                :
                : [e1] "i"(ESCAPE_SEQ_1),
        [e2] "i"(ESCAPE_SEQ_2),
        [e3] "i"(ESCAPE_SEQ_3),
        [e4] "i"(ESCAPE_SEQ_4),
        [s2] "i"(SHIFT_2),
        [s3] "i"(SHIFT_3),
        [s4] "i"(SHIFT_4)
        );
    }

/**
 Hook函数：CBitmapFont::Render3d 渲染循环
 作用：使其支持正确读取双字节字符
 */
    void install_CBitmapFont_Render3d_1() {
        TRACK_FUNCTION();
        std::string pattern = "44 89 E6 E8 ? ? ? ? 0F B6 00 49 8B 84 C7 E8 00 00 00 48 85 C0 0F 84";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 8;
        g_Render3d_1_BypassAddr = leaAddress + 0xB;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_Render3d_1);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_Render3d_1_BypassAddr);
        OptimizeNakedHook((uintptr_t) naked_CBitmapFont_Render3d_1);
        SET_SUCCESS();
    }


    __attribute__((naked)) void naked_CBitmapFont_Render3d_2() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "mov rbx, rax \n" // 保存当前指针，用于后面追加字符写入缓冲区
                "movzx eax, byte ptr [rax] \n"

                "cmp al, %c[e1] \n"
                "jz 1f \n"
                "cmp al, %c[e2] \n"
                "jz 2f \n"
                "cmp al, %c[e3] \n"
                "jz 3f \n"
                "cmp al, %c[e4] \n"
                "jz 4f \n"

                "jmp 7f \n"

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

                "add r15d, 2 \n" // 增加循环总计数

                "push rax \n" //保护RAX寄存器
                "lea rdi, [rbp-0x128] \n" //将缓冲区变量传递当作第一个参数
                "movsx rsi, byte ptr [rbx+1] \n" //将后面一个字节当作第二个参数
                "call qword ptr [rip + _g_CString_AppendCharAddress] \n"

                "lea rdi, [rbp-0x128] \n"
                "movsx rsi, byte ptr [rbx+2] \n"
                "call qword ptr [rip + _g_CString_AppendCharAddress] \n"
                "pop rax \n"

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"
                "7: \n"
                "mov rcx, [rbp - 0xF0] \n"
                "jmp qword ptr [rip + _g_Render3d_2_BypassAddr] \n"

                ".att_syntax prefix \n"
                :
                : [e1] "i"(ESCAPE_SEQ_1),
        [e2] "i"(ESCAPE_SEQ_2),
        [e3] "i"(ESCAPE_SEQ_3),
        [e4] "i"(ESCAPE_SEQ_4),
        [s2] "i"(SHIFT_2),
        [s3] "i"(SHIFT_3),
        [s4] "i"(SHIFT_4)
        );
    }

/**
 Hook函数：CBitmapFont::Render3d 字符初步处理循环，测量与处理换行
 作用：使其能够正确读取双字节字符
 */
    void install_CBitmapFont_Render3d_2() {
        TRACK_FUNCTION();
        std::string pattern = "0F B6 00 48 8B 8D 10 FF FF FF 48 8B 9C C1 E8";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;

        g_Render3d_2_BypassAddr = leaAddress + 0xA;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_Render3d_2);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_Render3d_2_BypassAddr);
        OptimizeNakedHook((uintptr_t) naked_CBitmapFont_Render3d_2);
        SET_SUCCESS();
    }


    void install() {
        //渲染
        install_CBitmapFont_Render3d_1();
        //文本预处理
        install_CBitmapFont_Render3d_2();
    }
}
