#include "memoryHelper.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>

#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/vm_prot.h>
#include <dlfcn.h>

static std::vector<int> ParsePattern(const std::string &pattern) {
    std::vector<int> bytes;
    std::istringstream iss(pattern);
    std::string word;
    while (iss >> word) {
        if (word == "?") {
            bytes.push_back(-1); // 模糊匹配占位符
        } else if (word == "S") {
            bytes.push_back(-2); // 字符串偏移占位符
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
    kern_return_t kr = mach_vm_protect(task, dst, size, FALSE,
                                       VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY | VM_PROT_EXECUTE);
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

bool ReadStringSafe(uintptr_t address, std::string &outStr, size_t maxLen = 256) {
    outStr.clear();
    if (!address) return false;

    mach_port_t task = mach_task_self();
    char *buffer = new char[maxLen]();
    mach_vm_size_t out_size = 0;

    kern_return_t kr = mach_vm_read_overwrite(task, address, maxLen, (mach_vm_address_t) buffer, &out_size);
    if (kr == KERN_SUCCESS) {
        buffer[maxLen - 1] = '\0';
        outStr = buffer;
        delete[] buffer;
        return true;
    }
    delete[] buffer;
    return false;
}

uintptr_t FindPattern(uintptr_t startAddress, size_t searchSize, const std::string &pattern,
                      const std::vector<std::string> &targetStrings) {
    std::vector<int> patternBytes = ParsePattern(pattern);
    size_t patternLen = patternBytes.size();
    if (patternLen == 0 || searchSize < patternLen) return 0;

    std::vector<size_t> stringOffsetIndices;
    for (size_t i = 0; i < patternLen;) {
        if (patternBytes[i] == -2) {
            stringOffsetIndices.push_back(i);
            while (i < patternLen && patternBytes[i] == -2) {
                i++;
            }
        } else {
            i++;
        }
    }

    if (stringOffsetIndices.size() != targetStrings.size()) {
        std::cerr << "[FindPattern] Error: Placeholder group count (" << stringOffsetIndices.size()
                  << ") does not match the number of target strings (" << targetStrings.size() << ") 不匹配！"
                  << std::endl;
        return 0;
    }

    auto *scanBytes = reinterpret_cast<uint8_t *>(startAddress);

    for (size_t i = 0; i <= searchSize - patternLen; ++i) {
        bool found = true;
        for (size_t j = 0; j < patternLen; ++j) {
            if (patternBytes[j] == -1 || patternBytes[j] == -2) continue;
            if (scanBytes[i + j] != patternBytes[j]) {
                found = false;
                break;
            }
        }

        if (found) {
            bool allStringsMatch = true;

            for (size_t k = 0; k < stringOffsetIndices.size(); ++k) {
                size_t offsetIndex = stringOffsetIndices[k];

                // 提取 4 字节偏移
                int32_t offset = 0;
                std::memcpy(&offset, &scanBytes[i + offsetIndex], sizeof(int32_t));

                // 计算当前占位符对应的 RIP 地址
                uintptr_t ripAddress = startAddress + i + offsetIndex + 4;
                uintptr_t stringAddress = ripAddress + offset;

                // 安全读取并对比
                std::string readStr;
                if (ReadStringSafe(stringAddress, readStr, targetStrings[k].length() + 1)) {
                    if (readStr != targetStrings[k]) {
                        allStringsMatch = false;
                        break;
                    }
                } else {
                    allStringsMatch = false;
                    break;
                }
            }

            if (allStringsMatch) {
                return startAddress + i;
            }
        }
    }
    std::cerr << "[FindPattern] Error: Pattern not found." << std::endl;
    return 0;
}

uintptr_t ScanMainModule(const std::string &pattern) {
    uintptr_t base = GetMainModuleBase();
    size_t size = GetMainModuleSize();
    return FindPattern(base, size, pattern, {});
}

uintptr_t ScanMainModule(const std::string &pattern, const std::vector<std::string> &targetStrings) {
    uintptr_t base = GetMainModuleBase();
    size_t size = GetMainModuleSize();
    return FindPattern(base, size, pattern, targetStrings);
}

uintptr_t
ScanMainModule(const std::string &pattern, const std::vector<std::string> &targetStrings, const char *symbolName,
               size_t maxSearchSize) {
    void *symAddr = dlsym(RTLD_DEFAULT, symbolName);
    if (!symAddr) {
        std::cerr << "[ScanMainModule] Error: Symbol not found " << symbolName << std::endl;
        return 0;
    }
    return FindPattern((uintptr_t) symAddr, maxSearchSize, pattern, targetStrings);
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

void OptimizeNakedHook(uintptr_t func_ptr, size_t max_scan_size) {
    if (!func_ptr) return;

    auto *code = reinterpret_cast<uint8_t *>(func_ptr);

    for (size_t i = 0; i <= max_scan_size - 6;) {
        uint8_t b0 = code[i];
        uint8_t b1 = code[i + 1];

        // 匹配 FF 25 (JMP) 或 FF 15 (CALL)
        if (b0 == 0xFF && (b1 == 0x25 || b1 == 0x15)) {
            bool isCall = (b1 == 0x15);
            uintptr_t instruction_addr = func_ptr + i;

            int32_t offset = *reinterpret_cast<int32_t *>(&code[i + 2]);
            uintptr_t variable_addr = instruction_addr + 6 + offset;

            uintptr_t target_addr = *reinterpret_cast<uintptr_t *>(variable_addr);
            // 计算新的相对跳转偏移量
            int64_t offset64 = static_cast<int64_t>(target_addr) - static_cast<int64_t>(instruction_addr + 5);

            // 检查 2GB 的 x86_64 rel32 安全边界
            if (offset64 >= -2147483648LL && offset64 <= 2147483647LL) {
                uint8_t patch_data[6];
                patch_data[0] = isCall ? 0xE8 : 0xE9;
                *reinterpret_cast<int32_t *>(&patch_data[1]) = static_cast<int32_t>(offset64);
                patch_data[5] = 0x90; // NOP
                if (WriteMemory(instruction_addr, patch_data, 6)) {
                    printf("[+] Optimized %s at %p -> Target: %p\n",
                           isCall ? "CALL" : "JMP", (void *) instruction_addr, (void *) target_addr);
                }
            }

            i += 6;

            if (!isCall) {
                // naked hook函数结尾基本都是 0F 0B，以此来判断是否需要提前结束扫描
                if (i + 1 < max_scan_size && code[i] == 0x0F && code[i + 1] == 0x0B) {
                    printf("[i] Found ud2 right after JMP. Stopping scan for this function. base_addr:0x%lx i:0x%lx\n",
                           func_ptr, i);
                    break;
                }
            }
        } else {
            i++;
        }
    }
}