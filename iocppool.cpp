#include "iocppool.h"
#include "usercommand.h"
#ifdef __WINDOWS__
// ÿһ���������ϲ������ٸ��߳�(Ϊ������޶ȵ��������������ܣ���������ĵ�)
#define WORKER_THREADS_PER_PROCESSOR 2
// ͬʱͶ�ݵ�Accept���������(���Ҫ����ʵ�ʵ�����������)
#define MAX_POST_ACCEPT              4000
// ���ݸ�Worker�̵߳��˳��ź�
#define EXIT_CODE                    NULL


// �ͷ�ָ��;����Դ�ĺ�

// �ͷ�ָ���
#define RELEASE(x)                      {if(x != NULL ){delete x;x=NULL;}}
// �ͷž����
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}
// �ͷ�Socket��
#define RELEASE_SOCKET(x)               {if(x !=INVALID_SOCKET) { closesocket(x);x=INVALID_SOCKET;}}

class IocpWorkThread:public sys::Thread{
public:
	void run()
	{
		while(isActive())
		{
		//	printf("wait at iocp work thread...\n");
			OVERLAPPED *pOverlapped = NULL;
			SocketContent		 *pSocketContext = NULL;
			DWORD                dwBytesTransfered = 0;
			BOOL bReturn = GetQueuedCompletionStatus(
				pool->m_hIOCompletionPort,
				&dwBytesTransfered,
				(PULONG_PTR)&pSocketContext,
				&pOverlapped,
				INFINITE
			);
			// ����յ������˳���־����ֱ���˳�
			if ( EXIT_CODE==(DWORD)pSocketContext )
			{
				break;
			}
			
			// �ж��Ƿ�����˴���
			if( !bReturn )  
			{  
				DWORD dwErr = GetLastError();
		//		printf("do error\n");
				// ��ʾһ����ʾ��Ϣ
				if( !pool->HandleError( pSocketContext,dwErr ) )
				{
					continue;
				}

				continue;  
			}  
			else  
			{
				// ��ȡ����Ĳ���
				IoContent* pIoContext = CONTAINING_RECORD(pOverlapped, IoContent, overlapped);  

				// �ж��Ƿ��пͻ��˶Ͽ���
				if((0 == dwBytesTransfered) && ( RECV_POSTED==pIoContext->optype || SEND_POSTED==pIoContext->optype))  
				{  
			//		pool->_ShowMessage( _T("�ͻ��� %s:%d �Ͽ�����."),inet_ntoa(pSocketContext->m_ClientAddr.sin_addr), ntohs(pSocketContext->m_ClientAddr.sin_port) );

					// �ͷŵ���Ӧ����Դ
					if (pool->closedelegate)
							pool->closedelegate->handleClose(pSocketContext);
					pool->_RemoveContext( pSocketContext );
			//		printf("do error\n");
 					continue;  
				}  
				else
				{
					if (!pSocketContext->checkValid())
					{
						pSocketContext->getHandle().close();
						if (pool->closedelegate)
							pool->closedelegate->handleClose(pSocketContext);
						pool->_RemoveContext( pSocketContext );
						continue;
					}
					switch( pIoContext->optype )  
					{  
						 // Accept  
						case ACCEPT_POSTED:
						{ 
							// Ϊ�����Ӵ���ɶ��ԣ�������ר�ŵ�_DoAccept�������д�����������
			//				printf("do post\n");
							pool->_DoAccpet( pSocketContext, pIoContext );						
						}
						break;

						// RECV
						case RECV_POSTED:
						{
							// Ϊ�����Ӵ���ɶ��ԣ�������ר�ŵ�_DoRecv�������д����������
			//				printf("do recv\n");
							pool->_DoRecv( pSocketContext,pIoContext ,dwBytesTransfered);
						}
						break;

						// SEND
						// �����Թ���д�ˣ�Ҫ������̫���ˣ���������⣬Send�������������һЩ
						case SEND_POSTED:
						{
				//			printf("do send\n");
							pool->_DoSend(pSocketContext,pIoContext,dwBytesTransfered);
						}
						break;
						default:
							break;
					} //switch

					
				}
			}
		}
		printf("work thread over\n");
	}
	IocpWorkThread(IocpPool *pool):pool(pool)
	{
	}
private:
	IocpPool *pool;
};

