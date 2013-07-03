#pragma once
#include "sys.h"
#include "package.h"
#include "deque"
/**
 * ��ǰ��tcp ���� ���msg�Ľ��� �ͻ�������Ϣ���ܷ��� 
 * conn->sendObject(&object); // object ���Ժܴ� �ڲ�����Ϊ�ֿ� �ȴ�����
 */
class Connection{
public:
	DYNAMIC_API Connection()
	{
		tag = NULL_TAG;
	}
	/**
	 * �������л��Ķ���
	 */
	DYNAMIC_API bool sendObject();

	/**
	 * ���Ͷ������Ϣ
	 */
	DYNAMIC_API bool sendPackCmd(void * cmd,unsigned int size);
	
	int tag; // ���ͱ��ĵĴ���ʽ
	enum{
		NULL_TAG = 0, // ԭʼ����
		ZLIB_TAG = 1 << 0, // ѹ��
		DES_TAG = 1 << 1, // DES����
		HASH_TAG = 1 << 2, // hash У��
		MAX_TAG = 1 << 3, // ������ϴ���ķ��� ���趨�ñ�ʶ�Ļ� �����ݴ���MAX_PACK_SIZEʱ ���Զ��ָ�,Ч�ʻ����
	};
	/**
	 * ���ô���ʽ
	 */
	DYNAMIC_API void setMethod(int method);
	/**
	 * �������
	 */
	DYNAMIC_API void doRecv();

	/**
	 * ����д������ ��Ҫ������
	 */
	DYNAMIC_API void startWrite(){}

	/**
	 * ����ֹͣд��
	 */
	DYNAMIC_API void stopWrite();

	stPackContent nowPackContent; // ��ǰpack ������
	sys::Socket socket;// ϵͳsocket	

	std::deque<stPackContent> sendBuffer;
	/**
	 * ����������������д����
	 */
	DYNAMIC_API void flushSendBuffer();
	/**
	 * ����������Ϊ��Ч
	 */
	DYNAMIC_API void setInvalid();
	/**
	 * ���ò������
	 */
	DYNAMIC_API void setHandle(const sys::Socket& socket){}

};