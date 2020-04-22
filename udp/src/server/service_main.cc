#include "udp_service.h"

int main(int argc, char const *argv[])
{
    ContactUDPService service;
    service.start(6669);
    return 0;
}

