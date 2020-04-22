//
//  socket_base.cc
//  SocketServer
//
//  Created by lemon on 2020/3/24.
//  Copyright © 2020 lemon. All rights reserved.
//

#include "socket_base.h"

namespace jx {

}


NetHost::NetHost() 
{
    this->ip.s_addr = 0;
    this->port = 0;
}

NetHost::NetHost(const sockaddr_in &addr) 
{
    this->ip.s_addr = addr.sin_addr.s_addr;
    this->port = ntohs(addr.sin_port);
}
