#include "selectpool.h"
class SelectWorkThread:public sys::Thread{
public:
	/**
	 * �����߳�
	 */
	void run()
	{
		while(isActive())
		{
			if (pool)
			{
				pool->check();
				sys::sTime::wait(100);
				pool->action();
			}
		}
	}
	/**
	 * �̳߳�
	 */
	SelectWorkThread(ConnectionSelectPool *pool)
	{
		this->pool = pool;
	}
	ConnectionSelectPool *pool;
};
void ConnectionSelectPool::start(WORD port)
{
	if (selectWorkThread) return;
	socket.setHandle(::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP));
	socket.bindAndListen(port);
	this->handle = socket.getHandle().getHandle();
	//socket.getHandle().listen(port);
	selectWorkThread = new SelectWorkThread(this);
	selectWorkThread->start();
}
void ConnectionSelectPool::start()
{
	if (selectWorkThread) return;
	selectWorkThread = new SelectWorkThread(this);
	selectWorkThread->start();
}

void ConnectionSelectPool::action()
{
	// use select to run at server system
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&errfds);
	int FD_SET_SIZE = 0;
	for (std::list<Socket*>::iterator iter = tasks.begin(); iter != tasks.end();++iter)
	{
		Socket * task = (Socket*)*iter;
		if (task->checkValid())
		{
			int fd = task->getHandle().getHandle();
			if (-1 == fd){
				task->setInvalid();
                printf("����ɾ��״̬ taskhandle == -1\n");
				continue;
			}
			if (fd > FD_SET_SIZE) FD_SET_SIZE = fd;
			FD_SET(fd,&rfds);
			FD_SET(fd,&wfds);
			FD_SET(fd,&errfds);
		}
	}
	struct timeval tv;
	tv.tv_usec = 500;
	tv.tv_sec = 0;
	if (FD_SETSIZE)
	{
		int ret = select(FD_SET_SIZE + 1,&rfds,&wfds,&errfds,&tv);
		if (0 <= ret)
		{
			for (std::list<Socket*>::iterator iter = tasks.begin(); iter != tasks.end();++iter)
			{
				Socket * task =(Socket*) *iter;
				if (task->checkValid())
				{
					int fd = task->getHandle().getHandle();
					if (fd == -1)
					{
						printf("error happened\n");
						task->setInvalid();
						continue;
					}
					if (FD_ISSET(fd,&errfds))
					{
						task->setInvalid();
                        printf("����ɾ��״̬.select ������error\n");
						FD_CLR(fd,&errfds);
						continue;
					}
					if (FD_ISSET(fd,&rfds))
					{
						task->done(delegate);
						FD_CLR(fd, &rfds);
					}
					if (FD_ISSET(fd,&wfds))
					{
						task->send(100);
						FD_CLR(fd, &wfds);
					}
				}
			}
		}
	}
}

/**
 * ����socket
 * \param socket ����
 */
bool ConnectionSelectPool::addSocket(Socket *socket)
{
	if (!socket) return  false;
	_mutex.lock();
	for (std::list<Socket*>::iterator iter = exttasks.begin(); iter != exttasks.end();++iter)
	{
		if (*iter == socket)
		{
			_mutex.unlock();
			return false;
		}
	}
	exttasks.push_back(socket);
	_mutex.unlock();
	return true;
}
bool ConnectionSelectPool::innerAddSocket(Socket *socket)
{
	if (!socket) return  false;
	for (std::list<Socket*>::iterator iter = tasks.begin(); iter != tasks.end();++iter)
	{
		if (*iter == socket) return false;
	}
	if (tasks.size() >= MAX_SELECT_COUNT)
		return false;
	tasks.push_back(socket);
	return true;
}
bool ConnectionSelectPool::checkAddSocket()
{
	_mutex.lock();
	for (std::list<Socket*>::iterator iter = exttasks.begin(); iter != exttasks.end();++iter)
	{
		if (*iter)
		{
			for (std::list<Socket*>::iterator pos = tasks.begin(); pos != tasks.end();++pos)
			{
				if (*iter == *pos) continue;
			}
			if (tasks.size() >= MAX_SELECT_COUNT)
			{
				exttasks.clear();
				_mutex.unlock();
				return false;
			}
			tasks.push_back(*iter);
			
		}
	}
	exttasks.clear();
	_mutex.unlock();
	return true;
}

/**
 * ����socket
 */
void ConnectionSelectPool::doMakeSocket()
{
	fd_set rfds;
	FD_ZERO(&rfds);
	int FD_SET_SIZE = 0;
	int serverFd = handle;
	if (serverFd > FD_SET_SIZE) FD_SET_SIZE = serverFd;
	FD_SET(serverFd,&rfds);
	struct timeval tv;
	tv.tv_usec = 500;
	tv.tv_sec = 0;
	if (FD_SETSIZE)
	{
		int ret = select(FD_SET_SIZE + 1,&rfds,&wfds,&errfds,&tv);
		if (0 <= ret)
		{
			if (-1 != serverFd && FD_ISSET(serverFd,&rfds))
			{
				Socket *vtask= Socket::makeSocket(handle);
				if (vtask)
				{
					if (innerAddSocket(vtask))
					{
						printf("select:��ȡ��һ�����ӣ���ǰ��������:%u\n",tasks.size());
					}
					else delete vtask;
				}
				FD_CLR(serverFd, &rfds);
			}
		}
	}			
}
void ConnectionSelectPool::check()
{
	this->checkAddSocket();
	for (std::list<Socket*>::iterator iter = tasks.begin(); iter != tasks.end();)
	{
		Socket * task = (Socket*)*iter;
		if (!task)
		{
			iter = tasks.erase(iter);
			continue;
		}	
		task->setInvalidTimeOut();
		if (!task->checkValid())
		{
			task->BeReleased();
			iter = tasks.erase(iter);
			if (closedelegate)
				closedelegate->handleClose(task);
			if (task->type == Socket::NODE_CLIENT)
       		{
				// �����client �ڵ㲻ɾ��ָ��
       		}
    		else
			{
				delete task;
			}
			printf("select pool ɾ��task ��ǰ��ʣ��:%u\n",tasks.size());
		}
		else
		{
			iter++;
		}
	}
}

void ConnectionSelectPool::clear()
{
	for (std::list<Socket*>::iterator iter = tasks.begin(); iter != tasks.end();++iter)
	{
		Socket * task = *iter;
		if (task && !task->checkValid())
		{
			task->BeReleased();
			if (closedelegate)
				closedelegate->handleClose(task);
		}
	}
}
