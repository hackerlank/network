#pragma once
#include "sys.h"
class ServerSocket{
public:
	/**
	 * ͨ���˿ڳ�ʼ��server socket
	 * \param port �˿�
	 */
	DYNAMIC_API bool init(unsigned short port);
	/**
	 * �ͷ���Դ
	 */
	DYNAMIC_API void release();
	/**
	 * ��ȡhandle
	 */
	DYNAMIC_API sys::Socket& getHandle();
private:
	sys::Socket socket;
};