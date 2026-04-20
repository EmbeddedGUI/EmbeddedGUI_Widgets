#include "egui.h"
#include "uicode_disp0.h"

static egui_core_t *s_core;

void uicode_disp0_init(egui_core_t *core)
{
    s_core = core;
    test_init_ui();
}

egui_core_t *uicode_get_core(void)
{
    return s_core;
}
