#include "egui.h"
#include "egui_view_expander.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define EXPANDER_ROOT_W           224
#define EXPANDER_ROOT_H           226
#define EXPANDER_PRIMARY_W        196
#define EXPANDER_PRIMARY_H        110
#define EXPANDER_PREVIEW_W        104
#define EXPANDER_PREVIEW_H        76
#define EXPANDER_BOTTOM_W         216
#define EXPANDER_BOTTOM_H         76
#define EXPANDER_RECORD_WAIT      90
#define EXPANDER_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_expander_t panel_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_expander_t panel_compact;
static egui_view_linearlayout_t read_only_column;
static egui_view_expander_t panel_read_only;
static egui_view_api_t panel_compact_api;
static egui_view_api_t panel_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Expander";
static uint8_t primary_index = 0;
static uint8_t primary_expanded_index = 0;
static uint8_t compact_index = 0;
static uint8_t compact_expanded_index = 0;

static const egui_view_expander_item_t primary_items[] = {
        {"WORK", "Workspace policy", "Ready", "Pinned groups stay open", "Draft rules stay visible", "Always on", EGUI_VIEW_EXPANDER_TONE_ACCENT, 1},
        {"SYNC", "Sync rules", "3 rules", "Metered uploads wait for Wi-Fi", "Night copy stays local", "Queue review", EGUI_VIEW_EXPANDER_TONE_SUCCESS, 0},
        {"RELEASE", "Release notes", "Hold", "Pilot warnings stay staged", "Manual signoff closes it", "Manual hold", EGUI_VIEW_EXPANDER_TONE_WARNING, 1},
};

static const egui_view_expander_item_t compact_items[] = {
        {"FAST", "Mode", "On", "One row open", "", "Review", EGUI_VIEW_EXPANDER_TONE_ACCENT, 1},
        {"BACKUP", "Backup", "Pause", "Waits for charge", "", "Resume", EGUI_VIEW_EXPANDER_TONE_WARNING, 0},
};

static const egui_view_expander_item_t read_only_items[] = {
        {"AUDIT", "Audit", "Lock", "History locked", "", "Read only", EGUI_VIEW_EXPANDER_TONE_NEUTRAL, 0},
        {"HISTORY", "History", "Arc", "Searchable notes", "", "Frozen", EGUI_VIEW_EXPANDER_TONE_SUCCESS, 0},
};

static void dismiss_primary_expander_focus(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&panel_primary));
#endif
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        dismiss_primary_expander_focus();
    }
    return 1;
}

static void apply_primary_state(uint8_t index, uint8_t expanded_index)
{
    primary_index = (uint8_t)(index % EGUI_ARRAY_SIZE(primary_items));
    primary_expanded_index = expanded_index;
    if (primary_expanded_index != EGUI_VIEW_EXPANDER_INDEX_NONE)
    {
        primary_expanded_index = (uint8_t)(primary_expanded_index % EGUI_ARRAY_SIZE(primary_items));
    }

    egui_view_expander_set_current_index(EGUI_VIEW_OF(&panel_primary), primary_index);
    egui_view_expander_set_expanded_index(EGUI_VIEW_OF(&panel_primary), primary_expanded_index);
}

static void apply_compact_state(uint8_t index, uint8_t expanded_index)
{
    compact_index = (uint8_t)(index % EGUI_ARRAY_SIZE(compact_items));
    compact_expanded_index = expanded_index;
    if (compact_expanded_index != EGUI_VIEW_EXPANDER_INDEX_NONE)
    {
        compact_expanded_index = (uint8_t)(compact_expanded_index % EGUI_ARRAY_SIZE(compact_items));
    }

    egui_view_expander_set_current_index(EGUI_VIEW_OF(&panel_compact), compact_index);
    egui_view_expander_set_expanded_index(EGUI_VIEW_OF(&panel_compact), compact_expanded_index);
}

