/**
 * free bsd ��socket ������
 */
#pragma once
#include "sys.h"
#include "socket.h"
#include "singleton.h"
#include "net.h"
/**
 * ����һ���߳���������socket �Ĺ���
 */
class ActionQueueThread;
class KqueuePool: public ConnectionPool,public Singleton<KqueuePool>{
public:
	/**
	* ��ʼ��������
	* \param handle ��� 
	*/
	DYNAMIC_API virtual void start(WORD port);
	/**
 	 * ����socket,���뵽�������߳���
 	 * */
	DYNAMIC_API bool addSocket(Socket *socket);
	/**
	 * ɾ��socket
	 * \param socket ��ɾ����socket
 	 */
	DYNAMIC_API void delSocket(Socket *socket);
	DYNAMIC_API KqueuePool()
	{
		action = NULL;
	}	
	/**
	 * ������Դ�ͷ�
	 */
	DYNAMIC_API void release();
public:
	int handle; // ���
	void handleClose(Socket *socket);
	void handleRecv(Socket *socket);
	void handleWrite(Socket *socket);
	/**
	 * ����n��socket
	 */
	void innerAddSocket(int size);
    Socket socket;
	ActionQueueThread * action;
	int kq;
	friend class ActionQueueThread;
};