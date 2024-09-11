[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]

# DNS Wrapper Service

>>> Note: This service is under development. Watch here for updates <<<

DNS Wrapper Service is a service that wraps DNS and forwards requests to external DNS based on configured rules. Incoming DNS requests are matched against configured rules. More details will be added soon.

## Flow daigram

![Flow Diagram](https://www.plantuml.com/plantuml/png/bLInRjmw4Epr5GkuELSk7buZ5N6Gn4408qNSfk4THn8HYdlPfUxiV7r0KRBEL8muAYpvoDcT6GxtLgEHE0w-ggQlgghldKO4Sk70WmiT2NwEq4WSwCWIwV__QCT21mwjwqQ1dNw-RghAjRJxKi2-XrPu84C7xn3YkskAFGBjdg3N5L7z6P7mw3HUjX8SM2orB6atgHb0FMkywOtsJKKqTTAqA22dhrKeccNSft9lRvIY6s29nvLWu0p9804dHfQX53XXJY24A4ypIwUlA5nhV1s7FOIuNJfhBhC70WtcCHCe--UdVc_jt82KHpc0N6YP8izIculD6ldu_2-yeY23O5rRhSk9tD4SVc3o8lCH764YB4LcKkRmPrsrvuxEHa8oT82gwT1SbCuwIc85XoiM4phK3mYM1EseI6yzSb1Gv5buKnLLDjBgRnZu13BUpoxfT3nBF0glwZi1jeihlL93CiejiJDbNDLp97CopfkznpP89Tm4Jf_RyCRU_3sLHFKzTmGHbjBvVxMqE9hIriMfzLgkp1RIJNckPPcyiBxh3m8JGG7dkV3DgA1M0BiUnHk2dwT8WApLietEFi-BS-yyQ3TbmHnw6eyQ1MOWXPmWcU6LTrqVEm52VSToDcBN8-4Lyp1XjCbYfj9L4bkCl-nrddzbmxUDmHVppraomMx7uFCCDBCXPUsj-iN7Y85Sc65vw-B2tezBU3QXeL4JoulJaC54nw6XvM9PU4IyQ9HiAAzOzcv2VCo8l24fdP90PhfLJN4JNvyxELtvRpLbkhf5iEFWVwNdDm00)

# Building

This project uses cmake. So it should be possible to build this on any system with properly configured CMake and C++ compiler.

## Dependencies

[Boost](https://www.boost.org/) libraries. Specifically following component libraries from boost are used: system, program_options, log, thread, log_setup.

## Linux/UNIX

On Linux following are the build steps:
1. Configure build: `cmake -DCMAKE_BUILD_TYPE=Debug -S. -Bbuild`
2. Build the project: `make -C build`

This will create an executable at the following location: `build/dns-wrapper`

## Windows

The project can be imported in MS Visual Studio as a CMake project. Visual Studio support building, executing and debugging this project.

This will create executable at the following location: `out\build\x64-Debug\dns-wrapper.exe`

Otherwise cmake along with cl compiler can be used to build it on windows. Specifically the following are the build steps:

1. Configure build: 
   ```
     cmake -Bbuild -S. 
        -DCMAKE_CXX_COMPILER=cl
        -DCMAKE_C_COMPILER=cl
        -DCMAKE_BUILD_TYPE:STRING="Debug"
   ```
2. Build the project:`cmake --build build`

# Configuration

`dns-wrapper` supports the following configuraion:

| Option | Type | Default | Required | Description |
| -- | -- | -- | -- | -- |
| logToConsoleAlso | bool | true | No | Confiures service to log to screen along with logging to file. |
| logFile | string | /var/log/dns-wrapper.log (UNIX) | No | Log file to use. |
| | | c:\temp\dns-wrapper.log | | |
| logLevel | string | info | No | Log level (one of the following: trace, debug, info, warning, error, fatal. |
| dnsPort | number | 53 | No | DNS port to use. |
| pidFile | string | /var/run/dns-wrapper.pid | No | PID file (only applicable on UNIX). |
| serverIp1 | string | 1.1.1.1 | No | Primary DNS Server |
| serverPort1 | number | 53 | No | DNS port to be used for Primary DNS Server. |
| protocol1 | string | udp | No | Protocol for primary DNS server (one of the following: udp, tcp). |
| serverIp2 | string | | No | Additional DNS Server |
| serverPort2 | number | 53 | No | DNS port to be used for additional DNS Server. |
| protocol2 | string | udp | No | Protocol for additional DNS server (one of the following: udp, tcp). |
| serverIp3 | string | | No | Additional DNS Server |
| serverPort3 | number | 53 | No | DNS port to be used for additional DNS Server. |
| protocol3 | string | udp | No | Protocol for additional DNS server (one of the following: udp, tcp). |

All configuration options should be present under section `main`.

## Configuration on UNIX

On UNIX the default location for configuration file is: `/etc/dnswrapper/config.ini`. Command line option `--config` can be specified to configure alternate location.

## Configuration on Windows

On Windows configuration can be done using either configuration file specified using command line option `--config` or using Windows registry. If command line option is not specified configuration is read from Windows Registry `HKEY_LOCAL_MACHINE\SOFTWARE\NetDevId\DnsWrapper\main`.

# Running Service

Dns Wrapper Service is can be executed as long running command line utility (on both UNIX and Windows) or as UNIX Daemon or as a Windows Svc Service.
To run it as UNIX daemon or Windows Svc Service command line option `--daemon` needs to be specified.

## Command line execution
To run as command line execute: `./build/dns-wrapper --config-file config/config.ini`

## Running as UNIX daemon
To run as UNIX daemon execute the command: `./build/dns-wrapper --config-file config/config.ini --daemon`

## Running as Windows Svc Service
### Creating Windows Service
To create service execute as (ADMIN user): `sc create dns-wrapper-service DisplayName= "Dns Wrapper Service" binPath= "<path>\dns-wrapper.exe --daemon"`
### Execute the service

Once service is created as per previous step, start Windows services (services.msc) and click on start for `DNS Wrapper Service`.

![Screenshot](doc/screenshots/WindowsServiceScreenshot.png "Screenshot")

# Testing Service

## UNIX

On UNIX execute the command: `dig +retry=0 @127.0.0.1 +noedns google.com`

```bash
; <<>> DiG 9.20.1 <<>> +retry -p 10053 @127.0.0.1 +noedns google.com
; (1 server found)
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 44976
;; flags: qr rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 0

;; QUESTION SECTION:
;google.com.			IN	A

;; ANSWER SECTION:
google.com.		300	IN	A	142.250.194.46

;; Query time: 100 msec
;; SERVER: 127.0.0.1#10053(127.0.0.1) (UDP)
;; WHEN: Wed Sep 11 18:15:26 IST 2024
;; MSG SIZE  rcvd: 54
```

## Windows

On Powershell execute the command: `Resolve-DnsName -Name google.com -server localhost`
```bash
Name                                           Type   TTL   Section    IPAddress
----                                           ----   ---   -------    ---------
google.com                                     A      275   Answer     142.250.194.46
```

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/net-dev-id/dns-wrapper.svg?style=for-the-badge
[contributors-url]: https://github.com/net-dev-id/dns-wrapper/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/net-dev-id/dns-wrapper.svg?style=for-the-badge
[forks-url]: https://github.com/net-dev-id/dns-wrapper/network/members
[stars-shield]: https://img.shields.io/github/stars/net-dev-id/dns-wrapper.svg?style=for-the-badge
[stars-url]: https://github.com/net-dev-id/dns-wrapper/stargazers
[issues-shield]: https://img.shields.io/github/issues/net-dev-id/dns-wrapper.svg?style=for-the-badge
[issues-url]: https://github.com/net-dev-id/dns-wrapper/issues
[license-shield]: https://img.shields.io/github/license/net-dev-id/dns-wrapper.svg?style=for-the-badge
[license-url]: https://github.com/net-dev-id/dns-wrapper/blob/master/LICENSE
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/neeraj-jakhar-39686212b

