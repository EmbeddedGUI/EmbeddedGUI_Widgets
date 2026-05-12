#ifndef _UICODE_DISP0_H_
#define _UICODE_DISP0_H_

#include "egui.h"
#include "hcw_palette.h"

#ifdef __cplusplus
extern "C" {
#endif

void uicode_disp0_init(egui_core_t *core);
egui_core_t *uicode_get_core(void);
void egui_core_layout_childs_user_root_view(egui_core_t *core, uint8_t is_orientation_horizontal, uint8_t align_type);

extern void test_init_ui(void);

#ifdef __cplusplus
}
#endif

#endif
