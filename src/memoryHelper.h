#pragma once

#include <cstdint>
#include <string>
#include <vector>

// ==========================================
// 内存读写
// ==========================================
bool ReadMemory(uintptr_t address, uint8_t *buffer, size_t size);

bool WriteMemory(uintptr_t address, const uint8_t *data, size_t size);

template<size_t N>
bool WriteMemory(uintptr_t address, const uint8_t (&data)[N]) {
    return WriteMemory(address, data, N);
}
// ==========================================
// 特征码搜索
// ==========================================

uintptr_t ScanMainModule(const std::string &pattern);

uintptr_t ScanMainModule(const std::string &pattern, const std::vector<std::string> &targetStrings);

uintptr_t
ScanMainModule(const std::string &pattern, const std::vector<std::string> &targetStrings, const char *symbolName,
               size_t maxSearchSize);


// ==========================================
// HOOK 和 替换调用
// ==========================================
// 强制使用 5字节 E9 JMP，如果跨度大于 32位整数范围则报错拦截
bool HookJMP(uintptr_t address, uintptr_t hookPtr);

// 强制使用 5字节 E8 CALL，如果原地址是 6字节指针 CALL (FF)，多出的一字节写 0x90 NOP
bool ReplaceCall(uintptr_t address, uintptr_t hookPtr);

// 自动扫描并优化裸汇编HOOK函数中的JMP/CALL，将其从读取内存地址变为直接偏移跳转
void OptimizeNakedHook(uintptr_t func_ptr, size_t max_scan_size = 200);