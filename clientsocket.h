#pragma once
#include "connection.h"
/**
 * �ͻ���socket �������
 * ��ȡconnection
 */
class ClientSocket{
public:
	ClientSocket()
	{
		connection = NULL;
	}
	/**
	 * ���ӵ� ip.port �ķ�����
	 */
	DYNAMIC_API bool connect(const char *ip,unsigned short port);
	
	/**
	 * ��ȡ����
	 * \reeturn ����
	 */
	DYNAMIC_API Connection* getConnection();
	
	/**
	 * �Ͽ�����
	 */
	DYNAMIC_API void disconnect();
private:
	Connection* connection;
};