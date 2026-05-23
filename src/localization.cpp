#include "localization.h"
#include "global.h"
#include "strConvert.h"
#include "pinyinHelper.h"
#include <CoreFoundation/CoreFoundation.h>

namespace localization {
/**
 修改函数：CTopbarGui::RefreshSpeedControlsWindow
 作用：将此函数使用的日期格式文本从"d w mw w y"修改为"y  mw d "
*/
    void install_CTopbarGui_RefreshSpeedControlsWindow() {
        TRACK_FUNCTION();
        uintptr_t matchAddress = ScanMainModule("64 20 77 20 6D 77 20 77 20 79");
        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t leaAddress = matchAddress;
        uint8_t patch[] = {0x79, 0x20, 0x0F, 0x20, 0x6D, 0x77, 0x20, 0x64, 0x20, 0x0E};
        WriteMemory(leaAddress, patch, 10);
        printf("eu4dll_mac [Success] %s WriteMemory 匹配地址:0x%lx 写入地址:0x%lx\n", __func__, matchAddress,
               leaAddress);
        SET_SUCCESS();
    }

/**
 Hook函数：LocalizeYmlAddKey
 作用：拦截替换本地化文件解析过程中对ConvertUTF8ToLatin1函数的调用，使其支持将未转码的UTF8文本转换为游戏内可显示的逃逸文本而不是用‘?’代替，从而无需预先转码yml文件，便于快速翻译MOD。
 */
    void install_LocalizeYmlAddKey() {
        TRACK_FUNCTION();
        uintptr_t matchAddress = ScanMainModule(
                "48 89 45 A0 4C 89 65 A8 4C 89 6D C0 48 8D 35 ? ? ? ? 48 89 DF 4C 89 EA");
        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t address = matchAddress + 0x19;
        ReplaceCall(address, (uintptr_t) strConvert::UTF8ToWindows1252);
        printf("eu4dll_mac [Success] %s ReplaceCall 匹配地址:0x%lx 写入地址:0x%lx\n", __func__,
               matchAddress, address);
        SET_SUCCESS();
    }

    struct CCountryTag {
        char tag[4];
        int16_t nIndex;
        uint8_t type;
        bool bIsValid;
    };
    struct SGotoBoxSearchEntry {
        int nDistance;
        int nProvince;
        CCountryTag Country;
        std::string AreaTag;
        std::string OriginalName;
        std::string CleanedName;
        std::string CleanedLocalName;
    };

    //辅助函数，用于设置模糊搜索匹配项的排序权重
    int GetSubstrScore(const std::string &text, const std::string &sub) {
        auto pos = text.find(sub);

        if (pos == -1) return -99;

        if (pos == 0) return -2;

        if (pos + sub.length() == text.length()) return 1;

        size_t mid = text.length() / 2;
        return (pos < mid) ? -1 : 0;
    }

    bool
    Proxy_CGotoBoxSearchList_Process_Comparing(bool flag, std::string *searchString, SGotoBoxSearchEntry *entry) {
        if (searchString->empty()) return false;
        if (searchString->find('\x10') != -1 || searchString->find('\x11') != -1 || searchString->find('\x12') != -1 ||
            searchString->find('\x13') != -1) {

            if (*searchString == entry->CleanedName) {
                entry->nDistance = -3;
                return true;
            }

            if (!flag) {
                auto score = GetSubstrScore(entry->CleanedName, *searchString);
                if (score > -99) {
                    entry->nDistance = score;
                    return true;
                }
            }
            return false;
        }

        auto pinyin = *PinyinHelper::getInstance().getPinyin(entry->OriginalName);
        if (flag) {
            return std::any_of(pinyin.begin(), pinyin.end(), [&searchString](const std::string &sub) {
                return *searchString == sub;
            });
        } else {
            int maxScore = 99;
            for (const std::string &sub: pinyin) {
                if (sub == *searchString) {
                    maxScore = -3;
                }
                auto score = GetSubstrScore(sub, *searchString);
                if (score > -99) {
                    if (score < maxScore) {
                        maxScore = score;
                    }
                }
            }
            if (maxScore != 99) {
                entry->nDistance = maxScore;
                return true;
            }
            return false;
        }
    }

    extern "C" uintptr_t g_Process_Comparing_Addr = (uintptr_t) Proxy_CGotoBoxSearchList_Process_Comparing;
    extern "C" uintptr_t g_Process_success_RetAddr = 0;
    extern "C" uintptr_t g_Process_fail_RetAddr = 0;

    __attribute__((naked)) void naked_CGotoBoxSearchList_Process() {
        __asm__ volatile (
                ".intel_syntax noprefix \n"
                "imul r12, r15, 0x70 \n"
                "movzx rdi, byte ptr [rbp-0x44] \n"

                "mov rsi, r13 \n"

                "push rdx \n"
                "lea rbx, [rdx + r12] \n"
                "mov rdx, rbx \n"

                "call [rip + _g_Process_Comparing_Addr] \n"
                "pop rdx \n"
                "test al, al \n"
                "jz 1f \n"
                "mov rax, [rbp - 0x30] \n"
                "mov byte ptr [rax + 0x18], 1 \n"
                "jmp [rip + _g_Process_success_RetAddr] \n"

                "1: \n"
                "jmp [rip + _g_Process_fail_RetAddr] \n"
                ".att_syntax prefix \n"
                );
    }

/**
 Hook函数：CGotoBoxSearchList::Process
 作用：拦截修改EU4查找功能的比对逻辑，使其支持拼音和首字母缩写搜索
 */
    void install_CGotoBoxSearchList_Process() {
        TRACK_FUNCTION();
        uintptr_t matchAddress = ScanMainModule("4D 6B E7 70 80 7D BC 00 74 21");
        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t address = matchAddress;
        g_Process_success_RetAddr = address + 0x1E;
        g_Process_fail_RetAddr = address + 0x76;

        HookJMP(address, (uintptr_t) naked_CGotoBoxSearchList_Process);
        printf("eu4dll_mac [Success] %s HookJMP 匹配地址:0x%lx 写入地址:0x%lx 匹配成功返回地址:0x%lx 匹配失败返回地址:0x%lx\n",
               __func__,
               matchAddress, address, g_Process_success_RetAddr, g_Process_fail_RetAddr);
        SET_SUCCESS();
    }

    std::string GetHostResourcePath() {
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        if (!mainBundle) return "";

        CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
        if (!resourcesURL) return "";

        char path[PATH_MAX];
        if (CFURLGetFileSystemRepresentation(resourcesURL, true, (UInt8 *) path, PATH_MAX)) {
            CFRelease(resourcesURL);
            return {path};
        }

        CFRelease(resourcesURL);
        return "";
    }

    template<typename T, typename SizeType = int>
    struct CPdxArray {
        void *vftable;         // 0x00: 虚函数表
        T *_pData;             // 0x08: 指向数组连续内存的指针
        SizeType _nCapacity;   // 0x10: 容量 (4 字节 int)
        SizeType _nSize;       // 0x14: 实际大小 (4 字节 int)
    };

    typedef void *(*fnCDLCManager_AccessInstance_t)(void *pThis);

    typedef void *(*fnCPdxCommandLine_HasOption_t)(void *pThis, const char *str);

    void *fnCDLCManager_AccessInstance_Addr = nullptr;
    void *fnCPdxCommandLine_HasOption_Addr = nullptr;

    bool Proxy_CPdxCommandLine_HasOption(void *obj, const char *str) {
        auto getInstance = (fnCDLCManager_AccessInstance_t) fnCDLCManager_AccessInstance_Addr;
        void *instance = getInstance(nullptr);
        auto *enabledMods = reinterpret_cast<CPdxArray<std::string, int> *>(
                (uintptr_t) instance + 0x70
        );

        for (int i = 0; i < enabledMods->_nSize; ++i) {
            const std::string &modNameOrPath = enabledMods->_pData[i];
            if (modNameOrPath.find("eu4_chinese.mod") != -1) {
                auto patch = GetHostResourcePath();
                if (patch.empty()) {
                    printf("加载字典资源路径失败！\n");
                    break;
                }
                patch += "/chinese_dict";
                if (!std::filesystem::exists(patch)) {
                    printf("字典路径不存在！%s\n", patch.c_str());
                    break;
                }
                Pinyin::setDictionaryPath(patch);
                install_CGotoBoxSearchList_Process();
                break;
            }
            printf("Mod [%d]: %s\n", i, modNameOrPath.c_str());
        }
        auto org = (fnCPdxCommandLine_HasOption_t) fnCPdxCommandLine_HasOption_Addr;
        return org(obj, str);
    }

/**
 Hook函数：_main
 作用：HOOK游戏初始化函数，在加载工作完成后检查是否启用了汉化模组，如果启用了就开启拼音搜索功能
 */
    void install_main() {
        TRACK_FUNCTION();
        uintptr_t matchAddress = ScanMainModule("48 8B BD 98 FA FF FF E8 ? ? ? ? 84 C0 0F 84");
        if (matchAddress == 0) {
            printf("eu4dll_mac [Error] %s 特征码查找失败！\n", __func__);
            return;
        }
        uintptr_t address = matchAddress + 7;
        ReplaceCall(address, (uintptr_t) Proxy_CPdxCommandLine_HasOption);
        printf("eu4dll_mac [Success] %s ReplaceCall 匹配地址:0x%lx 写入地址:0x%lx\n",
               __func__,
               matchAddress, address);
        SET_SUCCESS();
    }

    void install() {
        //游戏界面右上角日期显示格式修改为年月日
        install_CTopbarGui_RefreshSpeedControlsWindow();
        //加载本地化文件时自动将UTF8转码为逃逸文本，从而无需预先转码文件
        install_LocalizeYmlAddKey();

        fnCDLCManager_AccessInstance_Addr = findFn("_ZN11CDLCManager14AccessInstanceEv");
        fnCPdxCommandLine_HasOption_Addr = findFn("_ZNK15CPdxCommandLine9HasOptionEPKc");

        //根据是否加载汉化MOD来决定是否启用拼音搜索功能
        install_main();
    }
}
