
#include "input.h"
#include "global.h"
#include "strConvert.h"
#include <dlfcn.h>

namespace input {
    extern "C" {
    bool isImeComposing = false;
    bool isFirstEmpty = false;
    uintptr_t g_HandlePdxEvents_1_RetAddr = 0;
    uintptr_t g_HandleKeyEvent_1_RetAddr = 0;
    uintptr_t g_HandleKeyEvent_1_BypassAddr = 0;
    uintptr_t g_HandlePdxEvents_2_RetAddr = 0;
    void *g_InputUtf8ToEscapedStr_Addr = nullptr;
    typedef void (*fnInstanceAction_t)(void *pThis);
    typedef void (*fnCTextInputEvent_init_t)(void *pThis, char c);
    typedef void (*fnCInputEvent_init_t)(void *pThis, const void *pTextInputEvent);
    typedef int64_t (*fnCTextBuffer_GetCursorPositionInString_t)(void *pThis);
    fnCTextInputEvent_init_t fnCTextInputEvent_initCall;
    fnCInputEvent_init_t fnCInputEvent_initCall;
    fnInstanceAction_t fnCInputEvent_DestructorCall;
    fnInstanceAction_t fnCTextBuffer_EnterBackspaceCall;
    fnCTextBuffer_GetCursorPositionInString_t fnCTextBuffer_GetCursorPositionInStringCall;
    fnInstanceAction_t fnCTextBuffer_MoveLeftCall;
    fnInstanceAction_t fnCTextBuffer_MoveRightCall;
    }

    struct CCursorPosition {
        uint16_t Col;
        uint16_t Row;
    };

    void inputUtf8ToEscapedStr(const char *utf8_text,          // rdi: SDL text buffer [rbp-0x5C]
                               void *pEventHandler,     // rsi: CPdxEventHandler 对象指针 [rbp-0x70]
                               void *pKeyBoard,         // rdx: CKeyBoard 对象指针 (r15)
                               uint32_t timestamp,       // rcx: 事件时间戳 [rbp-0x54]
                               void *pTextInputEventMem, // r8 : 供 CTextInputEvent 存放的栈内存 (r12)
                               void *pInputEventMem      // r9 : 供 CInputEvent 存放的栈内存 [rbp-0xE8]
    ) {
        isImeComposing = false;
        std::string escaped = utf8_text;
        //printf("输入的文本：%s %s\n", utf8_text, escaped.c_str());
        strConvert::utf8ToEscapedStrReplace(&escaped);
        for (char c: escaped) {
            if (c == '\0') continue;

            // a. 调用原游戏的前置检查: pKeyBoard->vtable[0x28](0x303, 0)
            // 对应反汇编的 1014085ED 处逻辑
            // a. 正确调用 preCheck (vtable offset: 0x28)
            // 汇编: rdi = this, esi = 0x303, edx = arg_var_54, ecx = 0
            typedef void (*pfnPreCheck)(void *pThis, int eventType, uint32_t arg3, int arg4);

            // 关键修复：先获取虚表基址，再取虚函数地址
            void **keyboardVtable = *reinterpret_cast<void ***>(pKeyBoard);
            auto preCheck = reinterpret_cast<pfnPreCheck>(keyboardVtable[0x28 / 8]);
            preCheck(pKeyBoard, 0x303, timestamp, 0);



            // b. 构造 CTextInputEvent
            fnCTextInputEvent_initCall(pTextInputEventMem, c);
            // c. 构造 CInputEvent
            fnCInputEvent_initCall(pInputEventMem, pTextInputEventMem);
            // d. 调用 pEventHandler->HandleEvent(CInputEvent*)
            // 虚函数偏移在 0x20
            // d. 正确派发事件 dispatch (vtable offset: 0x20)
            typedef void (*pfnDispatch)(void *pThis, void *eventPtr);

            // 关键修复：获取 EventHandler 对象的虚表
            void **handlerVtable = *reinterpret_cast<void ***>(pEventHandler);
            auto dispatch = reinterpret_cast<pfnDispatch>(handlerVtable[0x20 / 8]);
            dispatch(pEventHandler, pInputEventMem);

            // e. 析构 CInputEvent 释放内存
            fnCInputEvent_DestructorCall(pInputEventMem);
        }
    }


