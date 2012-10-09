#include "log.h"

Logger::Logger(LogFacility _facility, 
				 LogLevel _level, const string &_file_name, bool _display_time) :
				 facility(_facility), level(_level),
				 file_name(_file_name), display_time(_display_time)
{
	if(facility == SYSLOG)
	{
		openlog("gnserver", LOG_PID | LOG_CONS, LOG_USER);
	}
}

Logger::~Logger()
{

}

Logger::LogLevel Logger::SetLogLevel(LogLevel _level)
{
	LogLevel last = level;
	
	level = _level;

	return last;
}

Logger::LogFacility Logger::SetLogFacility(LogFacility _facility)
{
	LogFacility last = facility;
	
	if(_facility == SYSLOG)
		openlog("gnserver", LOG_PID | LOG_CONS, LOG_USER);
	
	facility = _facility;

	return last;
}

void Logger::WriteLog(LogLevel _level, int log_errno, const char * format, ...)
{
	if(_level > level)
		return;
	
	char buf[LOG_BUFFER_LEN];
	
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, LOG_BUFFER_LEN, format, ap);
	buf[LOG_BUFFER_LEN-1] = 0;
	va_end(ap);

	if(log_errno >= 0)
	{
		int len = strlen(buf);
		if(len < LOG_BUFFER_LEN-3)
			snprintf(buf+len, LOG_BUFFER_LEN-len, ": %s", strerror(log_errno));
	}

	LogHelper(buf, _level);

}

void Logger::LogHelper(char * msg, LogLevel _level)
{
	if(facility == SYSLOG)
	{
		SysLogHelper(msg, _level);
		return;
	}
	
	const char *level_str = NULL;
	bool is_exit = false;
	switch (_level)
	{
		case CRIT:
			level_str = "critical";
			is_exit = true;
			break;
		case ERROR:
			level_str = "error";
			is_exit = true;
			break;
		case WARNING:
			level_str = "warning";
			break;
		case INFO:
			level_str = "information";
			break;
		case DEBUG:
			level_str = "debug";
			break;
		default:
			level_str = "???";
			break;
	}

	char time_str[32];
	if(display_time)
	{
		time_t now = time(NULL);
		struct tm *now_tm = localtime(&now);
		strftime(time_str, 32, "%Y/%m/%d %H:%M:%S", now_tm);
	}
	else
		strcpy(time_str, "");

	FILE *pfile = NULL;
	switch (facility)
	{
		case STDOUT:
			fprintf(stdout, "%s[%s] %s\n", time_str, level_str, msg);
			break;
		case STDERR:
			fprintf(stderr, "%s[%s] %s\n", time_str, level_str, msg);
			break;
		case CERTAIN_FILE:
			pfile = fopen(file_name.c_str(), "wa");
			fprintf(stdout, "%s[%s] %s\n", time_str, level_str, msg);
			fclose(pfile);
			break;
		default:
			break;
	}

	if(is_exit)
		exit (-1);
}

void Logger::SysLogHelper(char * msg, LogLevel _level)
{
	bool is_exit = false;
	
	switch (_level)
	{
		case CRIT:
			syslog(LOG_CRIT, "%s", msg);
			is_exit = true;
			break;
		case ERROR:
			syslog(LOG_ERR, "%s", msg);
			is_exit = true;
			break;
		case WARNING:
			syslog(LOG_WARNING, "%s", msg);
			break;
		case INFO:
			syslog(LOG_INFO, "%s", msg);
			break;
		case DEBUG:
			syslog(LOG_DEBUG, "%s", msg);
			break;
		default:
			break;
	}

	if(is_exit)
		exit (-1);
}

Logger global_logger(Logger::STDOUT, Logger::ERROR, "./log");


