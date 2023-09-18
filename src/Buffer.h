#pragma once

#include <stddef.h>

#include <algorithm>  //泛型算法
#include <string>
#include <vector>

/*
|  kCheapPrepend  |  readableIndex  |  writableIndex  |   end  |


|vector                                                        |
*/

class Buffer {
private:
    // 返回数组的原始头指针
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    void makeSpace(size_t len) {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            buffer_.resize(writerIndex_ + len);
        } else {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize), readerIndex_(kCheapPrepend), writerIndex_(kCheapPrepend) {}
    ~Buffer();

    /// @brief
    /// @return 未读的缓冲区长度
    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    /// @brief
    /// @return 可写的缓冲区长度
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    // 返回缓冲区中可读数据的起始地址
    const char* peek() const { return begin() + readerIndex_; }

    // onMessage string << Buffer
    void retrieve(size_t len) {
        if (len < readableBytes()) {
            readerIndex_ += len;
        } else {
            retrieveAll();
        }
    }

    // 缓冲区复位
    void retrieveAll() { readerIndex_ = writerIndex_ = kCheapPrepend; }

    std::string retrieveAllAsString() { return retrieveAsString(readableBytes()); }

    std::string retrieveAsString(size_t len) {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }

    char* beginWrite() { return begin() + writerIndex_; }
    const char* beginWrite() const { return begin() + writerIndex_; }

    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    // 从fd上读取数据
    ssize_t readFd(int fd, int* savedErrno);
    // 写fd中的数据
    ssize_t writeFd(int fd, int* saveErrno);
};
