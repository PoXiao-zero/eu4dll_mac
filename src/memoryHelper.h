#pragma once

#include <cstdint>
#include <string>
#include <vector>

// ==========================================
// 内存读写
// ==========================================
bool ReadMemory(uintptr_t address, uint8_t *buffer, size_t size);

bool WriteMemory(uintptr_t address, const uint8_t *data, size_t size);

// ==========================================
// 特征码搜索
// ==========================================
uintptr_t FindPattern(uintptr_t startAddress, size_t searchSize, const std::string &pattern);

uintptr_t ScanMainModule(const std::string &pattern);

// ==========================================
// HOOK 和 替换调用
// ==========================================
// 强制使用 5字节 E9 JMP，如果跨度大于 32位整数范围则报错拦截
bool HookJMP(uintptr_t address, uintptr_t hookPtr);

// 强制使用 5字节 E8 CALL，如果原地址是 6字节指针 CALL (FF)，多出的一字节写 0x90 NOP
bool ReplaceCall(uintptr_t address, uintptr_t hookPtr);