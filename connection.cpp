#include "connection.h"

/**
 * ���Ͷ������Ϣ
 */
bool Connection::sendPackCmd(void * cmd,unsigned int size)
{
	stPackTag ptag;
	if (tag & ZLIB_TAG)
	{
		ptag.tag |= stPackTag::ZLIB_PACK;
	}
	if (tag & DES_TAG)
	{
		ptag.tag |= stPackTag::DES_PACK;
	}
	if (tag & HASH_TAG)
	{
		ptag.tag |= stPackTag::HASH_PACK;
	}
	if (tag & MAX_TAG)
	{
		ptag.tag |= stPackTag::MAX_PACK;
	}
	stPackContent pack;
	pack.encode(ptag,cmd,size);
	if (pack.hadChildPack())
	{
		sendBuffer.insert(sendBuffer.end(),pack.splitpacks.begin(),pack.splitpacks.end());
	}
	else
	{
		sendBuffer.push_back(pack);
	}
	startWrite(); // ����д��Ϣ
	return true;
}


/**
 * ���ô���ʽ
 */
void Connection::setMethod(int method)
{
	tag = method;
}

/**
 * �������
 */
void Connection::doRecv()
{
	// ���pack δ������ ��return
	// ����pack �������� ��ֱ�����ĵ����������
	char buffer[8026] = {'\0'};
	int size = socket.recv(buffer,8026);
	if (size == -1)
	{
		setInvalid();
		return ;
	}
	char *temp = (char*) buffer;
	int offset = 0;
	offset = nowPackContent.decode(temp,offset,size);
	while(offset >= size) // ��β����
	{
		if (nowPackContent.hadChildPack() && nowPackContent.isOk())
		{
			nowPackContent.reset();
		}
		else if (nowPackContent.isOk())
		{
			// todo ����nowpack
			
			nowPackContent.reset();
		}
		else if (nowPackContent.isBad())
		{
			nowPackContent.reset();
		}
		offset = nowPackContent.decode(temp,offset,size);
	}
	if (offset > size)
	{
		// ������
	}
}

/**
 * ����������������д����
 */
void Connection::flushSendBuffer()
{
	while (!sendBuffer.empty())
	{
		stPackContent &content = sendBuffer.front();
		void *cmd = content.getContentPointer();
		unsigned int cmdLen = content.getContentSize();
		int res = socket.send(cmd,cmdLen);
		if (res == -1)
		{
			setInvalid();
			return;
		}	
		content.contentStartPos += res;
		if (content.contentStartPos >= content.content.size())
		{
			sendBuffer.pop_front();
		}
	}
}

/**
 * ����������Ϊ��Ч
 */
void Connection::setInvalid()
{
	socket.shutdown();
}