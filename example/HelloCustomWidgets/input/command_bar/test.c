#include "egui.h"
#include "egui_view_command_bar.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define COMMAND_BAR_ROOT_WIDTH        224
#define COMMAND_BAR_ROOT_HEIGHT       192
#define COMMAND_BAR_PRIMARY_WIDTH     196
#define COMMAND_BAR_PRIMARY_HEIGHT    88
#define COMMAND_BAR_PREVIEW_WIDTH     104
#define COMMAND_BAR_PREVIEW_HEIGHT    64
#define COMMAND_BAR_BOTTOM_ROW_WIDTH  216
#define COMMAND_BAR_BOTTOM_ROW_HEIGHT 64
#define COMMAND_BAR_RECORD_WAIT       90
#define COMMAND_BAR_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_command_bar_t bar_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_command_bar_t bar_compact;
static egui_view_command_bar_t bar_disabled;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Command Bar";

static const egui_view_command_bar_item_t primary_items_edit[] = {
        {"SV", "Save", EGUI_VIEW_COMMAND_BAR_TONE_ACCENT, 1, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"CL", "Clone", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"SH", "Share", EGUI_VIEW_COMMAND_BAR_TONE_ACCENT, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"...", "Overflow", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_OVERFLOW},
};

static const egui_view_command_bar_item_t primary_items_review[] = {
        {"AP", "Approve", EGUI_VIEW_COMMAND_BAR_TONE_SUCCESS, 1, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"NT", "Note", EGUI_VIEW_COMMAND_BAR_TONE_ACCENT, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"BL", "Block", EGUI_VIEW_COMMAND_BAR_TONE_DANGER, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"...", "Overflow", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_OVERFLOW},
};

static const egui_view_command_bar_item_t primary_items_layout[] = {
        {"AL", "Align", EGUI_VIEW_COMMAND_BAR_TONE_ACCENT, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"DS", "Dense", EGUI_VIEW_COMMAND_BAR_TONE_WARNING, 1, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"GR", "Grid", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"...", "Overflow", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_OVERFLOW},
};

static const egui_view_command_bar_item_t primary_items_publish[] = {
        {"SD", "Ship", EGUI_VIEW_COMMAND_BAR_TONE_ACCENT, 1, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"ST", "Stage", EGUI_VIEW_COMMAND_BAR_TONE_WARNING, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"HD", "Hold", EGUI_VIEW_COMMAND_BAR_TONE_DANGER, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"...", "Overflow", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_OVERFLOW},
};

static const egui_view_command_bar_item_t compact_items_quick[] = {
        {"SV", "Save", EGUI_VIEW_COMMAND_BAR_TONE_ACCENT, 1, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"SH", "Share", EGUI_VIEW_COMMAND_BAR_TONE_ACCENT, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"CL", "Clone", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"...", "Overflow", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_OVERFLOW},
};

static const egui_view_command_bar_item_t compact_items_review[] = {
        {"AP", "Approve", EGUI_VIEW_COMMAND_BAR_TONE_SUCCESS, 1, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"CM", "Comment", EGUI_VIEW_COMMAND_BAR_TONE_ACCENT, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"BL", "Block", EGUI_VIEW_COMMAND_BAR_TONE_DANGER, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"...", "Overflow", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 0, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_OVERFLOW},
};

static const egui_view_command_bar_item_t disabled_items[] = {
        {"SV", "Save", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 1, 1, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"SH", "Share", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 0, 0, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"AR", "Archive", EGUI_VIEW_COMMAND_BAR_TONE_DANGER, 0, 0, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_NORMAL},
        {"...", "Overflow", EGUI_VIEW_COMMAND_BAR_TONE_NEUTRAL, 0, 0, EGUI_VIEW_COMMAND_BAR_ITEM_KIND_OVERFLOW},
};

static const egui_view_command_bar_snapshot_t primary_snapshots[] = {
        {"Edit", "Page commands", "Canvas", "Save, share, or overflow", primary_items_edit, EGUI_ARRAY_SIZE(primary_items_edit), 0},
        {"Review", "Review commands", "Build", "Approve, note, or block", primary_items_review, EGUI_ARRAY_SIZE(primary_items_review), 1},
        {"Layout", "Layout commands", "Panels", "Tune density before ship", primary_items_layout, EGUI_ARRAY_SIZE(primary_items_layout), 1},
        {"Publish", "Publish commands", "Release", "Ship now, stage later", primary_items_publish, EGUI_ARRAY_SIZE(primary_items_publish), 0},
};

