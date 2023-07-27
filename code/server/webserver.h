/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnRAII.h"
#include "../http/httpconn.h"

class WebServer
{
public:
    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger, // 端口，触发模式，延时时间，优雅的关闭
        int sqlPort, const char *sqlUser, const char *sqlPwd,  // 数据库端口，用户，密码
        const char *dbName, int connPoolNum, int threadNum,    // 数据库名字，连接池数量，线程数量
        bool openLog, int logLevel, int logQueSize);           // 开启日志，日志等级，日志队列大小

    ~WebServer(); // 析构函数
    void Start(); // 开始

private:
    bool InitSocket_();                        // 初始化Socket，用于TCP
    void InitEventMode_(int trigMode);         // 初始化事件模式，边沿触发，ET
    void AddClient_(int fd, sockaddr_in addr); // 添加客户端

    void DealListen_();                // 处理监听
    void DealWrite_(HttpConn *client); // 处理写
    void DealRead_(HttpConn *client);  // 处理读

    void SendError_(int fd, const char *info); // 发送错误
    void ExtentTime_(HttpConn *client);        // 拓展时间
    void CloseConn_(HttpConn *client);         // 关闭连接

    void OnRead_(HttpConn *client);   // 读
    void OnWrite_(HttpConn *client);  // 写
    void OnProcess(HttpConn *client); // 正在处理

    static const int MAX_FD = 65536; // 最大文件描述符，类成员所有对象共享

    static int SetFdNonblock(int fd); // 设置文件描述符非阻塞，类成员所有对象共享

    int port_;
    bool openLinger_;
    int timeoutMS_; /* 毫秒MS */
    bool isClose_;
    int listenFd_;
    char *srcDir_;

    uint32_t listenEvent_;
    uint32_t connEvent_;

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};

#endif // WEBSERVER_H