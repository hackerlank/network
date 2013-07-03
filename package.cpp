#include "sys.h"
#include "package.h"
#include "zlib.h"

/**
 * �����Ĵ��
 * \param tag ����tag
 * \param cmd ��������
 * \param size �������ݴ�С
 * \return packsize ���ĵĴ�С
 */
int stPackContent::encode(stPackTag& ptag,void *cmd,int size)
{
	unsigned char tag = ptag.tag;
	int offset = 0;
	stPackTag packTag;
	packTag.tag = tag;
	offset += feedContent(&packTag,sizeof(packTag));
	// �ȼ�hash
	if (tag & stPackTag::HASH_PACK)
	{
		stPack_hash hash;
		hash.hash_code = calcHash((char*) cmd,size);
		offset += feedContent(&hash,sizeof(hash));
	}
	std::vector<char> buf;
	// �ټ���
	if (tag & stPackTag::DES_PACK) 
	{
		des(cmd,size,buf);
	}
	// ��ѹ��
	if (tag & stPackTag::ZLIB_PACK)
	{
		// ���zip ��ͷ
		stPack_zip ziphead;
		ziphead.sourceSize = size;
		offset += feedContent(&ziphead,sizeof(ziphead));
		zip(cmd,size,buf);
	}
	if (buf.size() == 0) return -1;

	// ������ͷ
	if (buf.size() < 255 )
	{
		content[0] |= stPackTag::MINI_PACK;
		stPackHead_min min;
		min.size = buf.size();
		offset += feedContent(&min,sizeof(min));
	}
	else if (buf.size() >= MAX_PACK_SIZE && (tag & stPackTag::MAX_PACK))
	{
		stPackHead_max max;
		max.size = buf.size();
		offset += feedContent(&max,sizeof(max));
	}
	else if (buf.size() >= MAX_PACK_SIZE )
	{
		content[0] |= stPackTag::SPLIT_PACK;
		// �����ķָ�
		offset += splitContent(&buf[0],buf.size());
		return offset;
	}
	else if (buf.size() <= MAX_PACK_SIZE)
	{
		content[0] |= stPackTag::NORMAL_PACK;
		stPackHead_none none;
		none.size = buf.size();
		offset += feedContent(&none,sizeof(none));
	}
	
	offset += feedContent(&buf[0],buf.size());
	return offset;
}
/**
 * ����buffer �е����ݵ�pack
 * \param buffer �����׵�ַ
 * \param startPos ��ʼ�����ĵ�ַ
 * \param allSize ����Ĵ�С
 * \return ��ǰ������λ�� -1 ��������
 */
