#ifndef _EMBEDDEDGUI_WIDGETS_SDK_COMPAT_H_
#define _EMBEDDEDGUI_WIDGETS_SDK_COMPAT_H_

#include "egui.h"

egui_core_t *uicode_get_core(void);
void egui_core_layout_childs_user_root_view(egui_core_t *core, uint8_t is_orientation_horizontal, uint8_t align_type);

#define EGUI_SDK_COMPAT_SELECT_2(_1, _2, _name, ...) _name
#define EGUI_SDK_COMPAT_SELECT_3(_1, _2, _3, _name, ...) _name
#define EGUI_SDK_COMPAT_SELECT_4(_1, _2, _3, _4, _name, ...) _name
#define EGUI_SDK_COMPAT_SELECT_5(_1, _2, _3, _4, _5, _name, ...) _name
#define EGUI_SDK_COMPAT_SELECT_6(_1, _2, _3, _4, _5, _6, _name, ...) _name
#define EGUI_SDK_COMPAT_SELECT_7(_1, _2, _3, _4, _5, _6, _7, _name, ...) _name
#define EGUI_SDK_COMPAT_SELECT_8(_1, _2, _3, _4, _5, _6, _7, _8, _name, ...) _name
#define EGUI_SDK_COMPAT_SELECT_9(_1, _2, _3, _4, _5, _6, _7, _8, _9, _name, ...) _name

__EGUI_STATIC_INLINE__ egui_canvas_t *egui_sdk_compat_get_canvas(void)
{
    egui_core_t *core = uicode_get_core();

    return core != NULL ? &core->canvas : NULL;
}

#define EGUI_SDK_COMPAT_INIT_1(_fn, _self) (_fn)((_self), uicode_get_core())
#define EGUI_SDK_COMPAT_INIT_2(_fn, _self, _core) (_fn)((_self), (_core))
#define EGUI_SDK_COMPAT_CALL_INIT(_fn, ...) EGUI_SDK_COMPAT_SELECT_2(__VA_ARGS__, EGUI_SDK_COMPAT_INIT_2, EGUI_SDK_COMPAT_INIT_1)(_fn, __VA_ARGS__)

#define EGUI_SDK_COMPAT_INIT_WITH_PARAMS_2(_fn, _self, _params) (_fn)((_self), uicode_get_core(), (_params))
#define EGUI_SDK_COMPAT_INIT_WITH_PARAMS_3(_fn, _self, _core, _params) (_fn)((_self), (_core), (_params))
#define EGUI_SDK_COMPAT_CALL_INIT_WITH_PARAMS(_fn, ...) \
    EGUI_SDK_COMPAT_SELECT_3(__VA_ARGS__, EGUI_SDK_COMPAT_INIT_WITH_PARAMS_3, EGUI_SDK_COMPAT_INIT_WITH_PARAMS_2)(_fn, __VA_ARGS__)

#define EGUI_SDK_COMPAT_LAYOUT_ROOT_2(_layout, _align) egui_core_layout_childs_user_root_view(uicode_get_core(), (_layout), (_align))
#define EGUI_SDK_COMPAT_LAYOUT_ROOT_3(_core, _layout, _align) egui_core_layout_childs_user_root_view((_core), (_layout), (_align))
#define EGUI_SDK_COMPAT_CALL_LAYOUT_ROOT(...) \
    EGUI_SDK_COMPAT_SELECT_3(__VA_ARGS__, EGUI_SDK_COMPAT_LAYOUT_ROOT_3, EGUI_SDK_COMPAT_LAYOUT_ROOT_2)(__VA_ARGS__)

#define egui_core_layout_childs_user_root_view(...) EGUI_SDK_COMPAT_CALL_LAYOUT_ROOT(__VA_ARGS__)

#define EGUI_SDK_COMPAT_TIMER_START_3(_handle, _ms, _period) egui_timer_start_timer(uicode_get_core(), (_handle), (_ms), (_period))
#define EGUI_SDK_COMPAT_TIMER_START_4(_core, _handle, _ms, _period) egui_timer_start_timer((_core), (_handle), (_ms), (_period))
#define EGUI_SDK_COMPAT_CALL_TIMER_START(...) \
    EGUI_SDK_COMPAT_SELECT_4(__VA_ARGS__, EGUI_SDK_COMPAT_TIMER_START_4, EGUI_SDK_COMPAT_TIMER_START_3)(__VA_ARGS__)