static const egui_view_command_bar_snapshot_t compact_snapshots[] = {
        {"Quick", "Compact rail", "Quick", "Tight icon rail", compact_items_quick, EGUI_ARRAY_SIZE(compact_items_quick), 0},
        {"Review", "Compact rail", "Review", "Compact review bar", compact_items_review, EGUI_ARRAY_SIZE(compact_items_review), 0},
};

static const egui_view_command_bar_snapshot_t disabled_snapshot = {
        "Locked", "Disabled rail", "Read only", "Visible but inactive", disabled_items, EGUI_ARRAY_SIZE(disabled_items), 0};

static int consume_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_UP || event->type == EGUI_MOTION_EVENT_ACTION_CANCEL)
    {
        egui_view_set_pressed(self, 0);
    }

    return 1;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int consume_preview_key(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}
#endif

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_command_bar_set_current_snapshot(EGUI_VIEW_OF(&bar_primary), index % EGUI_ARRAY_SIZE(primary_snapshots));
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_command_bar_set_current_snapshot(EGUI_VIEW_OF(&bar_compact), index % EGUI_ARRAY_SIZE(compact_snapshots));
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&bar_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&bar_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&bar_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&bar_primary), &event);
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), COMMAND_BAR_ROOT_WIDTH, COMMAND_BAR_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), COMMAND_BAR_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_command_bar_init(EGUI_VIEW_OF(&bar_primary));
    egui_view_set_size(EGUI_VIEW_OF(&bar_primary), COMMAND_BAR_PRIMARY_WIDTH, COMMAND_BAR_PRIMARY_HEIGHT);
    egui_view_command_bar_set_font(EGUI_VIEW_OF(&bar_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_command_bar_set_meta_font(EGUI_VIEW_OF(&bar_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_command_bar_set_snapshots(EGUI_VIEW_OF(&bar_primary), primary_snapshots, EGUI_ARRAY_SIZE(primary_snapshots));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&bar_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&bar_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bar_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), COMMAND_BAR_BOTTOM_ROW_WIDTH, COMMAND_BAR_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_command_bar_init(EGUI_VIEW_OF(&bar_compact));
    egui_view_set_size(EGUI_VIEW_OF(&bar_compact), COMMAND_BAR_PREVIEW_WIDTH, COMMAND_BAR_PREVIEW_HEIGHT);
    egui_view_command_bar_set_font(EGUI_VIEW_OF(&bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_command_bar_set_meta_font(EGUI_VIEW_OF(&bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_command_bar_set_snapshots(EGUI_VIEW_OF(&bar_compact), compact_snapshots, EGUI_ARRAY_SIZE(compact_snapshots));
    egui_view_command_bar_set_compact_mode(EGUI_VIEW_OF(&bar_compact), 1);
    static egui_view_api_t bar_compact_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&bar_compact), &bar_compact_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    egui_view_override_api_on_key(EGUI_VIEW_OF(&bar_compact), &bar_compact_api, consume_preview_key);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&bar_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&bar_compact));

    egui_view_command_bar_init(EGUI_VIEW_OF(&bar_disabled));
    egui_view_set_size(EGUI_VIEW_OF(&bar_disabled), COMMAND_BAR_PREVIEW_WIDTH, COMMAND_BAR_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&bar_disabled), 8, 0, 0, 0);
    egui_view_command_bar_set_font(EGUI_VIEW_OF(&bar_disabled), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_command_bar_set_meta_font(EGUI_VIEW_OF(&bar_disabled), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_command_bar_set_snapshots(EGUI_VIEW_OF(&bar_disabled), &disabled_snapshot, 1);
    egui_view_command_bar_set_compact_mode(EGUI_VIEW_OF(&bar_disabled), 1);
    egui_view_command_bar_set_disabled_mode(EGUI_VIEW_OF(&bar_disabled), 1);
    egui_view_command_bar_set_palette(EGUI_VIEW_OF(&bar_disabled), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xD8E0E7),
                                      EGUI_COLOR_HEX(0x50606F), EGUI_COLOR_HEX(0x8C98A5), EGUI_COLOR_HEX(0x90A0AE), EGUI_COLOR_HEX(0x93A594),
                                      EGUI_COLOR_HEX(0xB29A67), EGUI_COLOR_HEX(0xA48B88), EGUI_COLOR_HEX(0x95A2AF));
    static egui_view_api_t bar_disabled_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&bar_disabled), &bar_disabled_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    egui_view_override_api_on_key(EGUI_VIEW_OF(&bar_disabled), &bar_disabled_api, consume_preview_key);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&bar_disabled), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&bar_disabled));

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
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(1);
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(2);
            apply_primary_key(EGUI_KEY_CODE_END);
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_primary_snapshot(3);
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_RECORD_WAIT);
        return true;
    case 9:
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