class CheckValidThread:public sys::Thread{
public:
	void run()
	{
		while(isActive())
		{
			sys::sTime::wait(300); // �ȴ�1s
			if (pool) pool->checkValidTick();
		}
	}
	CheckValidThread(IocpPool *pool):pool(pool)
	{
	
	}
private:
	IocpPool *pool;
};

IocpPool::IocpPool(void):
							m_hShutdownEvent(NULL),
							m_hIOCompletionPort(NULL),
							m_strIP(DEFAULT_IP),
							m_nPort(DEFAULT_PORT),
							m_lpfnAcceptEx( NULL ),
							m_pListenContext( NULL )
{
	check = NULL;
}


IocpPool::~IocpPool(void)
{
	// ȷ����Դ�����ͷ�
	this->Stop();
}
//====================================================================================
//
//				    ϵͳ��ʼ������ֹ
//
//====================================================================================
////////////////////////////////////////////////////////////////////
// ��ʼ��WinSock 2.2
bool IocpPool::LoadSocketLib()
{    
	WSADATA wsaData;
	int nResult;
	nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	// ����(һ�㶼�����ܳ���)
	if (NO_ERROR != nResult)
	{
//		this->_ShowMessage(_T("��ʼ��WinSock 2.2ʧ�ܣ�\n"));
		return false; 
	}

	return true;
}

//////////////////////////////////////////////////////////////////
//	����������
bool IocpPool::Start()
{
	// ��ʼ���̻߳�����
	InitializeCriticalSection(&m_csContextList);

	// ����ϵͳ�˳����¼�֪ͨ
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// ��ʼ��IOCP
	if (false == _InitializeIOCP())
	{
		return false;
	}
	else
	{
	}

	// ��ʼ��Socket
	if( false==_InitializeListenSocket() )
	{
		this->_DeInitialize();
		return false;
	}
	else
	{
		
	}

	return true;
}


////////////////////////////////////////////////////////////////////
//	��ʼ����ϵͳ�˳���Ϣ���˳���ɶ˿ں��߳���Դ
void IocpPool::Stop()
{
	if( m_pListenContext!=NULL && m_pListenContext->getHandle().checkValid())
	{
		// ����ر���Ϣ֪ͨ
		SetEvent(m_hShutdownEvent);

//		for (int i = 0; i < m_nThreads; i++)
//		{
//			// ֪ͨ���е���ɶ˿ڲ����˳�
//			PostQueuedCompletionStatus(m_hIOCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
//		}

		// ����ͻ����б���Ϣ
		this->_ClearContextList();

		// �ͷ�������Դ
		this->_DeInitialize();

	}	
}


////////////////////////////////
// ��ʼ����ɶ˿�
bool IocpPool::_InitializeIOCP()
{
	// ������һ����ɶ˿�
	m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0 );

	if ( NULL == m_hIOCompletionPort)
	{
		//this->_ShowMessage(_T("������ɶ˿�ʧ�ܣ��������: %d!\n"), WSAGetLastError());
		return false;
	}

	// ���ݱ����еĴ�����������������Ӧ���߳���
int	m_nThreads = WORKER_THREADS_PER_PROCESSOR * _GetNoOfProcessors();
	
	// Ϊ�������̳߳�ʼ�����
	//m_phWorkerThreads = new HANDLE[m_nThreads];
	
	// ���ݼ�����������������������߳�
	//DWORD nThreadID;
	for (int i = 0; i < m_nThreads; i++)
	{
		IocpWorkThread * work = new IocpWorkThread(this);
		_works.push_back(work);
		work->start();
	}
	check = new CheckValidThread(this);
	check->start();
//	TRACE(" ���� _WorkerThread %d ��.\n", m_nThreads );

	return true;
}


/////////////////////////////////////////////////////////////////
// ��ʼ��Socket
bool IocpPool::_InitializeListenSocket()
{
	// AcceptEx �� GetAcceptExSockaddrs ��GUID�����ڵ�������ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;  
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 

	// ��������ַ��Ϣ�����ڰ�Socket
	struct sockaddr_in ServerAddress;

	// �������ڼ�����Socket����Ϣ
	m_pListenContext = new SocketContent;

	// ��Ҫʹ���ص�IO�������ʹ��WSASocket������Socket���ſ���֧���ص�IO����
//	m_pListenContext->socket = new Socket();
	m_pListenContext->setHandle(WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED));
	if (!m_pListenContext->getHandle().checkValid()) 
	{
	//	this->_ShowMessage("��ʼ��Socketʧ�ܣ��������: %d.\n", WSAGetLastError());
		return false;
	}
	else
	{
	//	TRACE("WSASocket() ���.\n");
	}

	// ��Listen Socket������ɶ˿���
	if( NULL== CreateIoCompletionPort( (HANDLE)m_pListenContext->getHandle().getHandle(), m_hIOCompletionPort,(DWORD)m_pListenContext, 0))  
	{  
	//	this->_ShowMessage("�� Listen Socket����ɶ˿�ʧ�ܣ��������: %d/n", WSAGetLastError());  
		m_pListenContext->getHandle().close();
		return false;
	}
	else
	{
	//	TRACE("Listen Socket����ɶ˿� ���.\n");
	}

	// ����ַ��Ϣ
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	// ������԰��κο��õ�IP��ַ�����߰�һ��ָ����IP��ַ 
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                      
	ServerAddress.sin_addr.s_addr = inet_addr(m_strIP.c_str());         
	ServerAddress.sin_port = htons(m_nPort);                          

	// �󶨵�ַ�Ͷ˿�
	if (SOCKET_ERROR == bind(m_pListenContext->getHandle().getHandle(), (struct sockaddr *) &ServerAddress, sizeof(ServerAddress))) 
	{
//		this->_ShowMessage("bind()����ִ�д���.\n");
		return false;
	}
	else
	{
//		TRACE("bind() ���.\n");
	}

	// ��ʼ���м���
	if (SOCKET_ERROR == listen(m_pListenContext->getHandle().getHandle(),SOMAXCONN))
	{
//		this->_ShowMessage("Listen()����ִ�г��ִ���.\n");
		return false;
	}
	else
	{
	//	TRACE("Listen() ���.\n");
	}

	// ʹ��AcceptEx��������Ϊ���������WinSock2�淶֮���΢�������ṩ����չ����
	// ������Ҫ�����ȡһ�º�����ָ�룬
	// ��ȡAcceptEx����ָ��
	DWORD dwBytes = 0;  
	if(SOCKET_ERROR == WSAIoctl(
		m_pListenContext->getHandle().getHandle(), 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidAcceptEx, 
		sizeof(GuidAcceptEx), 
		&m_lpfnAcceptEx, 
		sizeof(m_lpfnAcceptEx), 
		&dwBytes, 
		NULL, 
		NULL))  
	{  
	//	this->_ShowMessage("WSAIoctl δ�ܻ�ȡAcceptEx����ָ�롣�������: %d\n", WSAGetLastError()); 
		this->_DeInitialize();
		return false;  
	}  

	// ��ȡGetAcceptExSockAddrs����ָ�룬Ҳ��ͬ��
	if(SOCKET_ERROR == WSAIoctl(
		m_pListenContext->getHandle().getHandle(), 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs), 
		&m_lpfnGetAcceptExSockAddrs, 
		sizeof(m_lpfnGetAcceptExSockAddrs),   
		&dwBytes, 
		NULL, 
		NULL))  
	{  
	//	this->_ShowMessage("WSAIoctl δ�ܻ�ȡGuidGetAcceptExSockAddrs����ָ�롣�������: %d\n", WSAGetLastError());  
		this->_DeInitialize();
		return false; 
	}  


	// ΪAcceptEx ׼��������Ȼ��Ͷ��AcceptEx I/O����
	for( int i=0;i<MAX_POST_ACCEPT;i++ )
	{
		// �½�һ��IO_CONTEXT
		IoContent* pAcceptIoContext = m_pListenContext->GetNewIoContext();
		
		if( false==this->_PostAccept( pAcceptIoContext ) )
		{
			m_pListenContext->RemoveContext(pAcceptIoContext);
			return false;
		}
	}

	//this->_ShowMessage( _T("Ͷ�� %d ��AcceptEx�������"),MAX_POST_ACCEPT );

	return true;
}

