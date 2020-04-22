//
//  udp_service.h
//  SocketServer
//
//  Created by lemon on 2020/3/22.
//  Copyright © 2020 lemon. All rights reserved.
//

#ifndef __UDP_SERVICE_H__
#define __UDP_SERVICE_H__

#include <stdio.h>
#include <vector>
#include <set>
#include "../socket_base.h"

class ContactUDPService {
    int socketFd;
    int currentPort = 6669;
    std::set<NetHost> clientHosts;
    
private:
    void startService(int port);
    
    //  测试 port 是否合法
    bool validatePort(int port);

    // 将服务器持有的通讯录下发给通讯录中的所有成员
    void sendContactToAll();
public:
//    ContactUDPService():
    
    void start(int port);
    
};


#endif /* __UDP_SERVICE_H__ */
