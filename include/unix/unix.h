#ifndef __UNIX_H__
#define __UNIX_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Daemon {
  int netlinkfd;
} Daemon;

void ForkDaemon(void);

#ifdef __cplusplus
}
#endif

#endif /* __UNIX_H__ */
