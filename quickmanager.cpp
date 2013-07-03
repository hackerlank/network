#include "quickidmanager.h" 
/**
     * ����һ����Ʒ
     * \param object
     */
bool QuickIdManager::addObject(QuickObjectBase *object)
{
    if (!emptyIds.empty())
    {
        DWORD id = *emptyIds.begin();
        emptyIds.erase(emptyIds.begin());
        _objects[id] = object;
        object->setUniqueQuickId(id);
    }
    else
    {
        DWORD id = _objects.size();
        for (unsigned int i = 0; i < _objects.size();i++)
        {
            if (!_objects[i])
                id = i;
        }
        if (id < _objects.size())
        {
            _objects[id] = object;
        }
        else
            _objects.push_back(object);
        object->setUniqueQuickId(id);
    }
	return true;
}
/**
 * ɾ��һ����Ʒ
 * \param object
 **/
bool QuickIdManager::removeObject(QuickObjectBase *object)
{
    if (object && object->getUniqueQuickId() < _objects.size())
    {
        _objects[object->getUniqueQuickId()] = NULL;
        if (emptyIds.size() < maxEmptySize)
            emptyIds.insert(object->getUniqueQuickId());
        return true;
    }
    return false;
}
/**
 * ������Ʒ
 * \param ��������id
 * \return ��Ʒ
 **/
QuickObjectBase* QuickIdManager::findObject(DWORD uniqueQuickId)
{
    if (uniqueQuickId < _objects.size())
    {
        return _objects[uniqueQuickId];
    }
    return NULL;
}
/**
 * ����id������
 **/
QuickIdManager::QuickIdManager()
{
    maxEmptySize = 100;
}
/**
 * ���ÿ��м��ϵ����տռ�
 **/
void QuickIdManager::setMaxEmptySize(DWORD size)
{
    maxEmptySize = size;
}
/**
 * ��ȡ�洢������
 */
std::vector<QuickObjectBase * > & QuickIdManager::getObjects()
{
	return _objects;
}
/**
 * ��ȡ�ڵ������
 * \return �ڵ������
 */
unsigned int QuickIdManager::getSize()
{
	unsigned int count = 0;
	for (std::vector<QuickObjectBase*>::iterator iter = _objects.begin(); iter != _objects.end();++iter)
	{
		if (*iter) count++;
	}
	return count;
}