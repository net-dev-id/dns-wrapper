/*
 * Created Date: Thursday, August 22nd 2024, 2:41:54 am
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Thursday, 29th August 2024 2:24:24 am
 * Modified By: Neeraj Jakhar
 * -----
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#pragma once

#define CONFIG_FILE_PATH "/etc/dnswrapper/config.ini"
#define LOG_FILE "/var/log/dns-wrapper.log"
#define PID_FILE "/var/run/dns-wrapper.pid"
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
#define EC_INIT_OFFSET 10
