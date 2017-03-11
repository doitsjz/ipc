#include "ipc.h"
#include "ipc_struct.h"

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

void* MessageHandle::_RunThread(void *arg)
{
	MessageHandle* pHandle = static_cast<MessageHandle*>(arg);	
	pHandle->Run();
	return NULL;
}

int MessageHandle::Start()
{	
	if (m_bRunning)
		return 0;
	
	pthread_t tid;
	int r = pthread_create(&tid,NULL,_RunThread,this);
	if (r!=0)
		return r;
	
	pthread_detach(tid);

	m_bRunning = true;
	
	return r;
}

MessageHandle::MessageHandle():m_pObserver(NULL)
{
	m_bRunning = false;
}
void MessageHandle::RegisterObserver(IPCObserver* observer)
{
	m_pObserver= observer;
}
void MessageHandle::OnMessage(void* head,int tid,int msgType,int msgID,char *pBuf, int nLen,void* pUserData)
{
	MessageHandle* pMsgHandle = static_cast<MessageHandle*>(pUserData);	
	pMsgHandle->OnMessage(head,tid,msgType,msgID,pBuf,nLen);
}
void MessageHandle::OnMessage(void* head,int tid,int msgType,int msgID,char *pBuf, int nLen)
{
	//printf("[MessageHandle::OnMessage] tid=%d type=%d id=%d len=%d!\n",tid,msgType,msgID,nLen);
	if (!m_pObserver)
	{
		printf("[MessageHandle::OnMessage] Not RegisterObserver, Data Will Drop!\n");
		return;
	}
	switch (msgType)
	{
		case IPC_MSG_TYPE_REQUEST:
			m_pObserver->OnRequest(head,this,tid,msgID,pBuf,nLen);
			break;
		case IPC_MSG_TYPE_RESPONSE:
			m_pObserver->OnResponse(head,tid,msgID,pBuf,nLen);
			break;
		case IPC_MSG_TYPE_NOTIFICATION:
			m_pObserver->OnNotification(tid,msgID,pBuf,nLen);
			break;
		default:
			printf("[MessageHandle::OnMessage] Not Support this msgType=%d!\n",msgType);
			break;			
	}
}


