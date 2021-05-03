#ifndef _TEST_CANOPEN_H_
#define _TEST_CANOPEN_H_

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void cmd_nmt(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_sdo(BaseSequentialStream *chp, int argc, char *argv[]);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif
