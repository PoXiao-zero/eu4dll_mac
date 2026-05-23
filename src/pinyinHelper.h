#include <cpp-pinyin/Pinyin.h>
#include <cpp-pinyin/G2pglobal.h>
#include <memory>

class PinyinHelper {
public:
    static PinyinHelper &getInstance() {
        static PinyinHelper instance;
        return instance;
    }

    PinyinHelper(const PinyinHelper &) = delete;

    PinyinHelper &operator=(const PinyinHelper &) = delete;

    std::shared_ptr<std::vector<std::string>> getPinyin(const std::string &text);

private:
    PinyinHelper();

    ~PinyinHelper() = default;

    std::vector<std::string> getPinyinAndInitials(const std::string &str);

    std::unordered_map<std::string, std::shared_ptr<std::vector<std::string>>> cache_;
    std::unique_ptr<Pinyin::Pinyin> g2p_man;
};