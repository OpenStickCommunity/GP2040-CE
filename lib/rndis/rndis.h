#ifndef RNDIS_H_
#define RNDIS_H_

#ifdef __cplusplus
extern "C" {
#endif

int rndis_init(const char *hostname);
void rndis_task(void);

#ifdef __cplusplus
}
#endif

#endif
