#include "localization.h"
#include "global.h"
#include "strConvert.h"

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

    void install() {
        //游戏界面右上角日期显示格式修改为年月日
        install_CTopbarGui_RefreshSpeedControlsWindow();
        //加载本地化文件时自动将UTF8转码为逃逸文本，从而无需预先转码文件
        install_LocalizeYmlAddKey();
    }
}