#if EGUI_CONFIG_RECORDING_TEST
static void set_click_view_center(egui_sim_action_t *p_action, egui_view_t *view, int interval_ms)
{
    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = view->region_screen.location.x + view->region_screen.size.width / 2;
    p_action->y1 = view->region_screen.location.y + view->region_screen.size.height / 2;
    p_action->x2 = 0;
    p_action->y2 = 0;
    p_action->steps = 0;
    p_action->interval_ms = interval_ms;
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), EXPANDER_ROOT_W, EXPANDER_ROOT_H);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), EXPANDER_ROOT_W, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_expander_init(EGUI_VIEW_OF(&panel_primary));
    egui_view_set_size(EGUI_VIEW_OF(&panel_primary), EXPANDER_PRIMARY_W, EXPANDER_PRIMARY_H);
    egui_view_expander_set_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_expander_set_meta_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_expander_set_items(EGUI_VIEW_OF(&panel_primary), primary_items, EGUI_ARRAY_SIZE(primary_items));
    egui_view_expander_set_palette(EGUI_VIEW_OF(&panel_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xEAF0F7),
                                   EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                   EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_set_margin(EGUI_VIEW_OF(&panel_primary), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_primary), true);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&panel_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), EXPANDER_BOTTOM_W, EXPANDER_BOTTOM_H);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), EXPANDER_PREVIEW_W, EXPANDER_BOTTOM_H);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_expander_init(EGUI_VIEW_OF(&panel_compact));
    egui_view_set_size(EGUI_VIEW_OF(&panel_compact), EXPANDER_PREVIEW_W, EXPANDER_PREVIEW_H);
    egui_view_expander_set_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_expander_set_meta_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_expander_set_items(EGUI_VIEW_OF(&panel_compact), compact_items, EGUI_ARRAY_SIZE(compact_items));
    egui_view_expander_set_compact_mode(EGUI_VIEW_OF(&panel_compact), 1);
    egui_view_expander_set_palette(EGUI_VIEW_OF(&panel_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xEAF0F7),
                                   EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                   EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_expander_override_static_preview_api(EGUI_VIEW_OF(&panel_compact), &panel_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    panel_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&panel_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&read_only_column));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_column), EXPANDER_PREVIEW_W, EXPANDER_BOTTOM_H);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&read_only_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&read_only_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_column));

    egui_view_expander_init(EGUI_VIEW_OF(&panel_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&panel_read_only), EXPANDER_PREVIEW_W, EXPANDER_PREVIEW_H);
    egui_view_expander_set_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_expander_set_meta_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_expander_set_items(EGUI_VIEW_OF(&panel_read_only), read_only_items, EGUI_ARRAY_SIZE(read_only_items));
    egui_view_expander_set_compact_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_expander_set_read_only_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_expander_set_palette(EGUI_VIEW_OF(&panel_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xDBE2E8), EGUI_COLOR_HEX(0xF3F6F9),
                                   EGUI_COLOR_HEX(0x536474), EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0xB3BFCA), EGUI_COLOR_HEX(0xA7BDB6),
                                   EGUI_COLOR_HEX(0xC3AE88), EGUI_COLOR_HEX(0x9AA7B3));
    egui_view_expander_override_static_preview_api(EGUI_VIEW_OF(&panel_read_only), &panel_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    panel_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_column), EGUI_VIEW_OF(&panel_read_only));

    apply_primary_state(0, 0);
    apply_compact_state(0, 0);
    egui_view_expander_set_current_index(EGUI_VIEW_OF(&panel_read_only), 0);
    egui_view_expander_set_expanded_index(EGUI_VIEW_OF(&panel_read_only), 0);

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&read_only_column));
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
            apply_primary_state(0, 0);
            apply_compact_state(0, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, EXPANDER_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, EXPANDER_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_state(1, 1);
        }
        EGUI_SIM_SET_WAIT(p_action, EXPANDER_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, EXPANDER_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_state(1, EGUI_VIEW_EXPANDER_INDEX_NONE);
        }
        EGUI_SIM_SET_WAIT(p_action, EXPANDER_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, EXPANDER_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_state(2, 2);
        }
        EGUI_SIM_SET_WAIT(p_action, EXPANDER_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, EXPANDER_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_compact_state(1, 1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&panel_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, EXPANDER_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, EXPANDER_RECORD_FRAME_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            set_click_view_center(p_action, EGUI_VIEW_OF(&panel_compact), EXPANDER_RECORD_WAIT);
        }
        return true;
    case 11:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, EXPANDER_RECORD_FRAME_WAIT);
        return true;
    case 12:
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
