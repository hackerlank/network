/**
 * һ��������ڴ������
 * ��ʵ��ʹ���� ������ ������˼�ϵ�ָ��
 * ϵͳ�в������ʹ�ò�����ָ������� ��ָ��Ƿ���ʱ��.���ؿ�
 */
#pragma once
#include "sys.h"
#include "quickidmanager.h"
struct stRefMemory:public QuickObjectBase{
	void *pointer;
	int refQuickId;
	DYNAMIC_API stRefMemory()
	{
		refQuickId = -1;
	}
	/**
	 * ���ÿ���Ψһ����
	 * \param uniqueQuickId ���ÿ���Ψһ����
	 */
	DYNAMIC_API virtual void setUniqueQuickId(DWORD uniqueQuickId);
	/**
	 * ��ȡ����Ψһ����
	 * \return ����Ψһ����
	 */
	DYNAMIC_API virtual DWORD getUniqueQuickId() ;
};
