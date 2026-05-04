#include "memoryHelper.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>

#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/vm_prot.h>

static std::vector<int> ParsePattern(const std::string &pattern) {
    std::vector<int> bytes;
    std::istringstream iss(pattern);
    std::string word;
    while (iss >> word) {
        if (word[0] == '?') {
            bytes.push_back(-1); // 模糊匹配占位符
        } else {
            bytes.push_back(std::strtol(word.c_str(), nullptr, 16));
        }
    }
    return bytes;
}

uintptr_t GetMainModuleBase() {
    return (uintptr_t) _dyld_get_image_header(0);
}

size_t GetMainModuleSize() {
    const auto *header = (const struct mach_header_64 *) _dyld_get_image_header(0);
    unsigned long size = 0;
    getsegmentdata(header, "__TEXT", &size);
    return size;
}

bool ReadMemory(uintptr_t address, uint8_t *buffer, size_t size) {
    if (!address || !buffer || size == 0) return false;
    std::memcpy(buffer, reinterpret_cast<void *>(address), size);
    return true;
}

bool WriteMemory(uintptr_t address, const uint8_t *data, size_t size) {
    if (!address || !data || size == 0) return false;
    mach_port_t task = mach_task_self();
    mach_vm_address_t dst = address;
    // 1. 解除保护：必须使用 VM_PROT_COPY 强制触发写时复制 (Copy-On-Write)
    kern_return_t kr = mach_vm_protect(task, dst, size, FALSE, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY);
    if (kr != KERN_SUCCESS) {
        std::cerr << "[WriteMemory] Failed to unprotect memory at 0x"
                  << std::hex << address << std::dec
                  << ". Mach Error Code: " << kr << std::endl;
        return false;
    }
    // 2. 写入我们的 HOOK 代码 / Payload
    std::memcpy(reinterpret_cast<void *>(address), data, size);
    // 3. 恢复保护：改回 可读 + 可执行 (剥夺写权限，防止触发 W^X 保护机制)
    kr = mach_vm_protect(task, dst, size, FALSE, VM_PROT_READ | VM_PROT_EXECUTE);
    if (kr != KERN_SUCCESS) {
        std::cerr << "[WriteMemory] Failed to reprotect memory at 0x"
                  << std::hex << address << std::dec
                  << ". Mach Error Code: " << kr << std::endl;
        return false;
    }

    return true;
}

uintptr_t FindPattern(uintptr_t startAddress, size_t searchSize, const std::string &pattern) {
    std::vector<int> patternBytes = ParsePattern(pattern);
    size_t patternLen = patternBytes.size();
    if (patternLen == 0 || searchSize < patternLen) return 0;

    auto *scanBytes = reinterpret_cast<uint8_t *>(startAddress);

    uintptr_t foundAddress = 0;
    int matchCount = 0;

    for (size_t i = 0; i <= searchSize - patternLen; ++i) {
        bool found = true;
        for (size_t j = 0; j < patternLen; ++j) {
            if (patternBytes[j] != -1 && scanBytes[i + j] != patternBytes[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            foundAddress = startAddress + i;
            //matchCount++;
            return foundAddress;
        }
    }

//    if (matchCount == 1) {
//        return foundAddress;
//    } else if (matchCount > 1) {
//        std::cerr << "[FindPattern] Error: Pattern found " << matchCount
//                  << " times. It is not unique!" << std::endl;
//        return 0;
//    }

    std::cerr << "[FindPattern] Error: Pattern not found." << std::endl;
    return 0;
}

uintptr_t ScanMainModule(const std::string &pattern) {
    uintptr_t base = GetMainModuleBase();
    size_t size = GetMainModuleSize();
    return FindPattern(base, size, pattern);
}

bool HookJMP(uintptr_t address, uintptr_t hookPtr) {
    int64_t offset64 = static_cast<int64_t>(hookPtr) - static_cast<int64_t>(address) - 5;
    if (offset64 > 2147483647LL || offset64 < -2147483648LL) {
        std::cerr << "[HookJMP] Error: Hook pointer is too far! Offset: "
                  << offset64 << " exceeds 32-bit limit." << std::endl;
        return false;
    }
    auto offset = static_cast<int32_t>(offset64);
    uint8_t payload[5];
    payload[0] = 0xE9;
    std::memcpy(&payload[1], &offset, sizeof(offset));
    return WriteMemory(address, payload, 5);
}

bool ReplaceCall(uintptr_t address, uintptr_t hookPtr) {
    int64_t offset64 = static_cast<int64_t>(hookPtr) - static_cast<int64_t>(address) - 5;
    if (offset64 > 2147483647LL || offset64 < -2147483648LL) {
        std::cerr << "[ReplaceCall] Error: Hook pointer is too far! Offset: "
                  << offset64 << " exceeds 32-bit limit." << std::endl;
        return false;
    }
    auto offset = static_cast<int32_t>(offset64);
    uint8_t originalBytes[2] = {0};
    ReadMemory(address, originalBytes, 2);
    if (originalBytes[0] == 0xFF) {
        uint8_t payload[6];
        payload[0] = 0xE8;
        std::memcpy(&payload[1], &offset, sizeof(offset));
        payload[5] = 0x90;
        return WriteMemory(address, payload, 6);
    } else {
        uint8_t payload[5];
        payload[0] = 0xE8;
        std::memcpy(&payload[1], &offset, sizeof(offset));
        return WriteMemory(address, payload, 5);
    }
}