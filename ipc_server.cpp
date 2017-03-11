#include "ipc_server.h"

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

IPCServer::IPCServer()
{
	m_nListenSocket = m_nClientSocket = -1;
	memset(m_szMsgBuffer,0,sizeof(m_szMsgBuffer));
	
	m_ObjProtocal.RegisterMessageCallback(OnMessage,this);

	
	signal(SIGPIPE,SIG_IGN);//avoid process exit when send on a disconnect socket

	RegisterObserver(this);
}
IPCServer::~IPCServer()
{
	if (m_nListenSocket>0)
		close(m_nListenSocket);
	
	if (m_nClientSocket>0)
		close(m_nClientSocket);
}
int IPCServer::SendBindingRequest(const char* key,const char* user)
{
	if (m_nClientSocket<0)
		return -1;
	
	char buffer[IPC_MAX_BUF_LEN] = {0};
	int len = sizeof(buffer);
	m_ObjProtocal.FormatBindingMsg(key,user,buffer,len);

	int r = send(m_nClientSocket,buffer,len,0);
	if (r<0)
	{
		printf("[IPCClient::SendBinding] m_nClientSocket=%d r=%d!\n",m_nClientSocket,r);
		return -1;
	}
	
	printf("[IPCClient::SendBinding] Send OK!\n");
	
	return 0;
}
int IPCServer::SendRebootRequest()
{
	if (m_nClientSocket<0)
		return -1;
	
	char buffer[IPC_MAX_BUF_LEN] = {0};
	int len = sizeof(buffer);
	m_ObjProtocal.FormatRebootMsg(buffer,len);

	int r = send(m_nClientSocket,buffer,len,0);
	if (r<0)
	{
		printf("[IPCClient::SendRebootRequest] m_nClientSocket=%d r=%d!\n",m_nClientSocket,r);
		return -1;
	}
	
	printf("[IPCClient::SendRebootRequest] Send OK!\n");
	
	return 0;
}
int IPCServer::SendResetRequest()
{
	if (m_nClientSocket<0)
		return -1;
	
	char buffer[IPC_MAX_BUF_LEN] = {0};
	int len = sizeof(buffer);
	m_ObjProtocal.FormatResetMsg(buffer,len);

	int r = send(m_nClientSocket,buffer,len,0);
	if (r<0)
	{
		printf("[IPCClient::SendResetRequest] m_nClientSocket=%d r=%d!\n",m_nClientSocket,r);
		return -1;
	}
	
	printf("[IPCClient::SendResetRequest] Send OK!\n");
	
	return 0;
}
int IPCServer::SendDeviceInfoResponse(IPC_MSG_HEAD_T* head,int errcode)
{
	int r = SendCommonResponse(head,errcode);
	if (r<0)
	{
		printf("[IPCClient::SendDeviceInfoResponse] m_nClientSocket=%d r=%d!\n",m_nClientSocket,r);
		return -1;
	}	
	printf("[IPCClient::SendDeviceInfoResponse] Send OK!\n");	
	return 0;	
}
int IPCServer::SendHeartbeatResponse(IPC_MSG_HEAD_T* head,int errcode)
{
	int r = SendCommonResponse(head,errcode);
	if (r<0)
	{
		printf("[IPCClient::SendHeartbeatResponse] m_nClientSocket=%d r=%d!\n",m_nClientSocket,r);
		return -1;
	}	
	printf("[IPCClient::SendHeartbeatResponse] Send OK!\n");	
	return 0;	
}

