#include "ipc_server.h"

#include <unistd.h>

static const char* key="IPC_AUDIO";
static const char* user="admin";

int main(int argc,char** argv)
{
	IPCServer server;
	server.Start();

	while(true)
	{
		sleep(5);
		server.SendBindingRequest(key,user);
		server.SendRebootRequest();
		server.SendResetRequest();
	}

	return 0;
}

