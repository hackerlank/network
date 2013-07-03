#include "memorymanager.h"

/**
 * ����һЩ�ڴ�
 * \param size ��Ĵ�С
 * \return ����׸�ָ��
 */
MemoryNode *MemoryManager::allocNodes(unsigned int size)
{
	MemoryNode * header = new MemoryNode;
	for (int i = 0; i < 150; i++)
	{
		MemoryNode * node = new MemoryNode;
		node->content.resize(size);
		node->next = header;
		header = node;
	}
	return header;
}
/**
 * �������ǰ�ڴ�
 * \param size ��С
 * \return �ڴ�������Ϣ
 */
MemoryNode* MemoryManager::alloc(unsigned int size)
{
	MemoryNode * node = NULL;
	if (size == 0) return node;
	unsigned int index = 1;
	if (false == useLocal)
	{
		index = getIndexBySize(size);
	}
	// ��ǰ����Ϊindex
	if (index >= 1 && index <= 32)
	{
		index --;
		if (index >= nodes.size())
		{
			nodes.resize(index+1);
		}
		if (NULL == nodes[index])
		{	
			MemoryNode * header = allocNodes(size);
			nodes[index] = header;
		}
		MemoryNode * node = nodes[index];
		if (node)
		{
			nodes[index] = node->next;
			if (size != node->content.size())
				node->content.resize(size);
			node->pointer = node;
			quicknodes.addObject(node);
			return node;
		}
	}
	return node;
}

/**
 * �ͷŵ�ǰ�ڴ�����
 */
void MemoryManager::dealloc(stRefMemory& memory)
{
	
	MemoryNode *node = (MemoryNode*)quicknodes.findObject(memory.refQuickId);
	if (!node) return;
	if (node && node->pointer == memory.pointer)
	{
		if (useLocal && nodes.size())
		{
			node->next = nodes[0];
			nodes[0] = node; // ���տ�
			node->pointer = 0;
			node->refQuickId = -1;
		}
		else
		{
			unsigned int size = node->content.size();
			unsigned int index = getIndexBySize(size);
			// ��ǰ����Ϊindex
			if (index >= 1 && index <= 32)
			{
				index --;
				if (index >= nodes.size()) return;
				node->next = nodes[index];
				nodes[index] = node; // ���տ�
				quicknodes.removeObject(node);
				node->pointer = 0;
				node->refQuickId = -1;
			}
		}
	}
}
/**
 * �ڴ�ָ��
 * \param node ������ڴ��
 **/
 void MemoryManager::dealloc(MemoryNode* node)
 {
	if (!node) return;
	MemoryNode *newnode = (MemoryNode*)quicknodes.findObject(node->refQuickId);
	if (node && node == newnode)
	{
		if (useLocal)
		{
			node->next = nodes[0];
			nodes[0] = node; // ���տ�
			node->pointer = 0;
			node->refQuickId = -1;
		}
		else
		{
			unsigned int size = node->content.size();
			unsigned int index = getIndexBySize(size);
			// ��ǰ����Ϊindex
			if (index >= 1 && index <= 32)
			{
				index --;
				if (index >= nodes.size()) return;
				node->next = nodes[index];
				nodes[index] = node; // ���տ�
				quicknodes.removeObject(node);
				node->pointer = 0;
				node->refQuickId = -1;
			}
		}
	}
 }
/**
 * �������ò���ָ���Ŀ�
 * \param memory ����
 * \return �ڴ��
 */
MemoryNode *MemoryManager::findNode(stRefMemory& memory)
{
	MemoryNode *node = (MemoryNode*)quicknodes.findObject(memory.refQuickId);
	if (node && node->pointer == memory.pointer && node->refQuickId != -1)
	{
		return node;
	}
	return NULL;
}
MemoryManager::MemoryManager(bool useLocal)
{
	if (useLocal)
	{
		nodes.resize(1);
	}
	else
		nodes.resize(32); // ��ʼ32������
	this->useLocal = useLocal;
	nodeIndex = 0;
}
MemoryManager::MemoryManager()
{
	useLocal = true;
	nodes.resize(1);
	nodeIndex = 0;
}
/**
 * ���ݴ�С��ȡ������
 * \param size ���С
 * \return ������
 */
unsigned int MemoryManager::getIndexBySize(unsigned int size)
{
	unsigned int index = 0;
	for (int i = 31;i >= 0;i--)
	{
		if (size & ((unsigned int) 1 << i))
		{
			index = i;
			break;
		}
	}
	if (size > ((unsigned int) 1 << index) && size < ((unsigned int) 1<< (index+1)))
		index++;
	return index;
}