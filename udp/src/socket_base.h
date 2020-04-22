//
//  socket_base.h
//  SocketServer
//
//  Created by lemon on 2020/3/24.
//  Copyright © 2020 lemon. All rights reserved.
//

#ifndef __SOCKET_BASE_H__
#define __SOCKET_BASE_H__

#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <string>
#include <string.h>
#include "socket_headers.h"

/**
 需要定义几种消息头，现在暂未定义，但是直接使用了
 */
namespace jx
{

const int maxBufSize = 1024;

// const unsigned char header_hello = 0x00; // client向其他client发送hello包
// const unsigned char header_msg = 0x01;   // msg 包
// const unsigned char header_con = 0x03;   // service向client发送联系人包

inline void Log(const std::string &msg)
{
    std::cout << msg << std::endl;
}

// inline void Log(const std::string &msg, bool whenSomethingHappend)
// {
//     if (whenSomethingHappend) {
//         Log(msg);
//     }
// }

inline void TryStrErr()
{
    if (errno)
    {
        std::cerr << "strerror is: " << strerror(errno) << std::endl;
    }
}

inline void StrErrAndExit(const std::string &msg)
{
    std::cerr << msg << std::endl;
    TryStrErr();
    exit(1);
}

} // namespace jx

struct NetHost
{
    // ipv4 only, x.x.x.x 一共七个字符
    struct in_addr ip;
    int port;

public:
    NetHost();
    NetHost(const sockaddr_in &addr);
    inline struct sockaddr_in as_sockaddr_in()
    {
        struct sockaddr_in addr;
        addr.sin_addr.s_addr = this->ip.s_addr;
        addr.sin_port = htons(this->port);
        addr.sin_family = AF_INET;
        return addr;
    }
    // 如果要能被set存储必须实现`<`操作符的重载
    inline bool operator<(const struct NetHost &host) const
    {
        return this->port < host.port;
    }

    inline bool operator==(const struct NetHost &host) const
    {
        return (this->port == host.port && this->ip.s_addr == host.ip.s_addr);
    }

    inline const std::string debugDesc() const
    {
        return std::string(inet_ntoa(this->ip)) + ":" + std::to_string(this->port);
    }
};

#endif /* __SOCKET_BASE_H__ */
