#!/bin/bash

# 获取脚本所在的当前目录
cd "$(dirname "$0")"
SCRIPT_DIR="$(pwd)"
DYLIB_NAME="libeu4dll_mac.dylib"
DYLIB_SOURCE="$SCRIPT_DIR/$DYLIB_NAME"

# ==========================================
# 1. 获取系统语言并设置多语言文本
# ==========================================
SYS_LANG=$(defaults read -g AppleLanguages | grep -E -o '^[[:space:]]*"[^"]+"' | head -1 | tr -d '"[:space:]')

case "$SYS_LANG" in
    zh*)
        # 简体/繁体中文
        MSG_PROMPT="请将 eu4.app 拖入此终端窗口，然后按回车键确认: "
        MSG_ERR_NOT_APP="❌ 错误：无效的路径或文件不存在！"
        MSG_ERR_NOT_EU4="❌ 错误：验证失败！拖入的不是 eu4.app，安装脚本已退出。"
        MSG_ERR_NO_DYLIB="❌ 错误：在脚本同目录下找不到 $DYLIB_NAME ！"
        MSG_SUDO_REQ="⚠️  修改该 App 目录需要管理员权限，请输入您的 Mac 登录密码（输入时屏幕不显示字符，输完回车即可）:"
        MSG_EXIST="⚠️  检测到 App 内已存在 $DYLIB_NAME 。请选择操作："
        MSG_OPT_1="   [1] 替换/更新补丁"
        MSG_OPT_2="   [2] 卸载补丁"
        MSG_OPT_INPUT="请输入 1 或 2，然后回车: "
        MSG_OPT_ERR="❌ 输入无效，脚本退出。"
        MSG_ASK_FS="❓ 是否需要解决全屏窗口模式下菜单栏遮挡问题？(输入 y 确认，其他键跳过): "
        MSG_UNINSTALLING="正在清理文件和 Info.plist 配置..."
        MSG_UNINSTALL_DONE="✅ 卸载完成！App 已恢复原状。"
        MSG_COPYING="正在复制 $DYLIB_NAME 到 MacOS 目录..."
        MSG_PLIST="正在修改 Info.plist..."
        MSG_SIGN="正在修复应用签名以防崩溃..."
        MSG_CACHE="正在刷新 LaunchServices 缓存..."
        MSG_SUCCESS="✅ EU4 MAC双字节补丁安装完成，现在可以安装本地化MOD后尝试运行了！"
        ;;
    ja*)
        # 日语
        MSG_PROMPT="eu4.app をこのターミナルウィンドウにドラッグし、Enterキーを押して確認してください: "
        MSG_ERR_NOT_APP="❌ エラー：無効なパス、またはファイルが存在しません！"
        MSG_ERR_NOT_EU4="❌ エラー：検証に失敗しました！ドラッグされたのは eu4.app ではありません。スクリプトを終了します。"
        MSG_ERR_NO_DYLIB="❌ エラー：スクリプトと同じディレクトリに $DYLIB_NAME が見つかりません！"
        MSG_SUDO_REQ="⚠️  このAppディレクトリを変更するには管理者権限が必要です。Macのログインパスワードを入力してください（入力中の文字は画面に表示されません）:"
        MSG_EXIST="⚠️  App内に既に $DYLIB_NAME が存在します。操作を選択してください："
        MSG_OPT_1="   [1] パッチの置換/更新"
        MSG_OPT_2="   [2] パッチのアンインストール"
        MSG_OPT_INPUT="1 または 2 を入力して、Enterキーを押してください: "
        MSG_OPT_ERR="❌ 無効な入力です。スクリプトを終了します。"
        MSG_ASK_FS="❓ ボーダレスフルスクリーン時のメニューバーの重なり問題を修正しますか？ (y を入力して確認、その他のキーでスキップ): "
        MSG_UNINSTALLING="ファイルと Info.plist の設定をクリーンアップしています..."
        MSG_UNINSTALL_DONE="✅ アンインストールが完了しました！Appは元の状態に復元されました。"
        MSG_COPYING="$DYLIB_NAME を MacOS ディレクトリにコピーしています..."
        MSG_PLIST="Info.plist を変更しています..."
        MSG_SIGN="クラッシュ防止のため、アプリの署名を修復しています..."
        MSG_CACHE="LaunchServices キャッシュを更新しています..."
        MSG_SUCCESS="✅ EU4 MAC 2バイトパッチのインストールが完了しました！ローカライズMODをインストールしてゲームを実行できます。"
        ;;
    ko*)
        # 韩语
        MSG_PROMPT="eu4.app를 이 터미널 창으로 드래그하고 Enter를 눌러 확인하십시오: "
        MSG_ERR_NOT_APP="❌ 오류: 잘못된 경로이거나 파일이 존재하지 않습니다!"
        MSG_ERR_NOT_EU4="❌ 오류: 확인 실패! 드래그한 항목이 eu4.app가 아닙니다. 스크립트가 종료되었습니다."
        MSG_ERR_NO_DYLIB="❌ 오류: 스크립트와 같은 디렉토리에서 $DYLIB_NAME 을(를) 찾을 수 없습니다!"
        MSG_SUDO_REQ="⚠️  이 App 디렉토리를 수정하려면 관리자 권한이 필요합니다. Mac 로그인 암호를 입력하십시오 (입력 시 화면에 문자가 표시되지 않습니다):"
        MSG_EXIST="⚠️  App 내에 이미 $DYLIB_NAME 이(가) 존재합니다. 작업을 선택하십시오:"
        MSG_OPT_1="   [1] 패치 교체/업데이트"
        MSG_OPT_2="   [2] 패치 제거"
        MSG_OPT_INPUT="1 또는 2를 입력하고 Enter를 누르십시오: "
        MSG_OPT_ERR="❌ 잘못된 입력입니다. 스크립트를 종료합니다."
        MSG_ASK_FS="❓ 테두리 없는 전체 화면 모드에서 메뉴 표시줄 겹침 문제를 해결하시겠습니까? (확인하려면 y 입력, 다른 키는 건너뛰기): "
        MSG_UNINSTALLING="파일 및 Info.plist 구성을 정리하는 중..."
        MSG_UNINSTALL_DONE="✅ 제거 완료! App이 원래 상태로 복구되었습니다."
        MSG_COPYING="$DYLIB_NAME 을(를) MacOS 디렉토리로 복사하는 중..."
        MSG_PLIST="Info.plist 수정 중..."
        MSG_SIGN="충돌 방지를 위해 앱 서명을 복구하는 중..."
        MSG_CACHE="LaunchServices 캐시를 새로 고치는 중..."
        MSG_SUCCESS="✅ EU4 MAC 더블 바이트 패치 설치가 완료되었습니다. 이제 로컬라이제이션 MOD를 설치하고 게임을 실행할 수 있습니다!"
        ;;
    *)
        # 默认：英语 (English Default)
        MSG_PROMPT="Please drag eu4.app into this terminal window and press Enter to confirm: "
        MSG_ERR_NOT_APP="❌ Error: Invalid path or file does not exist!"
        MSG_ERR_NOT_EU4="❌ Error: Verification failed! The dragged item is not eu4.app. The script has exited."
        MSG_ERR_NO_DYLIB="❌ Error: $DYLIB_NAME not found in the same directory as the script!"
        MSG_SUDO_REQ="⚠️  Modifying this App directory requires administrator privileges. Please enter your Mac login password (characters will not be displayed on screen): "
        MSG_EXIST="⚠️  Detected that $DYLIB_NAME already exists in the App. Please select an action: "
        MSG_OPT_1="   [1] Replace/Update patch"
        MSG_OPT_2="   [2] Uninstall patch"
        MSG_OPT_INPUT="Please enter 1 or 2, then press Enter: "
        MSG_OPT_ERR="❌ Invalid input, script exited."
        MSG_ASK_FS="❓ Do you want to fix the menu bar overlapping issue in borderless fullscreen mode? (Enter y to confirm, other keys to skip): "
        MSG_UNINSTALLING="Cleaning up files and Info.plist configuration..."
        MSG_UNINSTALL_DONE="✅ Uninstallation complete! The App has been restored to its original state."
        MSG_COPYING="Copying $DYLIB_NAME to the MacOS directory..."
        MSG_PLIST="Modifying Info.plist..."
        MSG_SIGN="Fixing app signature to prevent crashes..."
        MSG_CACHE="Refreshing LaunchServices cache..."
        MSG_SUCCESS="✅ EU4 MAC Double-Byte Patch installation complete. You can now install localization MODs and try running the game!"
        ;;
