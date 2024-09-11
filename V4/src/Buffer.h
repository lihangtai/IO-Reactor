/*

IO多路复用中一个线程要管理处理多个fd （不可能线程和fd为1：1关系，）
阻塞IO会阻塞整个线程，所以一般设置非阻塞IO，阻塞在epoll_wait上

IO设备 《-》 内核缓冲区 《=》 用户态进程空间     （cpu与IO速度的不匹配）

数据复制过程本身是同步的。无论是阻塞还是非阻塞 I/O，一旦确定数据可用，read 操作会从内核缓冲区将数据复制到用户缓冲区，write 操作会将数据从用户缓冲区复制到内核缓冲区

该库实现了用户态的读写缓冲buffer，这样可以合并IO系统调用

buffer数据结构的设计（支持读写功能）：

读写共用同一块vector空间，当写空间不足的时候，可以更根据长度进行扩容
但当读空间小于所需长度的时候，程序会自动中止
包含一个8字节的包头 （可能留作协议头使用）


*/

#pragma once

#include <vector>
#include <string>
#include <assert.h>

class Buffer{

    public:
        static const size_t KCheapPprepend = 8;
        static const size_t KInitailSize = 1024;  //缓冲区初始大小
        explicit Buffer(size_t initialSize = KInitailSize)
            :buffer_(KCheapPprepend + initialSize),
            readerIndex_(KCheapPprepend),
            writerIndex_(KCheapPprepend)
        {};

        const size_t readableBytes()const{
            return writerIndex_ - readerIndex_;
        }

        const size_t writeableBytes()const{
            return buffer_.size() - writerIndex_;
        
        }

        const size_t prependableBytes()const{
            return readerIndex_;
        }

        const char* peek()const{
            return begin() + readerIndex_;   //返回可读取的位置
        }

        void retrieve(size_t len){
            if(len< readableBytes()){
                readerIndex_ += len;
            }
            else{
                readerIndex_ = KCheapPprepend;       //缓冲区所有数据全部读出，所以重置
                writerIndex_ = KCheapPprepend;      //此处函数与retrieveAsString函数的结合使用，完成可读位置的1字符串读取
            }
        }

        std::string retrieveAsString(size_t len){
            assert(len <= readableBytes());      //len不能大于可读字节数，但直接assert中止程序运行，鲁棒性不够
            std::string result(peek(),len);
            retrieve(len);
            return result;
        }

	    std::string retrieveAllAsString() {
		return retrieveAsString(readableBytes());
	    }

        void append(const char* data, size_t len){
            if(writeableBytes() < len){
                makeSpace(len);
            }
            std::copy(data, data+len, beginWrite() );
            writerIndex_+=len;
        }

        char* beginWrite(){
            return &*buffer_.begin() + writerIndex_;
        }

        const char* beginWrite()const{
            return &*buffer_.begin() + writerIndex_;
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

            if(writeableBytes() + prependableBytes() < len + KCheapPprepend){
                buffer_.resize(writerIndex_ + len);
            }
            else{
                auto readable = readableBytes();
                std::copy(begin()+readerIndex_, begin()+writerIndex_, begin()+KCheapPprepend);
                readerIndex_ = KCheapPprepend;
                writerIndex_ = readerIndex_ + readable;
            }
        }

    private:
        std::vector<char> buffer_;
        size_t readerIndex_;
        size_t writerIndex_;
};

