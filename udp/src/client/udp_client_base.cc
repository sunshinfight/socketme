//
//  udp_client_base.cc
//  SocketServer
//
//  Created by lemon on 2020/3/24.
//  Copyright © 2020 lemon. All rights reserved.
//

#include "udp_client_base.h"
#include <iostream>
#include "../models/base_datagram.pb.h"

using namespace std;

// simple wrapped for call back in thread
static void *recvThreadFunc(void *argv) {
    UDPClientBase *clientPtr = static_cast<UDPClientBase*>(argv);
    // detach thread.
    pthread_detach(pthread_self());
    
    clientPtr->waitingForMsg(nullptr);
    return NULL;
}

// TODO: 这个地方应该修改逻辑为，直等到收到了某个联系人的hello包才确认已经打洞成功，所以，应该等一小段时间，如果没有收到，那么再发送hello包，直至收到对方的hello包或是超过尝试次数为止。
void UDPClientBase::declSelf()
{
    if (this->contacts.empty())
    {
        jx::Log("contacts is empty.");
        return;
    }

    for (auto host : this->contacts)
    {
   		// hello 包
		jx::BaseDatagram datagram;
		datagram.set_header(jx::HelloHeader);
		auto contacts = new jx::ContactsContent;
		auto contact = contacts->add_contact();
		contact->set_ip(host.ip.s_addr);
		contact->set_port(host.port);
		datagram.set_allocated_contacts(contacts);
		
		auto datagram_len = datagram.ByteSizeLong();
		auto buf = new char[datagram_len];
		if(!datagram.SerializeToArray(buf, datagram_len)) {
			jx::Log("hello package serialize to array failed");
		}
		datagram.PrintDebugString();

        auto addr = host.as_sockaddr_in();
        auto sendRes = sendto(this->socketFd, buf, datagram_len, 0, 
                                (struct sockaddr *)&addr, (socklen_t)sizeof(addr));
		delete [] buf;
        if (-1 == sendRes)
        {
            jx::TryStrErr();
            // TODO: 重试
        }
        jx::Log("I have delSelf to host: " + host.debugDesc());
    }
}

void UDPClientBase::connect(const std::string &serverIp, int serverPort, pthread_t *recvthread)
{
    // linux host ip is: 49.233.151.22
    NetHost server;
    server.ip.s_addr = inet_addr(serverIp.c_str());
    server.port = serverPort;
    auto contactServerAddr = server.as_sockaddr_in();

    this->socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (-1 == this->socketFd)
    {
        jx::StrErrAndExit("create socket error");
        exit(1);
    }

    // 1.自己发送报文
    auto sendRes = sendto(this->socketFd, nullptr, 0, 0, 
                            (struct sockaddr *)&contactServerAddr, (socklen_t)sizeof(contactServerAddr));
    if (-1 == sendRes)
    {
        std::cerr << "send error." << std::endl;
        exit(1);
    }


    jx::Log("start create thread for waiting msg.");
    // 2.等待接收服务器的消息，来确定联系人
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);

    if(pthread_create(recvthread, &threadAttr, recvThreadFunc, this)) {
        jx::StrErrAndExit("create recv thread error.");
    }

    this->recvThread = *recvthread;
}

// header type: msg
void UDPClientBase::sendMsg(const std::string &msg, NetHost &host) 
{
    auto addr = host.as_sockaddr_in();
    socklen_t addrlen = sizeof(addr);

    jx::BaseDatagram datagram;
	datagram.set_header(jx::TextHeader);
    auto textMsg = new jx::TextContent;
    textMsg->set_allocated_textbody(new string(msg));
    datagram.set_allocated_text(textMsg);
	auto datagram_len = datagram.ByteSizeLong();
	auto buf = new char[datagram_len];
	if(!datagram.SerializeToArray(buf, datagram_len)) {
		jx::Log("Serialize to array failed.");
	}
	jx::Log("prepare to send msg: " + datagram.DebugString() + " to " + host.debugDesc());
    
    auto res = sendto(this->socketFd, buf, datagram_len, 0, (sockaddr*)&addr, addrlen);
	delete [] buf;
    if (-1 == res) {
        jx::TryStrErr();
    }
   	jx::Log("send to " + host.debugDesc() + " suceess"); 
}

void UDPClientBase::sendMsgToAll(const std::string &msg)
{
    for (auto contact: this->contacts) {
        this->sendMsg(msg, contact);
    }
}

void *UDPClientBase::waitingForMsg(void*argv)
{
    jx::Log("entry function waiting for msg.");

    while (true)
    {
        char buf[1024] = {0};
        sockaddr_in addr = {0};
        socklen_t addrLen = sizeof(addr);
        auto recvRes = recvfrom(this->socketFd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addrLen);
        if (-1 == recvRes)
        {
            std::cerr << "recv error." << std::endl;
        }
        jx::Log("received msg from: " + NetHost(addr).debugDesc());
		
		auto recv_datagram = new jx::BaseDatagram;
		if(!recv_datagram->ParseFromArray(buf, recvRes)) {
			jx::Log("recv datagram error");
		}
		recv_datagram->PrintDebugString();
        if (recv_datagram->header() == jx::ContactHeader) { // 这是服务器发送来的联系人
            NetHost host;
			for (int i = 0; i < recv_datagram->contacts().contact_size(); i++) {
				auto contacts = recv_datagram->contacts();
				auto contact = contacts.contact(i);
				host.ip.s_addr = contact.ip();
				host.port = contact.port();
				this->contacts.insert(host);
            	jx::Log("this is con, I have received message: " + host.debugDesc());
            	this->declSelf();
			}
        } else if (recv_datagram->header() == jx::TextHeader) {  // 这是消息
            auto recvHost = NetHost(addr);
            jx::Log("this is msg, I have received msg: " + 
                    std::string(&buf[1]) + " from " + recvHost.debugDesc());
            
            jx::Log("received size is: " + std::to_string(recvRes));

            // 跨线程传递参数需要使用堆
            this->msgCallBack(recv_datagram);
        } else if (jx::HelloHeader == recv_datagram->header()) {
            // hello 包
            auto recvHost = NetHost(addr);
            jx::Log("I have recv hello package: " + 
                   recv_datagram->DebugString() + " from: " + recvHost.debugDesc());
        } else {
			jx::Log("other msg type, like msg type.");
		}
		delete recv_datagram;
    }
}

void UDPClientBase::setRecvMsgHandler(MsgCallBackType handler)
{
    this->msgCallBack = handler;
}
