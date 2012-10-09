#ifndef _UTILITY_H
#define _UTILITY_H

#include "header.h"
#include "log.h"

#ifndef GNSERVER_CONFIG_PATH
#define GNSERVER_CONFIG_PATH "/etc/gn_server.conf"
#endif

void parse_cmdline(int argc, char *argv[]);

void print_usage();

int daemonize();

int get_profile_string(const char *app, const char *key, 
						 const char *default_str, char *ret_str, unsigned int ret_str_size,  
						 const char *file_path); //根据app部分下的key关键字从配置文件中读取相应信息

int get_processors_num();						 


#endif

