#include "refmemory.h"

/**
 * ���ÿ���Ψһ����
 * \param uniqueQuickId ���ÿ���Ψһ����
 */
void stRefMemory::setUniqueQuickId(DWORD uniqueQuickId)
{
	this->refQuickId = uniqueQuickId;
}
/**
 * ��ȡ����Ψһ����
 * \return ����Ψһ����
 */
DWORD stRefMemory::getUniqueQuickId()
{
	return this->refQuickId;
}