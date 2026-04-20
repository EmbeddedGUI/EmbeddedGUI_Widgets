#ifndef _UICODE_DISP0_H_
#define _UICODE_DISP0_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

void uicode_disp0_init(egui_core_t *core);
egui_core_t *uicode_get_core(void);

#ifdef __cplusplus
}
#endif

#endif
