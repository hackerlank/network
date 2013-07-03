#pragma once
#include "net.h"
/**
 * ʹ��select �������ӳ�
 */
class SelectWorkThread;
class ConnectionSelectPool:public ConnectionPool,public Singleton<ConnectionSelectPool>
{
public:
	DYNAMIC_API ConnectionSelectPool()
	{
		selectWorkThread = NULL;
	}
	/**
	* ����select ����
	*/
	DYNAMIC_API virtual void start();
	/**
	 * ����start ����
	 * \param ��port �� �����������߳�
	 */
	DYNAMIC_API virtual void start(WORD port);
	/**
	 * ����socket �������ӹ�����
 	 */
	DYNAMIC_API virtual bool addSocket(Socket *socket);
private:
	bool checkAddSocket();
	bool innerAddSocket(Socket *socket);
	friend class SelectWorkThread;
	std::list<Socket* > tasks;
	std::list<Socket* > exttasks; // �߳��ⲿ�����socket
	sys::Mutex _mutex;
	fd_set rfds,wfds,errfds;
	int handle; // �������
	SelectWorkThread * selectWorkThread; // �����߳�
	Socket socket; // ������socket

	/**
	 * ִ��io 
 	 */
	virtual void action();
	
	/**
	 * ���Ӵ�����ܵĻ���
	 * ���뵱ǰ��� handle
	 */
	virtual void doMakeSocket();
	/**
	 * �����Ч����
	 */
	virtual void check();
	/**
	 * �������
	 */
	virtual void clear();
};