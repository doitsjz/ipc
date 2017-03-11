#ifndef _IPC_CLIENT_H
#define _IPC_CLIENT_H

#include "ipc.h"
#include "ipc_protocal.h"

class IPCClient:public MessageHandle,IPCObserver
{
public:
	IPCClient(const char* serverIP=IPC_SERVER_IP);
	~IPCClient();
	//send reqest or notification to server
	int SendAlarmNotification(int alarmType,const char* remark,int iLen);
	int SendDeviceInfoRequest(const char* uid);
	//handle request from server
	void OnBindingRequest(IPC_MSG_HEAD_T* head,int tid,char *pBuf, int nLen);
	void OnRebootRequest(IPC_MSG_HEAD_T* head,int tid,char *pBuf, int nLen);
	void OnResetRequest(IPC_MSG_HEAD_T* head,int tid,char *pBuf, int nLen);

	//handle response from server
	void OnDeviceInfoResponse(int tid,char *pBuf, int nLen,int errcode);	
	void OnHeartbeatResponse(int tid,char *pBuf, int nLen,int errcode);	
private:
	int SendBindingResponse(IPC_MSG_HEAD_T* head,int errcode);
	int SendRebootResponse(IPC_MSG_HEAD_T* head,int errcode);
	int SendResetResponse(IPC_MSG_HEAD_T* head,int errcode);	
	int SendCommonResponse(IPC_MSG_HEAD_T* head,int errcode,char* msgBody=0,int bodyLen=0);
protected:
	virtual void OnRequest(void* head,MessageHandle* pMsgHandle,int tid,int msgID,char *pBuf, int nLen);
	virtual void OnResponse(void* head,int tid,int msgID,char *pBuf, int nLen);
	virtual void OnNotification(int tid,int msgID,char *pBuf, int nLen);	
private:
	virtual void Run();
	void DoWork();
	int Create();
	void Close();	
	int SendHearbeat();
private:
	int m_nClientSocket;
	bool m_bConnected;
	char m_szMsgBuffer[IPC_MAX_BUF_LEN];
	IPCProtocal m_ObjProtocal;
	const char* m_strServerIP;
	int m_nHeartBeatTimes;
};

#endif
