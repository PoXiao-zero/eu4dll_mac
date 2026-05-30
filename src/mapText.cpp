#include "mapText.h"
#include "global.h"

namespace mapText {
    extern "C" {
    uintptr_t g_AddNameArea_3_BypassAddr = 0;
    uintptr_t g_AddNameArea_1_RetAddr = 0;
    uintptr_t g_AddNameArea_1_BypassAddr = 0;

    uintptr_t g_FillVertexBuffer_1_BypassAddr = 0;
    uintptr_t g_FillVertexBuffer_2_BypassAddr = 0;

    uintptr_t g_CurveText_1_BypassAddr = 0;
    uintptr_t g_CurveText_2_BypassAddr = 0;
    uintptr_t g_CurveText_3_BypassAddr = 0;

    uint32_t g_CurveText_1_SkipByteCount = 0;

    uintptr_t g_AddNudgedNames_BypassAddr = 0;

    void *g_OriginalToUpper_Addr;

    }

    __attribute__((naked)) void naked_CGenerateNamesWork_AddNameArea_1() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "mov dword ptr [rbp-0xD8], 0 \n" //清空缓冲区变量ver_D8(4字节)，使CString_Append函数能正确识别字符长度
                "mov byte ptr [rbp-0xD8], al \n" //执行被覆盖的指令，先存入当前字符到缓冲区

                "cmp al, %c[e1] \n"
                "jz 1f \n"
                "cmp al, %c[e2] \n"
                "jz 1f \n"
                "cmp al, %c[e3] \n"
                "jz 1f \n"
                "cmp al, %c[e4] \n"
                "jz 1f \n"

                "jmp 2f \n"

                "1: \n"

                "mov bx, word ptr [r15+r14+1] \n" //如果是双字节文本，就读入后面两个字节
                "mov word ptr [rbp-0xD8 + 1], bx \n" //将后面两个字节追加进缓冲区

                "add r14, 2 \n"

                "2: \n"

                "lea rbx, [rbp-0x128] \n"
                "mov rdi, rbx \n"
                "lea rsi, [rbp-0xD8] \n"
                "call qword ptr [rip + _g_CString_AppendCharConstAddress] \n"

                "cmp r13, r14 \n" //原循环是r14不等于 源文本长度-1 就继续，由于手动递增了r14，如果末尾字是双字节文本，会导致r14超过r13致使无限循环导致崩溃。所以需要手动判断并跳过追加末尾字符区域
                "jbe 3f \n"

                "jmp qword ptr [rip + _g_AddNameArea_1_BypassAddr] \n"

                "3: \n"
                "mov dword ptr [rbp-0xD8], 0 \n"
                "jmp qword ptr [rip + _g_AddNameArea_1_RetAddr] \n"

                ".att_syntax prefix \n"
                :
                : [e1] "i"(ESCAPE_SEQ_1),
        [e2] "i"(ESCAPE_SEQ_2),
        [e3] "i"(ESCAPE_SEQ_3),
        [e4] "i"(ESCAPE_SEQ_4)
        );
    }

