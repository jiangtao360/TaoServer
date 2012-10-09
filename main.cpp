#include "tao_server.h"
#include "utility.h"

int main(int argc, char *argv[])
{
	parse_cmdline(argc, argv);
	
	int signo;
	sigset_t set;
	
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGPIPE);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	
	TaoServer tao;
	tao.Initialize();
	tao.Startup();

	
	while(true)
	{
		if(sigwait(&set, &signo) != 0)
		{						
			global_logger.WriteLog(Logger::ERROR, errno, 
				"Wait signal failed");
		}
	
		switch(signo)
		{
			case SIGALRM:
				global_logger.WriteLog(Logger::WARNING, -1, 
					"receive signal SIGALRM");
				exit (0);
				break;
			case SIGINT:
				global_logger.WriteLog(Logger::WARNING, -1, 
					"receive signal SIGINT");
				exit (0);
				break;
			case SIGPIPE:
				global_logger.WriteLog(Logger::WARNING, -1, 
					"receive signal SIGPIPE");
				exit (0);
				break;
			case SIGTERM:
				global_logger.WriteLog(Logger::WARNING, -1, 
					"receive signal SIGTERM");
				exit (0);
				break;
			default:
				global_logger.WriteLog(Logger::ERROR, -1, 
					"receive unknowed signal");
		}
	}


	return 0;
}
