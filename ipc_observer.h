#ifndef _IPC_OBSERVER_H
#define _IPC_OBSERVER_H

class MessageHandle;
class IPCObserver
{
public:
	virtual void OnRequest(void* head,MessageHandle* pMsgHandle,int tid,int msgID,char *pBuf, int nLen)=0;
	virtual void OnResponse(void* head,int tid,int msgID,char *pBuf, int nLen)=0;
	virtual void OnNotification(int tid,int msgID,char *pBuf, int nLen)=0;	
};

#endif