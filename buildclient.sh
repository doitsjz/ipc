#!/bin/sh
g++ -g -o client client.cpp ipc_client.cpp ipc_protocal.cpp  ipc.cpp  ipc_client_public.cpp -lpthread
#mipsel-linux-g++ -g -o client client.cpp ipc_client.cpp ipc_protocal.cpp  ipc.cpp  ipc_client_public.cpp -lpthread