#define EGUI_SDK_COMPAT_TIMER_STOP_1(_handle) egui_timer_stop_timer(uicode_get_core(), (_handle))
#define EGUI_SDK_COMPAT_TIMER_STOP_2(_core, _handle) egui_timer_stop_timer((_core), (_handle))
#define EGUI_SDK_COMPAT_CALL_TIMER_STOP(...) EGUI_SDK_COMPAT_SELECT_2(__VA_ARGS__, EGUI_SDK_COMPAT_TIMER_STOP_2, EGUI_SDK_COMPAT_TIMER_STOP_1)(__VA_ARGS__)

#define EGUI_SDK_COMPAT_TIMER_CHECK_1(_handle) egui_timer_check_timer_start(uicode_get_core(), (_handle))
#define EGUI_SDK_COMPAT_TIMER_CHECK_2(_core, _handle) egui_timer_check_timer_start((_core), (_handle))
#define EGUI_SDK_COMPAT_CALL_TIMER_CHECK(...) \
    EGUI_SDK_COMPAT_SELECT_2(__VA_ARGS__, EGUI_SDK_COMPAT_TIMER_CHECK_2, EGUI_SDK_COMPAT_TIMER_CHECK_1)(__VA_ARGS__)

#define egui_timer_start_timer(...) EGUI_SDK_COMPAT_CALL_TIMER_START(__VA_ARGS__)
#define egui_timer_stop_timer(...) EGUI_SDK_COMPAT_CALL_TIMER_STOP(__VA_ARGS__)
#define egui_timer_check_timer_start(...) EGUI_SDK_COMPAT_CALL_TIMER_CHECK(__VA_ARGS__)

#define egui_focus_manager_clear_focus() egui_focus_manager_clear_focus(uicode_get_core())
#define egui_canvas_get_mask() egui_canvas_get_mask(egui_sdk_compat_get_canvas())
#define egui_canvas_set_mask(_mask) egui_canvas_set_mask(egui_sdk_compat_get_canvas(), (_mask))
#define egui_canvas_get_extra_clip() egui_canvas_get_extra_clip(egui_sdk_compat_get_canvas())
#define egui_canvas_set_extra_clip(_clip_region) egui_canvas_set_extra_clip(egui_sdk_compat_get_canvas(), (_clip_region))
#define egui_canvas_clear_extra_clip() egui_canvas_clear_extra_clip(egui_sdk_compat_get_canvas())
#define egui_canvas_is_region_active(_region) egui_canvas_is_region_active(egui_sdk_compat_get_canvas(), (_region))

#define EGUI_SDK_COMPAT_CANVAS_OLD_3(_fn, _1, _2, _3) (_fn)(egui_sdk_compat_get_canvas(), (_1), (_2), (_3))
#define EGUI_SDK_COMPAT_CANVAS_NEW_4(_fn, _0, _1, _2, _3) (_fn)((_0), (_1), (_2), (_3))
#define EGUI_SDK_COMPAT_CALL_CANVAS_3(_fn, ...) EGUI_SDK_COMPAT_SELECT_4(__VA_ARGS__, EGUI_SDK_COMPAT_CANVAS_NEW_4, EGUI_SDK_COMPAT_CANVAS_OLD_3)(_fn, __VA_ARGS__)

#define EGUI_SDK_COMPAT_CANVAS_OLD_4(_fn, _1, _2, _3, _4) (_fn)(egui_sdk_compat_get_canvas(), (_1), (_2), (_3), (_4))
#define EGUI_SDK_COMPAT_CANVAS_NEW_5(_fn, _0, _1, _2, _3, _4) (_fn)((_0), (_1), (_2), (_3), (_4))
#define EGUI_SDK_COMPAT_CALL_CANVAS_4(_fn, ...) EGUI_SDK_COMPAT_SELECT_5(__VA_ARGS__, EGUI_SDK_COMPAT_CANVAS_NEW_5, EGUI_SDK_COMPAT_CANVAS_OLD_4)(_fn, __VA_ARGS__)

#define EGUI_SDK_COMPAT_CANVAS_OLD_5(_fn, _1, _2, _3, _4, _5) (_fn)(egui_sdk_compat_get_canvas(), (_1), (_2), (_3), (_4), (_5))
#define EGUI_SDK_COMPAT_CANVAS_NEW_6(_fn, _0, _1, _2, _3, _4, _5) (_fn)((_0), (_1), (_2), (_3), (_4), (_5))
#define EGUI_SDK_COMPAT_CALL_CANVAS_5(_fn, ...) EGUI_SDK_COMPAT_SELECT_6(__VA_ARGS__, EGUI_SDK_COMPAT_CANVAS_NEW_6, EGUI_SDK_COMPAT_CANVAS_OLD_5)(_fn, __VA_ARGS__)

