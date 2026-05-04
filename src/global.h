#include <cstdint>
#include "memoryHelper.h"
#import <dlfcn.h>
#include <mutex>
#include <unordered_map>
#include <utility>

#define ESCAPE_SEQ_1 0x10
#define ESCAPE_SEQ_2 0x11
#define ESCAPE_SEQ_3 0x12
#define ESCAPE_SEQ_4 0x13
#define SHIFT_2      0x0E
#define SHIFT_3      0x900
#define SHIFT_4      0x8F2
#define NO_FONT      0x98F
#define NOT_DEF      0x2026
#define LOW_SHIFT       0x0E

[[gnu::used]] inline void *g_CString_AppendCharAddress;
[[gnu::used]] inline void *g_CString_AppendCharConstAddress;

// 全局单例执行结果追踪器
class ExecutionTracker {
public:
    static ExecutionTracker &instance() {
        static ExecutionTracker inst;
        return inst;
    }

    void report(const std::string &name, bool success) {
        if (!success) {
            statusMap[name] = success;
        }
    }

    std::unordered_map<std::string, bool> statusMap;
    std::string findFnFailedRecord;
};

// 追踪辅助类
struct FuncGuard {
    std::string name;
    bool success = false;

    explicit FuncGuard(std::string n) : name(std::move(n)) {}

    ~FuncGuard() { ExecutionTracker::instance().report(name, success); }

    void markSuccess() { success = true; }
};

#define TRACK_FUNCTION() FuncGuard _guard(__FUNCTION__)
#define SET_SUCCESS() _guard.markSuccess()

inline void *findFn(const char *fnName) {
    void *result = dlsym(RTLD_DEFAULT, fnName);
    if (result == nullptr) {
        printf("findFn failed: %s \n", fnName);
        ExecutionTracker::instance().findFnFailedRecord += fnName;
        ExecutionTracker::instance().findFnFailedRecord += "\n";
    }
    return result;
}

inline void printStr(const char *str) {
    printf("hex: ");
    size_t len = strlen(str);
    for (int i = 0; i < len; i++) {
        printf("%02X ", (unsigned char) str[i]);
    }
    printf("\n\n");
}