////////////////////////////////////////////////////////////
//	����ͷŵ�������Դ
void IocpPool::_DeInitialize()
{
	// ɾ���ͻ����б�Ļ�����
	DeleteCriticalSection(&m_csContextList);

	// �ر�ϵͳ�˳��¼����
	RELEASE_HANDLE(m_hShutdownEvent);

	// �ͷŹ������߳̾��ָ��
	for(unsigned int i=0;i<_works.size();i++ )
	{
		if (_works[i])
			_works[i]->stop();
	}
	if (check)
	{
		check->stop();
	}
	//RELEASE(m_phWorkerThreads);

	// �ر�IOCP���
	RELEASE_HANDLE(m_hIOCompletionPort);

	// �رռ���Socket
	RELEASE(m_pListenContext);

//	this->_ShowMessage("�ͷ���Դ���.\n");
}


//====================================================================================
//
//				    Ͷ����ɶ˿�����
//
//====================================================================================


//////////////////////////////////////////////////////////////////
// Ͷ��Accept����
bool IocpPool::_PostAccept( IoContent* pAcceptIoContext )
{
	//ASSERT( INVALID_SOCKET!=m_pListenContext->m_Socket );

	// ׼������
	DWORD dwBytes = 0;  
	pAcceptIoContext->optype = ACCEPT_POSTED;  
	WSABUF *p_wbuf   = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pAcceptIoContext->overlapped;
	
	// Ϊ�Ժ�������Ŀͻ�����׼����Socket( ������봫ͳaccept�������� ) 
	SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);  
	if( INVALID_SOCKET ==  socket)  
	{  
	//	_ShowMessage("��������Accept��Socketʧ�ܣ��������: %d", WSAGetLastError()); 
		return false;  
	} 
	pAcceptIoContext->sockfd = socket;
	// Ͷ��AcceptEx
	if(FALSE == m_lpfnAcceptEx( m_pListenContext->getHandle().getHandle(),
						socket,
						p_wbuf->buf,0,// p_wbuf->len - ((sizeof(SOCKADDR_IN)+16)*2),   
								sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, p_ol)) 
	{  
		char buffer[1024]={'\0'};
		sprintf(buffer,"Ͷ�� AcceptEx ����ʧ�ܣ��������: %d\n", WSAGetLastError());
		if(WSA_IO_PENDING != WSAGetLastError())  
		{  
	//		_ShowMessage("Ͷ�� AcceptEx ����ʧ�ܣ��������: %d", WSAGetLastError());  
			return false;  
		}  
	} 

	return true;
}

////////////////////////////////////////////////////////////
// ���пͻ��������ʱ�򣬽��д���
// �����е㸴�ӣ���Ҫ�ǿ������Ļ����Ϳ����׵��ĵ���....
// ������������Ļ�����ɶ˿ڵĻ������������һ�����

// ��֮��Ҫ֪�����������ListenSocket��Context��������Ҫ����һ�ݳ������������Socket��
// ԭ����Context����Ҫ���������Ͷ����һ��Accept����
//
bool IocpPool::_DoAccpet( SocketContent* pSocketContext, IoContent* pIoContext )
{
	//if (m_arrayClientContext.size() >= 4000) return false;
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;  
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);  

	///////////////////////////////////////////////////////////////////////////
	// 1. ����ȡ������ͻ��˵ĵ�ַ��Ϣ
	// ��� m_lpfnGetAcceptExSockAddrs �����˰�~~~~~~
	// ��������ȡ�ÿͻ��˺ͱ��ض˵ĵ�ַ��Ϣ������˳��ȡ���ͻ��˷����ĵ�һ�����ݣ���ǿ����...
	this->m_lpfnGetAcceptExSockAddrs(pIoContext->m_wsaBuf.buf,0,// pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN)+16)*2), 
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);  

	printf(("�ͻ��� %s:%d ����.\n"), inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port) );
