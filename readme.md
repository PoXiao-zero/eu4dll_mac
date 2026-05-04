# eu4dll_mac

本项目是 [matanki-saito/EU4dll](https://github.com/matanki-saito/EU4dll) 项目的 macOS 实现，皆在使MAC玩家也能享受到原游戏不支持的本地化MOD。

在此非常感谢原项目的贡献，没有原项目也就没有这个项目。

This project is the macOS implementation of the [matanki-saito/EU4dll](https://github.com/matanki-saito/EU4dll) project, aiming to allow Mac players to enjoy localization mods that are not supported by the vanilla game.

Special thanks to the contributions of the original project; without it, this project wouldn't exist.

* 本项目基于`GOG平台EU4 1.37.5`版本开发，理论上也支持其他1.37版本，但不会支持低于1.37的版本。
* 原项目中使用 `¿` 字符开启颠倒姓名的功能未实现。


* This project is developed based on the `GOG version of EU4 1.37.5`. Theoretically, it supports other 1.37 versions, but versions below 1.37 will not be supported.
* The feature from the original project that uses the `¿` character to reverse names has not been implemented.

## 运行截图（Screenshots）

![游戏内效果展示](./screenshot.png)

### 已测试通过的环境（Tested Environments）
* **CPU Architecture**: Apple Silicon M2
* **Operating System**: macOS 14.3.1 (Sonoma)
* **Game Version**: GOG Platform EU4 1.37.5.0
* **SIP**: enabled

目前**尚未测试** Steam 版本的兼容性。

原因很简单：作者目前囊中羞涩买不起 Steam 版，正苦等 `Ultimate Bundle` (终极包) 出现史低价格。

*(如果有好心的大佬愿意赠送/赞助一份进行测试，作者将感激不尽！)*

Currently, compatibility with the Steam version has **not been tested**.

The reason is simple: the author is currently tight on budget and cannot afford the Steam version, and is desperately waiting for a historical low price for the `Ultimate Bundle`.

*(If any kind-hearted sponsor is willing to gift/sponsor a copy for testing, the author would be deeply grateful!)*

## 安装教程（Installation Guide）

在 [Releases](https://github.com/PoXiao-zero/eu4dll_mac/releases) 页面下载最新的压缩包，解压后您会看到 `libeu4dll_mac.dylib` (核心动态库) 和 `install.sh` (自动化脚本)。

Download the latest archive from the Releases page. After extracting it, you will see `libeu4dll_mac.dylib` (the core dynamic library) and `install.sh` (the automation script).

### 自动安装（Automatic Installation）

1. 打开 Mac 自带的 **终端** 应用程序。

   Open the built-in **Terminal** application on your Mac.
2. 输入`chmod +x `后拖入解压出来的 `install.sh`后回车。

   Input `chmod +x `, drag the extracted install.sh file into the terminal, and press Enter.
   
   示例（Example）：`chmod +x /xx/install.sh`
3. 再次将`install.sh` 拖入终端窗口，按下回车键。

   Drag install.sh into the terminal window again and press Enter.


### 手动安装（Manual Installation）

1. **复制文件（Copy the file）**：

   右键点击 `eu4.app` 选择“显示包内容”。将 `libeu4dll_mac.dylib` 复制到 `Contents/MacOS/` 目录下。
   
   Right-click on `eu4.app` and select "Show Package Contents". Copy `libeu4dll_mac.dylib` into the `Contents/MacOS/` directory.

2. **修改 Info.plist（Modify Info.plist）**：

   使用编辑器打开 `Contents/Info.plist`。
   
   Open `Contents/Info.plist` with a text editor.

   在末尾`</dict>`上方添加以下内容后保存：
   
   Add the following content right above the last `</dict>` at the end of the file, then save:
    ```xml
    <key>LSEnvironment</key>
    <dict>
	    <key>DYLD_INSERT_LIBRARIES</key>
	    <string>绝对路径（Absolute_Path）/eu4.app/Contents/MacOS/libeu4dll_mac.dylib</string>
    </dict>
    ```
3. **修复签名（Fix Signature）**：
   
   在终端执行以下命令：
   
   Run the following command in the terminal:
   ```bash
   xattr -cr /绝对路径（Absolute Path）/eu4.app
   codesign --force --deep --sign - /绝对路径（Absolute Path）/eu4.app
   ```
4. **刷新缓存（Refresh Cache）**：

   在终端执行以下命令使 Plist 修改立即生效：

   Run the following command in the terminal to make the Plist changes take effect immediately:
   ```bash
   /System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -f /绝对路径（Absolute_Path）/eu4.app
   ```
#### 解决原游戏在`全屏窗口`模式下菜单栏遮挡问题
#### Fix the menu bar overlap in `Borderless Windowed` mode for the vanilla game.
* **Info.plist**文件添加以下内容：
* Add the following content to the **Info.plist** file:
   ```xml
   <key>LSUIPresentationMode</key>
   <integer>4</integer>
   ```
