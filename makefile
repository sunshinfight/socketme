srcpath = ./udp:./udp/src:./udp/src/client:./udp/src/server:./udp/src/models/
vpath %.h 		$(srcpath)
vpath %.hpp 	$(srcpath)
vpath %.c 		$(srcpath)
vpath %.cc		$(srcpath)
vpath %.cpp		$(srcpath)

cxxstd = -std=c++11

protobuflib = $(shell pkg-config --cflags --libs protobuf)

cxxbuild = c++ $(cxxstd)

builddir = ./build/

objdir = $(builddir)objs/

clientobjsdir = $(builddir)objs/client/

serviceobjsdir = $(builddir)objs/service/

commonheaders = socket_base.h socket_headers.h

protofiles = $(%.proto)

all: client service

socket_base.o: socket_base.cc socket_base.h socket_headers.h
	$(cxxbuild) -c $< -o $(objdir)$@ 

client_main.o: client_main.cc udp_client_base.h
	$(cxxbuild) $(protobuflib) -c $< -o $(clientobjsdir)$@ 

udp_client_base.o: udp_client_base.cc udp_client_base.h $(commonheaders)
	$(cxxbuild) -c $< -o $(clientobjsdir)$@ 

client: client_main.o udp_client_base.o socket_base.o socket_base.o base_datagram.pb.o
	$(cxxbuild) $(clientobjsdir)client_main.o $(clientobjsdir)udp_client_base.o \
	$(objdir)socket_base.o $(objdir)base_datagram.pb.o -o $(builddir)$@.out $(protobuflib) 

udp_service.o: udp_service.cc udp_service.h $(commonheaders)
	$(cxxbuild) $(protobuflib) -c $< -o $(serviceobjsdir)$@ 

service_main.o: service_main.cc udp_service.h
	$(cxxbuild) $(protobuflib) -c -o $(serviceobjsdir)$@ $<

service: service_main.o udp_service.o socket_base.o
	$(cxxbuild) $(serviceobjsdir)service_main.o $(serviceobjsdir)udp_service.o $(objdir)socket_base.o $(objdir)base_datagram.pb.o -o $(builddir)$@.out \
	$(protobuflib)

base_datagram.pb.o: base_datagram.pb.cc base_datagram.pb.h
	$(cxxbuild) -c $< -o $(objdir)$@ 

cxx_models: $(protofiles)
	protoc --proto_path=./udp/src/models/ --cpp_out=./udp/src/models/ base_datagram.proto

# client: client_main.cpp UDPClientBase.cpp SocketBase.hpp SocketHeaders.h SocketBase.cpp
# 	clang++ -std=c++11 client_main.cpp UDPClientBase.cpp SocketBase.cpp -o client.out

# service: server_main.cpp UDPService.cpp SocketBase.hpp SocketHeaders.h SocketBase.cpp
# 	clang++ -std=c++11 server_main.cpp UDPService.cpp SocketBase.cpp -o service.out