//	this->_ShowMessage( _T("�ͻ��� %s:%d ��Ϣ��%s."),inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port),pIoContext->m_wsaBuf.buf );
	
	pSocketContext->ip =  inet_ntoa(ClientAddr->sin_addr);
	pSocketContext->port =  ntohs(ClientAddr->sin_port);
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. ������Ҫע�⣬���ﴫ��������ListenSocket�ϵ�Context�����Context���ǻ���Ҫ���ڼ�����һ������
	// �����һ���Ҫ��ListenSocket�ϵ�Context���Ƴ���һ��Ϊ�������Socket�½�һ��SocketContext
#if (0)
	SocketContent* pNewSocketContext = new SocketContent;
#else
	Ref<SocketContent> ref = Ref<SocketContent>::get();
	SocketContent* pNewSocketContext = ref.pointer();
	pNewSocketContext->ref = ref.getRef();
#endif
	pNewSocketContext->setHandle(pIoContext->sockfd);
	memcpy(&(pNewSocketContext->m_ClientAddr), ClientAddr, sizeof(SOCKADDR_IN));

	// ����������ϣ������Socket����ɶ˿ڰ�(��Ҳ��һ���ؼ�����)
	if( false==this->_AssociateWithIOCP( pNewSocketContext ) )
	{
		RELEASE( pNewSocketContext );
		return false;
	}  


	///////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. �������������µ�IoContext�����������Socket��Ͷ�ݵ�һ��Recv��������
	{
		IoContent* pNewIoContext = &(pNewSocketContext->recvIo);//GetNewIoContext();
		pNewIoContext->optype       = RECV_POSTED;
		pNewIoContext->socket = pNewSocketContext;
	//	pNewIoContext->socket->getHandle().setHandle(pNewSocketContext->socket->getHandle().getHandle());
		// ���Buffer��Ҫ���������Լ�����һ�ݳ���
		//memcpy( pNewIoContext->m_szBuffer,pIoContext->m_szBuffer,MAX_BUFFER_LEN );

		// �����֮�󣬾Ϳ��Կ�ʼ�����Socket��Ͷ�����������
		if( false==this->_PostRecv( pNewIoContext) )
		{
			pNewSocketContext->RemoveContext( pNewIoContext );
		//	RELEASE( pNewSocketContext );
			return false;
		}
	}
	{
		// 3.1 Ͷ��һ��SEND ��������ssssssssssssssssssss
		IoContent* pNewIoContext = &(pNewSocketContext->sendIo);// pNewSocketContext->GetNewIoContext();
		pNewIoContext->optype       = SEND_POSTED;
		pNewIoContext->socket = pNewSocketContext;
		//pNewIoContext->socket->getHandle().setHandle(pNewSocketContext->socket->getHandle().getHandle());
		// ���Buffer��Ҫ���������Լ�����һ�ݳ���
		//memcpy( pNewIoContext->m_szBuffer,pIoContext->m_szBuffer,MAX_BUFFER_LEN );

		// �����֮�󣬾Ϳ��Կ�ʼ�����Socket��Ͷ�����������
		if( false==this->_PostSend( pNewIoContext) )
		{
			pNewSocketContext->RemoveContext( pNewIoContext );
//			RELEASE( pNewSocketContext );
			//return false;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// 4. ���Ͷ�ݳɹ�����ô�Ͱ������Ч�Ŀͻ�����Ϣ�����뵽ContextList��ȥ(��Ҫͳһ���������ͷ���Դ)
	this->_AddToContextList( pNewSocketContext );
	if (this->closedelegate)
		this->closedelegate->handleAdd(pNewSocketContext);
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 5. ʹ�����֮�󣬰�Listen Socket���Ǹ�IoContext���ã�Ȼ��׼��Ͷ���µ�AcceptEx
	pIoContext->ResetBuffer();
	return this->_PostAccept( pIoContext ); 	
}

////////////////////////////////////////////////////////////////////
// Ͷ�ݽ�����������
bool IocpPool::_PostRecv( IoContent* pIoContext )
{
	// ��ʼ������
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf   = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->overlapped;

	pIoContext->ResetBuffer();
	pIoContext->optype = RECV_POSTED;
	if (!pIoContext->socket) return false;
	// ��ʼ����ɺ󣬣�Ͷ��WSARecv����
	int nBytesRecv = WSARecv( pIoContext->socket->getHandle().getHandle(), p_wbuf, 1, &pIoContext->msgLen, &dwFlags, p_ol, NULL );

	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		//this->_ShowMessage("Ͷ�ݵ�һ��WSARecvʧ�ܣ�");
		return false;
	}
	return true;
}
////////////////////////////////////////////////////////////////////
// Ͷ�ݷ�����������
bool IocpPool::_PostSend( IoContent* pIoContext )
{
	// ��ʼ������
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf   = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->overlapped;

	pIoContext->ResetBuffer();
	pIoContext->optype = SEND_POSTED;
	p_wbuf->buf = pIoContext->m_szBuffer;
	if (!pIoContext->socket) return false;
	int len = pIoContext->socket->copySendBuffer(pIoContext->m_szBuffer,MAX_BUFFER_LEN);
	p_wbuf->len = len; 
	if (!len) return true;
	// ��ʼ����ɺ󣬣�Ͷ��WSARecv����
	int nBytesRecv = WSASend( pIoContext->socket->getHandle().getHandle(), p_wbuf, 1, &dwBytes, dwFlags, p_ol, NULL );

	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
	//	this->_ShowMessage("Ͷ�ݵ�һ��WSARecvʧ�ܣ�");
		return false;
	}
	return true;
}

