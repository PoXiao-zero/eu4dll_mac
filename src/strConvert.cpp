#include "strConvert.h"
#include "global.h"
#include <codecvt>
#include <iostream>

namespace strConvert {

    std::string tempText;

    std::string convert_utf16le_to_utf8(const std::string &raw_data) {
        if (raw_data.empty()) return "";
        const auto *p_utf16 = reinterpret_cast<const char16_t *>(raw_data.data());
        size_t len = raw_data.size() / 2;
        std::u16string u16str(p_utf16, len);
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        return convert.to_bytes(u16str);
    }

    std::string escapedStrToUtf8(std::string *str) {
        if (str->empty()) return *str;
        uint64_t len = (*str).length();
        std::string utf16_bytes;
        for (int i = 0; i < len;) {
            unsigned char c = (*str)[i];
            if (c == ESCAPE_SEQ_1 || c == ESCAPE_SEQ_2 || c == ESCAPE_SEQ_3 || c == ESCAPE_SEQ_4) {
                if (i + 2 >= len) break;

                unsigned char b1 = (*str)[i + 1];
                unsigned char b2 = (*str)[i + 2];

                uint16_t eax = (b2 << 8) | b1;

                if (c == ESCAPE_SEQ_2) {
                    eax -= SHIFT_2;
                } else if (c == ESCAPE_SEQ_3) {
                    eax += SHIFT_3;
                } else if (c == ESCAPE_SEQ_4) {
                    eax += SHIFT_4;
                }

                // 存入 UTF-16LE 字节（先低位，再高位）
                utf16_bytes.push_back(eax & 0xFF);
                utf16_bytes.push_back((eax >> 8) & 0xFF);

                i += 3;
            } else {
                // 普通字符分支，低字节是自身，高字节补 0x00
                utf16_bytes.push_back(c);
                utf16_bytes.push_back(0x00);
                i += 1;
            }
        }

        return convert_utf16le_to_utf8(utf16_bytes);
    }

    std::string convert_utf8_to_utf16le(const std::string &utf8_data) {
        if (utf8_data.empty()) return "";
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        std::u16string u16str = convert.from_bytes(utf8_data);

        const auto *p_bytes = reinterpret_cast<const char *>(u16str.data());
        return {p_bytes, u16str.size() * sizeof(char16_t)};
    }

    bool is_escape_byte(uint8_t b) {
        switch (b) {
            case 0xA4:
            case 0xA3:
            case 0xA7:
            case 0x24:
            case 0x5B:
            case 0x00:
            case 0x5C:
            case 0x20:
            case 0x0D:
            case 0x0A:
            case 0x22:
            case 0x7B:
            case 0x7D:
            case 0x40:
            case 0x80:
            case 0x7E:
            case 0x2F:
            case 0x5F:
            case 0xBD:
            case 0x3B:
            case 0x5D:
            case 0x3D:
            case 0x23:
            case 0x3F:
            case 0x3A:
            case 0x3C:
            case 0x3E:
            case 0x2A:
            case 0x7C:
                return true;
            default:
                return false;
        }
    }

    std::string *utf8ToEscapedStr(std::string *str) {
        if (str->empty()) return str;
        // 先将输入的 UTF-8 字符串转换为 UTF-16LE 的字节流
        std::string utf16_bytes = convert_utf8_to_utf16le(*str);
        size_t len = utf16_bytes.size() / 2;
        const auto *p_utf16 = reinterpret_cast<const char16_t *>(utf16_bytes.data());
        tempText.clear();

        for (size_t i = 0; i < len; i++) {
            uint16_t cp = p_utf16[i];
            if (cp > 0x100 && cp < 0xA00) {
                cp = cp + 0xE000;
            }

            uint8_t high = (cp >> 8) & 0xFF;
            uint8_t low = cp & 0xFF;

            if (high == 0) {
                tempText.push_back(static_cast<char>(low));
                continue;
            }

            uint8_t escapeChr = ESCAPE_SEQ_1; // 默认 0x10

            // 根据高位决定是否改变转义标记 (+2)
            if (is_escape_byte(high)) {
                escapeChr += 2;
            }

            // 根据低位决定是否改变转义标记 (+1)
            if (is_escape_byte(low)) {
                escapeChr += 1;
            }

            // 根据转义标记修改高低字节，避开原有的坏字符
            switch (escapeChr) {
                case ESCAPE_SEQ_2: // 0x11 (仅低字节命中)
                    low += 14;     // 解码器中对应的逆运算是: eax -= 0x0E (即14)
                    break;
                case ESCAPE_SEQ_3: // 0x12 (仅高字节命中)
                    high -= 9;     // 解码器中对应的逆运算是: eax += 0x900 (9左移8位)
                    break;
                case ESCAPE_SEQ_4: // 0x13 (高低字节同时命中)
                    low += 14;
                    high -= 9;     // 解码器中对应的逆运算是: eax += 0x8F2 (即 0x900 - 0x0E)
                    break;
                case ESCAPE_SEQ_1: // 0x10
                default:
                    break;
            }

            // 写入结果串: [转义标记] [低字节] [高字节]
            tempText.push_back(static_cast<char>(escapeChr));
            tempText.push_back(static_cast<char>(low));
            tempText.push_back(static_cast<char>(high));
        }
        return &tempText;
    }

    void escapedStrToUtf8Replace(std::string *str) {
        if (str->empty()) return;
        *str = escapedStrToUtf8(str);
    }

    void utf8ToEscapedStrReplace(std::string *str) {
        if (str->empty()) return;
        *str = *utf8ToEscapedStr(str);
    }
}