#define EGUI_SDK_COMPAT_CANVAS_OLD_6(_fn, _1, _2, _3, _4, _5, _6) (_fn)(egui_sdk_compat_get_canvas(), (_1), (_2), (_3), (_4), (_5), (_6))
#define EGUI_SDK_COMPAT_CANVAS_NEW_7(_fn, _0, _1, _2, _3, _4, _5, _6) (_fn)((_0), (_1), (_2), (_3), (_4), (_5), (_6))
#define EGUI_SDK_COMPAT_CALL_CANVAS_6(_fn, ...) EGUI_SDK_COMPAT_SELECT_7(__VA_ARGS__, EGUI_SDK_COMPAT_CANVAS_NEW_7, EGUI_SDK_COMPAT_CANVAS_OLD_6)(_fn, __VA_ARGS__)

#define EGUI_SDK_COMPAT_CANVAS_OLD_7(_fn, _1, _2, _3, _4, _5, _6, _7) (_fn)(egui_sdk_compat_get_canvas(), (_1), (_2), (_3), (_4), (_5), (_6), (_7))
#define EGUI_SDK_COMPAT_CANVAS_NEW_8(_fn, _0, _1, _2, _3, _4, _5, _6, _7) (_fn)((_0), (_1), (_2), (_3), (_4), (_5), (_6), (_7))
#define EGUI_SDK_COMPAT_CALL_CANVAS_7(_fn, ...) EGUI_SDK_COMPAT_SELECT_8(__VA_ARGS__, EGUI_SDK_COMPAT_CANVAS_NEW_8, EGUI_SDK_COMPAT_CANVAS_OLD_7)(_fn, __VA_ARGS__)

#define EGUI_SDK_COMPAT_CANVAS_OLD_8(_fn, _1, _2, _3, _4, _5, _6, _7, _8) \
    (_fn)(egui_sdk_compat_get_canvas(), (_1), (_2), (_3), (_4), (_5), (_6), (_7), (_8))
#define EGUI_SDK_COMPAT_CANVAS_NEW_9(_fn, _0, _1, _2, _3, _4, _5, _6, _7, _8) (_fn)((_0), (_1), (_2), (_3), (_4), (_5), (_6), (_7), (_8))
#define EGUI_SDK_COMPAT_CALL_CANVAS_8(_fn, ...) EGUI_SDK_COMPAT_SELECT_9(__VA_ARGS__, EGUI_SDK_COMPAT_CANVAS_NEW_9, EGUI_SDK_COMPAT_CANVAS_OLD_8)(_fn, __VA_ARGS__)

#undef egui_canvas_draw_circle
#undef egui_canvas_draw_circle_fill
#if EGUI_CONFIG_CIRCLE_DEFAULT_ALGO_HQ
#define egui_canvas_draw_circle(...) EGUI_SDK_COMPAT_CALL_CANVAS_6(egui_canvas_draw_circle_hq, __VA_ARGS__)
#define egui_canvas_draw_circle_fill(...) EGUI_SDK_COMPAT_CALL_CANVAS_5(egui_canvas_draw_circle_fill_hq, __VA_ARGS__)
#else
#define egui_canvas_draw_circle(...) EGUI_SDK_COMPAT_CALL_CANVAS_6(egui_canvas_draw_circle_basic, __VA_ARGS__)
#define egui_canvas_draw_circle_fill(...) EGUI_SDK_COMPAT_CALL_CANVAS_5(egui_canvas_draw_circle_fill_basic, __VA_ARGS__)
#endif

