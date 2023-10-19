
#ifndef _REG_CONFIG_H_
#define _REG_CONFIG_H_

#include <stdint.h>

#include "ipmiDef.h"
#include "drvMch.h"

#include "epicsStdlib.h"
#include "aiRecord.h"

#ifdef __cplusplus
extern "C" {
#endif

void regConfInit();

void checkSensThresh(SdrFull fullSens, Sensor sens, aiRecord* pai);

#ifdef __cplusplus
}
#endif

#endif // _REG_CONFIG_H_
