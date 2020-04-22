//
//  udp_service.c
//  SocketServer
//
//  Created by lemon on 2020/3/22.
//  Copyright © 2020 lemon. All rights reserved.
//

#include "udp_service.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <pthread.h>
#include "../models/base_datagram.pb.h"

bool ContactUDPService::validatePort(int port)
{
    return true;
}

void ContactUDPService::startService(int port)
{
    this->socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (-1 == this->socketFd)
    {
        std::cerr << "create socket failed: " << strerror(errno) << std::endl;
        exit(1);
    }
    // config sockaddr_in
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 这个的意思是本地ip
    if (this->validatePort(port))
    {
        this->currentPort = port;
    }
    else
    {
        // TODO: 当提供的 port 不合法时
    }
    serverAddr.sin_port = htons(this->currentPort);
    int res = bind(this->socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (0 != res)
    { // case 绑定sock失败
        std::cerr << "bind sockaddr error: " << strerror(errno) << std::endl;
        exit(1);
    }
    std::cout << "start udp socket finished." << std::endl;
    std::cout << inet_ntoa(serverAddr.sin_addr) << std::endl;
}

void ContactUDPService::start(int port)
{
    this->startService(port);

    while (true)
    {
        char buffer[1024] = {0};

        struct sockaddr_in clientAddr = {0};
        socklen_t userAddrLen = sizeof(clientAddr);

        std::cout << "waiting for message..." << std::endl;
        auto recvRes = recvfrom(this->socketFd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &userAddrLen);
        if (-1 == recvRes)
        {
            std::cerr << "receive user message error" << std::endl;
            if (errno)
            {
                std::cerr << "strerr is: " << strerror(errno) << std::endl;
            }
            // 继续接收
            continue;
        }
        // 获取client信息
        NetHost comingHost(clientAddr);

        this->clientHosts.insert(comingHost);

        if (this->clientHosts.empty()) {
            std::cout << "clients is empty now." << std::endl;
        } else {
            std::cout << "clients now: " << std::endl;
        }
        for (auto host: this->clientHosts) {
            jx::Log(host.debugDesc());
        }

        this->sendContactToAll();
    }
}
			
void ContactUDPService::sendContactToAll()
{
    for (auto host : this->clientHosts)
    {
        for (auto contact : this->clientHosts)
        {
            if (contact == host)
            {
                continue;
            }
			
			auto contacts = new jx::ContactsContent;
			auto contactItem = contacts->add_contact();
			contactItem->set_ip(contact.ip.s_addr);
			contactItem->set_port(contact.port);
			
			jx::BaseDatagram datagram;
			datagram.set_header(jx::ContactHeader);
			datagram.set_allocated_contacts(contacts);

			auto datagram_len = datagram.ByteSizeLong();
            char *buf = new char[datagram_len];
			datagram.SerializeToArray(buf, datagram_len);
            auto addr = host.as_sockaddr_in();
            auto res = sendto(this->socketFd, buf, datagram_len, 0, (struct sockaddr *)&addr, (socklen_t)sizeof(addr));
			delete [] buf;
            if (-1 == res)
            {
                jx::Log("send contact[" + contact.debugDesc() + "] to host" + host.debugDesc() + " error.");
            }
            jx::TryStrErr();
            jx::Log("send " + contact.debugDesc() + " to " + host.debugDesc());
        }
    }
}
