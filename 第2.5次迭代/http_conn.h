/*线程池模版参数类，封装对逻辑任务的处理*/
#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include<unistd.h>//提供对 POSIX 操作系统 API 的访问功能:read\write\pipe;高级IO函数
#include<signal.h>//C标准函数库中的信号处理部分， 定义了程序执行时如何处理不同的信号。信号用作进程间通信， 报告异常行为（如除零）、用户的一些按键组合
#include<sys/types.h>//是Unix/Linux系统的基本系统数据类型的头文件，含有size_t，time_t，pid_t等类型
#include<sys/epoll.h>//epoll相关函数
#include<fcntl.h>//fcntl函数，file control
#include<sys/socket.h>//socket
#include<netinet/in.h>//socketaddr_in 结构体;htons系统调用
#include<arpa/inet.h>//inet_pton
#include <assert.h>//assert终止
#include <sys/stat.h>//stat/fstat/l_stat,用来获取文件属性
#include <string.h>
#include <pthread.h>//Linux系统下的多线程遵循POSIX线程接口，称为pthread。编写Linux下的多线程程序，需要使用头文件pthread.h，连接时需要使用库libpthread.a
#include <stdio.h>
#include <stdlib.h>//即standard library标准库头文件
#include <sys/mman.h>// mmap/munmap函数
#include <stdarg.h>//主要目的为让函数能够接收可变参数。C++的cstdarg头文件中也提供这样的功能
#include <errno.h>
#include <sys/wait.h>//是C语言C标准函式库里的标头档，定义了通过错误码来回报错误资讯的宏
#include <sys/uio.h>
#include <map>
#include "locker.h"

class http_conn
{
public:
    //文件名最大长度
    static const int FILENAME_LEN = 200;
    //读缓冲区大小
    static const int READ_BUFFER_SIZE = 2048;
    //写缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024;

    /*http请求方法，目前实现get，其它以后再说*/
    enum METHOD { GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH };
    /*解析客户请求时，主状态机所处状态*/
    enum CHECK_STATE {
		CHECK_STATE_REQUESTLINE = 0, 
		CHECK_STATE_HEADER, 
		CHECK_STATE_CONTENT 
	};
    /*服务器处理http请求的可能结果集*/
    enum HTTP_CODE {
		NO_REQUEST, 
		GET_REQUEST, 
		BAD_REQUEST, 
		NO_RESOURCE, 
		FORBIDDEN_REQUEST, 
		FILE_REQUEST, 
		INTERNAL_ERROR, 
		CLOSED_CONNECTION 
	};
	/*行的读取状态*/
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };

public:
    http_conn(){}
    ~http_conn(){}

public:
	//初始化新接受的连接
    void init( int sockfd, const sockaddr_in& addr );
	//关闭连接
    void close_conn( bool real_close = true );
	//处理用户请求
    void process();
	//非阻塞读
    bool read();
	//非阻塞写
    bool write();

private:
	//初始化连接
    void init();
	//解析请求
    HTTP_CODE process_read();
	//填充http应答
    bool process_write( HTTP_CODE ret );

	/*以下函数被process_read调用以分析http应答*/
    HTTP_CODE parse_request_line( char* text );
    HTTP_CODE parse_headers( char* text );
    HTTP_CODE parse_content( char* text );
    HTTP_CODE do_request();
    char* get_line() { return m_read_buf + m_start_line; }
    LINE_STATUS parse_line();

	/*以下函数被process_write调用以填充http应答*/
    void unmap();
    bool add_response( const char* format, ... );
    bool add_content( const char* content );
    bool add_status_line( int status, const char* title );
    bool add_headers( int content_length );
    bool add_content_length( int content_length );
    bool add_linger();
    bool add_blank_line();

public:
	//所有socket事件均注册到同一个epoll时间表，故设置epoll文件描述符为静态
    static int m_epollfd;
	//用户数量
    static int m_user_count;

private:
	//该http连接的socket
    int m_sockfd;
	//对方socket地址
    sockaddr_in m_address;

	//读缓冲区
    char m_read_buf[ READ_BUFFER_SIZE ];
	//标识读缓冲中已读入的客户数据的最后一个字节的下一个位置
    int m_read_idx;
	//当前正在分析的字符在读缓冲区的位置
    int m_checked_idx;
	//当前正在解析的行的起始位置
    int m_start_line;
	//写缓冲区
    char m_write_buf[ WRITE_BUFFER_SIZE ];
	//写缓冲区中待发的字节数
    int m_write_idx;

	//主状态机当前所处状态
    CHECK_STATE m_check_state;
	//记录请求方法
    METHOD m_method;

	//客户请求的目标文件的完整路径：doc_root + m_url(doc_root为网站根目录)
    char m_real_file[ FILENAME_LEN ];
	//客户请求的目标文件的文件名
    char* m_url;
	//http版本号，目前仅支持http/1.1
    char* m_version;
	//主机名
    char* m_host;
	//http请求消息体长度
    int m_content_length;
	//记录http请求是否要求保持连接
    bool m_linger;

	//客户请求的目标文件被mmap到内存中的起始位置
    char* m_file_address;
	//目标文件的状态。用以判断文件是否存在、是否为目录、是否可读并获取文件大小等信息
    struct stat m_file_stat;
	/*
	*  本程序采用writev来执行写操作，
	*  http应答头部与文档内容常常在不同内存（通过read函数和mmap函数），
	*  采用writev可同时读出
	*/
	//iovec结构体数组，该结构体描述一块内存区
    struct iovec m_iv[2];
	//被写内存块的数量
    int m_iv_count;
};

#endif
