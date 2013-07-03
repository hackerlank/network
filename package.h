#pragma once
#include "des.h"
#include <vector>
#pragma pack(1)
/**
 * ��ǰ�ı��İ�
 */
struct stPackTag{
	unsigned char tag; // ���ı�ʶ
	enum{
		NULL_PACK = 0, // ��ͨ�Ǽӹ�����
		DES_PACK = 1 << 0, // ���ܺ�ı���
		ZLIB_PACK = 1 << 1, // ��ǰ������ѹ������
		SPLIT_PACK = 1 << 2, // ��ǰ�Ƿָ�ı���
		HASH_PACK = 1 << 3, // ����hashУ��
		MINI_PACK = 1 << 4, // �����ֽ���С�� 256 �� >= 256 ����ʹ�øñ���
		MAX_PACK = 1 << 5,// ������
		NORMAL_PACK = 1 << 6, // ��ͨ����
	};
};
/*
 *�ɱ�ͷ hash ������֤����ʱ hash ͷ����
 */
struct stPack_hash{
	unsigned int hash_code;
};
/*
 *�ɱ�ͷ zip ��ʱzip ����ʱ zip ͷ����
 */
struct stPack_zip{
	unsigned int sourceSize;
};
struct stPackHead_min{
	unsigned char size; // ���㱨�Ĵ�С,��ʹ��zip 
};
struct stPackHead_none{
	unsigned short size; // ��ͨ���� ���ȱ�ȻС�� 2^16
};
struct stPackHead_max{
	unsigned int size; // ϵͳ�������󵥸����� ϣ����Ⱥ����ʹ��
};
struct stPackHead_split{
	unsigned short size; // �ӿ��ʵ�����ݴ�С
	unsigned int all_size; // ����ܴ�С
	unsigned short split_id;// ��ǰ���ӿ��
};
#pragma pack()
/**
 * ��ȡ��������
 **/
struct stPackContent{
	stPackContent(){
		packageState = PACK_NULL;
	};
	std::vector<char> content; // ��ǰ�����ܵ��ӿ�
	unsigned int contentStartPos; // �߼���������ʼλ��
	std::vector<stPackContent> splitpacks; // ����µ��ӿ�
	/**
	 *  ����buffer �е����ݵ�pack
	 * \param buffer �����׵�ַ
	 * \param startPos ��ʼ�����ĵ�ַ
	 * \param allSize ����Ĵ�С
	 * \return ��ǰ������λ��
	 */
	int decode(char *buffer,int startPos,unsigned int allsize);
	/**
	 * �������
	 */
	void reset();
	/**
	 * �����Ĵ��
	 * \param tag ����tag
	 * \param cmd ��������
	 * \param size �������ݴ�С
	 * \return packsize ���ĵĴ�С
	 */
	int encode(stPackTag& tag,void *cmd,int size);
	/**
	 * ��ǰ������ok��
	 * \return ����ok true false ���Ļ�����������
	 */
	bool isOk();
	/**
	 * ��ǰ������bad ��
	  * \return true���� ����
	 */
	bool isBad();

	/**
	 * ����Ƿ�����ӿ�
	 * \return true �����ӿ� false �������ӿ�
	 */
	bool hadChildPack();
	/**
	 * ��ȡ�߼������ָ��
	 */
	void *getContentPointer();
	/**
	 * ��ȡ���ݵĴ�С
	 */
	unsigned int getContentSize();
	/**
	 * �������ӵ���Կ
	 */
	void setKey(const_ZES_cblock &key);
private:
	enum{
		PACK_NULL = 0, // δ��ʶ����
		PACK_OK = 1 << 1, // �ñ���
		PACK_BAD =1 << 2 , // ������
	};
	enum{
		MAX_PACK_SIZE = 0xffffffff - 1,
	};
	int packageState; // ָʾpack �Ѿ����ܺ���
	const_ZES_cblock key; //���ĵ�keyֵ
	/**
	 * �����Ǻñ���
	 */
	void setOk();
	/**
	 * �����ǻ�����
	 */
	void setBad();
	/**
	 * \param offset ��ǰ�Ļ���ƫ��
	 * \param buffer ��ǰ�Ļ���
	 * \param needPackSize ��ǰ������Ҫ��Content�ĳ���
	 * \param allsize ��ǰ����Ĵ�С
	 * \out offset �µ�ƫ��
	 */
	int eatContent(char *buffer,int startPos,int offset,unsigned int needPackSize,unsigned int allsize);
	/**
	 * ��䱨��
	 * \param content ������ݵĵ�ַ
	 * \param ������ݵĴ�С
	 * \return ��ȡ�ĵ�ǰ��content�Ĵ�С
	 */
	int feedContent(void *content,unsigned int size);
	/**
	 * ��䱨��
	 * \param pack ����
	 * \param content ������ݵĵ�ַ
	 * \param ������ݵĴ�С
	 * \return ��ȡ�ĵ�ǰ��content�Ĵ�С
	 */
	int feedContent(stPackContent& pack,void *content,unsigned int size);
	/**
	 * ����hash ֵ
	 * \param buffer ���濪ʼ
	 * \param bufferSize �����С
	 * \return hash ֵ
	 */
	unsigned int calcHash(char *buffer,unsigned int bufferSize);
	/**
	 * des ����
	 * \param startPos,��ǰ�������ʼ��ַ
	 */
	void unDes(unsigned int startPos,unsigned int size);
	/**
	 * zip ��ѹ
	 * \param startPos ��ǰ�������ʼ��ַ
	 */
	bool unZip(unsigned int startPos,unsigned int size,unsigned int sourceSize);
	/**
	 * des ����
	 * \param buffer �����ܵ�buffer
	 * \param bufferSize buffer�Ĵ�С
	 * \param out ���
	 */
	void des(void *buffer,unsigned int bufferSize,std::vector<char> &out);
	/**
	 * zip ѹ��
	 * \param buffer ��ѹ����buffer
	 * \param bufferSize buffer �Ĵ�С
	 * \param out ���
	 */
	void zip(void *buffer,unsigned int bufferSize,std::vector<char> &out);
	/**
	 * �����ķָ�
	 * \param buffer���ָ����ݵ���ʼ��ַ
	 * \param bufferSize ���ݳ���
	 * \return ���ĵĴ�С
	 */
	int splitContent(void *buffer,unsigned int bufferSize);

	/**
	 * �ϲ�����
	 */
	void mergePackets();
};