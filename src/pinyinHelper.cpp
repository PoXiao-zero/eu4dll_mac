#include <string>
#include <vector>
#include <set>
#include "pinyinHelper.h"
#include "strConvert.h"

namespace {
    std::vector<std::string> splitUTF8(const std::string &str) {
        std::vector<std::string> chars;
        size_t i = 0;
        while (i < str.length()) {
            unsigned char c = str[i];
            size_t len = 1;
            if ((c & 0xE0) == 0xC0) len = 2;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xF8) == 0xF0) len = 4;
            else len = 1;

            chars.push_back(str.substr(i, len));
            i += len;
        }
        return chars;
    }

    void replaceUmlaut(std::string &py) {
        std::string from = "ü";
        std::string to = "u";
        size_t start_pos = 0;
        while ((start_pos = py.find(from, start_pos)) != std::string::npos) {
            py.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    void generateCombinations(const std::vector<std::vector<std::string>> &all_pinyins,// NOLINT(misc-no-recursion)
                              size_t index,
                              const std::string &current_full,
                              const std::string &current_initial,
                              std::set<std::string> &out_full,
                              std::set<std::string> &out_initial) {
        if (index == all_pinyins.size()) {
            if (!current_full.empty()) out_full.insert(current_full);
            if (!current_initial.empty()) out_initial.insert(current_initial);
            return;
        }

        const auto &current_char_pinyins = all_pinyins[index];

        for (const std::string &py: current_char_pinyins) {
            std::string next_full = current_full + py;
            std::string next_initial = current_initial;

            // 提取首字母
            if (!py.empty()) {
                if (py[0] >= 'a' && py[0] <= 'z') {
                    next_initial += py[0];
                } else {
                    next_initial += py;
                }
            }

            generateCombinations(all_pinyins, index + 1, next_full, next_initial, out_full, out_initial);
        }
    }
}

PinyinHelper::PinyinHelper() {
    g2p_man = std::make_unique<Pinyin::Pinyin>();
}

std::vector<std::string> PinyinHelper::getPinyinAndInitials(const std::string &str) {
    std::vector<std::string> chars = splitUTF8(str);

    std::vector<std::vector<std::string>> all_pinyins;
    for (const std::string &ch: chars) {
        std::vector<std::string> py_list = g2p_man->getDefaultPinyin(ch, Pinyin::ManTone::Style::NORMAL, true, false);
        if (py_list.empty()) {
            py_list.push_back(ch);
        }
        all_pinyins.push_back(py_list);
    }

    // 生成所有排列组合
    std::set<std::string> unique_full;
    std::set<std::string> unique_initial;

    generateCombinations(all_pinyins, 0, "", "", unique_full, unique_initial);

    // 将全拼和首字母合并到一个数组中返回
    std::vector<std::string> result;
    result.assign(unique_full.begin(), unique_full.end());
    result.insert(result.end(), unique_initial.begin(), unique_initial.end());

    // 将 ü 替换为 u
    for (std::string &py: result) {
        replaceUmlaut(py);
    }

    return result;
}

std::shared_ptr<std::vector<std::string>> PinyinHelper::getPinyin(const std::string &text) {

    auto it = cache_.find(text);
    if (it != cache_.end()) {
        return it->second;
    }

    auto utf8_str = strConvert::escapedStrToUtf8Val(text);
    std::vector<std::string> pinyin_data = getPinyinAndInitials(utf8_str);

    auto result_ptr = std::make_shared<std::vector<std::string>>(std::move(pinyin_data));

    cache_[text] = result_ptr;

    return result_ptr;
}