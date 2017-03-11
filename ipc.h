#ifndef _IPC_H
#define _IPC_H

#include "ipc_observer.h"

class MessageHandle
{
public:
	MessageHandle();
	static void OnMessage(void* head,int tid,int msgType,int msgID,char *pBuf, int nLen,void* pUserData);
	static void *_RunThread(void *arg);	
	void RegisterObserver(IPCObserver* observer);
	int Start();
private:
	IPCObserver* m_pObserver;
protected:	
	bool m_bRunning;
	void OnMessage(void* head,int tid,int msgType,int msgID,char *pBuf, int nLen);
	virtual void Run()=0;
};

#endif
