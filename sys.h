/**
 * email:jjl_2009_hi@163.com
 * Author jijinlong
 * Data 2011.10.14~2012.3.18~~
 * All Rights Resvered
 ***/
/**
 * ���ļ���ƽ̨��ص������ṩһ�µķ��ʽӿ�,����
 * �̵߳�ʹ��
 * �����ʹ��
 * ʱ���ʹ��
 * �����ʹ��
 * */

#pragma once
#include "list"
#include "vector"
#include "string"
#include <stdarg.h>
#include "stdio.h"
#include <time.h>
#include "stdlib.h"
#include "errno.h"
#include "set"
#include "port.h"

#define MAX_SELECT_COUNT 1000
#define MAX_EPOLL_COUNT 4000
#define BEGIN_TAG(name) 

namespace serialize{
	struct Stream;
	class Stores;
}
namespace sys{
	/*
	* bytes
	**/
	typedef std::vector<unsigned char> BYTES;
	typedef std::vector<unsigned char>::iterator BYTES_ITER;
	/**
	 * ��װʱ��ֵ
	 */
	struct stTimeValue{
		time_t second; // ����1970 �������
		unsigned int msecond; // ������
		stTimeValue()
		{
			second = 0;
			msecond = 0;
		}
		bool operator < (stTimeValue & value)
		{
			if (second < value.second) return true;
			if (second == value.second)
			{
				if (msecond < value.msecond) return true;
			}
			return false;
		}
		bool operator > (stTimeValue &value)
		{
			if (second > value.second) return true;
			if (second == value.second)
			{
				if (msecond > value.msecond) return true;
			}
			return false;
		}
		bool operator <= (stTimeValue & value)
		{
			if (second <= value.second) return true;
			if (msecond <= value.msecond) return true;
			return false;
		}
		bool operator != (stTimeValue & value)
		{
			if (second != value.second || msecond != value.msecond) return true;
			return false;
		}
		bool operator == (stTimeValue & value)
		{
			if (second == value.second && msecond == value.msecond) return true;
			return false;
		}
		bool operator >= (stTimeValue & value)
		{
			if (second >= value.second) return true;
			if (msecond >= value.msecond) return true;
			return false;
		}
		stTimeValue & operator = (int value)
		{
			second = value / 1000;
			msecond = value % 1000;
			return *this;
		}
		DYNAMIC_API bool parseRecord(serialize::Stream &record);
		DYNAMIC_API serialize::Stream toRecord();
		DYNAMIC_API bool parseRecord(serialize::Stores& sts);
		/**
		 * ���ӵĺ�����
		 */
		stTimeValue operator+(unsigned int value)
		{
			stTimeValue result;
			result.second = second + value / 1000;
			result.msecond = msecond + value % 1000;
			result.second += result.msecond  / 1000;
			result.msecond = result.msecond  % 1000;
			return result;
		}
		stTimeValue  operator+(stTimeValue &value)
		{
			stTimeValue result;
			result.second = second + value.second;
			result.msecond = msecond + value.msecond;
			result.second += result.msecond / 1000;
			result.msecond = result.msecond % 1000;
			return result;
		}
		stTimeValue  operator-(stTimeValue &value)
		{
			stTimeValue result;
			result.second = second - value.second;
			result.msecond = msecond - value.msecond;
			result.second += result.second + result.msecond / 1000;
			result.msecond = result.msecond % 1000;
			return result;
		}
		stTimeValue & operator=(const stTimeValue &value)
		{
			second = value.second;
			msecond = value.msecond;
			return *this;
		}
		/**
		 * ����
		 */
		/*stTimeValue & operator=(int value)
		{
			second = value;
			return *this;
		}*/
	};
	/*
	 * ��ʱ��
	 */
	struct sTime{
		/**
 		 * ��ȡ��ǰʱ��
 		 * \return ������
 		 * */
		DYNAMIC_API static stTimeValue getNowTime();
		/**
		 * ��ȡ��ǰ����
		 * \return ��ǰ����
		 **/
		DYNAMIC_API static time_t getNowSec();
		/**
 		 * ʹ�ü�����캯��
 		 * \param tick ʱ����
 		 * */
		DYNAMIC_API sTime(int tick)
		{
			startTime = getNowTime();
			this->tick = tick;
		}
		DYNAMIC_API sTime()
		{
			tick = 0;
		}
		
