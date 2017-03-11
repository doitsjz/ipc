#include "ipc_client.h"

#include <unistd.h>

static const char* uid="1234567890";

int main(int argc,char** argv)
{
	IPCClient client(argv[1]);//ip is your server for  test ,really is not set anything
	client.Start();
	
	while (true)
	{
		sleep(5);
		client.SendAlarmNotification(1,0,0);
		client.SendDeviceInfoRequest(uid);
	}

	return 0;
}