void SocketContent::wakeupSend()
{
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf   = &sendIo.m_wsaBuf;
	OVERLAPPED *p_ol = &sendIo.overlapped;

	sendIo.ResetBuffer();
	sendIo.optype = SEND_POSTED;
	p_wbuf->buf = sendIo.m_szBuffer;
	if (!sendIo.socket) return;
	stTestCmd testCmd;
	sendIo.socket->sendObject(&testCmd);
	p_wbuf->len = sendIo.socket->copySendBuffer(sendIo.m_szBuffer,MAX_BUFFER_LEN);
	// ��ʼ����ɺ󣬣�Ͷ��WSARecv����
	if (!p_wbuf->len) return;
	int nBytesRecv = WSASend( sendIo.socket->getHandle().getHandle(), p_wbuf, 1, &dwBytes, dwFlags, p_ol, NULL );

	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
	//	this->_ShowMessage("Ͷ�ݵ�һ��WSARecvʧ�ܣ�");
		return ;
	}
	return ;
}
/////////////////////////////////////////////////////////////////
// ���н��յ����ݵ����ʱ�򣬽��д���
bool IocpPool::_DoRecv( SocketContent* pSocketContext, IoContent* pIoContext ,DWORD len)
{
	// �Ȱ���һ�ε�������ʾ���֣�Ȼ�������״̬��������һ��Recv����
	SOCKADDR_IN* ClientAddr = &pSocketContext->m_ClientAddr;
	//printf("�յ�  %s:%d ��Ϣ��%s\n",inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port),pIoContext->m_wsaBuf.buf );
	pIoContext->socket->done(delegate,pIoContext->m_wsaBuf.buf,len);	
	// Ȼ��ʼͶ����һ��WSARecv����
	return _PostRecv( pIoContext );
}
/////////////////////////////////////////////////////////////////
// ���з��͵����ݵ����ʱ�򣬽��д���
bool IocpPool::_DoSend( SocketContent* pSocketContext, IoContent* pIoContext ,DWORD len)
{
	// �Ȱ���һ�ε�������ʾ���֣�Ȼ�������״̬��������һ��Recv����
	SOCKADDR_IN* ClientAddr = &pSocketContext->m_ClientAddr;
	//this->_ShowMessage( _T("�յ�  %s:%d ��Ϣ��%s"),inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port),pIoContext->m_wsaBuf.buf );

	// Ȼ��ʼͶ����һ��WSARecv����
	return _PostSend( pIoContext );
}



