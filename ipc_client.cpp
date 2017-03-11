#include "ipc_client.h"

#include <pthread.h>
#include <sys/types.h>	
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <cmath>

static const int MAX_HB_TIEMS = 3;

IPCClient::IPCClient(const char* serverIP)
{
	m_nClientSocket = -1;
	m_nHeartBeatTimes = 0;
	memset(m_szMsgBuffer,0,sizeof(m_szMsgBuffer));
	m_strServerIP = (serverIP==NULL)?IPC_SERVER_IP:serverIP;
	m_ObjProtocal.RegisterMessageCallback(MessageHandle::OnMessage,this);

	signal(SIGPIPE,SIG_IGN);//avoid process exit when send on a disconnect socket
	
	RegisterObserver(this);
}
IPCClient::~IPCClient()
{
	if (m_nClientSocket>0)
		close(m_nClientSocket);
}

void IPCClient::Run()
{
	DoWork();
}

void IPCClient::Close()
{
	if (m_nClientSocket>=0)
	{
		close(m_nClientSocket);
		m_nClientSocket = -1;
	}
}
int IPCClient::SendAlarmNotification(int alarmType,const char* remark,int iLen)
{
	if (m_nClientSocket<0)
		return -1;
	
	char buffer[IPC_MAX_BUF_LEN] = {0};
	int len = sizeof(buffer);
	m_ObjProtocal.FormatAlarmMsg(alarmType,buffer,len);

	int r = send(m_nClientSocket,buffer,len,0);
	if (r<0)
	{
		printf("[IPCClient::SendAlarmNotification] send type=%d m_nClientSocket=%d r=%d!\n",alarmType,m_nClientSocket,r);
		return -1;
	}
	printf("[IPCClient::SendAlarmNotification] send type=%d len=%d OK!\n",alarmType,r);
	
	return 0;
}
int IPCClient::SendHearbeat()
{
	if (m_nClientSocket<0)
		return -1;
	
	char buffer[IPC_MAX_BUF_LEN] = {0};
	int len = sizeof(buffer);
	m_ObjProtocal.FormatHeartbeatMsg(buffer,len);

	int r = send(m_nClientSocket,buffer,len,0);
	if (r<0)
	{
		printf("[IPCClient::SendHearbeat] m_nClientSocket=%d r=%d!\n",m_nClientSocket,r);
		return -1;
	}
	
	printf("[IPCClient::SendHearbeat] T=%d Send OK!\n",(int)time(NULL));
	
	return 0;
}
int IPCClient::SendDeviceInfoRequest(const char* uid)
{
	if (m_nClientSocket<0)
		return -1;
	
	char buffer[IPC_MAX_BUF_LEN] = {0};
	int len = sizeof(buffer);
	m_ObjProtocal.FormatDeviceInfoMsg(uid,buffer,len);

	int r = send(m_nClientSocket,buffer,len,0);
	if (r<0)
	{
		printf("[IPCClient::SendDeviceInfoRequest] m_nClientSocket=%d r=%d!\n",m_nClientSocket,r);
		return -1;
	}
	
	printf("[IPCClient::SendDeviceInfoRequest] Send OK!\n");
	
	return 0;
}

int IPCClient::SendBindingResponse(IPC_MSG_HEAD_T* head,int errcode)
{
	int r = SendCommonResponse(head,errcode);
	if (r<0)
	{
		printf("[IPCClient::SendBindingResponse] m_nClientSocket=%d r=%d!\n",m_nClientSocket,r);
		return -1;
	}	
	printf("[IPCClient::SendBindingResponse] Send OK!\n");
	return 0;	
}
int IPCClient::SendRebootResponse(IPC_MSG_HEAD_T* head,int errcode)
{
	int r = SendCommonResponse(head,errcode);
	if (r<0)
	{
		printf("[IPCClient::SendRebootResponse] m_nClientSocket=%d r=%d!\n",m_nClientSocket,r);
		return -1;
	}	
	printf("[IPCClient::SendRebootResponse] Send OK!\n");
	return 0;	
}
int IPCClient::SendResetResponse(IPC_MSG_HEAD_T* head,int errcode)
{
	int r = SendCommonResponse(head,errcode);
	if (r<0)
	{
		printf("[IPCClient::SendResetResponse] m_nClientSocket=%d r=%d!\n",m_nClientSocket,r);
		return -1;
	}	
	printf("[IPCClient::SendResetResponse] Send OK!\n");	
	return 0;	
}
int IPCClient::SendCommonResponse(IPC_MSG_HEAD_T* head,int errcode,char* msgBody,int bodyLen)
{
	if (m_nClientSocket<0)
		return -1;
	
	head->errcode = errcode;
	head->msgtype = IPC_MSG_TYPE_RESPONSE;
	head->len = 0;
	
	char buffer[IPC_MAX_BUF_LEN] = {0};
	memcpy(buffer,head,IPC_MSG_HEAD_LEN);
	if (msgBody && bodyLen)
		memcpy(buffer+IPC_MSG_HEAD_LEN,msgBody,bodyLen);

	head->len = bodyLen;
	int len = IPC_MSG_HEAD_LEN+head->len;
	
	int r = send(m_nClientSocket,head,len,0);
	if (r<0)
	{
		return -1;
	}
	
	return 0;	
}

