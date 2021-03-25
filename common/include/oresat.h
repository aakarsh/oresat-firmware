#ifndef _ORESAT_H_
#define _ORESAT_H_

#include "ch.h"
#include "hal.h"
#include "worker.h"

#define CO_USE_GLOBALS
#define CO_DRIVER_CUSTOM
#define ORESAT_DEFAULT_ID 0
#define ORESAT_DEFAULT_BITRATE 1000

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    CANDriver *cand;
    uint8_t node_id;
    uint16_t bitrate;
} oresat_config_t;

/* OreSat initialization and main process */
void oresat_init(oresat_config_t *config);
void oresat_start(void);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif
