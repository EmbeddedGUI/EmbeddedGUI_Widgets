#include "egui.h"
#include "egui_view_drop_down_button.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define DROP_DOWN_BUTTON_ROOT_WIDTH        224
#define DROP_DOWN_BUTTON_ROOT_HEIGHT       160
#define DROP_DOWN_BUTTON_PRIMARY_WIDTH     196
#define DROP_DOWN_BUTTON_PRIMARY_HEIGHT    76
#define DROP_DOWN_BUTTON_PREVIEW_WIDTH     104
#define DROP_DOWN_BUTTON_PREVIEW_HEIGHT    44
#define DROP_DOWN_BUTTON_BOTTOM_ROW_WIDTH  216
#define DROP_DOWN_BUTTON_BOTTOM_ROW_HEIGHT 44
#define DROP_DOWN_BUTTON_RECORD_WAIT       90
#define DROP_DOWN_BUTTON_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_drop_down_button_t button_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_drop_down_button_t button_compact;
static egui_view_drop_down_button_t button_read_only;
static egui_view_api_t button_compact_api;
static egui_view_api_t button_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Drop Down Button";

static const egui_view_drop_down_button_snapshot_t primary_snapshots[] = {
        {"Actions", "SO", "Sort", "Open sort presets", "", EGUI_VIEW_DROP_DOWN_BUTTON_TONE_ACCENT, 1},
        {"Actions", "LY", "Layout", "Open layout presets", "", EGUI_VIEW_DROP_DOWN_BUTTON_TONE_NEUTRAL, 0},
        {"Actions", "TH", "Theme", "Open appearance presets", "", EGUI_VIEW_DROP_DOWN_BUTTON_TONE_WARNING, 0},
};

static const egui_view_drop_down_button_snapshot_t compact_snapshots[] = {
        {"", "", "Sort", "", "", EGUI_VIEW_DROP_DOWN_BUTTON_TONE_ACCENT, 1},
        {"", "", "Filter", "", "", EGUI_VIEW_DROP_DOWN_BUTTON_TONE_NEUTRAL, 0},
};

static const egui_view_drop_down_button_snapshot_t read_only_snapshot = {
        "", "", "Locked", "", "", EGUI_VIEW_DROP_DOWN_BUTTON_TONE_NEUTRAL, 0};

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        egui_view_clear_focus(EGUI_VIEW_OF(&button_primary));
    }
#else
    EGUI_UNUSED(event);
#endif
    return 1;
}

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_drop_down_button_set_current_snapshot(EGUI_VIEW_OF(&button_primary), index % EGUI_ARRAY_SIZE(primary_snapshots));
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_drop_down_button_set_current_snapshot(EGUI_VIEW_OF(&button_compact), index % EGUI_ARRAY_SIZE(compact_snapshots));
}

static void on_primary_click(egui_view_t *self)
{
    uint8_t next = (egui_view_drop_down_button_get_current_snapshot(self) + 1U) % EGUI_ARRAY_SIZE(primary_snapshots);

    apply_primary_snapshot(next);
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&button_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&button_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&button_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&button_primary), &event);
}