esac

# ==========================================
# 2. 拖入 eu4.app 路径并验证
# ==========================================

# 安装前确保同目录下存在 dylib 文件
if [ ! -f "$DYLIB_SOURCE" ]; then
    echo -e "\033[31m$MSG_ERR_NO_DYLIB\033[0m"
    exit 1
fi

echo -e "\033[36m$MSG_PROMPT\033[0m\c"
read -r APP_PATH

# 路径清洗（处理终端拖入可能产生的引号和空格）
APP_PATH="${APP_PATH%\'}"
APP_PATH="${APP_PATH#\'}"
APP_PATH="${APP_PATH%"${APP_PATH##*[![:space:]]}"}"
APP_PATH="${APP_PATH#"${APP_PATH%%[![:space:]]*}"}"

if [ ! -d "$APP_PATH" ]; then
    echo -e "\033[31m$MSG_ERR_NOT_APP\033[0m"
    exit 1
fi

if [[ ! -f "$APP_PATH/Contents/MacOS/eu4" ]]; then
    echo -e "\033[31m$MSG_ERR_NOT_EU4\033[0m"
    exit 1
fi

# 定义内部路径
MACOS_DIR="$APP_PATH/Contents/MacOS"
PLIST_PATH="$APP_PATH/Contents/Info.plist"
DYLIB_DEST="$MACOS_DIR/$DYLIB_NAME"

# ==========================================
# 3. 权限智能检测
# ==========================================
SUDO_CMD=""
# 如果对 Contents 目录没有写权限，则需要 sudo
if [ ! -w "$APP_PATH/Contents" ]; then
    echo -e "\033[33m$MSG_SUDO_REQ\033[0m"
    sudo -v
    if [ $? -ne 0 ]; then
        exit 1
    fi
    SUDO_CMD="sudo"
fi

# ==========================================
# 4. 检测已安装状态 (替换/卸载逻辑)
# ==========================================
ACTION="install"

if [ -f "$DYLIB_DEST" ]; then
    echo -e "\033[33m$MSG_EXIST\033[0m"
    echo -e "\033[36m$MSG_OPT_1\033[0m"
    echo -e "\033[36m$MSG_OPT_2\033[0m"
    echo -e "$MSG_OPT_INPUT\c"
    read -r USER_CHOICE
    
    if [ "$USER_CHOICE" == "2" ]; then
        ACTION="uninstall"
    elif [ "$USER_CHOICE" != "1" ]; then
        echo -e "\033[31m$MSG_OPT_ERR\033[0m"
        exit 1
    fi
fi

# ==========================================
# 5. 执行：卸载分支
# ==========================================
if [ "$ACTION" == "uninstall" ]; then
    echo "$MSG_UNINSTALLING"
    # 删除 dylib
    $SUDO_CMD rm -f "$DYLIB_DEST"
    
    # 删除 Plist 中的注入和全屏配置（忽略找不到键值的报错）
    $SUDO_CMD /usr/libexec/PlistBuddy -c "Delete :LSEnvironment:DYLD_INSERT_LIBRARIES" "$PLIST_PATH" >/dev/null 2>&1
    $SUDO_CMD /usr/libexec/PlistBuddy -c "Delete :LSUIPresentationMode" "$PLIST_PATH" >/dev/null 2>&1
    
    # 重签与刷新
    echo "$MSG_SIGN"
    $SUDO_CMD xattr -cr "$APP_PATH" >/dev/null 2>&1
    $SUDO_CMD codesign --force --deep --sign - "$APP_PATH" >/dev/null 2>&1
    echo "$MSG_CACHE"
    $SUDO_CMD /System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -f "$APP_PATH"
    
    echo -e "\033[32m$MSG_UNINSTALL_DONE\033[0m"
    exit 0
fi

# ==========================================
# 6. 执行：安装/替换分支
# ==========================================

# 询问是否修复全屏遮挡
echo -e "\033[36m$MSG_ASK_FS\033[0m\c"
read -r FS_CHOICE

# 复制文件
echo "$MSG_COPYING"
$SUDO_CMD mkdir -p "$MACOS_DIR"
$SUDO_CMD cp "$DYLIB_SOURCE" "$DYLIB_DEST"

# 修改 Info.plist
echo "$MSG_PLIST"
# 确保存在 LSEnvironment 字典
$SUDO_CMD /usr/libexec/PlistBuddy -c "Print :LSEnvironment" "$PLIST_PATH" >/dev/null 2>&1
if [ $? -ne 0 ]; then
    $SUDO_CMD /usr/libexec/PlistBuddy -c "Add :LSEnvironment dict" "$PLIST_PATH"
fi
# 注入绝对路径
$SUDO_CMD /usr/libexec/PlistBuddy -c "Delete :LSEnvironment:DYLD_INSERT_LIBRARIES" "$PLIST_PATH" >/dev/null 2>&1
$SUDO_CMD /usr/libexec/PlistBuddy -c "Add :LSEnvironment:DYLD_INSERT_LIBRARIES string $DYLIB_DEST" "$PLIST_PATH"

# 处理全屏遮挡逻辑 (LSUIPresentationMode=4)
if [[ $(echo "$FS_CHOICE" | tr '[:upper:]' '[:lower:]') == "y" ]]; then
    $SUDO_CMD /usr/libexec/PlistBuddy -c "Delete :LSUIPresentationMode" "$PLIST_PATH" >/dev/null 2>&1
    $SUDO_CMD /usr/libexec/PlistBuddy -c "Add :LSUIPresentationMode integer 4" "$PLIST_PATH"
fi

# 修复签名
echo "$MSG_SIGN"
$SUDO_CMD xattr -cr "$APP_PATH" >/dev/null 2>&1
$SUDO_CMD codesign --force --deep --sign - "$APP_PATH" >/dev/null 2>&1

# 刷新缓存
echo "$MSG_CACHE"
$SUDO_CMD /System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -f "$APP_PATH"

# 完成
echo -e "\033[32m$MSG_SUCCESS\033[0m"
exit 0