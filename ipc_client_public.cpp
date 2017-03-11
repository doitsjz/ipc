#include "ipc_client.h"
#include <stdio.h>

/////////////////////handle request from server/////////////////////////////////
void IPCClient::OnBindingRequest(IPC_MSG_HEAD_T* head,int tid,char *pBuf, int nLen)
{
	BINDING_MSG_T* pMsg = reinterpret_cast<BINDING_MSG_T*>(pBuf);
	printf("[IPCClient::OnBindingRequest] key=%s user=%s!\n",pMsg->key,pMsg->user);
	SendBindingResponse(head,0);
}
void IPCClient::OnRebootRequest(IPC_MSG_HEAD_T* head,int tid,char *pBuf, int nLen)
{
	printf("[IPCClient::OnRebootRequest] nLen=%d\n",nLen);
	SendRebootResponse(head,0);
}
void IPCClient::OnResetRequest(IPC_MSG_HEAD_T* head,int tid,char *pBuf, int nLen)
{
	printf("[IPCClient::OnResetRequest] nLen=%d\n",nLen);
	SendResetResponse(head,0);
}
/////////////////////handle response from server/////////////////////////////////
void IPCClient::OnDeviceInfoResponse(int tid,char *pBuf, int nLen,int errcode)
{
	printf("[IPCClient::OnDeviceInfoResponse] errcode=%d nLen=%d\n",errcode,nLen);
}
void IPCClient::OnHeartbeatResponse(int tid,char *pBuf, int nLen,int errcode)
{
	printf("[IPCClient::OnHeartbeatResponse] errcode=%d nLen=%d m_nHeartBeatTimes=%d\n",errcode,nLen,m_nHeartBeatTimes);
}