    __attribute__((naked)) void naked_CSdlEvents_HandlePdxEvents_1() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "lea rdi, [rbp - 0x5C]\n"   // arg0: utf8_text 字符串首地址
                "mov rsi, [rbp - 0x70]\n"   // arg1: pEventHandler
                "mov rdx, r15\n"            // arg2: CKeyBoard* (原代码在 1014080F9 赋值给了r15)
                "mov ecx, [rbp - 0x54]\n"   // arg3: timestamp
                "mov r8, r12\n"             // arg4: CTextInputEvent 的内存地址 (原代码 r12=[rbp-108h])
                "lea r9, [rbp - 0xE8]\n"    // arg5: CInputEvent 的内存地址

                // 保护栈对齐 (调用 C 函数前要求 rsp 按 16 字节对齐)
                "push rbp\n"
                "mov rbp, rsp\n"
                "and rsp, -16\n"

                // 调用我们的 C++ 处理逻辑
                "call [rip + _g_InputUtf8ToEscapedStr_Addr] \n"

                // 恢复栈
                "mov rsp, rbp\n"
                "pop rbp\n"

                "jmp [rip + _g_HandlePdxEvents_1_RetAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CSdlEvents::HandlePdxEvents 0x303
 作用：拦截输入法输入完成事件，使其能读取全部输入文本并转换为游戏内能显示的逃逸文本
 */
    void install_CSdlEvents_HandlePdxEvents_1() {
        TRACK_FUNCTION();
        std::string pattern = "8A 5D A4 84 DB 0F 89 ? ? ? ? 80 FB DF";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_HandlePdxEvents_1_RetAddr = leaAddress + 0x325;
        HookJMP(leaAddress, (uintptr_t) naked_CSdlEvents_HandlePdxEvents_1);
        g_InputUtf8ToEscapedStr_Addr = (void *) inputUtf8ToEscapedStr;
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_HandlePdxEvents_1_RetAddr);
        OptimizeNakedHook((uintptr_t) naked_CSdlEvents_HandlePdxEvents_1);
        SET_SUCCESS();
    }

    bool isEscapedStr(unsigned char c) {
        if (c == ESCAPE_SEQ_1 || c == ESCAPE_SEQ_2 || c == ESCAPE_SEQ_3 || c == ESCAPE_SEQ_4) {
            return true;
        }
        return false;
    }

    bool proxy_CTextBuffer_HandleKeyEvent_HandleBackspace_1(void *pTextBuffer) {
        if (isImeComposing) {
            isFirstEmpty = true;
            return true; // 返回 true 表示我们处理了该事件，让游戏忽略退格键逻辑
        } else if (isFirstEmpty) {
            isFirstEmpty = false;
            return true; //首次空值时屏蔽退格，此时是刚好删完输入法内容
        }
        auto *stringBase = (std::string *) ((uintptr_t) pTextBuffer + 0x30);
        const char *str = (*stringBase).c_str();
        auto *cursorPos = (CCursorPosition *) ((uintptr_t) pTextBuffer + 0x4C);
        int64_t realIndex = fnCTextBuffer_GetCursorPositionInStringCall(pTextBuffer);
        int offset = 0;
        if (realIndex == 0) return false;
        if (isEscapedStr(str[realIndex - 1])) {
            offset += 2;
        } else if (realIndex >= 2 && isEscapedStr(str[realIndex - 2])) {
            offset += 1;
        } else if (!(realIndex >= 3 && isEscapedStr(str[realIndex - 3]))) {
            return false;
        }
        cursorPos->Col += offset;
        fnCTextBuffer_EnterBackspaceCall(pTextBuffer);
        fnCTextBuffer_EnterBackspaceCall(pTextBuffer);
        fnCTextBuffer_EnterBackspaceCall(pTextBuffer);
        return true;
    }

    extern "C" void *g_HandleKeyEvent_HandleBackspace_1_Addr = (void *) proxy_CTextBuffer_HandleKeyEvent_HandleBackspace_1;


    __attribute__((naked)) void naked_CTextBuffer_HandleKeyEvent_1() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"

                "push rbp\n"
                "mov rbp, rsp\n"
                "and rsp, -16\n"

                // r15 存放的是 CTextBuffer 的 this 指针，作为参数传给我们的 C++ 函数
                "mov rdi, r15\n"
                "call [rip + _g_HandleKeyEvent_HandleBackspace_1_Addr]\n"

                "mov rsp, rbp\n"
                "pop rbp\n"

                // 检查 C++ 函数的返回值 (存放在 AL 寄存器)
                "test al, al\n"
                "jnz 1f\n" // 如果返回 true (拦截)，跳到下面标签 1

                // ===== 分支 A：不拦截，执行原游戏正常退格逻辑 =====
                // 恢复被我们 Hook 覆盖掉的指令
                "mov rax, [r15]\n"
                "mov rdi, r15\n"
                // 跳回原程序继续执行
                "jmp [rip + _g_HandleKeyEvent_1_RetAddr] \n"
                // ===== 分支 B：已拦截，跳过游戏退格逻辑直接退出 =====
                "1:\n"
                // 观察游戏原函数的出口 loc_1014FFC8F，它通过 r13b 传递 handled(1) 标志
                // 我们直接跳转到其函数收尾部分，完美结束这次事件处理
                "jmp [rip + _g_HandleKeyEvent_1_BypassAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CTextBuffer::HandleKeyEvent
 作用：处理退格键删除逃逸文本时的逻辑，使其能正确删除
 */
    void install_CTextBuffer_HandleKeyEvent_1() {
        TRACK_FUNCTION();
        std::string pattern = "49 8B 07 4C 89 FF 84 DB 74 6B FF 90 40 01 00 00";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_HandleKeyEvent_1_RetAddr = leaAddress + 6;
        g_HandleKeyEvent_1_BypassAddr = leaAddress + 0x10;
        HookJMP(leaAddress, (uintptr_t) naked_CTextBuffer_HandleKeyEvent_1);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx 返回地址2:0x%lx\n",
               __func__,
               matchAddress, leaAddress, g_HandleKeyEvent_1_RetAddr, g_HandleKeyEvent_1_BypassAddr);
        OptimizeNakedHook((uintptr_t) naked_CTextBuffer_HandleKeyEvent_1);
        SET_SUCCESS();
    }


    __attribute__((naked)) void naked_CSdlEvents_HandlePdxEvents_2() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                "cmp eax, 0x302 \n"
                "jz 1f \n"
                "jmp 2f \n"
                "1: \n"
                "cmp byte ptr[rbp-0x5C], 0 \n"
                "setnz cl \n"
                "mov [rip+_isImeComposing], cl \n"
                "2: \n"
                "cmp eax, 0x301 \n"
                "jmp [rip + _g_HandlePdxEvents_2_RetAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CSdlEvents::HandlePdxEvents 0x302
 作用：检查输入法是否正在输入，用于拦截输入时的退格键
 */
    void install_CSdlEvents_HandlePdxEvents_2() {
        TRACK_FUNCTION();
        std::string pattern = "3D 01 03 00 00 0F 84 ? ? ? ? 3D 03 03 00 00";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        g_HandlePdxEvents_2_RetAddr = leaAddress + 5;
        HookJMP(leaAddress, (uintptr_t) naked_CSdlEvents_HandlePdxEvents_2);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx Hook地址:0x%lx 返回地址:0x%lx\n", __func__,
               matchAddress, leaAddress, g_HandlePdxEvents_2_RetAddr);
        OptimizeNakedHook((uintptr_t) naked_CSdlEvents_HandlePdxEvents_2);
        SET_SUCCESS();
    }

    void proxy_CTextBuffer_HandleKeyEvent_MoveLeft(void *pTextBuffer) {
        int64_t realIndex = fnCTextBuffer_GetCursorPositionInStringCall(pTextBuffer);
        if (realIndex >= 3) {
            auto *stringBase = (std::string *) ((uintptr_t) pTextBuffer + 0x30);
            const char *str = (*stringBase).c_str();
            if (isEscapedStr(str[realIndex - 3])) {
                fnCTextBuffer_MoveLeftCall(pTextBuffer);
                fnCTextBuffer_MoveLeftCall(pTextBuffer);
            }
        }
        fnCTextBuffer_MoveLeftCall(pTextBuffer);
    }