#define egui_canvas_draw_circle_basic(...) EGUI_SDK_COMPAT_CALL_CANVAS_6(egui_canvas_draw_circle_basic, __VA_ARGS__)
#define egui_canvas_draw_circle_fill_basic(...) EGUI_SDK_COMPAT_CALL_CANVAS_5(egui_canvas_draw_circle_fill_basic, __VA_ARGS__)
#define egui_canvas_draw_image_resize(...) EGUI_SDK_COMPAT_CALL_CANVAS_5(egui_canvas_draw_image_resize, __VA_ARGS__)
#define egui_canvas_draw_line(...) EGUI_SDK_COMPAT_CALL_CANVAS_7(egui_canvas_draw_line, __VA_ARGS__)
#define egui_canvas_draw_line_round_cap_hq(...) EGUI_SDK_COMPAT_CALL_CANVAS_7(egui_canvas_draw_line_round_cap_hq, __VA_ARGS__)
#define egui_canvas_draw_point(...) EGUI_SDK_COMPAT_CALL_CANVAS_4(egui_canvas_draw_point, __VA_ARGS__)
#define egui_canvas_draw_polygon(...) EGUI_SDK_COMPAT_CALL_CANVAS_5(egui_canvas_draw_polygon, __VA_ARGS__)
#define egui_canvas_draw_polygon_fill(...) EGUI_SDK_COMPAT_CALL_CANVAS_4(egui_canvas_draw_polygon_fill, __VA_ARGS__)
#define egui_canvas_draw_polyline_round_cap_hq(...) EGUI_SDK_COMPAT_CALL_CANVAS_5(egui_canvas_draw_polyline_round_cap_hq, __VA_ARGS__)
#define egui_canvas_draw_rectangle(...) EGUI_SDK_COMPAT_CALL_CANVAS_7(egui_canvas_draw_rectangle, __VA_ARGS__)
#define egui_canvas_draw_rectangle_fill(...) EGUI_SDK_COMPAT_CALL_CANVAS_6(egui_canvas_draw_rectangle_fill, __VA_ARGS__)
#define egui_canvas_draw_round_rectangle(...) EGUI_SDK_COMPAT_CALL_CANVAS_8(egui_canvas_draw_round_rectangle, __VA_ARGS__)
#define egui_canvas_draw_round_rectangle_fill(...) EGUI_SDK_COMPAT_CALL_CANVAS_7(egui_canvas_draw_round_rectangle_fill, __VA_ARGS__)
#define egui_canvas_draw_text(...) EGUI_SDK_COMPAT_CALL_CANVAS_6(egui_canvas_draw_text, __VA_ARGS__)
#define egui_canvas_draw_text_in_rect(...) EGUI_SDK_COMPAT_CALL_CANVAS_6(egui_canvas_draw_text_in_rect, __VA_ARGS__)
#define egui_canvas_draw_triangle(...) EGUI_SDK_COMPAT_CALL_CANVAS_8(egui_canvas_draw_triangle, __VA_ARGS__)
#define egui_canvas_draw_triangle_fill(...) EGUI_SDK_COMPAT_CALL_CANVAS_8(egui_canvas_draw_triangle_fill, __VA_ARGS__)
#define egui_canvas_draw_arc_round_cap_hq(...) EGUI_SDK_COMPAT_CALL_CANVAS_8(egui_canvas_draw_arc_round_cap_hq, __VA_ARGS__)

#define egui_view_activity_ring_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_activity_ring_init, __VA_ARGS__)
#define egui_view_autocomplete_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_autocomplete_init, __VA_ARGS__)
#define egui_view_autocomplete_init_with_params(...) EGUI_SDK_COMPAT_CALL_INIT_WITH_PARAMS(egui_view_autocomplete_init_with_params, __VA_ARGS__)
#define egui_view_button_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_button_init, __VA_ARGS__)
#define egui_view_checkbox_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_checkbox_init, __VA_ARGS__)
#define egui_view_combobox_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_combobox_init, __VA_ARGS__)
#define egui_view_divider_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_divider_init, __VA_ARGS__)
#define egui_view_gridlayout_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_gridlayout_init, __VA_ARGS__)
#define egui_view_group_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_group_init, __VA_ARGS__)
#define egui_view_image_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_image_init, __VA_ARGS__)
#define egui_view_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_init, __VA_ARGS__)
#define egui_view_label_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_label_init, __VA_ARGS__)
#define egui_view_linearlayout_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_linearlayout_init, __VA_ARGS__)
#define egui_view_notification_badge_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_notification_badge_init, __VA_ARGS__)
#define egui_view_progress_bar_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_progress_bar_init, __VA_ARGS__)
#define egui_view_radio_button_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_radio_button_init, __VA_ARGS__)
#define egui_view_segmented_control_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_segmented_control_init, __VA_ARGS__)
#define egui_view_segmented_control_init_with_params(...) \
    EGUI_SDK_COMPAT_CALL_INIT_WITH_PARAMS(egui_view_segmented_control_init_with_params, __VA_ARGS__)
#define egui_view_slider_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_slider_init, __VA_ARGS__)
#define egui_view_spinner_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_spinner_init, __VA_ARGS__)
#define egui_view_switch_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_switch_init, __VA_ARGS__)
#define egui_view_textblock_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_textblock_init, __VA_ARGS__)
#define egui_view_textinput_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_textinput_init, __VA_ARGS__)
#define egui_view_toggle_button_init(...) EGUI_SDK_COMPAT_CALL_INIT(egui_view_toggle_button_init, __VA_ARGS__)

#endif