int IPCClient::Create()
{
	m_nHeartBeatTimes = 0;//reset

	Close();
	
	m_nClientSocket = socket(AF_INET,SOCK_STREAM,0);

	struct timeval timeout;
	timeout.tv_sec = 3; 
	timeout.tv_usec = 0;
	setsockopt(m_nClientSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(timeout));
	setsockopt(m_nClientSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout));
	struct linger  so_linger;
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	setsockopt(m_nClientSocket,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));
	
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(IPC_SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(m_strServerIP);
	int addr_len = sizeof(server_addr);

	if (m_nClientSocket < 0)
	{
		printf("[IPCClient::Create] socket error=%s!\n",strerror(errno));
		return -1;
	}
	int r = connect(m_nClientSocket, (struct sockaddr *)&server_addr, addr_len);  
	printf("[IPCClient::Create] connect ip=%s r=%d!\n",m_strServerIP,r);
	if(r <0)
	{  
		printf("[IPCClient::Create] m_nClientSocket=%d connect failed=%s!\n",m_nClientSocket,strerror(errno)); 
		Close();
		return -2;
	}  

	return 0;
}
void IPCClient::DoWork()
{
	char buffer[IPC_MAX_BUF_LEN] = {0};	
	struct timeval	tv_select;
	tv_select.tv_sec = 0;
	tv_select.tv_usec = 500*1000;//500ms
	m_bRunning = true;

	printf("[IPCClient::DoWork]  m_nClientSocket=%d!\n",m_nClientSocket);
	
	while (true)
	{
		if (m_nClientSocket<0 || m_nHeartBeatTimes>=MAX_HB_TIEMS)
		{
			Create();
			printf("[IPCClient::DoWork] Create m_nClientSocket=%d times=%d!\n",m_nClientSocket,m_nHeartBeatTimes);
		}
		
		int max_fd = m_nClientSocket;
		fd_set	read_set;
		fd_set	write_set;
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);

		if (m_nClientSocket > 0)
		{
			FD_SET(m_nClientSocket, &read_set);
			FD_SET(m_nClientSocket, &write_set);
		}

		int r = select(max_fd + 1, &read_set, &write_set, NULL, &tv_select);
		if (r < 0) 
		{
			printf("[IPCClient::DoWork] iRet=%d error=%s!\n",r,strerror(errno));
			Close();
			continue;
		}
		else if (0 == r)
		{
			//printf("[IPCClient::DoWork] iRet=%d!\n",iRet);
			continue;
		}
		else
		{
			if (-1 != m_nClientSocket && FD_ISSET(m_nClientSocket, &read_set))
			{
				int nRecvLen = recv(m_nClientSocket, buffer, IPC_MAX_BUF_LEN,0);
				if (nRecvLen > 0)
				{
					m_ObjProtocal.Parse(buffer,nRecvLen);
				}
				else
				{
					printf("[IPCClient::DoWork] nRecvLen=%d Will ReCreate err=%s!\n",nRecvLen,strerror(errno));
					Close();
				}
			}
			if (-1 != m_nClientSocket && FD_ISSET(m_nClientSocket, &write_set))
			{
				static time_t begTime = time(NULL);
				time_t endTime = time(NULL);
				if (abs(endTime-begTime)>=5)
				{
					begTime = endTime;
					SendHearbeat();
					m_nHeartBeatTimes++;
				}
			}
		}
	}
}
void IPCClient::OnRequest(void* head,MessageHandle* pMsgHandle,int tid,int msgID,char *pBuf, int nLen)
{
	switch (msgID)
	{
		case IPC_MSGID_BINDING:
			OnBindingRequest(static_cast<PIPC_MSG_HEAD_T>(head),tid,pBuf,nLen);break;
		case IPC_MSGID_REBOOT:
			OnRebootRequest(static_cast<PIPC_MSG_HEAD_T>(head),tid,pBuf,nLen);break;
		case IPC_MSGID_RESET:
			OnResetRequest(static_cast<PIPC_MSG_HEAD_T>(head),tid,pBuf,nLen);break;
		default:
			printf("[IPCClient::OnRequest] tid=%d id=%d len=%d!Invalid!\n",tid,msgID,nLen); 			
	}
}
void IPCClient::OnResponse(void* head,int tid,int msgID,char *pBuf, int nLen)
{
	PIPC_MSG_HEAD_T pHead = static_cast<PIPC_MSG_HEAD_T>(head);	

	//recv any response as server alive
	m_nHeartBeatTimes = 0;
	
	switch (msgID)
	{
		case IPC_MSGID_DEVICE_INFO:
		{
			OnDeviceInfoResponse(tid,pBuf,nLen,pHead->errcode);break;
		}
		case IPC_MSGID_HEARTBEAT:
		{
			OnHeartbeatResponse(tid,pBuf,nLen,pHead->errcode);break;
			break;
		}
		default:			
		{
			printf("[IPCClient::OnResponse] tid=%d id=%d len=%d!\n",tid,msgID,nLen);	
			break;
		}			
	}
}
void IPCClient::OnNotification(int tid,int msgID,char *pBuf, int nLen)
{
	printf("[IPCClient::OnNotification] tid=%d id=%d len=%d!\n",tid,msgID,nLen);		
}