/**
 Hook函数：CTextBuffer::HandleKeyEvent
 作用：按下向左方向键时的光标处理
 */
    void install_CTextBuffer_HandleKeyEvent_2() {
        TRACK_FUNCTION();
        std::string pattern = "49 8B 07 4C 89 FF FF 90 D8 00 00 00 E9";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 6;
        ReplaceCall(leaAddress, (uintptr_t) proxy_CTextBuffer_HandleKeyEvent_MoveLeft);
        printf("eu4dll_mac [Success] %s ReplaceCall 匹配地址:0x%lx 写入地址:0x%lx\n", __func__, matchAddress,
               leaAddress);
        SET_SUCCESS();
    }

    void proxy_CTextBuffer_HandleKeyEvent_MoveRight(void *pTextBuffer) {
        auto *stringBase = (std::string *) ((uintptr_t) pTextBuffer + 0x30);
        auto len = stringBase->length();
        int64_t realIndex = fnCTextBuffer_GetCursorPositionInStringCall(pTextBuffer);
        if ((len - realIndex) >= 3) {
            const char *str = (*stringBase).c_str();
            if (isEscapedStr(str[realIndex])) {
                fnCTextBuffer_MoveRightCall(pTextBuffer);
                fnCTextBuffer_MoveRightCall(pTextBuffer);
            }
        }
        fnCTextBuffer_MoveRightCall(pTextBuffer);
    }