/////////////////////////////////////////////////////
// �����(Socket)�󶨵���ɶ˿���
bool IocpPool::_AssociateWithIOCP( SocketContent *pContext )
{
	// �����ںͿͻ���ͨ�ŵ�SOCKET�󶨵���ɶ˿���
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pContext->getHandle().getHandle(), m_hIOCompletionPort, (DWORD)pContext, 0);

	if (NULL == hTemp)
	{
		//this->_ShowMessage(("ִ��CreateIoCompletionPort()���ִ���.������룺%d"),GetLastError());
		return false;
	}

	return true;
}




//====================================================================================
//
//				    ContextList ��ز���
//
//====================================================================================


//////////////////////////////////////////////////////////////
// ���ͻ��˵������Ϣ�洢��������
void IocpPool::_AddToContextList( SocketContent *pHandleData )
{
	EnterCriticalSection(&m_csContextList);

	m_arrayClientContext.push_back(pHandleData);	
	
	LeaveCriticalSection(&m_csContextList);
}

/////////////////////////////////////////////////////////Z///////
//	�Ƴ�ĳ���ض���Context
void IocpPool::_RemoveContext( SocketContent *pSocketContext )
{
	EnterCriticalSection(&m_csContextList);

	for( CLIENT_CONTEXT_ITER iter = m_arrayClientContext.begin();
			iter !=  m_arrayClientContext.end();++iter )
	{
		if( pSocketContext== *iter )
		{
			printf(".....................................��ǰɾ��������:%s %u\n",pSocketContext->getIp().c_str(),pSocketContext->port);
			pSocketContext->getHandle().close(); // �ٴ�������Ч
			RELEASE( pSocketContext );			
			m_arrayClientContext.erase(iter);
			break;
		}
	}
	printf(".....................................��ǰ����ʣ����:%lu\n",m_arrayClientContext.size());
	LeaveCriticalSection(&m_csContextList);
}

////////////////////////////////////////////////////////////////
// ��տͻ�����Ϣ
void IocpPool::_ClearContextList()
{
	EnterCriticalSection(&m_csContextList);

	for( CLIENT_CONTEXT_ITER iter = m_arrayClientContext.begin();
			iter !=  m_arrayClientContext.end();++iter )
	{
		if(*iter )
		{
			delete *iter;		
		}
	}

	m_arrayClientContext.clear();

	LeaveCriticalSection(&m_csContextList);
}

// ִ�ж�ʱ�����Ϊ

void IocpPool::checkValidTick()
{
	if (!m_hIOCompletionPort) return;
	EnterCriticalSection(&m_csContextList);
	int  count = 0;
	for( CLIENT_CONTEXT_ITER iter = m_arrayClientContext.begin();
			iter !=  m_arrayClientContext.end();++iter )
	{
		if(*iter )
		{
			(*iter)->setInvalidTimeOut();
			if (!(*iter)->checkValid())
				count ++;
		}
	}
//	if (count)
//		printf("-------------------------------��ǰ����������Ч�ĸ���Ϊ:%u\n",count);
	LeaveCriticalSection(&m_csContextList);
}
//====================================================================================
//
//				       ����������������
//
//====================================================================================



////////////////////////////////////////////////////////////////////
// ��ñ�����IP��ַ
std::string IocpPool::GetLocalIP()
{
	// ��ñ���������
	char hostname[MAX_PATH] = {0};
	gethostname(hostname,MAX_PATH);                
	struct hostent FAR* lpHostEnt = gethostbyname(hostname);
	if(lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}

	// ȡ��IP��ַ�б��еĵ�һ��Ϊ���ص�IP(��Ϊһ̨�������ܻ�󶨶��IP)
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];      

	// ��IP��ַת�����ַ�����ʽ
	struct in_addr inAddr;
	memmove(&inAddr,lpAddr,4);
	m_strIP = std::string( inet_ntoa(inAddr) );        

	return m_strIP;
}

///////////////////////////////////////////////////////////////////
// ��ñ����д�����������
int IocpPool::_GetNoOfProcessors()
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}