/**
 Hook函数：CGenerateNamesWork::AddNameArea 加空格循环
 作用：不断尝试在每个字中间添加X个空格，使其总体宽度能够填满地图区域。找到最合适的间距后，调用FillVertexBuffer生成顶点。
 */
    void install_CGenerateNamesWork_AddNameArea_1() {
        TRACK_FUNCTION();
        std::string pattern = "43 8A 04 37 88 85 28 FF FF FF";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 4;

        g_AddNameArea_1_BypassAddr = leaAddress + 0x1C;
        g_AddNameArea_1_RetAddr = leaAddress + 0x72;

        HookJMP(leaAddress, (uintptr_t) naked_CGenerateNamesWork_AddNameArea_1);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx 返回地址2:0x%lx\n",
               __func__,
               matchAddress, leaAddress, g_AddNameArea_1_RetAddr, g_AddNameArea_1_BypassAddr);
        SET_SUCCESS();
    }

    __attribute__((naked)) void proxy_CGenerateNamesWork_AddNameArea_ToUpper_2() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "push    rbp \n"
                "mov     rbp, rsp \n"
                "push    r15 \n"
                "push    r14 \n"
                "push    r12 \n"
                "push    rbx \n"
                "mov     r15, rdi \n"
                "movzx   r14d, byte ptr [rdi] \n"
                "test    r14b, 1 \n"
                "jnz     1f \n"
                "inc     r15 \n"
                "shr     r14, 1 \n"
                "jmp     2f \n"

                "1: \n"
                "mov     r14, [r15+8] \n"
                "mov     r15, [r15+0x10] \n"

                "2: \n"
                "test    r14, r14 \n"
                "jz      4f \n"
                "mov     al, [r15] \n"
                "test    al, al \n"
                "jz      4f \n"
                "mov     ebx, 1 \n"
                "mov     r12, r15 \n"

                "3: \n"
                "cmp al, %c[e1] \n"
                "jz 5f \n"
                "cmp al, %c[e2] \n"
                "jz 5f \n"
                "cmp al, %c[e3] \n"
                "jz 5f \n"
                "cmp al, %c[e4] \n"
                "jz 5f \n"

                "movsx edi, al \n"
                "call [rip + _g_OriginalToUpper_Addr] \n"
                "mov [r12], al \n"
                "jmp 6f \n"

                "5: \n"
                "add ebx, 2 \n"

                "6: \n"
                "mov     eax, ebx \n"
                "cmp     r14, rax \n"
                "jbe     4f \n"
                "lea     r12, [r15+rax] \n"
                "mov     al, [r15+rax] \n"
                "inc     ebx \n"
                "test    al, al \n"
                "jnz     3b \n"

                "4: \n"
                "pop     rbx \n"
                "pop     r12 \n"
                "pop     r14 \n"
                "pop     r15 \n"
                "pop     rbp \n"
                "retn \n"

                ".att_syntax prefix \n"
                :
                : [e1] "i"(ESCAPE_SEQ_1),
        [e2] "i"(ESCAPE_SEQ_2),
        [e3] "i"(ESCAPE_SEQ_3),
        [e4] "i"(ESCAPE_SEQ_4)
        );
    }