static void apply_primary_touch_click(void)
{
    egui_motion_event_t event = {0};

    event.location.x = EGUI_VIEW_OF(&button_primary)->region_screen.location.x + EGUI_VIEW_OF(&button_primary)->region_screen.size.width / 2;
    event.location.y = EGUI_VIEW_OF(&button_primary)->region_screen.location.y + EGUI_VIEW_OF(&button_primary)->region_screen.size.height / 2;

    event.type = EGUI_MOTION_EVENT_ACTION_DOWN;
    EGUI_VIEW_OF(&button_primary)->api->dispatch_touch_event(EGUI_VIEW_OF(&button_primary), &event);
    event.type = EGUI_MOTION_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&button_primary)->api->dispatch_touch_event(EGUI_VIEW_OF(&button_primary), &event);
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), DROP_DOWN_BUTTON_ROOT_WIDTH, DROP_DOWN_BUTTON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), DROP_DOWN_BUTTON_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_drop_down_button_init(EGUI_VIEW_OF(&button_primary));
    egui_view_set_size(EGUI_VIEW_OF(&button_primary), DROP_DOWN_BUTTON_PRIMARY_WIDTH, DROP_DOWN_BUTTON_PRIMARY_HEIGHT);
    egui_view_drop_down_button_set_font(EGUI_VIEW_OF(&button_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_drop_down_button_set_meta_font(EGUI_VIEW_OF(&button_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_drop_down_button_set_snapshots(EGUI_VIEW_OF(&button_primary), primary_snapshots, EGUI_ARRAY_SIZE(primary_snapshots));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&button_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&button_primary), 0, 0, 0, 8);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&button_primary), on_primary_click);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&button_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), DROP_DOWN_BUTTON_BOTTOM_ROW_WIDTH, DROP_DOWN_BUTTON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_drop_down_button_init(EGUI_VIEW_OF(&button_compact));
    egui_view_set_size(EGUI_VIEW_OF(&button_compact), DROP_DOWN_BUTTON_PREVIEW_WIDTH, DROP_DOWN_BUTTON_PREVIEW_HEIGHT);
    egui_view_drop_down_button_set_font(EGUI_VIEW_OF(&button_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_drop_down_button_set_meta_font(EGUI_VIEW_OF(&button_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_drop_down_button_set_snapshots(EGUI_VIEW_OF(&button_compact), compact_snapshots, EGUI_ARRAY_SIZE(compact_snapshots));
    egui_view_drop_down_button_set_compact_mode(EGUI_VIEW_OF(&button_compact), 1);
    egui_view_drop_down_button_override_static_preview_api(EGUI_VIEW_OF(&button_compact), &button_compact_api);
    button_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&button_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&button_compact));

    egui_view_drop_down_button_init(EGUI_VIEW_OF(&button_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&button_read_only), DROP_DOWN_BUTTON_PREVIEW_WIDTH, DROP_DOWN_BUTTON_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&button_read_only), 8, 0, 0, 0);
    egui_view_drop_down_button_set_font(EGUI_VIEW_OF(&button_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_drop_down_button_set_meta_font(EGUI_VIEW_OF(&button_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_drop_down_button_set_snapshots(EGUI_VIEW_OF(&button_read_only), &read_only_snapshot, 1);
    egui_view_drop_down_button_set_compact_mode(EGUI_VIEW_OF(&button_read_only), 1);
    egui_view_drop_down_button_set_read_only_mode(EGUI_VIEW_OF(&button_read_only), 1);
    egui_view_drop_down_button_set_palette(EGUI_VIEW_OF(&button_read_only), EGUI_COLOR_HEX(0xF9FBFD), EGUI_COLOR_HEX(0xD6DFE7), EGUI_COLOR_HEX(0x5B6875),
                                           EGUI_COLOR_HEX(0x8F9CA8), EGUI_COLOR_HEX(0x93A4B2), EGUI_COLOR_HEX(0x93A1A5), EGUI_COLOR_HEX(0xA79B88),
                                           EGUI_COLOR_HEX(0x9E949B), EGUI_COLOR_HEX(0x98A8B4));
    egui_view_drop_down_button_override_static_preview_api(EGUI_VIEW_OF(&button_read_only), &button_read_only_api);
    button_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&button_read_only), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&button_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

#if EGUI_CONFIG_RECORDING_TEST
bool egui_port_get_recording_action(int action_index, egui_sim_action_t *p_action)
{
    static int last_action = -1;
    int first_call = action_index != last_action;

    last_action = action_index;

    switch (action_index)
    {
    case 0:
        if (first_call)
        {
            apply_primary_snapshot(0);
            apply_compact_snapshot(0);
        }
        EGUI_SIM_SET_WAIT(p_action, DROP_DOWN_BUTTON_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DROP_DOWN_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_touch_click();
        }
        EGUI_SIM_SET_WAIT(p_action, DROP_DOWN_BUTTON_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DROP_DOWN_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_ENTER);
        }
        EGUI_SIM_SET_WAIT(p_action, DROP_DOWN_BUTTON_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DROP_DOWN_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, DROP_DOWN_BUTTON_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
