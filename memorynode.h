#pragma once
#include "sys.h"
#include "vector"
#include "refmemory.h"
/**
 * һ���ڴ�ڵ�
 */
class MemoryNode:public stRefMemory{
public:
	std::vector<char> content;// ����
	MemoryNode *next; // ��һ�����п�
	DYNAMIC_API MemoryNode()
	{
		next = NULL;
	}
};