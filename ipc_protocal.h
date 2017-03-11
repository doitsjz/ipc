#ifndef _IPC_PROTOCAL_H
#define _IPC_PROTOCAL_H

#include "ipc_struct.h"

//little endian
typedef struct IPC_MSG_HEAD_T
{
	IPC_MSG_HEAD_T()
	{
		magic = 0xFF;
		version = 0x01;
		reserved = 0;
		len = tid = 0;
		msgtype = IPC_MSG_TYPE_UNKOWN;
		msgid = IPC_MSGID_INVALID;
		errcode=0;
	}
	unsigned char magic;//0xFF
	unsigned char version;//0x01
	unsigned char msgtype;//@ref IPC_MSG_TYPE
	unsigned char reserved;
	unsigned int   errcode;
	
	unsigned int  msgid;//@ref IPC_MSGID
	unsigned int tid;//transation id
	unsigned int len;//body len ,exclude head
	
}IPC_MSG_HEAD_T,*PIPC_MSG_HEAD_T;

#define IPC_MSG_HEAD_LEN	(sizeof(IPC_MSG_HEAD_T))
#define IPC_MAX_BUF_LEN		512
#define IPC_SERVER_PORT		4321
#define IPC_SERVER_IP	"192.168.10.1"

typedef void (*MessageCallback)(void* head,int tid,int msgType,int msgID,char *pBuf, int nLen,void* pUserData);

class IPCProtocal
{
public:
	IPCProtocal();
	~IPCProtocal();
	void RegisterMessageCallback(MessageCallback fn,void* pUserData);
	int Parse(const char* body,int len);
	int FormatAlarmMsg(int alarmType,char* msg,int &len);
	int FormatBindingMsg(const char* key,const char* user,char* msg,int &len);
	int FormatHeartbeatMsg(char* msg,int &len,int seconds=5);
	int FormatResetMsg(char* msg,int &len);
	int FormatRebootMsg(char* msg,int &len);
	int FormatDeviceInfoMsg(const char* uid,char* msg,int &len);
private:
	void InitHead(IPC_MSG_HEAD_T* &pHead,char* msg);
private:
	MessageCallback m_fn;
	char   m_szMsgBuffer[IPC_MAX_BUF_LEN];
	int     m_nOffset;
	void* m_pUserData;
	unsigned int m_tid;
};

#endif