		/**
 		 * ��鵱ǰ��ʱ�Ƿ�ʱ
 		 * */
		DYNAMIC_API bool checkOver();
		/**
 		 * �ȴ� time ʱ��
 		 * \param �ȴ�ʱ�� 
 		 ***/
		static void wait(unsigned int time);
		
		int tick; // ��ʱ��ʱ����
		stTimeValue startTime; // ��ʱ����ʱ��,tick �������
		static long long debugNowTime;
	};
	/**
 	 * ����ʱ��
 	 * */
	class AynaTime{
	public:
		/**
 		 * ����ʱ�� 
 		 * \param ����
 		 * */
		DYNAMIC_API AynaTime(sys::stTimeValue nowTime)
		{
			ayna(nowTime.second);	
		}
        DYNAMIC_API AynaTime()
        {
            ayna(time(NULL));
        }
		/*
 		 * ����ʱ�� �õ� ����
 		 * \param nowTime ����
         * \ ʱ�䶼�Ǵ�1 ��ʼ����
 		 * **/
		DYNAMIC_API void ayna(time_t nowTime);
		unsigned short week; // ���ڼ� ��1��ʼ����
		unsigned short day; // ����
		unsigned short year; // ���� 19xx
		unsigned short month; // �·� ��1 ��ʼ����
		unsigned int sec; // ����
		unsigned int mday; // ����
		unsigned int hour; // ��ʱ
		unsigned int min; // ����
        
        /**
         * ���ַ�������Ϊ��ǰ����
         **/
        DYNAMIC_API static unsigned int parseTime(const char *content);
		/**
		 * ��ǰʱ��ת��Ϊ����
		 */
		DYNAMIC_API std::string toString();
	};
	/*
	 * ��������� 
	 **/
	struct sRandom{
		/**
 		 * �Ƿ��Ǹ���
 		 * \param value ����ֵ 
 		 * \return �Ƿ� > value % 100
 		 * */
		static bool isPercent(unsigned int value);
		
		/*
 		 * ���� min max ֮��������
 		 * \return �����
 		 * **/
		static unsigned int randBetween(unsigned int min,unsigned int max);
	};
	/*
 	* \brief ��ʱ���ص�
	**/	
	struct stTimeTickCallback{
		/*
 		 * ��ʱ���ص�
 		 * \param delay ��ǰtick ʱ��
 		 * **/
		virtual void tick(unsigned int delay){}
	};
	/**
 	 * �߳�ʵ��
 	 * */
	class ThreadManager;
	class Thread{
		friend class ThreadManager;
	public:
		/**
 		 * �߳�ֹͣ
 		 * */
		virtual void stop(){_stop = true;wait();}
		/**
 		 * �߳̿���
 		 * */
		bool start(bool wait = true);
		/**
 		 * �ȴ��߳̽���
 		 * */
		void wait();
		/**
 		 * �߳�����
 		 * */
		virtual void run(){}
		/**
 		 * ����߳��Ƿ�������
 		 * */
		bool isActive(){return !_stop;}
		/**
 		 * �̹߳��캯��
 		 * */
		Thread(){_stop = true;}
		~Thread(){}
	private:
		/**
 		 * �߳���������
 		 * */
		static void * threadWork(void*arg);
#if __LINUX__ || __MAC__
		pthread_t id; // �߳�di
#else
		void* hThread;
		int id;
#endif
	protected: 
		bool _stop; // ��ʾ�Ƿ�����
	};
	/**
	* \brief �ٽ�������װ��ϵͳ�ٽ�����������ʹ��ϵͳ�ٽ���ʱ����Ҫ�ֹ���ʼ���������ٽ�������Ĳ���
	*
	*/
	class Mutex 
	{

	public:
		DYNAMIC_API Mutex() ;

		/**
		* \brief ��������������һ�����������
		*
		*/
		DYNAMIC_API ~Mutex();

		/**
		* \brief ����һ��������
		*
		*/
		DYNAMIC_API inline void lock();

		/**
		* \brief ����һ��������
		*
		*/
		DYNAMIC_API inline void unlock();
#if __LINUX__ || __MAC__