int IPCServer::SendCommonResponse(IPC_MSG_HEAD_T* head,int errcode,char* msgBody,int bodyLen)
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
	//printf("[IPCServer::SendCommonResponse] len=%d!\n",len);
	return 0;	
}
void IPCServer::Run()
{
	m_nListenSocket = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(IPC_SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int addr_len = sizeof(server_addr);

	struct timeval timeout;
	timeout.tv_sec = 3; 
	timeout.tv_usec = 0;
	setsockopt(m_nListenSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(timeout));
	setsockopt(m_nListenSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout));

	struct linger  so_linger;
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	setsockopt(m_nListenSocket,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));

	int flag = 1;
	setsockopt(m_nListenSocket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	
	int r = bind(m_nListenSocket, (struct sockaddr *) &server_addr, addr_len);
	if (r!=0)
	{
		printf("[IPCServer::Run] bind error=%s!",strerror(errno));	
		close(m_nListenSocket);
		m_nListenSocket = -1;
		return ;
	}
	
	listen(m_nListenSocket,5);

	DoWork();
}
void IPCServer::DoWork()
{
	fd_set	read_set;
	struct timeval	tv_select;
	tv_select.tv_sec = 1;
	tv_select.tv_usec = 0;
	int max_fd = m_nListenSocket;
	int r = 0;
	
	struct sockaddr_in addr_client;
	socklen_t addr_len = sizeof(addr_client);;

	char buffer[IPC_MAX_BUF_LEN] = {0};
	
	while(true)
	{
		FD_ZERO(&read_set);
		FD_SET(m_nListenSocket, &read_set); 
		if (m_nClientSocket > 0)
		{
			FD_SET(m_nClientSocket, &read_set);
			max_fd = m_nClientSocket > max_fd ? m_nClientSocket : max_fd;
		}

		r = select(max_fd + 1, &read_set, NULL, NULL, &tv_select);
		if (r < 0) 
		{
			close(m_nClientSocket);
			m_nClientSocket = -1;
			continue;
		}
		else if (0 == r)
		{
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
					close(m_nClientSocket);
					m_nClientSocket = -1;
				}
			}
			else if (FD_ISSET(m_nListenSocket, &read_set))
			{
				int nAcceptSocket = accept(m_nListenSocket, (struct sockaddr *)&addr_client, &addr_len);
				if (nAcceptSocket > 0 )
				{
					if (m_nClientSocket > 0)
					{
						close(m_nClientSocket);
					}
					m_nClientSocket = nAcceptSocket;
					printf("[IPCServer::DoWork] New Client IP=%s PORT=%d m_nClientSocket=%d!\n",inet_ntoa(addr_client.sin_addr),ntohs(addr_client.sin_port),m_nClientSocket);
				}
			}
		}		
	}
}
void IPCServer::OnRequest(void* head,MessageHandle* pMsgHandle,int tid,int msgID,char *pBuf, int nLen)
{
	//printf("[IPCServer::OnRequest] tid=%d id=0x%x nLen=%d\n",tid,msgID,nLen);
	switch (msgID)
	{
		case IPC_MSGID_DEVICE_INFO:
		{
			OnDeviceInfoRequest(static_cast<PIPC_MSG_HEAD_T>(head),tid,pBuf,nLen);break;
		}
		case IPC_MSGID_HEARTBEAT:
		{
			OnHeartbeatRequest(static_cast<PIPC_MSG_HEAD_T>(head),tid,pBuf,nLen);break;
		}
			
		default:
			printf("[IPCServer::OnRequest] tid=%d id=%d len=%d!Invalid!\n",tid,msgID,nLen); 			
	}
}
void IPCServer::OnResponse(void* head,int tid,int msgID,char *pBuf, int nLen)
{
	PIPC_MSG_HEAD_T pHead = static_cast<PIPC_MSG_HEAD_T>(head);
	//printf("[IPCServer::OnResponse] tid=%d id=%d nLen=%d\n",tid,msgID,nLen);
	switch (msgID)
	{
		case IPC_MSGID_BINDING:
		{
			OnBindingResponse(tid,pBuf,nLen,pHead->errcode);
			break;
		}
		case IPC_MSGID_RESET:
		{
			OnResetResponse(tid,pBuf,nLen,pHead->errcode);
			break;
		}
		case IPC_MSGID_REBOOT:
		{
			OnRebootResponse(tid,pBuf,nLen,pHead->errcode);
			break;
		}
		default:			
		{
			printf("[IPCServer::OnResponse] tid=%d id=%d len=%d!\n",tid,msgID,nLen);		
			break;
		}			
	}
}
void IPCServer::OnNotification(int tid,int msgID,char *pBuf, int nLen)
{
	switch (msgID)
	{
		case IPC_MSGID_ALARM:
			OnAlarmNotification(tid,pBuf,nLen);break;
		default:
			printf("[IPCServer::OnNotification] tid=%d id=0x%x len=%d!\n",tid,msgID,nLen);		
	}
}

