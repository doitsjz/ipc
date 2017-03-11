#ifndef _IPC_SERVER_H
#define _IPC_SERVER_H

#include "ipc.h"
#include "ipc_protocal.h"

class IPCServer:public MessageHandle,IPCObserver
{
public:
	IPCServer();
	~IPCServer();
	//send request to client
	int SendBindingRequest(const char* key,const char* user);
	int SendRebootRequest();
	int SendResetRequest();	
	//handle requet from client
	void OnDeviceInfoRequest(IPC_MSG_HEAD_T* head,int tid,char *pBuf, int nLen);
	void OnHeartbeatRequest(IPC_MSG_HEAD_T* head,int tid,char *pBuf, int nLen);

	//handle notification from client
	void OnAlarmNotification(int tid,char *pBuf, int nLen);

	//handle response from client
	void OnBindingResponse(int tid,char *pBuf, int nLen,int errcode);
	void OnRebootResponse(int tid,char *pBuf, int nLen,int errcode);
	void OnResetResponse(int tid,char *pBuf, int nLen,int errcode);
private:
	int SendDeviceInfoResponse(IPC_MSG_HEAD_T* head,int errcode);
	int SendHeartbeatResponse(IPC_MSG_HEAD_T* head,int errcode);
	int SendCommonResponse(IPC_MSG_HEAD_T* head,int errcode,char* msgBody=0,int bodyLen=0);
	
protected:
	virtual void OnRequest(void* head,MessageHandle* pMsgHandle,int tid,int msgID,char *pBuf, int nLen);
	virtual void OnResponse(void* head,int tid,int msgID,char *pBuf, int nLen);
	virtual void OnNotification(int tid,int msgID,char *pBuf, int nLen);	
	
private:
	virtual void Run();	
	void DoWork();
private:
	int m_nListenSocket;
	int m_nClientSocket;
	bool m_bRunning;
	char m_szMsgBuffer[IPC_MAX_BUF_LEN];
	IPCProtocal m_ObjProtocal;
};

#endif
