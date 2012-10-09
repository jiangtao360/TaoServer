#ifndef _LOG_H
#define _LOG_H

#include "header.h"

const int LOG_BUFFER_LEN = 1024;

class Logger
{
public:
	enum LogFacility
	{
		STDOUT,
		STDERR,
		CERTAIN_FILE,  
		SYSLOG,
	};
	enum LogLevel
	{
		CRIT,
		ERROR,
		WARNING,
		INFO,
		DEBUG,
	};

	Logger(LogFacility _facility, LogLevel _level, 
		      const string &_file_name, bool _display_time = false);
	~Logger();
	
	LogLevel SetLogLevel(LogLevel _level);
	LogFacility SetLogFacility(LogFacility _facility);
	void WriteLog(LogLevel _level, int log_errno, const char *format, ...);

private:
	void LogHelper(char * msg, LogLevel _level);
	void SysLogHelper(char * msg, LogLevel _level);
	
private:
	LogFacility facility;
	LogLevel level;
	string file_name;
	bool display_time;
};

extern Logger global_logger;

#endif
