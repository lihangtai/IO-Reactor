#include"logger.h"
#include<thread>
#include<memory>
#include<assert.h>
#include"asynclogger.h"
#include"../currentthread.h"
#include<mutex>
#include<string.h>
#include <iostream>

static std::unique_ptr<AsyncLogger> asyncLogger;
static std::once_flag g_once_flag;

thread_local char t_time[64];	//��ǰ�̵߳�ʱ����ַ��������ں�ʱ��
thread_local time_t t_lastSecond;	//��ǰ�̵߳����µ���־��Ϣ������

//��ʼ����־�ȼ�
Logger::LogLevel InitLogLevel()
{
	if(getenv("LGG_DEBUF"))
		return Logger::LogLevel::DEBUG;
	else
		return Logger::LogLevel::INFO;
}

//ȫ�ֱ���:��־�ȼ�
Logger::LogLevel g_LogLevel=InitLogLevel();

// ��ȡȫ����־�ȼ������ǵ�ǰ����ĵȼ�
Logger::LogLevel Logger::GlobalLogLevel()
{
	return g_LogLevel;
}


// Ĭ������������������׼���
void DefaultOutput(const char* msg, int len)
{
	fwrite(msg, 1, len, stdout);
}

// Ĭ�ϳ�ˢ��������ˢ��׼�����
void DefaultFlush()
{
	fflush(stdout);
}

//std::string Logger::logFileName_ = "../li22.log";
std::string Logger::log_file_basename_ = "./li22";	//�����ȥ��ȡ��������ȥ�ϳ�һ����־�ļ���

void OnceInit()
{
	asyncLogger = std::make_unique<AsyncLogger>(Logger::LogFileName(),1024*1024*50);//rollsize=100MB
	asyncLogger->start();
}

void AsyncOutput(const char* logline, int len)
{
	std::call_once(g_once_flag, OnceInit);
	printf("AsyncOutput() call_once...\n");
	asyncLogger->Append(logline, len);
}

// ȫ�ֱ������������
Logger::OutputFunc g_output = AsyncOutput;
// ȫ�ֱ�������ˢ����
Logger::FlushFunc  g_flush = DefaultFlush;

// helper class for known string length at compile time(����ʱ�䣩
class T
{
public:
	T(const char* str, unsigned len)
		:str_(str),
		len_(len)
	{
	}

	const char* str_;
	const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v)
{
	s.Append(v.str_, v.len_);
	return s;
}

//��־�ȼ��ַ������飬���������
const char* g_loglevel_name[static_cast<int>(Logger::LogLevel::NUM_LOG_LEVELS)] =
{
	"TRACE ",
	"DEBUG ",
	"INFO  ",
	"WARN  ",
	"ERROR ",
	"FATAL "
};

//Logger::Logger(const char* basename, int line, Logger::LogLevel level, const char* funcName)
//	:stream_()
//	,level_(level)
//	, basename_(basename)
//	,line_(line)
//	,time_(Timestamp::now())
//{
//	formatTime();	//ʱ�����
//	CurrentThread::tid();	//�����߳�
//	stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
//	stream_ << T(g_loglevel_name[static_cast<int>(level_)], 6);	//��־�ȼ����ַ����Ƕ�����
//
//	stream_ << funcName << "():";
//}
// 
//�������������ж�Ӧ�������
Logger::Logger(const char* FileName, int line, LogLevel level, const char* funcName)
	:impl_(level, FileName,line)
{
	impl_.stream_ << funcName << ' ';
}

Logger::Logger(const char* file, int line)
	:impl_(LogLevel::INFO,file,line)
{
}
Logger::Logger(const char* file, int line, LogLevel level)
	:impl_(level,file,line)
{
}
Logger::Logger(const char* file, int line, bool toAbort)
	:impl_(toAbort?LogLevel::FATAL:LogLevel::ERROR,file,line)
{
}

Logger::~Logger()
{
	//stream_ << " - " << basename_ << " : " << line_ << '\n';
	impl_.finish();
	const LogStream::Buffer& buf(Stream().buffer());
	g_output(buf.Data(), buf.Length());
	std::cout << "create a logger" << std::endl;
}




Logger::Impl::Impl(LogLevel level, const std::string& file, int line)
	: time_(Timestamp::now())
	, stream_()
	, level_(level)
	, line_(line)
	, filename_(file)
{
	formatTime();	//ʱ�����
	CurrentThread::tid();	//�����߳�
	stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());	//����߳�id
	stream_ << T(g_loglevel_name[static_cast<int>(level_)], 6);	//��־�ȼ����ַ����Ƕ�����
}

void Logger::Impl::formatTime()
{
	int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
	//�õ�����
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
	//�õ���΢��
	int microseconds = static_cast<time_t>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
	if (seconds != t_lastSecond) {	//��������ȣ�˵��ҲҪ��ʽ������
		t_lastSecond = seconds;
		//struct tm tm_time;
		//gmtime_r(&seconds, &tm_time);//����ת��Ϊ�������α�׼ʱ�䣬�ͱ���ʱ�䲻һ��
		struct tm tm_time;
		memset(&tm_time, 0, sizeof(tm_time));
		localtime_r(&seconds,&tm_time);

		int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		assert(len == 17);
	}
	char buf[12] = { 0 };
	int lenMicro =sprintf(buf, ".%06d ", microseconds);
	stream_ << T(t_time, 17) << T(buf, lenMicro);
}

void Logger::Impl::finish()
{
	stream_ << " - " << filename_ << ':' << line_ << '\n';
}

// �����������
void Logger::SetOutput(OutputFunc out)
{
	g_output = out;
}
// ���ó�ˢ����
void Logger::SetFlush(FlushFunc flush)
{
	g_flush = flush;
}