#pragma once
/**
 * email:jjl_2009_hi@163.com
 * Author jijinlong
 * Data 2011.10.14~2012.3.18~~
 * All Rights Resvered
 ***/

#include "sys.h"
#include "stdio.h"
#include "vector"
#include "stdlib.h"
#include "memory.h"
#include "string"
using namespace serialize;
namespace sys{
	/*
	 * �ж������ַ����Ƿ����
	 **/
	struct EqualStr{
		bool operator()(const char*s1,const char *s2)const
		{
			return (s1 == s2) || (s1 && s2 && strcmp(s1, s2) == 0);
		}
	};
	/*
 	 * ����hash ֵ
	 **/
	struct HashStr{
		static int calc(const char* str) ;
		int operator()(const char *str) const
		{
			return calc(str);
		}
	};
	/*
	 * ��װ�ַ�������
	 **/
	struct sString{
		/**
 		 * �ַ���Ĭ�Ϲ��캯��
 		 * */
		DYNAMIC_API sString(){}
		/**
 		 * ʹ���ַ�������
 		 * \param content �ַ���
 		 * */
		DYNAMIC_API sString(const char*content);
		/*
 		 *  ʹ��string ����
 		 *  \param content stl::string
 		 * **/
		DYNAMIC_API sString(std::string& content);
		/*
 		 * תΪGB2312 ����
 		 * **/
		DYNAMIC_API sString toGB2312();
				/*
 		 * ��[text] jjj [\text] ת��Ϊ <text> jjj </text>
 		 * \return �µ��ַ���
 		 * **/
		DYNAMIC_API sString toXmlString();
		
		/*
 		 * �ָ��ַ��� 
 		 * \param str �ָ���"*&&^"
 		 * \param strs ���ַ���
 		 * **/
		DYNAMIC_API void split(const char *str,std::vector<std::string>& strs);
		/*
 		 * ͷβ�ָ��� ȥ��
 		 * */
		DYNAMIC_API void trim();
		
		/*
 		 * �����ݿ�����buf��
 		 * \param buf Ŀ�Ļ���
 		 * \param size �����С
 		 * \return �����Ĵ�С
 		 * **/
		DYNAMIC_API unsigned int toBinary(void * buf,unsigned int size);
		
		/*
 		 * �����ַ�����С
 		 * **/
		DYNAMIC_API unsigned int size();
		
		/*
 		 * תΪΪstd::string
 		 * \return stl ��׼�Ӵ�
 		 * **/
		DYNAMIC_API std::string toString();
		
		/*
 		 * �������������
 		 * \param value ����������
 		 * \param size ���ݳ���
 		 * **/
		DYNAMIC_API static void printBinary(const char *value,int size);
		
		/**
 	 	 * �����ַ���
 	 	 * \param content �Ӵ�
 	 	 * \param size ��С
 	 	 * */
		DYNAMIC_API sString(const char *content,int size);
		
		/**
 		 * ���� �����
 		 * */
		DYNAMIC_API sString &operator = (const sString &content);
		DYNAMIC_API bool operator == (const sString &content);
		DYNAMIC_API bool operator != (const sString &content);
		DYNAMIC_API sString operator +(const sString &content);
		DYNAMIC_API sString& operator += (const sString &content);
		
		/*
 	 	 * �����ַ���ָ��
 	 	 * **/
		DYNAMIC_API const char *str();
		
		/**
 		 * β�� ���� �ַ�
 		 * \param c �ַ�
 		 * */
		DYNAMIC_API void append(char c);
		
		/**
		 * β�� �����ַ���
		 * \param str �ַ���
		 * */
		DYNAMIC_API void append(const char *str);
		
		/**
 		 * ���
 		 * */
		DYNAMIC_API void clear();
		
		std::vector<char> contents; // ����
		int hash_code; // hash ֵ
		
		/**
 		 * \param s1 ԭ�ַ���
 		 * \param s2 Ŀ���ַ���
 		 * \return �Ƿ����
 		 * */
		DYNAMIC_API static bool cmp(const sString & s1,const sString &s2 ) ;
		
		/**
 		 * �����ַ�������
 		 * \param str �ַ���
 		 * \return �ַ�������
 		 * */
		DYNAMIC_API static int mstrlen(const char *str);
		
		/**
 		 * ���� �ַ�������
 		 * \param str �ַ���
 		 * \param size ����ַ�����
 		 * \return ʵ���ַ�������
 		 * */
		DYNAMIC_API static int mstrnlen(const char *str,int size);
		
		/**
 		 * �Ƚ� �� �ַ��� �Ƿ����  
 		 * \param str �ַ���
 		 * \return �Ƿ����
 		 * */
		DYNAMIC_API bool equal(const char *str);
		DYNAMIC_API bool parseRecord(Stream &record);
		DYNAMIC_API Stream toRecord();
		DYNAMIC_API bool operator < (const sString &c) const
		{
			if (c.contents.size() > contents.size()) return false;
			return -1 == strcmp(&c.contents[0],&contents[0]);
		}
		DYNAMIC_API bool operator !=(const sString &i) const
		{
			return 0 != strcmp(&i.contents[0],&contents[0]);

		}
	
		DYNAMIC_API static void split(const char * string,std::vector<std::string>& strings,const char* tag=" ")
		{
			char * buf = new char [strnlen(string,sizeof(short) * 1024)];
            strncpy(buf,string,sizeof(short) * 1024);
            char *nexttoken =NULL;
			char *token = NULL;
			token = strtok_s(buf,tag,&nexttoken);
			while ( token != NULL)
			{
				strings.push_back(token);
               // printf("<%s>",token);
				token = strtok_s(nexttoken,tag,&nexttoken);
			}
            delete [] buf;
		}
        DYNAMIC_API static void split(char * string,std::vector<char*>& strings,const char* tag=" ")
		{
            char *token = NULL;
			 char *nexttoken =NULL;
			token = strtok_s(string,tag,&nexttoken);
			while ( token != NULL)
			{
				strings.push_back(token);
				token = strtok_s(nexttoken,tag,&nexttoken);
			}
		}

		DYNAMIC_API static const char*replace(const char *str,const char src,const char dest)
		{
			char *temp = new char[strnlen(str,1024)];
			strncpy(temp,str,1024);
			char *temp1 = temp;
			while (*temp1)
			{
				if (*temp1 == src)
					*temp1 = dest;
				temp1++;
			}
			return temp;
		} 
	};
}; // end namespace sys
