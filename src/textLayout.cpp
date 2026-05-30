#include "textLayout.h"
#include "global.h"

namespace textLayout {

    extern "C" {
    uintptr_t g_GetHeightOfString_BypassAddr = 0;

    uintptr_t g_GetWidthOfString_RetAddr = 0;
    uintptr_t g_GetWidthOfString_BypassAddr = 0;

    uintptr_t g_GetRequiredSize_BypassAddr = 0;

    uintptr_t g_GetActualRequiredSize_BypassAddr = 0;

    uintptr_t g_GetActualRealRequiredSizeActually_1_BypassAddr = 0;

    uintptr_t g_GetActualRealRequiredSizeActually_2_BypassAddr = 0;

    uintptr_t g_GetActualRealRequiredSizeActually_3_BypassAddr = 0;

    float g_GetActualRealRequiredSizeActually_2_HistoryWidths[5];
    }

    __attribute__((naked)) void naked_CBitmapFont_GetHeightOfString() {
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

                "add r13d, 2 \n" // 增加循环总计数

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"

                "7: \n"
                "mov rax, qword ptr [rbx+rax*8+0xE8] \n"
                "jmp qword ptr [rip + _g_GetHeightOfString_BypassAddr] \n"

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
 Hook函数：CBitmapFont::GetHeightOfString 循环遍历有效字符并计算总体高度后返回。
 作用：没感觉到什么太明显的影响，通过HOOK查看输入的文本好像都是教程相关的
 */
    void install_CBitmapFont_GetHeightOfString() {
        TRACK_FUNCTION();
        std::string pattern = "0F B6 00 48 8B 84 C3 E8 00 00 00 48 85 C0";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_GetHeightOfString_BypassAddr = leaAddress + 0xB;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_GetHeightOfString);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_GetHeightOfString_BypassAddr);
        SET_SUCCESS();
    }


    __attribute__((naked)) void naked_CBitmapFont_GetWidthOfString() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "lea rbx, [r14+r12] \n"

                // 检查自定义转义字符
                "cmp sil, %c[e1] \n"
                "jz 1f \n"
                "cmp sil, %c[e2] \n"
                "jz 2f \n"
                "cmp sil, %c[e3] \n"
                "jz 3f \n"
                "cmp sil, %c[e4] \n"
                "jz 4f \n"

                "jmp 7f \n"

                "1: \n"
                "movzx esi, word ptr [rbx+1] \n"
                "jmp 5f \n"

                "2: \n"
                "movzx esi, word ptr [rbx+1] \n"
                "sub esi, %c[s2] \n"
                "jmp 5f \n"

                "3: \n"
                "movzx esi, word ptr [rbx+1] \n"
                "add esi, %c[s3] \n"
                "jmp 5f \n"

                "4: \n"
                "movzx esi, word ptr [rbx+1] \n"
                "add esi, %c[s4] \n"

                // 通用收尾
                "5: \n"
                "add r15d, 2 \n" // 增加循环总计数

                "cmp r15d, edx \n" //下标越界检测，越界就直接退出循环。比如检查的字符串末尾恰好就是**标识符0x10-0x13**时会错误的越界读取后面两个字节当成文字，导致换行符0x0A出现在错误的位置(标识符后面)。
                "jge 8f \n"

                "cmp esi, 256 \n"
                "jb 7f \n"
                "add esi, 1712 \n"

                "7: \n"
                "mov rbx, [rdi+rsi*8+0xE8] \n"
                "jmp qword ptr [rip + _g_GetWidthOfString_RetAddr] \n"

                "8: \n"
                "jmp qword ptr [rip + _g_GetWidthOfString_BypassAddr] \n"
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
 Hook函数：CBitmapFont::GetWidthOfString 循环遍历有效字符并计算最大宽度后返回
 作用：使其能正确识别双字节字符
 */
    void install_CBitmapFont_GetWidthOfString() {
        TRACK_FUNCTION();
        std::string pattern = "48 8B 9C F7 E8 00 00 00 48 85 DB";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_GetWidthOfString_RetAddr = leaAddress + 8;
        g_GetWidthOfString_BypassAddr = leaAddress + 0x1C6;
        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_GetWidthOfString);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx 返回地址2:0x%lx\n",
               __func__,
               matchAddress, leaAddress, g_GetWidthOfString_RetAddr, g_GetWidthOfString_BypassAddr);
        SET_SUCCESS();
    }


    __attribute__((naked)) void naked_CBitmapFont_GetRequiredSize() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "mov rbx, rax \n"
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

                "5: \n"

                "add dword ptr [rbp - 0xB4], 2 \n" // 增加循环总计数

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"
                "7: \n"
                "mov rbx, qword ptr [r12+rax*8+0xE8] \n"
                "jmp qword ptr [rip + _g_GetRequiredSize_BypassAddr] \n"

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
 Hook函数：CBitmapFont::GetRequiredSize(超出时截断并添加...)
 作用：没感觉到什么明显的影响，通过HOOK查看输入文本，主要都是按钮文本
 */
    void install_CBitmapFont_GetRequiredSize() {
        TRACK_FUNCTION();
        std::string pattern = "0F B6 00 49 8B 9C C4 E8 00 00 00 48 85 DB";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_GetRequiredSize_BypassAddr = leaAddress + 0xB;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_GetRequiredSize);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_GetRequiredSize_BypassAddr);
        SET_SUCCESS();
    }

    __attribute__((naked)) void naked_CBitmapFont_GetActualRealRequiredSizeActually_1() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "mov rbx, rax \n"
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

                "5: \n"

                "add dword ptr [rbp - 0xB4], 2 \n" // 增加循环总计数

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"
                "7: \n"
                "mov rbx, qword ptr [r13+rax*8+0xE8] \n"
                "jmp qword ptr [rip + _g_GetActualRealRequiredSizeActually_1_BypassAddr] \n"

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
 Hook函数：CBitmapFont::GetActualRealRequiredSizeActually(超出时截断并添加...)
 作用：主要是屏幕右侧的数据概览窗口
 */
    void install_CBitmapFont_GetActualRealRequiredSizeActually_1() {
        TRACK_FUNCTION();
        std::string pattern = "0F B6 00 49 8B 9C C5 E8 00 00 00 48 85 DB";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_GetActualRealRequiredSizeActually_1_BypassAddr = leaAddress + 0xB;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_GetActualRealRequiredSizeActually_1);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_GetActualRealRequiredSizeActually_1_BypassAddr);
        SET_SUCCESS();
    }

    __attribute__((naked)) void naked_CBitmapFont_GetActualRequiredSize() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "mov rcx, rax \n"
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
                "movzx eax, word ptr [rcx+1] \n"
                "jmp 5f \n"

                "2: \n"
                "movzx eax, word ptr [rcx+1] \n"
                "sub eax, %c[s2] \n"
                "jmp 5f \n"

                "3: \n"
                "movzx eax, word ptr [rcx+1] \n"
                "add eax, %c[s3] \n"
                "jmp 5f \n"

                "4: \n"
                "movzx eax, word ptr [rcx+1] \n"
                "add eax, %c[s4] \n"

                "5: \n"

                "add r13d, 2 \n" // 增加循环总计数

                "cmp eax, 256 \n"
                "jb 7f \n"
                "add eax, 1712 \n"

                "7: \n"
                "mov rcx, [rbp - 0x1128] \n"
                "jmp qword ptr [rip + _g_GetActualRequiredSize_BypassAddr] \n"

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
 Hook函数：CBitmapFont::GetActualRequiredSize
 作用：主要对地图文本显示大小有显著影响，防止越界
 */
    void install_CBitmapFont_GetActualRequiredSize() {
        TRACK_FUNCTION();
        std::string pattern = "0F B6 00 48 8B 8D D8 EE FF FF";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_GetActualRequiredSize_BypassAddr = leaAddress + 0xA;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_GetActualRequiredSize);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_GetActualRequiredSize_BypassAddr);
        SET_SUCCESS();
    }

    __attribute__((naked)) void naked_CBitmapFont_GetActualRealRequiredSizeActually_2() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                //"mov dword ptr [rbp-0x100], 0x43A00000 \n"
                // 1. 把前4个历史行宽往前平移 (模拟队列 pop_front)
                "mov ebx, dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 4] \n"
                "mov dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 0], ebx \n"
                "mov ebx, dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 8] \n"
                "mov dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 4], ebx \n"
                "mov ebx, dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 12] \n"
                "mov dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 8], ebx \n"
                "mov ebx, dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 16] \n"
                "mov dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 12], ebx \n"
                // 2. 把当前的 lineWidth ([rbp-0xB8]) 存入最新的位置
                "mov ebx, dword ptr [rbp-0xB8] \n"
                "mov dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 16], ebx \n"
                // 3. 恢复被覆盖的原指令，跳回原流程
                "mov ebx, [rbp - 0xB4] \n"
                "jmp qword ptr [rip + _g_GetActualRealRequiredSizeActually_2_BypassAddr] \n"

                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CBitmapFont::GetActualRealRequiredSizeActually
 作用：在循环的开始存储上一轮行宽
 */
    void install_CBitmapFont_GetActualRealRequiredSizeActually_2() {
        TRACK_FUNCTION();
        std::string pattern = "8B 9D 4C FF FF FF FF C3 89 9D 4C FF FF FF 4C 89 E7";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_GetActualRealRequiredSizeActually_2_BypassAddr = leaAddress + 6;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_GetActualRealRequiredSizeActually_2);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_GetActualRealRequiredSizeActually_2_BypassAddr);
        SET_SUCCESS();
    }

    __attribute__((naked)) void naked_CBitmapFont_GetActualRealRequiredSizeActually_3() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "mov ecx, dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 4] \n"
                "mov dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 0], ecx \n"
                "mov ecx, dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 8] \n"
                "mov dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 4], ecx \n"
                "mov ecx, dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 12] \n"
                "mov dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 8], ecx \n"
                "mov ecx, dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 16] \n"
                "mov dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 12], ecx \n"
                // 2. 把当前的 lineWidth ([rbp-0xB8]) 存入最新的位置
                "mov ecx, dword ptr [rbp-0xB8] \n"
                "mov dword ptr [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths + 16], ecx \n"


                // 准备循环，r8 = 4 (从最新的倒数第1个字符开始查，直到0)
                "mov r8, 4 \n"
                "movss xmm1, dword ptr [rbp-0xD4] \n" // 加载 ellipsisWidth (...) 的宽度
                "movss xmm2, dword ptr [rbp-0x100] \n"// 加载 maxLineWidthF (行的最大允许宽度)
                "lea r9, [rip + _g_GetActualRealRequiredSizeActually_2_HistoryWidths] \n"   // 你的历史数组地址

                "1: \n"
                // 取出第 r8 个历史行宽
                "movss xmm0, dword ptr [r9 + r8*4] \n"
                // 计算: 历史行宽 + ellipsisWidth
                "addss xmm0, xmm1 \n"
                // 比较: maxLineWidthF 和 (历史行宽 + ellipsisWidth)
                "ucomiss xmm2, xmm0 \n"
                "jae 2f \n"   // 如果 maxLineWidthF >= xmm0，说明放得下！跳转
                "dec r8 \n"
                "jns 1b \n"  // 如果 r8 >= 0，继续往前找下一个更早的字符
                // 如果连退 5 个字符都放不下（极小概率，比如文本框非常非常窄）
                // 默认 r8 退回 0（和原版行为一样，退5个）
                "xor r8, r8 \n"
                "2: \n"
                // 此时，r8 就是我们动态计算出“最完美”的保留数量 (0 到 4)
                // 还原原指令，读取队列头部绝对索引
                "mov rcx, qword ptr [rbp-0x120] \n"
                // 核心: 把算出的偏移加上去！
                "add rcx, r8 \n"
                "jmp qword ptr [rip + _g_GetActualRealRequiredSizeActually_3_BypassAddr] \n"

                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CBitmapFont::GetActualRealRequiredSizeActually
 作用：不使用原有的强制五字符截断逻辑，改为根据宽度计算截断位置
 */
    void install_CBitmapFont_GetActualRealRequiredSizeActually_3() {
        TRACK_FUNCTION();
        std::string pattern = "48 83 BD E8 FE FF FF 05 0F 82 ? ? ? ? 48 8B 85 C8 FE FF FF";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 0x15;
        g_GetActualRealRequiredSizeActually_3_BypassAddr = leaAddress + 7;

        HookJMP(leaAddress, (uintptr_t) naked_CBitmapFont_GetActualRealRequiredSizeActually_3);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_GetActualRealRequiredSizeActually_3_BypassAddr);
        SET_SUCCESS();
    }

/**
 Hook函数：CBitmapFont::GetActualRealRequiredSizeActually
 作用：阻止截断
 */
    void install_CBitmapFont_GetActualRealRequiredSizeActually_4() {
        TRACK_FUNCTION();
        std::string pattern = "48 83 BD E8 FE FF FF 05 0F 82 ? ? ? ? 48 8B 85 C8 FE FF FF";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 8;
        uint8_t patch[] = {0x90, 0xE9};
        WriteMemory(leaAddress, patch, 2);
        printf("eu4dll_mac [Success] %s WriteMemory 匹配地址:0x%lx 写入地址:0x%lx\n", __func__, matchAddress,
               leaAddress);
        SET_SUCCESS();
    }

    void install() {
        install_CBitmapFont_GetHeightOfString();
        install_CBitmapFont_GetWidthOfString();
        install_CBitmapFont_GetRequiredSize();
        install_CBitmapFont_GetActualRealRequiredSizeActually_1();
        install_CBitmapFont_GetActualRequiredSize();
        install_CBitmapFont_GetActualRealRequiredSizeActually_2();
        install_CBitmapFont_GetActualRealRequiredSizeActually_3();
        //install_CBitmapFont_GetActualRealRequiredSizeActually_4();// 阻止截断
    }
}
