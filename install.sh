#!/bin/bash

# 获取脚本所在的当前目录
cd "$(dirname "$0")"
SCRIPT_DIR="$(pwd)"
DYLIB_NAME="libeu4dll_mac.dylib"
DYLIB_SOURCE="$SCRIPT_DIR/$DYLIB_NAME"
INSERT_TOOL="$SCRIPT_DIR/insert_dylib"
DICT_NAME="chinese_dict"
DICT_SOURCE="$SCRIPT_DIR/$DICT_NAME"

# ==========================================
# 1. 获取系统语言并设置多语言文本
# ==========================================
SYS_LANG=$(defaults read -g AppleLanguages | grep -E -o '^[[:space:]]*"[^"]+"' | head -1 | tr -d '"[:space:]')

case "$SYS_LANG" in
    zh*)
        MSG_PROMPT="请将 eu4.app 拖入此终端窗口，然后按回车键确认: "
        MSG_ERR_NOT_APP="❌ 错误：无效的路径或文件不存在！"
        MSG_ERR_NOT_EU4="❌ 错误：验证失败！拖入的不是 eu4.app，安装脚本已退出。"
        MSG_ERR_NO_DYLIB="❌ 错误：在脚本同目录下找不到 $DYLIB_NAME ！"
        MSG_ERR_NO_TOOL="❌ 错误：在脚本同目录下找不到注入工具 insert_dylib ！"
        MSG_SUDO_REQ="⚠️  修改该 App 目录需要管理员权限，请输入您的 Mac 登录密码（输入时屏幕不显示字符，输完回车即可）:"
        MSG_EXIST="⚠️  检测到可执行文件已被注入过补丁。请选择操作："
        MSG_OPT_1="   [1] 替换/更新补丁"
        MSG_OPT_2="   [2] 卸载补丁"
        MSG_OPT_INPUT="请输入 1 或 2，然后回车: "
        MSG_OPT_ERR="❌ 输入无效，脚本退出。"
        MSG_ASK_FS="❓ 是否需要解决全屏窗口模式下菜单栏遮挡问题？(输入 y 确认，其他键跳过): "
        MSG_UNINSTALLING="正在清理文件并恢复可执行文件..."
        MSG_UNINSTALL_DONE="✅ 卸载完成！App 已恢复原状。"
        MSG_BACKUP="正在备份可执行文件..."
        MSG_COPYING="正在复制 $DYLIB_NAME 到 Frameworks 目录..."
        MSG_COPY_DICT="正在复制中文拼音字典到 Resources 目录..."
        MSG_WARN_NO_DICT="⚠️ 当前目录下找不到拼音字典目录 ($DICT_NAME)，缺失字典将导致拼音和首字母查找功能无法使用。"
        MSG_INJECTING="正在向二进制可执行文件注入补丁..."
        MSG_ERR_INJECT_FAIL="❌ 错误：二进制注入失败！详情如下："
        MSG_ERR_NO_BACKUP_DIRTY="❌ 致命错误：检测到游戏已被注入，但找不到原版备份文件！为防止损坏，请在 Steam 中验证游戏完整性后再试。"
        MSG_WARN_NO_BACKUP="⚠️ 警告：找不到备份文件，无法恢复原版可执行文件。建议在 Steam 验证完整性。"
        MSG_PLIST="正在配置 Info.plist..."
        MSG_SIGN="正在修复应用签名以防崩溃..."
        MSG_CACHE="正在刷新 LaunchServices 缓存..."
        MSG_SUCCESS="🎉 EU4 MAC双字节补丁安装完成！现在可以开始游戏啦！后续如需卸载补丁可再次运行本安装脚本。本补丁问题反馈地址：https://github.com/PoXiao-zero/eu4dll_mac"
        DICT_BEHAVIOR="auto"
        ;;
    ja*)
        MSG_PROMPT="eu4.app をこのターミナルウィンドウにドラッグし、Enterキーを押して確認してください: "
        MSG_ERR_NOT_APP="❌ エラー：無効なパス、またはファイルが存在しません！"
        MSG_ERR_NOT_EU4="❌ エラー：検証に失敗しました！ドラッグされたのは eu4.app ではありません。スクリプトを終了します。"
        MSG_ERR_NO_DYLIB="❌ エラー：スクリプトと同じディレクトリに $DYLIB_NAME が見つかりません！"
        MSG_ERR_NO_TOOL="❌ エラー：スクリプトと同じディレクトリにツール insert_dylib が見つかりません！"
        MSG_SUDO_REQ="⚠️  このAppディレクトリを変更するには管理者権限が必要です。Macのログインパスワードを入力してください（入力中の文字は画面に表示されません）:"
        MSG_EXIST="⚠️  実行ファイルには既にパッチがインジェクトされています。操作を選択してください："
        MSG_OPT_1="   [1] パッチの置換/更新"
        MSG_OPT_2="   [2] パッチのアンインストール"
        MSG_OPT_INPUT="1 または 2 を入力して、Enterキーを押してください: "
        MSG_OPT_ERR="❌ 無効な入力です。スクリプトを終了します。"
        MSG_ASK_FS="❓ ボーダレスフルスクリーン時のメニューバーの重なり問題を修正しますか？ (y を入力して確認、その他のキーでスキップ): "
        MSG_UNINSTALLING="ファイルをクリーンアップし、実行ファイルを復元しています..."
        MSG_UNINSTALL_DONE="✅ アンインストールが完了しました！Appは元の状態に復元されました。"
        MSG_BACKUP="実行ファイルをバックアップしています..."
        MSG_COPYING="$DYLIB_NAME を Frameworks ディレクトリにコピーしています..."
        MSG_INJECTING="実行ファイルにパッチをインジェクトしています..."
        MSG_ERR_INJECT_FAIL="❌ エラー：バイナリインジェクトに失敗しました！詳細："
        MSG_ERR_NO_BACKUP_DIRTY="❌ 致命的なエラー：ゲームは既にインジェクトされていますが、元のバックアップファイルが見つかりません！破損を防ぐため、Steamでゲームファイルの整合性を確認してから再試行してください。"
        MSG_WARN_NO_BACKUP="⚠️ 警告：バックアップファイルが見つからないため、元の実行ファイルを復元できません。Steamで整合性を確認することをお勧めします。"
        MSG_PLIST="Info.plist を構成しています..."
        MSG_SIGN="クラッシュ防止のため、アプリの署名を修復しています..."
        MSG_CACHE="LaunchServices キャッシュを更新しています..."
        MSG_SUCCESS="🎉 EU4 MAC 2バイトパッチのインストールが完了しました！これでゲームを開始できます！後でパッチをアンインストールする必要がある場合は、このインストールスクリプトを再度実行してください。本パッチのフィードバック先：https://github.com/PoXiao-zero/eu4dll_mac"
        DICT_BEHAVIOR="skip"
        ;;
    ko*)
        MSG_PROMPT="eu4.app를 이 터미널 창으로 드래그하고 Enter를 눌러 확인하십시오: "
        MSG_ERR_NOT_APP="❌ 오류: 잘못된 경로이거나 파일이 존재하지 않습니다!"
        MSG_ERR_NOT_EU4="❌ 오류: 확인 실패! 드래그한 항목이 eu4.app가 아닙니다. 스크립트가 종료되었습니다."
        MSG_ERR_NO_DYLIB="❌ 오류: 스크립트와 같은 디렉토리에서 $DYLIB_NAME 을(를) 찾을 수 없습니다!"
        MSG_ERR_NO_TOOL="❌ 오류: 스크립트와 같은 디렉토리에서 insert_dylib 도구를 찾을 수 없습니다!"
        MSG_SUDO_REQ="⚠️  이 App 디렉토리를 수정하려면 관리자 권한이 필요합니다. Mac 로그인 암호를 입력하십시오 (입력 시 화면에 문자가 표시되지 않습니다):"
        MSG_EXIST="⚠️  실행 파일에 이미 패치가 주입되어 있습니다. 작업을 선택하십시오:"
        MSG_OPT_1="   [1] 패치 교체/업데이트"
        MSG_OPT_2="   [2] 패치 제거"
        MSG_OPT_INPUT="1 또는 2를 입력하고 Enter를 누르십시오: "
        MSG_OPT_ERR="❌ 잘못된 입력입니다. 스크립트를 종료합니다."
        MSG_ASK_FS="❓ 테두리 없는 전체 화면 모드에서 메뉴 표시줄 겹침 문제를 해결하시겠습니까? (확인하려면 y 입력, 다른 키는 건너뛰기): "
        MSG_UNINSTALLING="파일을 정리하고 실행 파일을 복원하는 중..."
        MSG_UNINSTALL_DONE="✅ 제거 완료! App이 원래 상태로 복구되었습니다."
        MSG_BACKUP="실행 파일을 백업하는 중..."
        MSG_COPYING="$DYLIB_NAME 을(를) Frameworks 디렉토리로 복사하는 중..."
        MSG_INJECTING="실행 파일에 패치를 주입하는 중..."
        MSG_ERR_INJECT_FAIL="❌ 오류: 바이너리 주입 실패! 상세:"
        MSG_ERR_NO_BACKUP_DIRTY="❌ 치명적인 오류: 게임이 이미 주입된 것으로 감지되었으나, 원본 백업 파일을 찾을 수 없습니다! 손상을 방지하려면 Steam에서 게임 무결성을 확인한 후 다시 시도하십시오."
        MSG_WARN_NO_BACKUP="⚠️ 경고: 백업 파일을 찾을 수 없어 원본 실행 파일을 복원할 수 없습니다. Steam에서 무결성을 확인하는 것이 좋습니다."
        MSG_PLIST="Info.plist 구성 중..."
        MSG_SIGN="충돌 방지를 위해 앱 서명을 복구하는 중..."
        MSG_CACHE="LaunchServices 캐시를 새로 고치는 중..."
        MSG_SUCCESS="🎉 EU4 MAC 더블 바이트 패치 설치가 완료되었습니다! 이제 게임을 시작할 수 있습니다! 나중에 패치를 제거해야 할 경우 이 설치 스크립트를 다시 실행하시면 됩니다. 패치 관련 피드백 주소: https://github.com/PoXiao-zero/eu4dll_mac"
        DICT_BEHAVIOR="skip"
        ;;
    *)
        MSG_PROMPT="Please drag eu4.app into this terminal window and press Enter to confirm: "
        MSG_ERR_NOT_APP="❌ Error: Invalid path or file does not exist!"
        MSG_ERR_NOT_EU4="❌ Error: Verification failed! The dragged item is not eu4.app. The script has exited."
        MSG_ERR_NO_DYLIB="❌ Error: $DYLIB_NAME not found in the same directory as the script!"
        MSG_ERR_NO_TOOL="❌ Error: Tool insert_dylib not found in the same directory as the script!"
        MSG_SUDO_REQ="⚠️  Modifying this App directory requires administrator privileges. Please enter your Mac login password (characters will not be displayed on screen): "
        MSG_EXIST="⚠️  Detected that the executable is already injected with the patch. Please select an action: "
        MSG_OPT_1="   [1] Replace/Update patch"
        MSG_OPT_2="   [2] Uninstall patch"
        MSG_OPT_INPUT="Please enter 1 or 2, then press Enter: "
        MSG_OPT_ERR="❌ Invalid input, script exited."
        MSG_ASK_FS="❓ Do you want to fix the menu bar overlapping issue in borderless fullscreen mode? (Enter y to confirm, other keys to skip): "
        MSG_UNINSTALLING="Cleaning up files and restoring the executable..."
        MSG_UNINSTALL_DONE="✅ Uninstallation complete! The App has been restored to its original state."
        MSG_BACKUP="Backing up the executable..."
        MSG_COPYING="Copying $DYLIB_NAME to the Frameworks directory..."
        MSG_ASK_DICT="❓ Do you want to install the Chinese Pinyin dictionary? (Enter y to confirm, other keys to skip): "
        MSG_COPY_DICT="Copying Chinese Pinyin dictionary to Resources directory..."
        MSG_WARN_NO_DICT="⚠️ Chinese Pinyin dictionary ($DICT_NAME) not found, skipping."
        MSG_INJECTING="Injecting patch into the binary executable..."
        MSG_ERR_INJECT_FAIL="❌ Error: Binary injection failed! Details:"
        MSG_ERR_NO_BACKUP_DIRTY="❌ Fatal Error: The game is already injected, but the original backup file is missing! To prevent damage, please verify game integrity in Steam and try again."
        MSG_WARN_NO_BACKUP="⚠️ Warning: Backup file not found, skipping executable restoration. It is recommended to verify game integrity in Steam."
        MSG_PLIST="Configuring Info.plist..."
        MSG_SIGN="Fixing app signature to prevent crashes..."
        MSG_CACHE="Refreshing LaunchServices cache..."
        MSG_SUCCESS="🎉 EU4 MAC Double-Byte Patch installation complete! You can now start the game! If you need to uninstall the patch later, you can run this installation script again. Feedback/Issues: https://github.com/PoXiao-zero/eu4dll_mac"
        DICT_BEHAVIOR="ask"
        ;;
