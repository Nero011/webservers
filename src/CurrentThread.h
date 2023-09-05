#pragma once

#include <syscall.h>
#include <unistd.h>

namespace CurrentThread {
extern __thread int t_cachedTid;

void cacheTid();

inline int tid() {
    // __builtin_expect告诉编译器，括号内的值更可能为后者，可以对分支进行编译优化
    if (__builtin_expect(t_cachedTid == 0, 0)) {
        cacheTid();
    }
    return t_cachedTid;
}

}  // namespace CurrentThread