/**
 Hook函数：CTextBuffer::HandleKeyEvent
 作用：按下向右方向键时的光标处理
 */
    void install_CTextBuffer_HandleKeyEvent_3() {
        TRACK_FUNCTION();
        std::string pattern = "49 8B 07 4C 89 FF FF 90 E8 00 00 00 E9";
        uintptr_t matchAddress = ScanMainModule(pattern);

        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        leaAddress = leaAddress + 6;
        ReplaceCall(leaAddress, (uintptr_t) proxy_CTextBuffer_HandleKeyEvent_MoveRight);
        printf("eu4dll_mac [Success] %s ReplaceCall 匹配地址:0x%lx 写入地址:0x%lx\n", __func__, matchAddress,
               leaAddress);
        SET_SUCCESS();
    }


    void install() {
        fnCTextInputEvent_initCall = (fnCTextInputEvent_init_t) findFn("_ZN15CTextInputEventC1Ec");
        fnCInputEvent_initCall = (fnCInputEvent_init_t) findFn("_ZN11CInputEventC1ERK15CTextInputEvent");
        fnCInputEvent_DestructorCall = (fnInstanceAction_t) findFn("_ZN11CInputEventD1Ev");
        fnCTextBuffer_EnterBackspaceCall = (fnInstanceAction_t) dlsym(RTLD_DEFAULT,
                                                                      "_ZN11CTextBuffer14EnterBackspaceEv");
        fnCTextBuffer_GetCursorPositionInStringCall = (fnCTextBuffer_GetCursorPositionInString_t) dlsym(RTLD_DEFAULT,
                                                                                                        "_ZN11CTextBuffer25GetCursorPositionInStringEv");
        fnCTextBuffer_MoveLeftCall = (fnInstanceAction_t) findFn("_ZN11CTextBuffer8MoveLeftEv");
        fnCTextBuffer_MoveRightCall = (fnInstanceAction_t) findFn("_ZN11CTextBuffer9MoveRightEv");

        //拦截输入完成事件，使其支持UTF8文本输入
        install_CSdlEvents_HandlePdxEvents_1();
        //拦截输入中事件，使其在输入法输入中时按下的退格键不对已输入文本生效
        install_CSdlEvents_HandlePdxEvents_2();
        //处理退格键删除逃逸字符时的逻辑，使其能正确删除三字节
        install_CTextBuffer_HandleKeyEvent_1();
        //在文本输入框中按下←左方向键时，使其能正确越过逃逸字符
        install_CTextBuffer_HandleKeyEvent_2();
        //在文本输入框中按下→右方向键时，使其能正确越过逃逸字符
        install_CTextBuffer_HandleKeyEvent_3();
    }

}
