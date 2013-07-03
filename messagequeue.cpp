#include "messagequeue.h"
/*
 * ����һһ����Ϣ��
 **/
void MQ::add(const void *cmd,int len)
{
	_mutex.lock();
	std::vector<char> t;
	t.resize(len);
	bcopy(cmd,&t[0],len);
	_ts.push(t);
	_mutex.unlock();
}
/**
 * ����ȡ��Ϣ
 */
void MQ::pick()
{
	_mutex.lock();
	while(!_ts.empty())
	{
		_temp_ts.push(_ts.front());
		_ts.pop();
	}
	_mutex.unlock();
	while(!_temp_ts.empty())
	{
		std::vector<char> t = _temp_ts.front();
		done(&t[0],t.size());
		_temp_ts.pop();
	}
}