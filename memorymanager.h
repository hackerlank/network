#pragma once
#include "quickidmanager.h"
#include "memorynode.h"
/**
 * �ڴ������
 * ���ݵ�ǰ�ڴ���С ���ټ�����������ڴ�,�ڴ��С��2^n�η� ���֧��32 �η��ļ���
 * ���ٷ���
 */
class MemoryManager{
public:
	/**
	 * ��useLocal Ϊ����ʱ,�ڴ����ֻΪһ���������
	 * \param useLocal ���ز���
	 */
	DYNAMIC_API MemoryManager(bool useLocal);
	DYNAMIC_API MemoryManager();
	DYNAMIC_API static MemoryManager & getMe()
	{
		static MemoryManager me(false);
		return me;
	}
	/**
	 * �������ǰ�ڴ�
	 * \param size ��С
	 * \return �ڴ�������Ϣ
	 */
	DYNAMIC_API MemoryNode* alloc(unsigned int size);
	
	/**
	 * �ͷŵ�ǰ�ڴ�����
	 * \param memory �ڴ�����
	 */
	DYNAMIC_API void dealloc(stRefMemory& memory);
	/**
	 * �ڴ�ָ��
	 * \param node ������ڴ��
	 **/
	DYNAMIC_API void dealloc(MemoryNode* node);
	/**
	 * �������ò���ָ���Ŀ�
	 * \param memory ����
	 * \return �ڴ��
	 */
	DYNAMIC_API MemoryNode *findNode(stRefMemory& memory);

	DYNAMIC_API unsigned int blockSize()
	{
		return quicknodes.getSize();
	}
private:
	QuickIdManager quicknodes;
	std::vector<MemoryNode*> nodes;
	/**
	 * ����һЩ�ڴ�
	 * \param size ��Ĵ�С
	 * \return ����׸�ָ��
	 */
	MemoryNode *allocNodes(unsigned int size);
	/**
	 * ���ݴ�С��ȡ������
	 * \param size ���С
	 * \return ������
	 */
	unsigned int getIndexBySize(unsigned int size);

	bool useLocal; // �Ƿ����ɵ�һ���ڴ����
	unsigned int nodeIndex;
};

#define theMemoryManager MemoryManager::getMe()