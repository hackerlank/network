#pragma once
class Socket;
class DealCmdDelegate{
public:
	/**
	 * ������Ϣ�Ĵ���
	 */
	DYNAMIC_API virtual void doPackCmd(Socket*socket,void *cmd,int len) = 0;
	DYNAMIC_API virtual void doObjectCmd(Socket*socket,void *cmd,int len) = 0;
};
/**
 * һ������ǿ��ɾ����
 */
class SocketCloseDelegate{
public:
	/**
	 *  ����socket �Ĺر�
	 * \param socket ����
	 **/
	DYNAMIC_API virtual void handleClose(Socket *socket) = 0; 
	/**
	 * ����socket ������
	 * \param socket ����
	 */
	DYNAMIC_API virtual void handleAdd(Socket *socket) = 0;
};