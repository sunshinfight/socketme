//
//  udp_client_base.h
//  SocketServer
//
//  Created by lemon on 2020/3/24.
//  Copyright © 2020 lemon. All rights reserved.
//

#ifndef __UDP_CLIENT_BASE_H__
#define __UDP_CLIENT_BASE_H__

#include <stdio.h>
#include "../socket_base.h"
#include <pthread.h>
#include <string>
#include <vector>
#include <set>

typedef void (*MsgCallBackType)(void*);

struct ClientContact {
    NetHost host;
    bool isConnected;
};

class UDPClientBase {
    int socketFd;
    std::set<NetHost> contacts;
    pthread_t recvThread;
    MsgCallBackType msgCallBack;
private:
    /// 向通讯录中的成员告知自己
    void declSelf();
    
public:
    /// 连接，serverIp为中间服务器的地址
    void connect(const std::string &serverIp, int serverPort, pthread_t *recvthread);
    /// 向所有的成员发送消息
    void sendMsgToAll(const std::string &msg);
    /// 向某个成员发送消息
    void sendMsg(const std::string &msg, NetHost &host);

    /// 为thread提供函数指针
    void *waitingForMsg(void*argv);

    // 这种读写模型后面考虑换成队列
    void setRecvMsgHandler(MsgCallBackType handler);
};

#endif /* udp_client_base.h */