int stPackContent::decode(char *buffer,int startPos,unsigned int allsize)
{
	// ��ɽ�� ��ѹ�� ���� ��������
	int offset = startPos;
	unsigned int needPackSize = sizeof(stPackTag);
	unsigned int startPackPos = 0;
	offset = eatContent(buffer,startPos,offset,needPackSize,allsize);

	if (content.size() < needPackSize)
	{
		return offset;
	}
	//�Ѿ���ȡ��tag�� �����ֲ�ͬ�ı���
	stPackTag * tag =(stPackTag*) &content[startPackPos];
	startPackPos += sizeof(stPackTag);
	// �������hashУ�� 
	stPack_hash *hash = NULL;
	if ((tag->tag & stPackTag::HASH_PACK) == 1)
	{
		//stPack_hash ��Ҫ����hash
		needPackSize += sizeof(stPack_hash);

		offset = eatContent(buffer,startPos,offset,needPackSize,allsize);// ����Ҫ�Ի���

		if (content.size() < needPackSize)
		{
			return offset;
		}
		hash = (stPack_hash*)&content[startPackPos];
		startPackPos += sizeof(stPack_hash);
	}
	stPack_zip *zip = NULL;
	if ((tag->tag & stPackTag::ZLIB_PACK) == 1) // �����zlib ���� ��ô�ճ�int �����ʵ�����ݵĴ�С
	{
		needPackSize += sizeof(stPack_zip);
		offset = eatContent(buffer,startPos,offset,needPackSize,allsize);
		if (content.size() < needPackSize)
		{
			return offset;
		}
		zip = (stPack_zip*) & content[startPackPos];
		startPackPos += sizeof(stPack_zip);
	}
	// ���˷ָ���������ı������ݳ��ȶ�һ��
	unsigned int contentSize = 0;
	if (tag->tag & stPackTag::MINI_PACK)
	{
		needPackSize += sizeof(stPackHead_min);
		offset = eatContent(buffer,startPos,offset,needPackSize,allsize); // ����Ҫ�Ի���

		if (content.size() < needPackSize)
		{
			return offset;
		}
		stPackHead_min *none = (stPackHead_min*) &content[startPackPos];
		// ��ȡ��pack��ͷ
		if (none->size >= MAX_PACK_SIZE)
		{
			setBad();
			return offset;
		}
		needPackSize +=  none->size;
		// ��ȡ��pack��ͷ
		if (none->size >= MAX_PACK_SIZE)
		{
			setBad();
			return offset;
		}
		contentSize = none->size;
		needPackSize +=  none->size;
		startPackPos += sizeof(stPackHead_min);
	}
	else if (tag->tag & stPackTag::NORMAL_PACK)
	{
		needPackSize += sizeof(stPackHead_none);
		
		offset = eatContent(buffer,startPos,offset,needPackSize,allsize); // ����Ҫ�Ի���

		if (content.size() < needPackSize)
		{
			return offset;
		}
		stPackHead_none *none = (stPackHead_none*) &content[startPackPos];
		// ��ȡ��pack��ͷ
		if (none->size >= MAX_PACK_SIZE)
		{
			setBad();
			return offset;
		}
		needPackSize +=  none->size;
		startPackPos += sizeof(stPackHead_none);
		contentSize = none->size;
	}
	else if (tag->tag & stPackTag::MAX_PACK)
	{
		needPackSize += sizeof(stPackHead_max);
		
		offset = eatContent(buffer,startPos,offset,needPackSize,allsize); // ����Ҫ�Ի���

		if (content.size() < needPackSize)
		{
			return offset;
		}
		stPackHead_max *none = (stPackHead_max*) &content[startPackPos];
		// ��ȡ��pack��ͷ
		if (none->size >= MAX_PACK_SIZE)
		{
			setBad();
			return offset;
		}
		needPackSize +=  none->size;
		startPackPos += sizeof(stPackHead_max);
		contentSize = none->size;
	}
	else if ((tag->tag & stPackTag::SPLIT_PACK) == 1)
	{
		needPackSize += sizeof(stPackHead_split);
		
		offset = eatContent(buffer,startPos,offset,needPackSize,allsize); // ����Ҫ�Ի���

		if (content.size() < needPackSize)
		{
			return offset;
		}
		stPackHead_split *none = (stPackHead_split*) &content[startPackPos];
		// ��ȡ��pack��ͷ
		if (none->size >= MAX_PACK_SIZE)
		{
			setBad();
			return offset;
		}
		needPackSize +=  none->size;
		// ��ȡ��pack��ͷ
		if (none->size >= MAX_PACK_SIZE)
		{
			setBad();
			return offset;
		}
		needPackSize +=  none->size;
		startPackPos += sizeof(stPackHead_split);

		offset = eatContent(buffer,startPos,offset,needPackSize,allsize);// ����Ҫ�Ի���
		if (content.size() < needPackSize)
		{
			return offset;
		}
		
		if (none->split_id >= MAX_PACK_SIZE || none->split_id >= (none->all_size / MAX_PACK_SIZE + 1))
		{
			setBad();
			return offset;
		}
		if (none->split_id >= splitpacks.size()) splitpacks.resize(none->split_id + 1);
		stPackContent pack;
		pack.content = this->content;
		pack.contentStartPos = startPackPos;
		splitpacks[none->split_id] = pack;
		setOk();
		contentStartPos = startPackPos; // ��¼�߼������ݵ���ʼλ��
		if (splitpacks.size() == none->all_size / MAX_PACK_SIZE + 1)
		{
			// ��ʼ�ϲ�����
			mergePackets();
			contentSize = none->all_size;
		}
		else
		{
			return offset;
		}
	}
	if (!((tag->tag & stPackTag::SPLIT_PACK)))
	{
		offset = eatContent(buffer,startPos,offset,needPackSize,allsize);// ����Ҫ�Ի���
		if (content.size() < needPackSize)
		{
			return offset;
		}
	}
	if (tag->tag & stPackTag::HASH_PACK)
	{
		// �Ƚ�����֤
		if (hash)
		{
			if (hash->hash_code != calcHash(&content[startPackPos],contentSize))
			{
				setBad();
				return offset;
			}
		}
		else
		{
			setBad();
			return offset;
		}
	}
	// �߼��������� content[startPackPos];
	if (tag->tag& stPackTag::ZLIB_PACK)
	{
		// �ȴ����ѹ
		if (!zip)
		{
			setBad();
			return offset;
		}
		if (!unZip(startPackPos,contentSize,zip->sourceSize)) // ��֪���Ľ�ѹ�����ݿ�
		{
			setBad();
			return offset;
		}
	}
	if (tag->tag & stPackTag::DES_PACK)
	{
		// �������
		unDes(startPackPos,contentSize); // �ý��ܵ����ݿ�
	}
	setOk();
	contentStartPos = startPackPos;
	return offset;
}
/**
 * \param offset ��ǰ�Ļ���ƫ��
 * \param buffer ��ǰ�Ļ���
 * \param needPackSize ��ǰ������Ҫ��Content�ĳ���
 * \param allsize ��ǰ����Ĵ�С
 * \return �µ�ƫ��
 */