esac

# ==========================================
# 2. 检查依赖与路径验证
# ==========================================

if [ ! -f "$DYLIB_SOURCE" ]; then
    echo -e "\033[31m$MSG_ERR_NO_DYLIB\033[0m"
    exit 1
fi

if [ ! -f "$INSERT_TOOL" ]; then
    echo -e "\033[31m$MSG_ERR_NO_TOOL\033[0m"
    exit 1
fi

echo -e "\033[36m$MSG_PROMPT\033[0m\c"
read -r APP_PATH

# 路径清洗（处理终端拖入可能产生的引号和空格）
APP_PATH="${APP_PATH%\'}"
APP_PATH="${APP_PATH#\'}"
APP_PATH="${APP_PATH//\\ / }"
APP_PATH="${APP_PATH%"${APP_PATH##*[![:space:]]}"}"
APP_PATH="${APP_PATH#"${APP_PATH%%[![:space:]]*}"}"

if [ ! -d "$APP_PATH" ]; then
    echo -e "\033[31m$MSG_ERR_NOT_APP\033[0m"
    exit 1
fi

EXEC_PATH="$APP_PATH/Contents/MacOS/eu4"
if [[ ! -f "$EXEC_PATH" ]]; then
    echo -e "\033[31m$MSG_ERR_NOT_EU4\033[0m"
    exit 1
fi

# 定义内部路径
FRAMEWORKS_DIR="$APP_PATH/Contents/Frameworks"
RESOURCES_DIR="$APP_PATH/Contents/Resources"
PLIST_PATH="$APP_PATH/Contents/Info.plist"
DYLIB_DEST="$FRAMEWORKS_DIR/$DYLIB_NAME"
BACKUP_PATH="${EXEC_PATH}_bak"
INJECT_DYLIB_PATH="@executable_path/../Frameworks/$DYLIB_NAME"

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
# 4. 检测已安装状态
# ==========================================
ACTION="install"
IS_INJECTED=0

# 使用 grep 快速查找二进制文件中是否已包含我们的 dylib 名字
if grep -qF "$DYLIB_NAME" "$EXEC_PATH"; then
    IS_INJECTED=1
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

    # 强制恢复原版文件
    if [ -f "$BACKUP_PATH" ]; then
        $SUDO_CMD rm -f "$EXEC_PATH"
        $SUDO_CMD cp -p "$BACKUP_PATH" "$EXEC_PATH"
        # 恢复成功后删除备份文件，重置回初始未安装状态
        $SUDO_CMD rm -f "$BACKUP_PATH"
    else
        echo -e "\033[33m$MSG_WARN_NO_BACKUP\033[0m"
    fi

    # 删除旧版本可能残留在 MacOS 的，以及现在在 Frameworks 的 dylib
    $SUDO_CMD rm -f "$APP_PATH/Contents/MacOS/$DYLIB_NAME"
    $SUDO_CMD rm -f "$DYLIB_DEST"

    # 清理拼音字典
    $SUDO_CMD rm -rf "$RESOURCES_DIR/$DICT_NAME"

    # 删除 Plist 中的旧注入配置（兼容老版本卸载逻辑）和全屏配置
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

if [ -f "$APP_PATH/Contents/MacOS/$DYLIB_NAME" ]; then
    # 清理旧版脚本遗留注入配置
    $SUDO_CMD rm -f "$APP_PATH/Contents/MacOS/$DYLIB_NAME"
    $SUDO_CMD /usr/libexec/PlistBuddy -c "Delete :LSEnvironment:DYLD_INSERT_LIBRARIES" "$PLIST_PATH" >/dev/null 2>&1
fi

# --- 智能备份策略 ---
if [ -f "$BACKUP_PATH" ]; then
    if [ $IS_INJECTED -ne 1 ]; then
        # 备份存在，但当前主文件没有被注入（可能因为 Steam 游戏更新覆盖了原文件）
        # 这时当前文件是最新的纯净原版，我们需要覆盖旧备份
        $SUDO_CMD rm -f "$BACKUP_PATH"
        $SUDO_CMD cp -p "$EXEC_PATH" "$BACKUP_PATH"
    fi
else
    if [ $IS_INJECTED -eq 1 ]; then
        # 致命错误校验：没有备份却显示已注入（防止直接备份一个被污染的文件导致死循环）
        echo -e "\033[31m$MSG_ERR_NO_BACKUP_DIRTY\033[0m"
        exit 1
    fi
    # 不存在备份且未被注入，直接备份纯净原文件
    echo "$MSG_BACKUP"
    $SUDO_CMD cp -p "$EXEC_PATH" "$BACKUP_PATH"
fi

# 当主文件未被注入过时才执行注入操作
if [ $IS_INJECTED -ne 1 ]; then
  # 先赋予 insert_dylib 可执行权限
  xattr -cr "$INSERT_TOOL"
  chmod +x "$INSERT_TOOL"
  # 执行 insert_dylib 注入
  echo "$MSG_INJECTING"
  INJECT_OUT=$($SUDO_CMD "$INSERT_TOOL" --inplace --all-yes "$INJECT_DYLIB_PATH" "$EXEC_PATH" 2>&1)

  # 检测提取特征字符串判断是否成功
  if [[ ! "$INJECT_OUT" =~ Added.*to ]]; then
      echo -e "\033[31m$MSG_ERR_INJECT_FAIL\033[0m"
      echo "$INJECT_OUT"
      # 如果注入失败（如：空间不足等异常报错），强行恢复备份文件防止游戏损坏
      $SUDO_CMD rm -f "$EXEC_PATH"
      $SUDO_CMD cp -p "$BACKUP_PATH" "$EXEC_PATH"
      exit 1
  fi
fi

# 复制 dylib 到 Frameworks
echo "$MSG_COPYING"
$SUDO_CMD cp "$DYLIB_SOURCE" "$DYLIB_DEST"

# 复制中文拼音字典逻辑
if [ "$DICT_BEHAVIOR" != "skip" ]; then
    if [ -e "$DICT_SOURCE" ]; then
        DO_COPY_DICT=0
        if [ "$DICT_BEHAVIOR" == "ask" ]; then
            echo -e "\033[36m$MSG_ASK_DICT\033[0m\c"
            read -r DICT_CHOICE
            if [[ $(echo "$DICT_CHOICE" | tr '[:upper:]' '[:lower:]') == "y" ]]; then
                DO_COPY_DICT=1
            fi
        else
            DO_COPY_DICT=1 # 中文环境 auto 自动复制
        fi

        if [ "$DO_COPY_DICT" -eq 1 ]; then
            echo "$MSG_COPY_DICT"
            $SUDO_CMD cp -R "$DICT_SOURCE" "$RESOURCES_DIR/"
        fi
    else
        # 找不到字典文件时予以提示但继续安装
        echo -e "\033[33m$MSG_WARN_NO_DICT\033[0m"
    fi
fi

if ! grep -qF "LSUIPresentationMode" "$PLIST_PATH"; then
  # 询问是否修复全屏遮挡
  echo -e "\033[36m$MSG_ASK_FS\033[0m\c"
  read -r FS_CHOICE

  # 处理全屏遮挡逻辑 (LSUIPresentationMode=4)
  if [[ $(echo "$FS_CHOICE" | tr '[:upper:]' '[:lower:]') == "y" ]]; then
      echo "$MSG_PLIST"
      $SUDO_CMD /usr/libexec/PlistBuddy -c "Delete :LSUIPresentationMode" "$PLIST_PATH" >/dev/null 2>&1
      $SUDO_CMD /usr/libexec/PlistBuddy -c "Add :LSUIPresentationMode integer 4" "$PLIST_PATH"
  fi
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