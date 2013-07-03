#pragma once
#include "sys.h"
#include "socket.h"
#include "singleton.h"
#include "net.h"
#ifdef __WINDOWS__
// winsock 2 ��ͷ�ļ��Ϳ�

// ���������� (1024*8)
// ֮����Ϊʲô����8K��Ҳ��һ�������ϵľ���ֵ
// ���ȷʵ�ͻ��˷�����ÿ�����ݶ��Ƚ��٣���ô�����õ�СһЩ��ʡ�ڴ�
#define MAX_BUFFER_LEN        8192  
// Ĭ�϶˿�
#define DEFAULT_PORT          12345    
// Ĭ��IP��ַ
#define DEFAULT_IP            ("127.0.0.1")


//////////////////////////////////////////////////////////////////
// ����ɶ˿���Ͷ�ݵ�I/O����������
typedef enum _OPERATION_TYPE  
{  
	ACCEPT_POSTED,                     // ��־Ͷ�ݵ�Accept����
	SEND_POSTED,                       // ��־Ͷ�ݵ��Ƿ��Ͳ���
	RECV_POSTED,                       // ��־Ͷ�ݵ��ǽ��ղ���
	NULL_POSTED                        // ���ڳ�ʼ����������

}OPERATION_TYPE;

//====================================================================================
//
//				��IO���ݽṹ�嶨��(����ÿһ���ص������Ĳ���)
//
//====================================================================================

class IoContent
{
public:
	OVERLAPPED     overlapped;                               // ÿһ���ص�����������ص��ṹ(���ÿһ��Socket��ÿһ����������Ҫ��һ��)              
	union{
	Socket	       *socket;                               // ������������ʹ�õ�Socket
	SOCKET sockfd; // ���Ӿ��
	};
	WSABUF         m_wsaBuf;                                   // WSA���͵Ļ����������ڸ��ص�������������
	char           m_szBuffer[MAX_BUFFER_LEN];                 // �����WSABUF�������ַ��Ļ�����
	OPERATION_TYPE optype;                                   // ��ʶ�������������(��Ӧ�����ö��)
	DWORD			msgLen;
	// ��ʼ��
	IoContent()
	{
		bzero(&overlapped, sizeof(overlapped));  
		bzero( m_szBuffer,MAX_BUFFER_LEN );
		m_wsaBuf.buf = m_szBuffer;
		m_wsaBuf.len = MAX_BUFFER_LEN;
		optype     = NULL_POSTED;
		socket = NULL;
		msgLen = 0;
	}
	// �ͷŵ�Socket
	~IoContent()
	{
	//	socket->getHandle().close();
	}
	// ���û���������
	void ResetBuffer()
	{
		bzero( m_szBuffer,MAX_BUFFER_LEN );
	}

};


//====================================================================================
//
//				��������ݽṹ�嶨��(����ÿһ����ɶ˿ڣ�Ҳ����ÿһ��Socket�Ĳ���)
//
//====================================================================================

class SocketContent:public Socket
{  
public:
	//Socket      		*socket;                                  // ÿһ���ͻ������ӵ�Socket
	SOCKADDR_IN		m_ClientAddr;                              // �ͻ��˵ĵ�ַ
	std::vector<IoContent*> ioContents;             // �ͻ���������������������ݣ�
	                                                       // Ҳ����˵����ÿһ���ͻ���Socket���ǿ���������ͬʱͶ�ݶ��IO�����
	//IoContent acceptIo;
	IoContent sendIo;
	IoContent recvIo;
	// ��ʼ��
	DYNAMIC_API SocketContent()
	{
		memset(&m_ClientAddr, 0, sizeof(m_ClientAddr)); 
		createTime = sys::sTime::getNowTime();
	//	socket = NULL;
	}

	// �ͷ���Դ
	DYNAMIC_API ~SocketContent()
	{
		getHandle().close();
		// �ͷŵ����е�IO����������
		for(unsigned int i=0;i < ioContents.size();i++ )
		{
			delete ioContents[i];
		}
		ioContents.clear();
	}

	// ��ȡһ���µ�IoContext
	DYNAMIC_API IoContent* GetNewIoContext()
	{
		IoContent* p = new IoContent;
		ioContents.push_back(p);
		return p;
	}

	// ���������Ƴ�һ��ָ����IoContext
	DYNAMIC_API void RemoveContext( IoContent* pContext )
	{
		for(unsigned int i=0;i< ioContents.size();i++ )
		{
			if( pContext==ioContents[0])
			{
				delete pContext;
				pContext = NULL;
				ioContents[0] = NULL;				
				break;
			}
		}
	}
	DYNAMIC_API void wakeupSend();
};
 
