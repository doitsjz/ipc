#include "ipc_server.h"
#include <stdio.h>

void IPCServer::OnDeviceInfoRequest(IPC_MSG_HEAD_T* head,int tid,char *pBuf, int nLen)
{
	DEVICE_INFO_MSG_T* pMsg = reinterpret_cast<DEVICE_INFO_MSG_T*>(pBuf);
	printf("[IPCServer::OnDeviceInfoRequest] uid=%s!\n",pMsg->uid);
	//write your code from here 
	int errcode = 0;

	//end your code
	
	SendDeviceInfoResponse(head,errcode);
}
void IPCServer::OnHeartbeatRequest(IPC_MSG_HEAD_T* head,int tid,char *pBuf, int nLen)
{
	HEARTBEAT_MSG_T* pMsg = reinterpret_cast<HEARTBEAT_MSG_T*>(pBuf);
	printf("[IPCServer::OnHeartbeatRequest] seconds=%d!\n",pMsg->seconds);
	int errcode = 0;
	SendHeartbeatResponse(head,errcode);
}
void IPCServer::OnAlarmNotification(int tid,char *pBuf, int nLen)
{
	ALARM_MSG_T* pMsg = reinterpret_cast<ALARM_MSG_T*>(pBuf);
	printf("[IPCServer::OnAlarmNotification] AlarmType=%d!\n",pMsg->alarmType);
	//write your code from here 
}

void IPCServer::OnBindingResponse(int tid,char *pBuf, int nLen,int errcode)
{
	printf("[IPCServer::OnBindingResponse] errcode=%d nLen=%d\n",errcode,nLen);
}
void IPCServer::OnRebootResponse(int tid,char *pBuf, int nLen,int errcode)
{
	printf("[IPCServer::OnRebootResponse] errcode=%d nLen=%d\n",errcode,nLen);
}
void IPCServer::OnResetResponse(int tid,char *pBuf, int nLen,int errcode)
{
	printf("[IPCServer::OnResetResponse] errcode=%d nLen=%d\n",errcode,nLen);
}

