#include "strConvert.h"
#include "global.h"

namespace strConvert {

    const size_t MAX_OUT = 32768;
    std::string tempText;

    // @formatter:off
    bool is_escape_byte(uint8_t b) {
        switch (b) {
            case 0xA4: case 0xA3: case 0xA7: case 0x24: case 0x5B:
            case 0x00: case 0x5C: case 0x20: case 0x0D: case 0x0A:
            case 0x22: case 0x7B: case 0x7D: case 0x40: case 0x80:
            case 0x7E: case 0x2F: case 0x5F: case 0xBD: case 0x3B:
            case 0x5D: case 0x3D: case 0x23: case 0x3F: case 0x3A:
            case 0x3C: case 0x3E: case 0x2A: case 0x7C:
                return true;
            default:
                return false;
        }
    }

    char mapSpecialUnicodeToWin1252(uint32_t cp) {
        switch (cp) {
            case 0x20AC: return (char) 0x80;
            case 0x201A: return (char) 0x82;
            case 0x0192: return (char) 0x83;
            case 0x201E: return (char) 0x84;
            case 0x2026: return (char) 0x85;
            case 0x2020: return (char) 0x86;
            case 0x2021: return (char) 0x87;
            case 0x02C6: return (char) 0x88;
            case 0x2030: return (char) 0x89;
            case 0x0160: return (char) 0x8A;
            case 0x2039: return (char) 0x8B;
            case 0x0152: return (char) 0x8C;
            case 0x017D: return (char) 0x8E;
            case 0x2018: return (char) 0x91;
            case 0x2019: return (char) 0x92;
            case 0x201C: return (char) 0x93;
            case 0x201D: return (char) 0x94;
            case 0x2022: return (char) 0x95;
            case 0x2013: return (char) 0x96;
            case 0x2014: return (char) 0x97;
            case 0x02DC: return (char) 0x98;
            case 0x2122: return (char) 0x99;
            case 0x0161: return (char) 0x9A;
            case 0x203A: return (char) 0x9B;
            case 0x0153: return (char) 0x9C;
            case 0x017E: return (char) 0x9E;
            case 0x0178: return (char) 0x9F;
            default:     return '\0';
        }
    }

    uint32_t mapWin1252ToSpecialUnicode(uint8_t c) {
        switch (c) {
            case 0x80: return 0x20AC; // €
            case 0x82: return 0x201A; // ‚
            case 0x83: return 0x0192; // ƒ
            case 0x84: return 0x201E; // „
            case 0x85: return 0x2026; // …
            case 0x86: return 0x2020; // †
            case 0x87: return 0x2021; // ‡
            case 0x88: return 0x02C6; // ˆ
            case 0x89: return 0x2030; // ‰
            case 0x8A: return 0x0160; // Š
            case 0x8B: return 0x2039; // ‹
            case 0x8C: return 0x0152; // Œ
            case 0x8E: return 0x017D; // Ž
            case 0x91: return 0x2018; // ‘
            case 0x92: return 0x2019; // ’
            case 0x93: return 0x201C; // “
            case 0x94: return 0x201D; // ”
            case 0x95: return 0x2022; // •
            case 0x96: return 0x2013; // –
            case 0x97: return 0x2014; // —
            case 0x98: return 0x02DC; // ˜
            case 0x99: return 0x2122; // ™
            case 0x9A: return 0x0161; // š
            case 0x9B: return 0x203A; // ›
            case 0x9C: return 0x0153; // œ
            case 0x9E: return 0x017E; // ž
            case 0x9F: return 0x0178; // Ÿ
            default:   return 0;
        }
    }
    // @formatter:on

    void UTF8ToWindows1252(const char *utf8_in, char *out_buffer) {
        if (!utf8_in || !out_buffer) return;
        size_t out_pos = 0;
        size_t utf8_len = strlen(utf8_in);

        for (size_t i = 0; i < utf8_len;) {
            auto c = (uint8_t) utf8_in[i];
            uint32_t cp;

            if (c < 0x80) { // 1字节
                cp = c;
                i += 1;
            } else if ((c & 0xE0) == 0xC0 && i + 1 < utf8_len) { // 2字节
                cp = ((c & 0x1F) << 6) | (utf8_in[i + 1] & 0x3F);
                i += 2;
            } else if ((c & 0xF0) == 0xE0 && i + 2 < utf8_len) { // 3字节 (包含大多数中文/日文)
                cp = ((c & 0x0F) << 12) | ((utf8_in[i + 1] & 0x3F) << 6) | (utf8_in[i + 2] & 0x3F);
                i += 3;
            } else if ((c & 0xF0) == 0xF0 && i + 3 < utf8_len) {// 4字节 emoji表情
                out_buffer[out_pos++] = '?';
                out_buffer[out_pos++] = '?';
                out_buffer[out_pos++] = '?';
                out_buffer[out_pos++] = '?';
                i += 4;
                continue;
            } else {
                out_buffer[out_pos++] = '?';
                i += 1;
                continue;
            }

            bool need_escape = false;

            if (cp <= 0xFF) {
                if (cp >= 0x80 && cp <= 0x9F) {
                    need_escape = true;
                } else {
                    if (out_pos + 1 >= MAX_OUT) break;
                    out_buffer[out_pos++] = (char) cp;
                }
            } else {
                char mapped = mapSpecialUnicodeToWin1252(cp);
                if (mapped == '\0') {
                    need_escape = true;
                } else {
                    if (out_pos + 1 >= MAX_OUT) break;
                    out_buffer[out_pos++] = mapped;
                }
            }

            if (need_escape) {
                auto cp16 = (uint16_t) (cp & 0xFFFF);

                if (cp16 > 0x100 && cp16 < 0xA00) {
                    cp16 += 0xE000;
                }

                uint8_t high = (cp16 >> 8) & 0xFF;
                uint8_t low = cp16 & 0xFF;

                if (high == 0) {
                    if (out_pos + 1 >= MAX_OUT) break;
                    out_buffer[out_pos++] = (char) low;
                } else {
                    if (out_pos + 3 >= MAX_OUT) break;

                    uint8_t escapeChr = ESCAPE_SEQ_1;

                    if (is_escape_byte(high)) escapeChr += 2;
                    if (is_escape_byte(low)) escapeChr += 1;

                    switch (escapeChr) {
                        case ESCAPE_SEQ_2:
                            low += 14;
                            break;
                        case ESCAPE_SEQ_3:
                            high -= 9;
                            break;
                        case ESCAPE_SEQ_4:
                            low += 14;
                            high -= 9;
                            break;
                        case ESCAPE_SEQ_1:
                        default:
                            break;
                    }

                    out_buffer[out_pos++] = (char) escapeChr;
                    out_buffer[out_pos++] = (char) low;
                    out_buffer[out_pos++] = (char) high;
                }
            }
        }
        out_buffer[out_pos] = '\0';
    }