class IocpWorkThread;
class CheckValidThread;
/**
 * Iocp �������Ӻ�io ������� 
 */
class IocpPool:public ConnectionPool,public Singleton<IocpPool>
{
public:
	DYNAMIC_API IocpPool(void);
	DYNAMIC_API ~IocpPool(void);

public:
	/**
	* ��ʼ��������
	* \param handle ��� 
	*/
	DYNAMIC_API virtual void init(Socket *server){}
	/**
 	 * ����socket,���뵽�������߳���
 	 * */
	DYNAMIC_API bool addSocket(Socket *socket);
	// ����������
	DYNAMIC_API bool Start();

	//	ֹͣ������
	DYNAMIC_API void Stop();
	DYNAMIC_API virtual void start(WORD port)
	{
		SetPort(port);
		Start();
	}

	// ����Socket��
	DYNAMIC_API static bool LoadSocketLib();

	// ж��Socket�⣬��������
	DYNAMIC_API void UnloadSocketLib() { WSACleanup(); }

	// ��ñ�����IP��ַ
	DYNAMIC_API std::string GetLocalIP();

	// ���ü����˿�
	DYNAMIC_API void SetPort( const int& nPort ) { m_nPort=nPort; }
protected:

	// ��ʼ��IOCP
	bool _InitializeIOCP();

	// ��ʼ��Socket
	bool _InitializeListenSocket();

	// ����ͷ���Դ
	void _DeInitialize();

	// Ͷ��Accept����
	bool _PostAccept( IoContent* pAcceptIoContext ); 

	// Ͷ�ݽ�����������
	bool _PostRecv( IoContent* pIoContext );
	
	// Ͷ�ݷ�����������
	bool _PostSend( IoContent* pIoContext );

	// ���пͻ��������ʱ�򣬽��д���
	bool _DoAccpet( SocketContent* pSocketContext, IoContent* pIoContext );

	// ���н��յ����ݵ����ʱ�򣬽��д���
	bool _DoRecv( SocketContent* pSocketContext, IoContent* pIoContext ,DWORD len);
	
	// ���з��͵����ݵ����ʱ�򣬽��д���
	bool _DoSend( SocketContent* pSocketContext, IoContent* pIoContext ,DWORD len);
	// ���ͻ��˵������Ϣ�洢��������
	void _AddToContextList( SocketContent *pSocketContext );

	// ���ͻ��˵���Ϣ���������Ƴ�
	void _RemoveContext( SocketContent *pSocketContext );

	// ��տͻ�����Ϣ
	void _ClearContextList();

	// ������󶨵���ɶ˿���
	bool _AssociateWithIOCP( SocketContent *pContext);

	// ������ɶ˿��ϵĴ���
	bool HandleError( SocketContent *pContext,const DWORD& dwErr );

	// �̺߳�����ΪIOCP�������Ĺ������߳�
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);

	// ��ñ����Ĵ���������
	int _GetNoOfProcessors();

	// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ�
	bool _IsSocketAlive(SOCKET s);
	/**
	 * ִ�ж�ʱ�����Ϊ
	 */
	void checkValidTick();
private:
	CheckValidThread *check; // ��ʱ����߳�
	HANDLE                       m_hShutdownEvent;              // ����֪ͨ�߳�ϵͳ�˳����¼���Ϊ���ܹ����õ��˳��߳�

	HANDLE                       m_hIOCompletionPort;           // ��ɶ˿ڵľ��

	std::vector<IocpWorkThread*> _works;             // �������̵߳ľ��ָ��

	std::string                  m_strIP;                       // �������˵�IP��ַ
	int                          m_nPort;                       // �������˵ļ����˿�

	CRITICAL_SECTION             m_csContextList;               // ����Worker�߳�ͬ���Ļ�����

	std::list<SocketContent*>  m_arrayClientContext;          // �ͻ���Socket��Context��Ϣ        
	typedef std::list<SocketContent*>::iterator CLIENT_CONTEXT_ITER;
	SocketContent*          m_pListenContext;              // ���ڼ�����Socket��Context��Ϣ

	LPFN_ACCEPTEX                m_lpfnAcceptEx;                // AcceptEx �� GetAcceptExSockaddrs �ĺ���ָ�룬���ڵ�����������չ����
	LPFN_GETACCEPTEXSOCKADDRS    m_lpfnGetAcceptExSockAddrs; 
	friend class IocpWorkThread;
	friend class CheckValidThread;
};

#endif