int stPackContent::eatContent(char *buffer,int startPos,int offset,unsigned int needPackSize,unsigned int allsize)
{
	if (content.size() < needPackSize)
	{
		startPos = offset;
		offset = needPackSize - content.size();
		if (offset < allsize - startPos)
			offset = allsize - startPos;
		content.insert(content.end(),buffer+startPos,buffer + startPos + offset);
		offset += startPos;
	}
	return offset;
}
/**
 * �������
 */
void stPackContent::reset()
{
	packageState = PACK_NULL;
	content.clear();
}
/**
*��ǰ������ok��
 * \return ����ok true false ���Ļ�����������
 */
bool stPackContent::isOk()
{
	return (PACK_OK == packageState);
}
/**
 * ��ǰ������bad ��
  * \return true���� ����
 */
bool stPackContent::isBad()
{
	return (PACK_BAD == packageState);
}
/**
 * �����Ǻñ���
 */
void stPackContent::setOk()
{
	packageState = PACK_OK;
}
/**
 * �����ǻ�����
 */
void stPackContent::setBad()
{
	packageState = PACK_BAD;
}
/**
 * des ����
 * \param startPos,��ǰ�������ʼ��ַ
 * \param size ��С
 */
void stPackContent::unDes(unsigned int startPos,unsigned int size)
{
	CEncrypt enc;
	enc.setEncMethod(CEncrypt::ENCDEC_DES);
	enc.set_key_des(&key);
	enc.encdec(&content[startPos],size,true);
}
/**
 * zip ��ѹ
 * \param startPos ��ǰ�������ʼ��ַ
 * \param size ��С
 * \param sourceSize Դ��С
 */
bool stPackContent::unZip(unsigned int startPos,unsigned int size,unsigned int sourceSize)
{
	std::vector<char> buf;
	buf.resize(sourceSize);
	uLongf destLen = sourceSize;
	int  zOk = uncompress((Bytef*)&buf[0], &destLen,(Bytef*) &content[startPos], size);
	if (zOk == Z_OK && destLen == sourceSize)
	{
		content.resize(startPos + destLen);
		bcopy(&buf[0],&content[startPos],destLen);
		return true;
	}
	else
	{
		return false;
	}
}
/**
 * des ����
 * \param buffer �����ܵ�buffer
 * \param bufferSize buffer�Ĵ�С
 * \param out ���
 */
void stPackContent::des(void *buffer,unsigned int bufferSize,std::vector<char> &out)
{
	CEncrypt enc;
	out.resize(bufferSize);
	enc.setEncMethod(CEncrypt::ENCDEC_DES);
	enc.set_key_des(&key);
	enc.encdec(&out[0],out.size(),true);
}
/**
 * �������ӵ���Կ
 */
void stPackContent::setKey(const_ZES_cblock &key)
{
	bcopy(&key[0],&this->key[0],sizeof(const_ZES_cblock));
}
/**
 * zip ѹ��
 * \param buffer ��ѹ����buffer
 * \param bufferSize buffer �Ĵ�С
 * \param out ���
 */