		pthread_mutex_t _mutex;
#else
		void* m_hMutex;    /**< ϵͳ������ */
#endif
	};
	class ThreadManager{
	public:
		static ThreadManager *threadM;
		DYNAMIC_API static ThreadManager& getMe()
		{
			return *threadM;
		}
		DYNAMIC_API static void setThreadManager(ThreadManager* tm)
		{
			threadM = tm;
		}
		/**
		 * �ȴ��߳̽��� �����������
 		 * */
		DYNAMIC_API void waitAll();
		/**
 		 * ֹͣ�����߳�
 		 * */
		DYNAMIC_API void stopAll();
		DYNAMIC_API void addThread(Thread * thread);
		DYNAMIC_API ThreadManager(){
		}
	private:
		std::set<Thread*> threads;
		
		Mutex _mutex;
	};
    /**
     * ����ip ��ַ
     **/
    class IpUtil{
    public:
        /**
         * ����Ψһ���� ��ȡ IP
         **/
        static std::string getIP(std::string uniqueName);
        /**
         * ����Ψһ���� ��ȡPort
         **/
        static unsigned short getPort(std::string uniqueName);
        /**
         * ʹ��Ψһ���ƽ�����ip.port
         **/
        IpUtil(std::string unqiueName);
        std::string ip; // ip��ַ
        unsigned short port; // �˿�
		static void split(const char * string,std::vector<std::string>& strings,const char* tag=" ")
		{
			char *nexttoken = NULL;
			char buf[1024] = {'\0'};
			strncpy(buf,string,1024);
			char *token = strtok_s(buf,tag,&nexttoken);
			while ( token != NULL)
			{
				strings.push_back(token);
				token = strtok_s(nexttoken,tag,&nexttoken);
			}
		}
    
	};
	DYNAMIC_API extern sys::ThreadManager theThreadManager;
	

	class Socket{
	public:
		Socket();
		~Socket();
		int send(void *buf,int size);
		int recv(void *buf,int size);
		void close();
		void shutdown();
		int connect(const sockaddr *name,int namelen);
#if __LINUX__ || __MAC__
		int getHandle(){return _socket;}
		void setHandle(int _socket)
		{
			if (checkValid())
			{
				close();
			}
			this->_socket = _socket;
		}
#else
		unsigned int getHandle(){return _socket;}
		void setHandle(SOCKET _socket)
		{
			if (checkValid())
			{
				close();
			}
			this->_socket = _socket;
		}
#endif
		void bind(const sockaddr *name,int namelen);
		void listen(int size);
		bool checkValid();
		void setnonblocking();
	private:
#if __LINUX__ || __MAC__
		int _socket;
#else
		unsigned int _socket;
#endif
	};
	
	/**
 	 * �ṩһ���򵥵���־���,ϵͳ
 	 *  ֻ��ӡ���ļ��ڣ��ļ�������洢
 	 * */
	class Logger{
	public:
		/**
 		 * \param ��־��
 		 * */
		Logger(const char *name);
		/**
 		 * �����ͷ�
 		 * */
		~Logger();
		/**
 	 	 * ��ӡ������
 	 	 * \param pattern ��ʽ���ַ���
 	 	 * */
		void trace(const char * pattern,...);
		/**
 		 * ����ڴ滺��
 		 * */
		void flush();
	private:
		unsigned int _yday; // ������
		std::string _loggerFile; // ��־����
		unsigned int _bufferSize; // ��־�����С
		std::vector<std::string> _contents; // ��־����
		FILE * _logFileHandle; // ��־�ļ����
	};
	/**
	 * ��ӡ���뺯��
	 */
	class EnterFunction{
	public:
		EnterFunction(std::string funcName,std::string fileName,int line)
		{
			this->funcName = funcName;
			this->fileName = fileName;
			this->line = line;
			printf("\n enter [%s] \n", funcName.c_str());
		}

		~EnterFunction()
		{
			printf("\n leave [%s] \n", funcName.c_str());
		}
		std::string fileName;
		std::string funcName;
		int line;
	};
#define ENTER_FUNCTION\
	sys::EnterFunction function##__LINE__(__FUNCTION__,__FILE__,__LINE__);
	
	/**
	 * ϵͳ����ŵ��ű��Ľӿ�,��Ҫ��ʱ�� �� ���
	 */
	class SysLuaLib{
	public:
#ifdef __USE_LUA_EXT
		static void addLibExt(lua_State* L);
#endif
	};
};
