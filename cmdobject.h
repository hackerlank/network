#pragma once
#include "libtag.h"
#include "cmd_serialize.h"
/**
 * ��Ϣ��,Я������ϢΨһ��ʾ
 */
class CmdObject:public cmd::Object{
public:
	unsigned int __msg__id__; // ��Ϣ��
	/**
	 * ��������
	 * \param cmd ����
	 * \param cmdLen ���ݳ���
	 */
	DYNAMIC_API virtual void parsecmd(void *cmd,int cmdLen);
	/**
	 * �����Ϣ�ṹ������
	 * ��Ϣ�ṹ�� �е��������ݴ��
	 */
	DYNAMIC_API virtual void clear();
	/**
	 * ����Ϣת��Ϊ�ַ���
	 */
	DYNAMIC_API cmd::Stream toStream();
	/**
	 * ʹ��Ψһid ��ʾ��Ϣ��
	 * \param id Ψһ
	 */
	DYNAMIC_API CmdObject(unsigned int id):__msg__id__(id){}
};