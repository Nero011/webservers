#include <time.h>

#include <iostream>
#include <string>

class Timestamp {
private:
    int64_t microSecond;  // 长整型保存的时间
public:
    Timestamp();
    explicit Timestamp(int64_t time);
    ~Timestamp();
    static Timestamp now();
    std::string toString() const;
};
