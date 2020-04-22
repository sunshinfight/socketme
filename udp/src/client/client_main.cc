#include "udp_client_base.h"
#include <unistd.h>
#include "../models/base_datagram.pb.h"

using namespace std;

void msgCallBack(void*argv)
{
	
	jx::Log("msg call back received message");
    jx::BaseDatagram datagram;
	auto data = static_cast<jx::BaseDatagram*>(argv);
	datagram.CopyFrom(*data);
    if (datagram.has_text()) {
        if (datagram.header() == jx::TextHeader) {
            cout << "received text msg: " << endl;
            cout << datagram.text().textbody() << endl;
        } else {
            cout << "error msg header and content." << endl;
        }
    } else {
        cout << "the other msg type not supported now." << endl;
    }
}


int main(int argc, char const *argv[])
{
    auto ip = argv[1];
    UDPClientBase client;
    
    pthread_t thread;
    client.connect(ip, 6669, &thread);
    client.setRecvMsgHandler(msgCallBack);

    string input;
    while(getline(cin, input)) {
        client.sendMsgToAll(input);
    }
    pthread_exit(NULL);
    return 0;
}