/**
 Hook函数：CGenerateNamesWork::AddNameArea
 作用：替换ToUpper CALL，避免将标识符后面的字节当成小写字符从而导致文字改变
 */
    void install_CGenerateNamesWork_AddNameArea_2() {
        TRACK_FUNCTION();
        std::string pattern = "FF FF FF E8 ? ? ? ? 31 C0 4C 8D 85 B8 FD FF FF";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 3;
        g_OriginalToUpper_Addr = findFn("__toupper");
        ReplaceCall(leaAddress, (uintptr_t) proxy_CGenerateNamesWork_AddNameArea_ToUpper_2);
        printf("eu4dll_mac [Success] %s ReplaceCall 匹配地址:0x%lx 写入地址:0x%lx ToUpper地址:%p\n", __func__,
               matchAddress, leaAddress, g_OriginalToUpper_Addr);
        SET_SUCCESS();
    }


    __attribute__((naked)) void naked_CGenerateNamesWork_AddNameArea_3() {
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

                "add ebx, 2 \n" // 增加循环总计数

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"
                "7: \n"

                "mov rax, [r14+rax*8+0xE8] \n"

                "jmp qword ptr [rip + _g_AddNameArea_3_BypassAddr] \n"

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
 Hook函数：CGenerateNamesWork::AddNameArea 通过循环遍历获取有效字符数，来决定顶点缓冲区大小
 作用：使其能正确识别字符数
 */
    void install_CGenerateNamesWork_AddNameArea_3() {
        TRACK_FUNCTION();
        std::string pattern = "0F B6 00 49 8B 84 C6 E8 00 00 00 48 85 C0 74 0D";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_AddNameArea_3_BypassAddr = leaAddress + 0xB;

        HookJMP(leaAddress, (uintptr_t) naked_CGenerateNamesWork_AddNameArea_3);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_AddNameArea_3_BypassAddr);
        SET_SUCCESS();
    }


    __attribute__((naked)) void naked_CBitmapFont_FillVertexBuffer_1() {
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

                "add ebx, 2 \n" // 增加循环总计数

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"

                "7: \n"

                "mov rax, [r15+rax*8+0xE8] \n"
                "jmp qword ptr [rip + _g_FillVertexBuffer_1_BypassAddr] \n"

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
 Hook函数：CBitmapFont::FillVertexBuffer 字符顶点生成循环
 作用：使其能正确识别双字节字符
 */
    void install_CBitmapFont_FillVertexBuffer_1() {
        TRACK_FUNCTION();
        std::string pattern = "0F B6 00 49 8B 84 C7 E8 00 00 00 48 85 C0 0F 84 46 03 00 00";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_FillVertexBuffer_1_BypassAddr = leaAddress + 0xB;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_FillVertexBuffer_1);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_FillVertexBuffer_1_BypassAddr);
        SET_SUCCESS();
    }

    __attribute__((naked)) void naked_CBitmapFont_FillVertexBuffer_2() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "mov r13, rax \n"
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
                "movzx eax, word ptr [r13+1] \n"
                "jmp 5f \n"

                "2: \n"
                "movzx eax, word ptr [r13+1] \n"
                "sub eax, %c[s2] \n"
                "jmp 5f \n"

                "3: \n"
                "movzx eax, word ptr [r13+1] \n"
                "add eax, %c[s3] \n"
                "jmp 5f \n"

                "4: \n"
                "movzx eax, word ptr [r13+1] \n"
                "add eax, %c[s4] \n"

                // 通用收尾
                "5: \n"

                "push rax \n"

                "lea rdi, [rbp - 0x1000] \n"
                "movzx rsi, byte ptr [r13+1] \n"
                "call [rip + _g_CString_AppendCharAddress] \n"

                "lea rdi, [rbp - 0x1000] \n"
                "movzx rsi, byte ptr [r13+2] \n"
                "call [rip + _g_CString_AppendCharAddress] \n"

                "pop rax \n"

                "add r12d, 2 \n" // 增加循环总计数

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"
                "7: \n"
                "mov r13, [r15+rax*8+0xE8] \n"
                "jmp qword ptr [rip + _g_FillVertexBuffer_2_BypassAddr] \n"

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
 Hook函数：CBitmapFont::FillVertexBuffer 字符初步处理循环，计算宽度与处理换行
 作用：使其能正确识别双字节字符
 */
    void install_CBitmapFont_FillVertexBuffer_2() {
        TRACK_FUNCTION();
        std::string pattern = "0F B6 00 4D 8B AC C7 E8 00 00 00 4D 85 ED";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_FillVertexBuffer_2_BypassAddr = leaAddress + 0xB;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_FillVertexBuffer_2);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_FillVertexBuffer_2_BypassAddr);
        SET_SUCCESS();
    }


    __attribute__((naked)) void naked_CurveText_1() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                // 为指针加上跳过字节数才是当前真实索引
                "mov r15d, dword ptr [rip + _g_CurveText_1_SkipByteCount] \n"
                "add rax, r15 \n"

                "mov r15, rax \n"
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
                "movzx eax, word ptr [r15+1] \n"
                "jmp 5f \n"

                "2: \n"
                "movzx eax, word ptr [r15+1] \n"
                "sub eax, %c[s2] \n"
                "jmp 5f \n"

                "3: \n"
                "movzx eax, word ptr [r15+1] \n"
                "add eax, %c[s3] \n"
                "jmp 5f \n"

                "4: \n"
                "movzx eax, word ptr [r15+1] \n"
                "add eax, %c[s4] \n"

                // 通用收尾
                "5: \n"
                "add dword ptr [rip + _g_CurveText_1_SkipByteCount], 2 \n" //增加外部跳过字节计数

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"

                "7: \n"

                "mov r15, [r12+rax*8] \n"

                "jmp qword ptr [rip + _g_CurveText_1_BypassAddr] \n"

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
 Hook函数：CurveText
 作用：将FillVertexBuffer函数生成的文字顶点网格贴合地图曲线
 */
    void install_CurveText_1() {
        TRACK_FUNCTION();
        std::string pattern = "0F B6 00 4D 8B 3C C4 4D 85 FF";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_CurveText_1_BypassAddr = leaAddress + 7;

        HookJMP(leaAddress, (uintptr_t) naked_CurveText_1);

        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_CurveText_1_BypassAddr);
        SET_SUCCESS();
    }


    __attribute__((naked)) void naked_CurveText_2() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                "mov r14d, 0 \n"
                "mov dword ptr[rip + _g_CurveText_1_SkipByteCount], 0 \n" //循环开始时清空上一轮计数

                "jmp qword ptr [rip + _g_CurveText_2_BypassAddr] \n"

                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CurveText
 作用：将外部维护的计数在每一次循环前清零
 */
    void install_CurveText_2() {
        TRACK_FUNCTION();
        std::string pattern = "F3 41 0F 2A CE 41 BE 00 00 00 00";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 5;
        g_CurveText_2_BypassAddr = leaAddress + 6;

        HookJMP(leaAddress, (uintptr_t) naked_CurveText_2);

        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_CurveText_2_BypassAddr);
        SET_SUCCESS();
    }


    __attribute__((naked)) void naked_CurveText_3() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                "mov eax,[rbp - 0x90] \n"

                "jmp qword ptr [rip + _g_CurveText_3_BypassAddr] \n"

                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CurveText
 作用：将已存储的文本真实长度传递给变量进行对比，避免循环内重复遍历。
 */
    void install_CurveText_3() {
        TRACK_FUNCTION();
        std::string pattern = "F3 0F 11 45 CC 4C 89 EF E8";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 5;
        g_CurveText_3_BypassAddr = leaAddress + 8;

        HookJMP(leaAddress, (uintptr_t) naked_CurveText_3);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_CurveText_3_BypassAddr);
        SET_SUCCESS();
    }


    uint64_t proxy_CurveText_GetSize_CStringGetSize_4(std::string *cstring_this) {
        uint64_t count = 0;
        uint64_t size = cstring_this->length();
        for (int i = 0; i < size; i++) {
            unsigned char c = (*cstring_this)[i];
            if (c == ESCAPE_SEQ_1 || c == ESCAPE_SEQ_2 || c == ESCAPE_SEQ_3 || c == ESCAPE_SEQ_4) {
                i += 2;
            }
            count++;
        }
        return count;
    }

