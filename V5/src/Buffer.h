#pragma once 

#include <vector>
#include <string>
#include <assert.h>


/*
 对缓冲区进行读写之后，还需要针对具体的情况调用函数设置指针的位置
*/
class Buffer{

public:
    static const size_t KCheapPrepend = 8;
    static const size_t KInitailSize = 1024;
    explicit Buffer(size_t initailizeSize =  KInitailSize)
        : buffer_(KCheapPrepend + initailizeSize )
        , readerIndex_(KCheapPrepend)
        , writerIndex_(KCheapPrepend){}

    const size_t readableBytes() const{
        return writerIndex_ - readerIndex_;

    }

    const size_t writeableBytes() const{
        return buffer_.size() - writerIndex_;
    }

    const size_t prependableBytes()const{
        return readerIndex_;
    }

    const char* peek()const{
        return begin() + readerIndex_;
    }

    void retrieve(size_t len){
        if (len < readableBytes()){
            readerIndex_ += len;
        }
        else{
            
            retrieveAll();
        }
    }

    void retrieveAll(){

        readerIndex_ = KCheapPrepend;
        writerIndex_ = KCheapPrepend;
    }

    std::string retrieveAsString(size_t len){
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    std::string retrieveAllAsString(){
        return retrieveAsString(readableBytes());
    }

    void append(const char* data, size_t len){
        if(writeableBytes() < len){
            makeSpace(len);
        }
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    const char* beginWrite()const{
        return begin() + writerIndex_;
    }

    ssize_t readFd(int fd, int* saveErrno);
    ssize_t writeFd(int fd, int* saveErrno);

private:
    
    char* begin(){
        return &*buffer_.begin();
    }

    const char* begin()const{
        return &*buffer_.begin();
    }

    void makeSpace(size_t len){
        if(writeableBytes() + prependableBytes() < len + KCheapPrepend){
            buffer_.resize(writerIndex_ + len);
        }
        else{
            auto readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + KCheapPrepend);
            readerIndex_ = KCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }
private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};
