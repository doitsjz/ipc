#!/bin/sh
g++ -g -o server server.cpp ipc_server.cpp ipc_protocal.cpp ipc.cpp ipc_server_public.cpp -lpthread
#mipsel-linux-g++ -g -o server server.cpp ipc_server.cpp ipc_protocal.cpp ipc.cpp ipc_server_public.cpp -lpthread

