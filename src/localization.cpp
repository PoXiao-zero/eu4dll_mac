#include "localization.h"
#include "global.h"

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

    void install() {
        //游戏界面右上角日期显示格式修改为年月日
        install_CTopbarGui_RefreshSpeedControlsWindow();
    }
}
