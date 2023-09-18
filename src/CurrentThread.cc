#include "CurrentThread.h"

namespace CurrentThread {
__thread int t_cachedTid = 0;  // 用于保存tid

// 通过系统调用获取tid
void cacheTid() {
    if (t_cachedTid == 0) {
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
}
}  // namespace CurrentThread