/**
 Hook函数：CurveText
 作用：替换getSize CALL获取真实字符长度以便循环遍历
 */
    void install_CurveText_4() {
        TRACK_FUNCTION();
        std::string pattern = "41 89 C6 4C 89 EF E8 ? ? ? ? 41 89 C7 4C 89 EF E8 ? ? ? ? 48 89 85 70 FF FF FF";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 6;
        ReplaceCall(leaAddress, (uintptr_t) proxy_CurveText_GetSize_CStringGetSize_4);
        uintptr_t replaceAddress1 = leaAddress;
        leaAddress = leaAddress + 11;
        ReplaceCall(leaAddress, (uintptr_t) proxy_CurveText_GetSize_CStringGetSize_4);
        printf("eu4dll_mac [Success] %s ReplaceCall 匹配地址:0x%lx 写入地址1:0x%lx 写入地址2:0x%lx\n", __func__,
               matchAddress, replaceAddress1, leaAddress);
        SET_SUCCESS();
    }


    __attribute__((naked)) void naked_CCountryNameCollection_AddNudgedNames() {
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

                "5: \n"

                "add r13d, 2 \n" // 增加循环总计数

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"
                "7: \n"

                "mov rax, [r14+rax*8+0xE8] \n"

                "jmp qword ptr [rip + _g_AddNudgedNames_BypassAddr] \n"

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
 Hook函数：CCountryNameCollection::AddNudgedNames
 作用：使其正确识别双字节字符。不知道做啥的，HOOK看文本也没能看出个啥，文本量远不及AddNameArea
 */
    void install_CCountryNameCollection_AddNudgedNames() {
        TRACK_FUNCTION();
        std::string pattern = "44 89 EE E8 ? ? ? ? 0F B6 00 49 8B 84 C6 E8 00 00 00 48 85 C0 74";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 8;
        g_AddNudgedNames_BypassAddr = leaAddress + 0xB;

        HookJMP(leaAddress, (uintptr_t) naked_CCountryNameCollection_AddNudgedNames);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_AddNudgedNames_BypassAddr);
        SET_SUCCESS();
    }


    void install() {
        //尝试在文本中添加空格来填满地图
        install_CGenerateNamesWork_AddNameArea_1();
        //替换到大小CALL，使其能正确略过双字节文本
        install_CGenerateNamesWork_AddNameArea_2();
        //使其能正确识别字符数，来生成缓冲区
        install_CGenerateNamesWork_AddNameArea_3();
        //字符顶点生成
        install_CBitmapFont_FillVertexBuffer_1();
        //文本初处理循环
        install_CBitmapFont_FillVertexBuffer_2();
        //贴合地图曲线
        install_CurveText_1();
        install_CurveText_2();
        install_CurveText_3();
        install_CurveText_4();
        //不知道什么用
        install_CCountryNameCollection_AddNudgedNames();
    }

}
