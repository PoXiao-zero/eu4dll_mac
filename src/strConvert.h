#include <cstdint>
#include <string>

namespace strConvert {
    void escapedStrToUtf8Replace(std::string *str);

    void utf8ToEscapedStrReplace(std::string *str);

    std::string* utf8ToEscapedStr(std::string *str);

    std::string escapedStrToUtf8(std::string *str);
}

extern "C" {
[[gnu::used]] inline void *escapedStrToUtf8ReplaceAddress = (void *) strConvert::escapedStrToUtf8Replace;
[[gnu::used]] inline void *utf8ToEscapedStrReplaceAddress = (void *) strConvert::utf8ToEscapedStrReplace;
[[gnu::used]] inline void *utf8ToEscapedStrAddress = (void *) strConvert::utf8ToEscapedStr;
[[gnu::used]] inline void *escapedStrToUtf8Address = (void *) strConvert::escapedStrToUtf8;
}