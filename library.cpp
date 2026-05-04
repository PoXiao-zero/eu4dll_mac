#include "library.h"
#include <iostream>
#include <CoreFoundation/CoreFoundation.h>
#include "./src/base.h"
#include "./src/mainText.h"
#include "./src/tooltipAndButtonText.h"
#include "./src/mapText.h"
#include "./src/text3D.h"
#include "./src/textLayout.h"
#include "./src/saveFileName.h"
#include "./src/input.h"
#include "./src/global.h"
#include "./src/localization.h"

void hello() {
    std::cout << "Hello, World!" << std::endl;
}

void ShowErrorMessageBox(const char *title, const char *msg) {
    CFStringRef cfTitle = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);
    CFStringRef cfMsg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
    CFOptionFlags result;
    CFUserNotificationDisplayAlert(
            0, kCFUserNotificationPlainAlertLevel,
            NULL, NULL, NULL,
            cfTitle, cfMsg,
            CFSTR("exit"), CFSTR("continue"), NULL,
            &result
    );
    if (cfTitle) CFRelease(cfTitle);
    if (cfMsg) CFRelease(cfMsg);
    if (result == 0){
        exit(0);
    }
}

// 动态库加载时的构造函数
__attribute__((constructor))
static void ctor() {
    printf("eu4dll_mac [Main] 开始加载所有 HOOK 插件...\n");

    uintptr_t address = ScanMainModule("45 55 34 20 76 31");
    if (address == 0) {
        printf("eu4dll_mac [Main] 宿主程序不是EU4，停止加载。\n");
        return;
    }
    auto eu4VerStr = std::string((char *) address);
    if (eu4VerStr.rfind("EU4 v1.37", 0) != 0) {
        printf("eu4dll_mac [Main] 不受支持的版本：%s \n", eu4VerStr.c_str());
        ShowErrorMessageBox("Unsupported Game Version",eu4VerStr.c_str());
        return;
    }
    printf("eu4dll_mac [Main] eu4版本号：%s\n", eu4VerStr.c_str());

    // 支持的基础
    base::install();
    // 文字排版
    textLayout::install();
    // 主要文本显示
    mainText::install();
    // 浮动提示和部分按钮
    tooltipAndButtonText::install();
    // 地图文本
    mapText::install();
    // 战斗3D文本
    text3D::install();
    // 存档名
    saveFileName::install();
    // 允许输入非ASCII编码
    input::install();
    // 游戏右上角日期格式改为 年月日
    localization::install();

    printf("eu4dll_mac [Main] 所有插件加载完毕！\n");

    ExecutionTracker tracker = ExecutionTracker::instance();

    if (!tracker.statusMap.empty() || !tracker.findFnFailedRecord.empty()) {
        std::string msgContent = "Game Version: ";
        msgContent += eu4VerStr;
        msgContent += "\n\n";
        if (!tracker.statusMap.empty()) {
            msgContent += "Patch installation failed for: \n";
            for (const auto &[k, v]: tracker.statusMap) {
                msgContent += k;
                msgContent += "\n";
            }
            msgContent += "\n";
        }

        if (!tracker.findFnFailedRecord.empty()) {
            msgContent += "Failed to find the following functions: \n";
            msgContent += tracker.findFnFailedRecord;
            msgContent += "\n";
        }
        ShowErrorMessageBox("eu4dll_mac error",msgContent.c_str());
    }
}
