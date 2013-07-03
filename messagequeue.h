#pragma once
#include "sys.h"
#include "deque"
#include "queue"
/**
 * ����һ���ĸ�Ч��MQ
 * �޹���������
 * */
class MQ{
public:
	/*
	 * ����һһ����Ϣ��
	 **/
	void add(const void *cmd,int len);
	
	/**
	 * ����ȡ��Ϣ
	 */
	void pick();
	
protected:
	virtual void done(void *cmd,int len) = 0;

	std::queue<std::vector<char> > _ts;
	std::queue<std::vector<char> > _temp_ts;
	sys::Mutex _mutex;
};
