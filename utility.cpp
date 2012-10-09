#include "utility.h"

void parse_cmdline(int argc, char *argv[])
{
	int opt;

	if(argc == 1)
	{
		return;
	}

	while((opt = getopt(argc, argv, "dh")) != -1)
	{
		switch(opt)
		{
		case 'd':
			if(daemonize() < 0)
			{
				global_logger.WriteLog(Logger::ERROR, -1, "daemonize failed");
			}
			break;

		case 'h':
			print_usage();
			exit (0);
			break;

		default:
			global_logger.WriteLog(Logger::ERROR, -1, "unknown option");
			break;
		}
	}
}

void print_usage()
{
	printf("no option: run in foregrounding\n");
	printf("-d: run in daemon\n");
	printf("-h: print usage\n");
}

int daemonize()
{
	unsigned int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

	umask(0);

	if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
		return -1;

	if((pid = fork()) < 0)
		return -1;
	if(pid != 0)
		exit(0);

	setsid();

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGHUP, &sa, NULL) < 0)
		return -1;

	if((pid = fork()) < 0)
		return -1;	
	if(pid != 0)
		exit(0);

	if(chdir("/"))
		return -1;

	if(rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for(i=0; i< rl.rlim_max; i++)
		close(i);

	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	if(fd0 != 0 || fd1 != 1 || fd2 != 2)
		return -1;

	return 0;
}

int get_profile_string(const char *app, const char *key, 
						 const char *default_str, char *ret_str, unsigned int ret_str_size,  
						 const char *file_path)
{
	FILE *p = fopen(file_path, "r");
	if(p == NULL)
		return -1;

	int ret = 0;
	char buffer[256];

	bool app_found = false;
	while(fgets(buffer, 256, p))
	{
		if(strstr(buffer, "["))
		{
			if(app_found)
				break;

			if(strstr(buffer, app))
			{
				app_found = true;
			}
		}
		else
		{
			if(app_found)
			{
				if(strstr(buffer, key))
				{
					char *pstr = strstr(buffer, "=");
					pstr++;

					int len = strlen(buffer);
					if(buffer[len-2] == '\r')
						len -= 2;
					else if(buffer[len-1] == '\n')
						len--;

					while(len > 0)
					{
						if(buffer[len-1] != ' ')
							break;
						len--;
					}
					while(pstr < buffer+len)
					{
						if(*pstr != ' ')
							break;
						pstr++;
					}
					
					unsigned int value_len = len-(pstr-buffer);
					if(value_len > 0)
					{
						ret = value_len < (ret_str_size-1) ? value_len : (ret_str_size-1);
						strncpy(ret_str, pstr, ret);
						ret_str[ret] = 0;
					}
				}
			}
		}
	}
	if(ret == 0 && default_str != NULL)
	{
		ret = strlen(default_str) < (ret_str_size-1) ? strlen(default_str) : (ret_str_size-1);
		strncpy(ret_str, default_str, ret);
		ret_str[ret] = 0;;
	}

	return ret;
}

int get_processors_num()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}



