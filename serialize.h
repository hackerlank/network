/**
 * email:ji.jin.dragon@gmail.com
 * Author jijinlong
 * Data 2011.10.14~~~
 * All Rights Resvered
 ***/
#pragma once
#include "sstream"
#include "stdio.h"
#include "iostream"
#include "string.h"
#include "string"
#include "vector"
#include "set"
#include <map>
#include "sys.h"
#include "sString.h"
#ifdef _USE_ZLIB_ 
	#include "zlib.h"
#endif
#include "list"
#ifdef _USE_BASE64_
	#include "Base64.h"
#endif

#define IMP_SERIALIZE_DATA_START(__class__)\
	void __class__::__v_store__(serialize::Stores &sts,BYTE tag)\

#define IMP_SERIALIZE_DATA_FUNC()\
	void __v_store__(serialize::Stores &sts,BYTE tag)\

#define IMP_SERIALIZE_DATA_FUNC_CONDITION(__result__)\
	void __v_store__(serialize::Stores &sts,BYTE tag){\
		if ((!(__result__))&& tag == __STORE__ ) return;

#if (0)
#define SERIALIZE_DATA(__value__)\
	switch(tag)\
	{\
		case __STORE__:\
		{\
			sts.addStore(__value__,#__value__);\
		}break;\
		case __LOAD__:\
		{\
			sts.getStore(__value__,#__value__);\
		}break;\
	}
#define SERIALIZE_DATA_CONDITION(__value__,__result__)\
	switch(tag)\
	{\
		case __STORE__:\
		{\
			if ((__result__))\
			sts.addStore(__value__,#__value__);\
		}break;\
		case __LOAD__:\
		{\
			sts.getStore(__value__,#__value__);\
		}break;\
	}
#endif

#define SERIALIZE_DATA(__value__,index)\
	switch(tag)\
	{\
		case __STORE__:\
		{\
			sts.addStore(__value__,index);\
		}break;\
		case __LOAD__:\
		{\
			sts.getStore(__value__,index);\
		}break;\
	}
#define SERIALIZE_DATA_CONDITION(__value__,index,__result__)\
	switch(tag)\
	{\
		case __STORE__:\
		{\
			if ((__result__))\
			sts.addStore(__value__,index);\
		}break;\
		case __LOAD__:\
		{\
			sts.getStore(__value__,index);\
		}break;\
	}

#define IMP_SERIALIZE_DATA_END }
#define IMP_SERIALIZE_DATA_FUNC_END } 
#define CALL_SERIALIZE(__class__)\
		__class__::__store__(ss,tag);
#define __STORE__ 1
#define __LOAD__ 2

#define DEC_SERIALIZE_DATA void __v_store__(serialize::Stores &ss,BYTE tag)

namespace serialize{
	const DWORD MAX_NAME_SIZE= 32;
	#pragma pack(1)
	const BYTE __DATA_OBJECT__=1; // point ����
	const BYTE __DATA_OBJECT_MAP__ =2; // map ����<normal,point> ����
	const BYTE __DATA_OBJECT_VECTOR__ = 3;
	const BYTE __DATA__INT__ = 4;
	const BYTE __DATA_FLOAT__ = 5;
	const BYTE __DATA_UNSIGNED_INT__ = 6;
	const BYTE __DATA_CHAR__ = 7;
	const BYTE __DATA_LONG__ = 8;
	const BYTE __DATA_UNSINGED_CHAR__ = 9;
	const BYTE __DATA_UNSINGED_LONG__ = 10;
	const BYTE __DATA_UNSINGED_FLOAT__ = 11;
	const BYTE __DATA_LONG_LONG__ = 12;
	const BYTE __DATA_UNSINGED_LONG_LONG__ = 13;	
	const BYTE __VOID_STAR_POINT__ = 14;
	const BYTE __DATA_STRING__ = 15;
	typedef int SEARCH_ARG; 
	/**
	 * ��¼ͷ���̶�����
	 **/
	struct CoreData{
		BYTE tag; // ����
		int hashcode; // ���� ȡǰ4����У��
		DWORD size; // ��С
		CoreData()
		{
			bzero(this,sizeof(CoreData));
		}
	};
	#pragma pack()
	/**
	 * ��¼
	 **/
	struct Store{
		CoreData coreData; // ��¼ͷ
		std::vector<char> content; // ��¼����
		DYNAMIC_API Store()
		{
		}
		/**
		 * ��ʼ��
		 * \param content �����׵�ַ
		 * \param size ��С
		 * \param name ��¼����
		 **/
		DYNAMIC_API Store(void *content,int size,SEARCH_ARG name)
		{
			this->content.resize(size);
			bcopy(content,&this->content[0],size);
			//strncpy(coreData.name,name,MAX_NAME_SIZE -1 );
			coreData.hashcode = name;//sys::HashStr::calc(name);
			coreData.tag = 1;
			coreData.size = size;
		}
		/**
		 *  ��ʼ��
		 * \param tag ��¼���� __DATA_DWORD__ ��
		 * \param content �����׵�ַ
		 * \param size ��С
		 * \param name ��¼���� 
		 **/
		DYNAMIC_API Store(BYTE tag,void *content,int size,SEARCH_ARG name)
		{
			this->content.resize(size);
			bcopy(content,&this->content[0],size);
			//strncpy(coreData.name,name,MAX_NAME_SIZE -1 );
			coreData.hashcode = name;//sys::HashStr::calc(name);
			coreData.tag = tag;
			coreData.size = size;
		}
		/**
		 * ��ֵ����
		 **/
		DYNAMIC_API Store & operator=(const Store & store)
		{
			content.resize(0);
			content.insert(content.begin(),store.content.begin(), store.content.end());	
			//strncpy(this->coreData.name,store.coreData.name,MAX_NAME_SIZE-1);
			this->coreData.hashcode = store.coreData.hashcode;
			coreData.tag = store.coreData.tag;
			coreData.size = store.coreData.size;
			return *this;
		}
		DYNAMIC_API bool equal(SEARCH_ARG name)
		{
			return (coreData.hashcode == name /*sys::HashStr::calc(name)*/);
		}
		/**
		 * �������
		 */
		DYNAMIC_API void setIndex(int index)
		{
			this->coreData.hashcode = index;
		}
		/** 
		 * ��ȡ���index
		 **/
		DYNAMIC_API int getIndex()
		{
			return coreData.hashcode;
		}
	};
	/**
	 * ��
	 **/
	struct Stream{
		std::vector<char> content; // ����
		int offset;
		DYNAMIC_API Stream()
		{
			NCHAR = '\0';
			offset = 0;
		}
		/**
		 * ������
		 */
		DYNAMIC_API void reset();
		/**
		 * ��ʼ����
		 * \param buf�׵�ַ
		 * \param size buf����
		 **/
		DYNAMIC_API Stream(void *str,unsigned int size)
		{
			content.resize(size);
			bcopy(str,&content[0],size);	
			offset = 0;
			NCHAR = '\0';
		}
		DYNAMIC_API Stream & operator = (const Stream & str)
		{
			content = str.content;
			offset = str.offset;
			NCHAR = '\0';
			return *this;
		}
		/**
		 * ���ô���С
		 * \param size ����С
		 **/
		DYNAMIC_API void resize(DWORD size)
		{
			content.resize(size);
		}
#ifdef _USE_ZLIB_
		/**
		 * ѹ��
		 **/
		void compress(char *buffer,unsigned int size)
		{
			//toBase64();
			bzero(buffer,size);
			uLongf lSize = size;
			if (!lSize) return;
			::compress((unsigned char*)buffer,&lSize,(unsigned char*)&content[0],this->size());
			content.resize(lSize);
			bcopy(buffer,&content[0],lSize);
		}
		/**
		 * ��ѹ
		 **/
		void uncompress(char *buffer,unsigned int size)
		{
			bzero(buffer,size);
			uLongf lSize = size;
			::uncompress((unsigned char *)buffer,&lSize,(unsigned char*)&content[0],this->size());
			content.resize(lSize);
			bcopy(buffer,&content[0],lSize);
			//fromBase64(buffer,size);
		}
#endif
#ifdef _USE_BASE64_
		void toBase64()
		{
			std::string temp;
			Base64::encode(&content[0],this->size(),temp);
			content.resize(temp.size() + 1);
			bcopy(temp.c_str(),&content[0],content.size());	
		}
		void fromBase64(char *buffer,unsigned int size)
		{
			size_t tSize = size;
			bzero(buffer,size);
			std::string temp = &content[0];
			Base64::decode(temp,(unsigned char*)buffer,tSize);
			printf("ccc<%d>",tSize);
			content.resize(tSize);
			bcopy(buffer,&content[0],tSize);
		}
#endif
		char NCHAR; // �շ���
		/**
		 * ����������ʷ���
		 **/
		DYNAMIC_API char & operator[](unsigned int i )
		{
			if (i < content.size())
				return content[i];
			return NCHAR;
		}
		DYNAMIC_API char *c_str()
		{
			if (this->content.empty()) return NULL;
			return &this->content[0];
		}
		DYNAMIC_API unsigned int size(){return (int)content.size();}
		
		/**
		 * д���ļ�
		 * \param fileName �ļ���
		 **/
		DYNAMIC_API void writeToFile(const char * fileName);
		/**
		 * ���ļ��ж���
		 * \param fileName �ļ���
		 **/
		DYNAMIC_API void readFromFile(const char *fileName);

		/**
		 * ����һ��Store
		 */
		DYNAMIC_API void addStore(Store & store);
		/**
		 * ��ȡ��һ��Store
		 */
		DYNAMIC_API bool pickStore(Store &store);

		/**
		 * ����һ������
		 * \param data ָ��
		 * \param size ���ݳ���
		 */
		DYNAMIC_API void addData(void *data,int size);
		/**
		 * ��ȡָ�����ȵ�����
		 * \param data �ⲿ���ݻ���
		 * \param size �������ݵĳ���
		 * \return �ɹ�true ʧ��false
		 */
		DYNAMIC_API bool pickData(void *data,int size);
	};
	class Stores;	
	/**
	 * ���л����� ��Ҫ�̳�֮
	 * */
	struct Object{
		/**
		 * ��2��������ת��Ϊ����
		 * \param content 2��������
		 * \param size ���ݴ�С
		 **/
		DYNAMIC_API void parse(void * content,unsigned int size)
		{
			if (!size) return;
			Stream ss(content,size);
			parseRecord(ss);
		}
		/**
		 * �����ݿ�����content ��,
		 * \param content ����
		 * \param size �����С
		 **/
		DYNAMIC_API void store(void *content,unsigned int size)
		{
			Stream ss = toRecord();
			if (ss.size() > size) return;
			bcopy(&ss[0],content,ss.size() < size ? ss.size() : size); 
		}
		/**
		 * �����ݿ�����content ��,
		 * \param content ���� vector
		 **/
		DYNAMIC_API void store(std::vector<char>& content)
		{
			Stream ss = toRecord();
			content.resize(ss.size());
			bcopy(&ss[0],&content[0],ss.size()); 
		}
		/**
		 * �����ݿ�����content ��,
		 * \param content ���� vector
		 **/
		DYNAMIC_API void store(std::vector<unsigned char>& content)
		{
			Stream ss = toRecord();
			content.resize(ss.size());
			bcopy(&ss[0],&content[0],ss.size()); 
		}
		/**
		 * ��ȡ������
		 * \return ��
		 **/
		DYNAMIC_API virtual	Stream toRecord()
		{
			Stream ss;
			__store__(ss,__STORE__);
			return ss;
		}
		/**
		 * ����ת��Ϊ����
		 * \param record ��
		 * \return �ɹ� ʧ��
		 **/
		DYNAMIC_API virtual bool parseRecord(Stream &record)
		{
			if (!record.size()) return false;
			__store__(record,__LOAD__);
			return true;	
		}
		/**
		* ��תΪ���� ���߶���תΪ��
		* \param ss in/out ��
		* \param tag __LOAD__ ת��Ϊ���� __STORE__ תΪ��
		* ������ʵ��
		**/
		DYNAMIC_API virtual void __store__(Stream & ss,unsigned char tag);
		DYNAMIC_API virtual void __v_store__(Stores & sts,unsigned char tag){
			//printf("û�ж����ʼ������%x\n",this);
		};
		/**
 		 * Ԥ������
 		 * */
		DYNAMIC_API virtual void preLoad(){}
		/**
		 * Ԥ���洢
		 * */
		DYNAMIC_API virtual void preStore(){}	
		/**
 		 * ���سɹ�
 		 * */
		DYNAMIC_API virtual void beLoaded(){}
		/*
 		 * �洢�ɹ�
 		 * */
		DYNAMIC_API virtual void beStored(){}
		/**
		 * ��ȡ���������
		 * ���������Ҫָ��־û���,��Ҫʵ�ָú���
		 * \return �ö��������
		 */
		DYNAMIC_API virtual int takeKind() {return -1;}

	};

#define DEC_STORE_CELL_FUNC(__class__,__type__)\
DYNAMIC_API bool addStore(__class__ value,SEARCH_ARG name);\
DYNAMIC_API bool getStore(__class__ &value,SEARCH_ARG name);

#define DO_MAKE_STREAM(__type__)\
DYNAMIC_API void makeStream(__type__& value,Stream& stream)\
{\
	stream = Stream(&value,sizeof(__type__));\
}\
DYNAMIC_API void makeStream(__type__* value,Stream& stream)\
{\
	stream = Stream(value,sizeof(__type__));\
}

#define DO_PARSE_STREAM(__type__)\
DYNAMIC_API void parseStream(__type__& value,Stream& stream)\
{\
	if (stream.size() ==0) return;\
	if (sizeof(__type__) == stream.size())\
	{\
		value = *(__type__*) (&stream.content[0]);\
		return;\
	}\
	switch (stream.size())	\
	{\
		case sizeof(char):\
		{\
			value = (__type__)*(char*) (&stream.content[0]);\
		}break;\
		case sizeof(short):\
		{\
			 value = (__type__)*(short*) (&stream.content[0]);\
		}break;\
		case sizeof(int):\
		{\
			 value = (__type__)*(int*) (&stream.content[0]);\
		}break;\
		case sizeof(long long):\
		{\
			 value = (__type__)*(long long*) (&stream.content[0]);\
		}break;\
	}\
}\
DYNAMIC_API void parseStream(__type__* value,Stream& stream)\
{\
	if (stream.size() ==0) return;\
	if (sizeof(__type__) == stream.size())\
	{\
		value = (__type__*) (&stream.content[0]);\
		return;\
	}\
	switch (stream.size())	\
	{\
		case sizeof(char):\
		{\
			value = (__type__*)(char*) (&stream.content[0]);\
		}break;\
		case sizeof(short):\
		{\
			 value = (__type__*)(short*) (&stream.content[0]);\
		}break;\
		case sizeof(int):\
		{\
			 value = (__type__*)(int*) (&stream.content[0]);\
		}break;\
		case sizeof(long long):\
		{\
			 value = (__type__*)(long long*) (&stream.content[0]);\
		}break;\
	}\
}
	/**
	 * �洢
	 **/
	class Stores{
	public:
		Stores(){
		}
		/**
		 * תΪ������
		 * \return ��
		 **/
		DYNAMIC_API Stream toRecord();
		
		/**
		 * תΪ��¼����
		 * \param record ��
		 **/
		DYNAMIC_API void parseRecord(Stream& record);
	
		/**
		 * ����һ��store 
		 **/
		DYNAMIC_API void addStore(Store& store);
	
		/**
		 * ��ȡһ��store
		 */
		DYNAMIC_API bool pickStore(Store& store,SEARCH_ARG name);
		

		DO_MAKE_STREAM(int);
		DO_MAKE_STREAM(float);
		DO_MAKE_STREAM(unsigned int);
		DO_MAKE_STREAM(double);
		DO_MAKE_STREAM(long long);
		DO_MAKE_STREAM(unsigned long long);
		DO_MAKE_STREAM(char);
		DO_MAKE_STREAM(unsigned char);
		DO_MAKE_STREAM(long);
		DO_MAKE_STREAM(short);
		DO_MAKE_STREAM(unsigned short);
		DO_MAKE_STREAM(unsigned long);
		
		DO_PARSE_STREAM(int);
		DO_PARSE_STREAM(float);
		DO_PARSE_STREAM(unsigned int);
		DO_PARSE_STREAM(double);
		DO_PARSE_STREAM(long long);
		DO_PARSE_STREAM(unsigned long long);
		DO_PARSE_STREAM(char);
		DO_PARSE_STREAM(unsigned char);
		DO_PARSE_STREAM(long);
		DO_PARSE_STREAM(unsigned short);
		DO_PARSE_STREAM(short);
		DO_PARSE_STREAM(unsigned long);
		/**
		 * ��T stream ת��Ϊ����
		 */
		template<typename T>
		void parseStream(T&object,Stream &stream)
		{
			object.parseRecord(stream);
		}
		/**
		 * ��T stream ת��Ϊ ����ָ��
		 */
		template<typename T>
		void parseStream(T * &object,Stream &stream)
		{
			int kind = 0;
			if (stream.pickData(&kind,sizeof(kind)))
			{
				if (object == NULL)
					object = T::create(kind);
				if (object)
					object->parseRecord(stream);
			}
		}
		/**
		 * ��T ����ת��Ϊstream
		 */
		template<typename T>
		void makeStream(T& object,Stream &stream)
		{
			stream = object.toRecord();
		}
		/**
		 * ��ָ�����ת��Ϊstream
		 **/
		template<typename T>
		void makeStream(T*& object,Stream &stream)
		{
			// ָ�����ͽ����⴦�����ݿ�
			int kind = object->takeKind();
			stream.addData(&kind,sizeof(kind));
			stream = object->toRecord();
		}
		/**
		 * ������string ʹstring ת��Ϊstream ����
		 */
		DYNAMIC_API void makeStream(std::string str,Stream &stream);
		/**
		 * ��string �л�ȡstream
		 */
		DYNAMIC_API void parseStream(std::string & str,Stream &stream);
		
		/**
		 * �־û�string ����stream
		 */
		DYNAMIC_API void addStore(std::string str,SEARCH_ARG name);

		template<typename T>
		void addStore(T ty,SEARCH_ARG name)
		{
			Stream s;
			makeStream(ty,s);
			Store store (__DATA_OBJECT__,&(s.content[0]),s.size(),name);
			addStore(store);
		}
		/**
		 * ���л� point ���� �̳��� Object
		 * \param object ����ָ��
		 * \param name ��¼����
		 **/
		template <typename T>
		void addStore(T *object,SEARCH_ARG name)
		{
			if (!object) return;
			Stream s;
			makeStream(object,s);
			Store store (__DATA_OBJECT__,&(s.content[0]),s.size(),name);
			addStore(store);
		}
		/**
		 * �־û���map ����
		 */
		template<typename T,typename O>
		void doMakeCollectionStream(T objects,O o,Stream& s)
		{
			if (objects.empty()) return;
			std::vector<char> content;
			content.resize(1024);
			DWORD offset = 0;
			for (typename T::iterator iter = objects.begin(); iter != objects.end();++iter)
			{
				O& rO = *iter;
				Stream s;
				makeStream(rO,s);
				DWORD size = s.size();
				if (content.size() < offset + size)
				{
					content.resize(offset + size*2);
				}
				bcopy(&size,&content[offset],sizeof(DWORD));
				bcopy(&s.content[0],&content[offset + sizeof(DWORD)],s.size());
				offset += sizeof(DWORD) + s.size();
			}
			s = Stream(&content[0],offset);
		}
		/**
		 * ���Ӽ�¼���洢��
		 * \param value std::vector<char>  ��ֵ
		 * \param name ��¼����
		 **/
		template <typename T>
		void addStore(std::list<T> objects,SEARCH_ARG name)
		{
			Stream stream;
			doMakeCollectionStream(objects,stream);
			Store store(__DATA_OBJECT_VECTOR__,&stream.content[0],stream.size(),name);
			addStore(store);
		}
		/**
		 * ���Ӽ�¼���洢��
		 * \param value std::vector<char>  ��ֵ
		 * \param name ��¼����
		 **/
		template<typename T>
		void addStore(std::vector<T> objects,SEARCH_ARG name)
		{
			Stream stream;
			T t;
			doMakeCollectionStream(objects,t,stream);
			Store store(__DATA_OBJECT_VECTOR__,&stream.content[0],stream.size(),name);
			addStore(store);
		}

		/**
		 * ���л� <set> int unsigned short �Ȼ�������
		 * \param objects ���ݼ���
		 * \param name ��¼����
		 **/
		template<typename T>
		void addStore(std::set<T> &objects,SEARCH_ARG name)
		{
			Stream stream;
			doMakeCollectionStream(objects,stream);
			Store store(__DATA_OBJECT_VECTOR__,&stream.content[0],stream.size(),name);
			addStore(store);
		}
		/**
		 * ���л� {name,point} ���ݼ��� �̳��� Object
		 * \param objects ���ݼ���
		 * \param name ��¼����
		 **/
		template<typename Key,typename Value>
		void addStore(std::map<Key,Value>& objects,SEARCH_ARG name)
		{
			std::vector<char> content;
			content.resize(1024);
			DWORD offset = 0;
			for (typename std::map<Key,Value>::iterator iter = objects.begin(); iter != objects.end();++iter)
			{
				// ��¼key�ĳ��� DWORD
				// ��¼key��ֵ size
				// ��¼value�ĳ��� DWORD 
				// ��¼value��ֵ size
				Key key = iter->first;
				Stream ss;
				makeStream(key,ss);
				DWORD keySize = ss.size();
				Value  value = iter->second;
				Stream s; 
				makeStream(value,s);
				DWORD valueSize = s.size(); 
				if (content.size() < offset + keySize)
				{
					content.resize(offset + keySize * 2 + valueSize + sizeof(DWORD)*2);
				}

				bcopy(&keySize,&content[offset],ss.size());
				offset += sizeof(DWORD);

				bcopy(&ss.content[0],&content[offset],keySize);
				offset += keySize;
				
				bcopy(&valueSize,&content[offset],sizeof(DWORD));
				offset += sizeof(DWORD);
				if (content.size() < offset + valueSize)
				{
					content.resize(offset + valueSize * 2 + sizeof(DWORD) * 2);
				}

				bcopy(&s.content[0],&content[offset],s.size());
				offset += s.size();
			}
			Store store(__DATA_OBJECT_MAP__,&content[0],offset,name);
			addStore(store);
		}
		/**
		 * ��ȡ {name,point} ���ݼ��� �̳��� Object
		 * \param objects ���ݼ���
		 * \param name ��¼����
		 **/
		template<typename Key,typename Value>
		bool getStore(std::map<Key,Value> &objects,SEARCH_ARG name)
		{
			Store nowstore;
			pickStore(nowstore,name);
			if (nowstore.equal(name))
			{
				Store *store = &nowstore;
				switch( store->coreData.tag)
				{
					case __DATA_OBJECT_MAP__:
					{
						DWORD offset = 0;
						if (store->coreData.size != store->content.size()) return false;
						while(offset < store->coreData.size )
						{
							DWORD keySize = *(DWORD*)&store->content[offset];
							if (keySize > store->coreData.size)
							{
								printf("[find an error %s:%u,%u]\n",__FILE__,__LINE__,keySize);
								return false;
							}
							
							Stream skey;
							skey.content.resize(keySize);
							bcopy(&store->content[offset+sizeof(DWORD)],&skey.content[0],keySize);
							Key key;
							parseStream(key,skey);
							offset += sizeof(DWORD) + keySize;

							DWORD valueSize = 0;
							bcopy(&store->content[offset],&valueSize,sizeof(DWORD));
							offset += sizeof(DWORD);
							if (valueSize > store->coreData.size)
							{
								return false;
							}	
							Stream sr;
							sr.content.resize(valueSize);
							bcopy(&store->content[offset],&sr.content[0],valueSize);
							Value object;
							parseStream(object,sr);
							objects[key] = (object);
							offset += valueSize;
						}
						return true;
					}break;
				}

			}
			return false;
		}
		/**
		 * ��ȡ {name,point} ���ݼ��� �̳��� Object
		 * \param objects ���ݼ���
		 * \param name ��¼����
		 **/
		template<class T>
		bool getStore(std::set<T>& objects,SEARCH_ARG name)
		{
			Store nowstore;
			pickStore(nowstore,name);
			if (nowstore.equal(name))
			{
				Store *store = &nowstore;
				switch( store->coreData.tag)
				{
					case __DATA_OBJECT_VECTOR__:
					{
						DWORD offset = 0;
						if (store->coreData.size != store->content.size()) return false;
						while(offset < store->coreData.size )
						{
							DWORD size = *(DWORD*)&store->content[offset];
							if (size > store->coreData.size)
							{
								printf("[find an error %s:%u, headerSize:%u,coreSize:%u]\n",__FILE__,__LINE__,size,store->coreData.size);
								break;
							}
							Stream sR;
							sR.content.resize(size);
							bcopy(&store->content[offset+sizeof(DWORD)],&sR.content[0],size);
							T object;
							parseStream(object,sR);
							objects.insert(object);
							offset += sizeof(DWORD) + size;
						}
						return true;
					}break;
				}
			}
			return false;
		}

		/**
		 * ��ȡ {name,point} ���ݼ��� �̳��� Object
		 * \param objects ���ݼ���
		 * \param name ��¼����
		 **/
		template<class T>
		bool getStore(std::list<T>& objects,SEARCH_ARG name)
		{
			Store nowstore;
			pickStore(nowstore,name);
			if (nowstore.equal(name))
			{
				Store *store = &nowstore;
				switch( store->coreData.tag)
				{
					case __DATA_OBJECT_VECTOR__:
					{
						DWORD offset = 0;
						if (store->coreData.size != store->content.size()) return false;
						while(offset < store->coreData.size )
						{
							DWORD size = *(DWORD*)&store->content[offset];
							if (size > store->coreData.size)
							{
								printf("[find an error %s:%u,%u]\n",__FILE__,__LINE__,size);
								break;
							}
							Stream sR;
							sR.content.resize(size);
							bcopy(&store->content[offset+sizeof(DWORD)],&sR.content[0],size);
							T object;
							parseStream(object,sR);
							objects.push_back(object);
							offset += sizeof(DWORD) + size;
						}
						return true;
					}break;
				}
			}
			return false;
		}
		
		/**
		 * ��ȡ {name,point} ���ݼ��� �̳��� Object
		 * \param objects ���ݼ���
		 * \param name ��¼����
		 **/
		template<class T>
		bool getStore(std::vector<T>& objects,SEARCH_ARG name)
		{
			Store nowstore;
			pickStore(nowstore,name);
			if (nowstore.equal(name))
			{
				Store *store = &nowstore;
				switch( store->coreData.tag)
				{
					case __DATA_OBJECT_VECTOR__:
					{
						DWORD offset = 0;
						if (store->coreData.size != store->content.size()) return false;
						while(offset < store->coreData.size )
						{
							DWORD size = *(DWORD*)&store->content[offset];
							if (size > store->coreData.size)
							{
								printf("[find an error %s:%u,%u]\n",__FILE__,__LINE__,size);
								break;
							}
							Stream sR;
							sR.content.resize(size);
							bcopy(&store->content[offset+sizeof(DWORD)],&sR.content[0],size);
							T object;
							parseStream(object,sR);
							objects.push_back(object);
							offset += sizeof(DWORD) + size;
						}
						return true;
					}break;
				}
			}
			return false;
		}

		/**
		 * ��ȡ point ���� �̳��� Object
		 * \param object ����
		 * \param name ��¼����
		 **/
		template <typename T>
		bool getStore(T* &object,SEARCH_ARG name)
		{
		//	printf("[stores] store object %s\n",name.c_str());
			Store nowstore;
			pickStore(nowstore,name);
			if (nowstore.equal(name))
			{
				Store *store = &nowstore;
				if (store->content.empty()) return false;
				serialize::Stream ss(&store->content[0],store->coreData.size);
				parseStream(object,ss);
			}
			return false;
		}

		/**
		 * ��ȡ������������
		 * \param name ��¼����
		 * \param t ��¼
		 **/
		template <class T>
		bool getStore(T &t,SEARCH_ARG name)
		{
			Store nowstore;
			pickStore(nowstore,name);
			if (nowstore.equal(name))
			{
				Store *store = &nowstore;
				if (store->content.empty()) return false;
				Stream ss(&store->content[0],store->coreData.size);
				parseStream(t,ss);
				return true;
			}
			return false;
		}

		DEC_STORE_CELL_FUNC(int,__DATA__INT__);
		DEC_STORE_CELL_FUNC(unsigned short,__DATA__INT__);
		DEC_STORE_CELL_FUNC(float,__DATA_FLOAT__);
		DEC_STORE_CELL_FUNC(unsigned int ,__DATA_UNSIGNED_INT__);
		DEC_STORE_CELL_FUNC(char,__DATA_CHAR__);
		DEC_STORE_CELL_FUNC(long,__DATA_LONG__);
		DEC_STORE_CELL_FUNC(unsigned char,__DATA_UNSINGED_CHAR__);
		DEC_STORE_CELL_FUNC(unsigned long,__DATA_UNSINGED_LONG__);
		DEC_STORE_CELL_FUNC(long long,__DATA_LONG_LONG__);
		DEC_STORE_CELL_FUNC(unsigned long long,__DATA_UNSINGED_LONG_LONG__);
		/**
		 * ����һ��store
		 * \param value ������ʼ��ַ
		 * \param size ���ݳ���
		 * \param name ������
		 */
		bool addStore(const void *value,unsigned int size,SEARCH_ARG name);
			
	private:
#if (0)
		Stream stream; // ���õ�stream �˰治��
#endif	
		std::vector<Store> stores; // �������õ�store �洢
	};
};
