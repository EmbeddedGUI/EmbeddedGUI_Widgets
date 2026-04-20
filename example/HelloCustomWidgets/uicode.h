#ifndef _UICODE_H_
#define _UICODE_H_

#include "egui.h"
#include "sdk_compat.h"
#include "uicode_disp0.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

void uicode_create_ui(void);
egui_core_t *uicode_get_core(void);

extern void test_init_ui(void);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* _UICODE_H_ */
