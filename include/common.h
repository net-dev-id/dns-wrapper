/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#define DAEMON_NAME "dns-wrapper"

#ifndef WIN32
#define CONFIG_FILE_PATH "/etc/dnswrapper/config.ini"
#define LOG_FILE "/var/log/dns-wrapper.log"
#define PID_FILE "/var/run/dns-wrapper.pid"
#define RULES_FILE "/etc/dnswrapper/rules.txt"
#else
#define CONFIG_FILE_PATH "__WIN_REGISTRY__"
#define LOG_FILE "c:\\temp\\dns-wrapper.log"
#endif /* WIN32 */

#define MAX_SERVERS 3
#define SERVER_IP_1 "1.1.1.1"
#define DNS_PORT 53
#define UDP_TIMEOUT 10 /* drop UDP queries after TIMEOUT seconds */

#define EC_GOOD 0
#define EC_BADCONF 1
#define EC_BADNET 2
#define EC_FILE 3
#define EC_NOMEM 4
#define EC_MISC 5
#define EC_SIGNAL 6
#define EC_BADCOMMANDLINE 7
#define EC_FORK 8
#define EC_INIT_OFFSET 10

#define ETH_ADDR_LEN 6
