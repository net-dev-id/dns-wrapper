/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "win32/service.hpp"
#include "win32/win32daemon.hpp"
#include <tchar.h>

static SERVICE_STATUS_HANDLE g_ServiceStatusHandle = NULL;
static HANDLE g_StopEvent = INVALID_HANDLE_VALUE;
static DWORD g_CurrentState = 0;

static void WINAPI ServiceExecutor(DWORD argc, LPTSTR *argv);
static DWORD WINAPI HandlerEx(DWORD control, DWORD eventType, void *eventData,
                              void *context);
static void ReportStatus(DWORD state);

int ServiceMain(int, char **) {
  char* serviceName = (char*)(DAEMON_NAME);
  SERVICE_TABLE_ENTRY serviceTable[] = {{serviceName, &ServiceExecutor},
                                        {NULL, NULL}};

  if (StartServiceCtrlDispatcher(serviceTable))
    return 0;
  else if (GetLastError() == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
    return -1; // Program not started as a service.
  else
    return -2; // Other error
}

static void WINAPI ServiceExecutor(DWORD, LPTSTR *) {
  g_ServiceStatusHandle =
      RegisterServiceCtrlHandlerEx(DAEMON_NAME, &HandlerEx, NULL);
  ReportStatus(SERVICE_START_PENDING);
  g_StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  Win32Daemon daemon(g_StopEvent);
  daemon.Initialize();
  ReportStatus(SERVICE_RUNNING);
  daemon.Start();
  ReportStatus(SERVICE_STOP_PENDING);
  CloseHandle(g_StopEvent);
  ReportStatus(SERVICE_STOPPED);
}

static DWORD WINAPI HandlerEx(DWORD control, DWORD, void *, void *) {
  switch (control) {
  case SERVICE_CONTROL_SHUTDOWN:
  case SERVICE_CONTROL_STOP:
    ReportStatus(SERVICE_STOP_PENDING);
    SetEvent(g_StopEvent);
    break;
  default:
    ReportStatus(g_CurrentState);
    break;
  }

  return NO_ERROR;
}

static void ReportStatus(DWORD state) {
  g_CurrentState = state;
  SERVICE_STATUS serviceStatus = {
      SERVICE_WIN32_OWN_PROCESS,
      g_CurrentState,
      state == (DWORD) SERVICE_START_PENDING
          ? 0
          : (DWORD)(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN),
      NO_ERROR,
      0,
      0,
      0,
  };
  SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
}