    void Windows1252ToUTF8(const char *win1252_in, char *out_buffer) {
        if (!win1252_in || !out_buffer) return;

        size_t out_pos = 0;
        size_t win1252_len = strlen(win1252_in);

        for (size_t i = 0; i < win1252_len;) {
            auto c = (uint8_t) win1252_in[i];

            if (c == ESCAPE_SEQ_1 || c == ESCAPE_SEQ_2 || c == ESCAPE_SEQ_3 || c == ESCAPE_SEQ_4) {
                if (i + 2 > win1252_len) {
                    break;
                }

                uint8_t escapeChr = c;
                auto low = (uint8_t) win1252_in[i + 1];
                auto high = (uint8_t) win1252_in[i + 2];
                i += 3;

                switch (escapeChr) {
                    case ESCAPE_SEQ_2:
                        low -= 14;
                        break;
                    case ESCAPE_SEQ_3:
                        high += 9;
                        break;
                    case ESCAPE_SEQ_4:
                        low -= 14;
                        high += 9;
                        break;
                    case ESCAPE_SEQ_1:
                    default:
                        break;
                }

                uint16_t cp16 = ((uint16_t) high << 8) | low;

                if (cp16 > 0xE100 && cp16 < 0xEA00) {
                    cp16 -= 0xE000;
                }

                if (cp16 <= 0x7F) {
                    if (out_pos + 1 >= MAX_OUT) break;
                    out_buffer[out_pos++] = (char) cp16;
                } else if (cp16 <= 0x7FF) {
                    if (out_pos + 2 >= MAX_OUT) break;
                    out_buffer[out_pos++] = (char) (0xC0 | (cp16 >> 6));
                    out_buffer[out_pos++] = (char) (0x80 | (cp16 & 0x3F));
                } else {
                    if (out_pos + 3 >= MAX_OUT) break;
                    out_buffer[out_pos++] = (char) (0xE0 | (cp16 >> 12));
                    out_buffer[out_pos++] = (char) (0x80 | ((cp16 >> 6) & 0x3F));
                    out_buffer[out_pos++] = (char) (0x80 | (cp16 & 0x3F));
                }

            } else {
                i += 1;
                uint32_t cp = c;

                if (c >= 0x80 && c <= 0x9F) {
                    uint32_t mapped = mapWin1252ToSpecialUnicode(c);
                    if (mapped != 0) {
                        cp = mapped;
                    }
                }

                if (cp <= 0x7F) {
                    if (out_pos + 1 >= MAX_OUT) break;
                    out_buffer[out_pos++] = (char) cp;
                } else if (cp <= 0x7FF) {
                    if (out_pos + 2 >= MAX_OUT) break;
                    out_buffer[out_pos++] = (char) (0xC0 | (cp >> 6));
                    out_buffer[out_pos++] = (char) (0x80 | (cp & 0x3F));
                } else {
                    if (out_pos + 3 >= MAX_OUT) break;
                    out_buffer[out_pos++] = (char) (0xE0 | (cp >> 12));
                    out_buffer[out_pos++] = (char) (0x80 | ((cp >> 6) & 0x3F));
                    out_buffer[out_pos++] = (char) (0x80 | (cp & 0x3F));
                }
            }
        }

        out_buffer[out_pos] = '\0';
    }

    std::string *escapedStrToUtf8(std::string *str) {
        if (str->empty()) return str;
        char *out = new char[str->length() * 2];
        out[0] = '\0';
        Windows1252ToUTF8(str->c_str(), out);
        tempText = out;
        delete[] out;
        return &tempText;
    }

    std::string escapedStrToUtf8Val(std::string str) {
        if (str.empty()) return str;
        char *out = new char[str.length() * 2];
        out[0] = '\0';
        Windows1252ToUTF8(str.c_str(), out);
        std::string result = out;
        delete[] out;
        return result;
    }

    std::string *utf8ToEscapedStr(std::string *str) {
        if (str->empty()) return str;
        char *out = new char[str->length() * 2];
        out[0] = '\0';
        UTF8ToWindows1252(str->c_str(), out);
        tempText = out;
        delete[] out;
        return &tempText;
    }

    void escapedStrToUtf8Replace(std::string *str) {
        if (str->empty()) return;
        *str = *escapedStrToUtf8(str);
    }

    void utf8ToEscapedStrReplace(std::string *str) {
        if (str->empty()) return;
        *str = *utf8ToEscapedStr(str);
    }
}