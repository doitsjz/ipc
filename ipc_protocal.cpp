#include "ipc_protocal.h"

#include <string.h>
#include <algorithm>
#include <stdio.h>

IPCProtocal::IPCProtocal()
{
	m_fn = NULL;	
	m_pUserData = NULL;
	m_nOffset = 0;
	m_tid = 0;
	memset(m_szMsgBuffer,0,sizeof(m_szMsgBuffer));
}
IPCProtocal::~IPCProtocal()
{
}
void IPCProtocal::RegisterMessageCallback(MessageCallback fn,void* pUserData)
{
	m_fn = fn;
	m_pUserData = pUserData;
}
void IPCProtocal::InitHead(IPC_MSG_HEAD_T* &pHead,char* msg)
{
	pHead = reinterpret_cast<IPC_MSG_HEAD_T*>(msg);
	IPC_MSG_HEAD_T head;
	memcpy(pHead,&head,sizeof(head));
	//not lock,only call by one thread 
	pHead->tid = ++m_tid;	
}
	
int IPCProtocal::FormatAlarmMsg(int alarmType,char* msg,int& len)
{
	if (len<IPC_MSG_HEAD_LEN+sizeof(ALARM_MSG_T))
		return -1;

	IPC_MSG_HEAD_T* pHead = NULL;
	InitHead(pHead,msg);
	
	pHead->msgtype = IPC_MSG_TYPE_NOTIFICATION;
	pHead->msgid = IPC_MSGID_ALARM;
	pHead->len = sizeof(ALARM_MSG_T);
	ALARM_MSG_T* pAlarmMsg = reinterpret_cast<ALARM_MSG_T*>(msg+IPC_MSG_HEAD_LEN);
	pAlarmMsg->alarmType = alarmType;

	len = IPC_MSG_HEAD_LEN+sizeof(ALARM_MSG_T);

	return 0;	
}
int IPCProtocal::FormatBindingMsg(const char* key,const char* user,char* msg,int &len)
{
	if (len<IPC_MSG_HEAD_LEN+sizeof(BINDING_MSG_T))
		return -1;

	IPC_MSG_HEAD_T* pHead = NULL;
	InitHead(pHead,msg);
	
	pHead->msgtype = IPC_MSG_TYPE_REQUEST;
	pHead->msgid = IPC_MSGID_BINDING;
	pHead->len = sizeof(BINDING_MSG_T);
	BINDING_MSG_T* pMsg = reinterpret_cast<BINDING_MSG_T*>(msg+IPC_MSG_HEAD_LEN);
	memcpy(pMsg->key,key,sizeof(pMsg->key)-1);
	memcpy(pMsg->user,user,sizeof(pMsg->user)-1);

	len = IPC_MSG_HEAD_LEN+sizeof(BINDING_MSG_T);

	return 0;		
}
int IPCProtocal::FormatHeartbeatMsg(char* msg,int &len,int seconds)
{
	if (len<IPC_MSG_HEAD_LEN+sizeof(HEARTBEAT_MSG_T))
		return -1;

	IPC_MSG_HEAD_T* pHead = NULL;
	InitHead(pHead,msg);
	
	pHead->msgtype = IPC_MSG_TYPE_REQUEST;
	pHead->msgid = IPC_MSGID_HEARTBEAT;
	pHead->len = sizeof(HEARTBEAT_MSG_T);
	HEARTBEAT_MSG_T* pMsg = reinterpret_cast<HEARTBEAT_MSG_T*>(msg+IPC_MSG_HEAD_LEN);
	pMsg->seconds = seconds;
	len = IPC_MSG_HEAD_LEN+sizeof(HEARTBEAT_MSG_T);

	return 0;	
}
int IPCProtocal::FormatResetMsg(char* msg,int &len)
{
	if (len<IPC_MSG_HEAD_LEN)
		return -1;

	IPC_MSG_HEAD_T* pHead = NULL;
	InitHead(pHead,msg);
	
	pHead->msgtype = IPC_MSG_TYPE_REQUEST;
	pHead->msgid = IPC_MSGID_RESET;
	len = IPC_MSG_HEAD_LEN;

	return 0;	
}
int IPCProtocal::FormatRebootMsg(char* msg,int &len)
{
	if (len<IPC_MSG_HEAD_LEN)
		return -1;

	IPC_MSG_HEAD_T* pHead = NULL;
	InitHead(pHead,msg);
	
	pHead->msgtype = IPC_MSG_TYPE_REQUEST;
	pHead->msgid = IPC_MSGID_REBOOT;
	len = IPC_MSG_HEAD_LEN;

	return 0;	
}
int IPCProtocal::FormatDeviceInfoMsg(const char* uid,char* msg,int &len)
{
	if (len<IPC_MSG_HEAD_LEN+sizeof(DEVICE_INFO_MSG_T))
		return -1;

	IPC_MSG_HEAD_T* pHead = NULL;
	InitHead(pHead,msg);
	
	pHead->msgtype = IPC_MSG_TYPE_REQUEST;
	pHead->msgid = IPC_MSGID_DEVICE_INFO;

	pHead->len = sizeof(DEVICE_INFO_MSG_T);
	DEVICE_INFO_MSG_T* pMsg = reinterpret_cast<DEVICE_INFO_MSG_T*>(msg+IPC_MSG_HEAD_LEN);
	memcpy(pMsg->uid,uid,sizeof(pMsg->uid));
	
	len = IPC_MSG_HEAD_LEN+sizeof(DEVICE_INFO_MSG_T);

	return 0;		
}

int IPCProtocal::Parse(const char* body,int len)
{
	while (len > 0)
	{
		int nMinLen = std::min(IPC_MAX_BUF_LEN - m_nOffset, len);
		memcpy(m_szMsgBuffer + m_nOffset, body, nMinLen);
		
		len -= nMinLen;
		m_nOffset += nMinLen;

		int nPacketLen = m_nOffset;
		//printf("[IPCProtocal::Parse] nTotalLen=%d len=%d nMinLen=%d m_nOffset=%d!\n",nTotalLen,len,nMinLen,m_nOffset);
		
		if (nPacketLen >= IPC_MSG_HEAD_LEN)
		{
			int nPacketOffSet = 0;
			while (nPacketLen >= IPC_MSG_HEAD_LEN)
			{
				IPC_MSG_HEAD_T *pHead = (IPC_MSG_HEAD_T *)(m_szMsgBuffer + nPacketOffSet);
				int bodyLen = pHead->len;
				if (nPacketLen >= (bodyLen + IPC_MSG_HEAD_LEN))
				{
					//only data ,not include head
					if (m_fn)
					{
						m_fn(pHead,pHead->tid,pHead->msgtype,pHead->msgid, (m_szMsgBuffer + nPacketOffSet+IPC_MSG_HEAD_LEN), pHead->len,m_pUserData);
						//printf("[IPCProtocal::Parse] tid=%d type=%d id=0x%x len=%d!\n",
						//	pHead->tid,pHead->msgtype,pHead->msgid,bodyLen);
					}						
					
					nPacketOffSet += bodyLen + IPC_MSG_HEAD_LEN;
					nPacketLen -= bodyLen + IPC_MSG_HEAD_LEN;
				}
				else//small data ,not engouh
				{
					break;
				}
			}
			
			memcpy(m_szMsgBuffer, m_szMsgBuffer + nPacketOffSet, m_nOffset - nPacketOffSet);
			m_nOffset -= nPacketOffSet;
		}
	}
}