void stPackContent::zip(void *buffer,unsigned int bufferSize,std::vector<char> &out)
{
	out.resize((bufferSize + 12)*1.5);
	uLongf destLen = out.size();
	int zRes = compress((Bytef *)&out[0], &destLen,(Bytef *)buffer,bufferSize);
	if (zRes == Z_OK)
		out.resize(destLen);
}
/**
 * ����hash ֵ
 * \param buffer ���濪ʼ
 * \param bufferSize �����С
 * \return hash ֵ
 */
unsigned int stPackContent::calcHash(char *buffer,unsigned int bufferSize)
{
	unsigned int hash = 0;
	int i; 
	for (i=0; *buffer && i < bufferSize; i++)
	{
		if ((i & 1) == 0)
		{
			hash ^= ((hash << 7) ^ (*buffer++) ^ (hash >> 3));
		}
		else
		{
			hash ^= (~((hash << 11) ^ (*buffer++) ^ (hash >> 5)));
		}
	}

	return (hash & 0x7FFFFFFF); 
}
/**
 * ��䱨��
 * \param content ������ݵĵ�ַ
 * \param ������ݵĴ�С
 * \return ��ȡ�ĵ�ǰ��content�Ĵ�С
 */
int stPackContent::feedContent(void *content,unsigned int size)
{
	this->content.insert(this->content.end(),(char*)content,(char*)content+size);
	return this->content.size();
}
/**
 * ��䱨��
 * \param pack ����
 * \param content ������ݵĵ�ַ
 * \param ������ݵĴ�С
 * \return ��ȡ�ĵ�ǰ��content�Ĵ�С
 */
int stPackContent::feedContent(stPackContent& pack,void *content,unsigned int size)
{
	pack.content.insert(pack.content.end(),(char*)content,(char*)content+size);
	return pack.content.size();
}
/**
 * �����ķָ�
 * \param buffer���ָ����ݵ���ʼ��ַ
 * \param bufferSize ���ݳ���
 * \return ���ĵĴ�С
 */
int stPackContent::splitContent(void *buffer,unsigned int bufferSize)
{
	if (bufferSize < MAX_PACK_SIZE) return -1;
	// ��ǰcontent ��hash + tag �� ��ͷ
	stPackHead_split head;
	head.all_size = bufferSize;
	unsigned int packCount = bufferSize / MAX_PACK_SIZE;
	int offset = 0;
	unsigned int index = 0;
	for ( index = 0; index < packCount;index++)
	{
		head.size = MAX_PACK_SIZE;
		head.split_id = index;
		stPackContent buf;
		feedContent(buf,&content[0],content.size());
		feedContent(buf,&head,sizeof(head));
		feedContent(buf,((char*)buffer)+offset,head.size);
		splitpacks.push_back(buf);
		offset += head.size;
	}
	head.size = bufferSize % MAX_PACK_SIZE;
	if (head.size)
	{
		head.size = MAX_PACK_SIZE;
		head.split_id = index;
		stPackContent buf;
		feedContent(buf,&content[0],content.size());
		feedContent(buf,&head,sizeof(head));
		feedContent(buf,((char*)buffer)+offset,head.size);
		splitpacks.push_back(buf); // �õ��ӿ���
		offset += head.size;
	}
	content.clear();
	return offset;
}

/**
 * �ϲ�����
 */
void stPackContent::mergePackets()
{
	unsigned int offset = contentStartPos;
	for(int index = 0;index < splitpacks.size();index++)
	{
		stPackContent &pack = splitpacks[index];
		if (pack.content.size() > pack.contentStartPos)
		{
			if (offset + pack.content.size() - pack.contentStartPos < content.size())
			{
				content.resize(offset + pack.content.size() - pack.contentStartPos);
			}
			bcopy(&pack.content[pack.contentStartPos],&content[offset],pack.content.size() - pack.contentStartPos);
		}
	}
}

/**
 * ����Ƿ�����ӿ�
 * \return true �����ӿ� false �������ӿ�
 */
bool stPackContent::hadChildPack()
{
	return splitpacks.size();
}

/**
 * ��ȡ�߼������ָ��
 */
void *stPackContent::getContentPointer()
{
	if (contentStartPos >= content.size()) return NULL;
	return &content[contentStartPos];
}
/**
 * ��ȡ���ݵĴ�С
 */
unsigned int stPackContent::getContentSize()
{
	return content.size() - contentStartPos;
}