/////////////////////////////////////////////////////////////////////
// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ���������һ����Ч��Socket��Ͷ��WSARecv����������쳣
// ʹ�õķ����ǳ��������socket�������ݣ��ж����socket���õķ���ֵ
// ��Ϊ����ͻ��������쳣�Ͽ�(����ͻ��˱������߰ε����ߵ�)��ʱ�򣬷����������޷��յ��ͻ��˶Ͽ���֪ͨ��

bool IocpPool::_IsSocketAlive(SOCKET s)
{
	int nByteSent=send(s,"",0,0);
	if (-1 == nByteSent) return false;
	return true;
}

///////////////////////////////////////////////////////////////////
// ��ʾ��������ɶ˿��ϵĴ���
bool IocpPool::HandleError( SocketContent *pContext,const DWORD& dwErr )
{
	// ����ǳ�ʱ�ˣ����ټ����Ȱ�  
	if(WAIT_TIMEOUT == dwErr)  
	{  	
		// ȷ�Ͽͻ����Ƿ񻹻���...
		if( !( pContext->getHandle().checkValid()) )
		{
			if (closedelegate)
				closedelegate->handleClose(pContext);
	//		this->_ShowMessage( _T("��⵽�ͻ����쳣�˳���") );
			this->_RemoveContext( pContext );
			return true;
		}
		else
		{
	//		this->_ShowMessage( _T("���������ʱ��������...") );
			return true;
		}
	}  

	// �����ǿͻ����쳣�˳���
	else if( ERROR_NETNAME_DELETED==dwErr )
	{
		if (closedelegate)
				closedelegate->handleClose(pContext);
	//	this->_ShowMessage( _T("��⵽�ͻ����쳣�˳���") );
		this->_RemoveContext( pContext );
		return true;
	}

	else
	{
	//	this->_ShowMessage( _T("��ɶ˿ڲ������ִ����߳��˳���������룺%d"),dwErr );
		return false;
	}
}

bool IocpPool::addSocket(Socket *socket)
{
	SocketContent* pNewSocketContext = new SocketContent();// socket;
	pNewSocketContext->setHandle(socket->getHandle().getHandle());

	// ����������ϣ������Socket����ɶ˿ڰ�(��Ҳ��һ���ؼ�����)
	if( false==this->_AssociateWithIOCP( pNewSocketContext ) )
	{
		RELEASE( pNewSocketContext );
		return false;
	}  


	///////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. �������������µ�IoContext�����������Socket��Ͷ�ݵ�һ��Recv��������
	{
		IoContent* pNewIoContext = &pNewSocketContext->recvIo;//GetNewIoContext();
		pNewIoContext->optype       = RECV_POSTED;
		pNewIoContext->socket = pNewSocketContext;
		// ���Buffer��Ҫ���������Լ�����һ�ݳ���
		//memcpy( pNewIoContext->m_szBuffer,pIoContext->m_szBuffer,MAX_BUFFER_LEN );

		// �����֮�󣬾Ϳ��Կ�ʼ�����Socket��Ͷ�����������
		if( false==this->_PostRecv( pNewIoContext) )
		{
			pNewSocketContext->RemoveContext( pNewIoContext );
			return false;
		}
	}
	{
		// 3.1 Ͷ��һ��SEND ��������
		IoContent* pNewIoContext = &pNewSocketContext->sendIo;//GetNewIoContext();
		pNewIoContext->optype       = SEND_POSTED;
		pNewIoContext->socket = pNewSocketContext;
		// ���Buffer��Ҫ���������Լ�����һ�ݳ���
		//memcpy( pNewIoContext->m_szBuffer,pIoContext->m_szBuffer,MAX_BUFFER_LEN );

		// �����֮�󣬾Ϳ��Կ�ʼ�����Socket��Ͷ�����������
		if( false==this->_PostSend( pNewIoContext) )
		{
			pNewSocketContext->RemoveContext( pNewIoContext );
			return false;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// 4. ���Ͷ�ݳɹ�����ô�Ͱ������Ч�Ŀͻ�����Ϣ�����뵽ContextList��ȥ(��Ҫͳһ���������ͷ���Դ)
	this->_AddToContextList( pNewSocketContext );
	
	return true;
}


#endif