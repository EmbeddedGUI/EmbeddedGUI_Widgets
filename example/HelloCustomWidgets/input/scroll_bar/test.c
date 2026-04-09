#include <string.h>

#include "egui.h"
#include "egui_view_scroll_bar.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SCROLL_BAR_ROOT_WIDTH        224
#define SCROLL_BAR_ROOT_HEIGHT       238
#define SCROLL_BAR_PRIMARY_WIDTH     196
#define SCROLL_BAR_PRIMARY_HEIGHT    146
#define SCROLL_BAR_PREVIEW_WIDTH     104
#define SCROLL_BAR_PREVIEW_HEIGHT    52
#define SCROLL_BAR_BOTTOM_ROW_WIDTH  216
#define SCROLL_BAR_BOTTOM_ROW_HEIGHT 52
#define SCROLL_BAR_RECORD_WAIT       110
#define SCROLL_BAR_RECORD_FRAME_WAIT 150

typedef struct scroll_bar_demo_snapshot scroll_bar_demo_snapshot_t;
struct scroll_bar_demo_snapshot
{
    const char *label;
    const char *helper;
    egui_dim_t content_length;
    egui_dim_t viewport_length;
    egui_dim_t offset;
    egui_dim_t line_step;
    egui_dim_t page_step;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_scroll_bar_t scroll_bar_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_scroll_bar_t scroll_bar_compact;
static egui_view_scroll_bar_t scroll_bar_read_only;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Scroll Bar";

static const scroll_bar_demo_snapshot_t primary_snapshots[] = {
        {"Document rail", "Buttons, track, and thumb", 840, 220, 168, 24, 160},
        {"Timeline rail", "Viewport keeps the thumb honest", 640, 180, 92, 18, 120},
        {"Audit rail", "Read long logs without a slider", 980, 260, 420, 32, 210},
};

static const scroll_bar_demo_snapshot_t compact_snapshots[] = {
        {"", "", 540, 160, 96, 16, 96},
        {"", "", 720, 240, 188, 24, 144},
};

static const scroll_bar_demo_snapshot_t read_only_snapshot = {"", "", 620, 200, 140, 20, 120};

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

static void apply_snapshot(egui_view_t *view, const scroll_bar_demo_snapshot_t *snapshot)
{
    egui_view_scroll_bar_set_label(view, snapshot->label == NULL ? "" : snapshot->label);
    egui_view_scroll_bar_set_helper(view, snapshot->helper == NULL ? "" : snapshot->helper);
    egui_view_scroll_bar_set_content_metrics(view, snapshot->content_length, snapshot->viewport_length);
    egui_view_scroll_bar_set_step_size(view, snapshot->line_step, snapshot->page_step);
    egui_view_scroll_bar_set_offset(view, snapshot->offset);
    egui_view_scroll_bar_set_current_part(view, EGUI_VIEW_SCROLL_BAR_PART_THUMB);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot(EGUI_VIEW_OF(&scroll_bar_primary), &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)]);
}

static void apply_compact_snapshot(uint8_t index)
{
    apply_snapshot(EGUI_VIEW_OF(&scroll_bar_compact), &compact_snapshots[index % EGUI_ARRAY_SIZE(compact_snapshots)]);
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&scroll_bar_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&scroll_bar_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&scroll_bar_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&scroll_bar_primary), &event);
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SCROLL_BAR_ROOT_WIDTH, SCROLL_BAR_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SCROLL_BAR_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_scroll_bar_init(EGUI_VIEW_OF(&scroll_bar_primary));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_bar_primary), SCROLL_BAR_PRIMARY_WIDTH, SCROLL_BAR_PRIMARY_HEIGHT);
    egui_view_scroll_bar_set_font(EGUI_VIEW_OF(&scroll_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_scroll_bar_set_meta_font(EGUI_VIEW_OF(&scroll_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_bar_set_palette(EGUI_VIEW_OF(&scroll_bar_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD7DFE7), EGUI_COLOR_HEX(0x1A2630),
                                     EGUI_COLOR_HEX(0x72808E), EGUI_COLOR_HEX(0x2563EB), EGUI_COLOR_HEX(0x5BA5F8));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&scroll_bar_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&scroll_bar_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&scroll_bar_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SCROLL_BAR_BOTTOM_ROW_WIDTH, SCROLL_BAR_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_scroll_bar_init(EGUI_VIEW_OF(&scroll_bar_compact));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_bar_compact), SCROLL_BAR_PREVIEW_WIDTH, SCROLL_BAR_PREVIEW_HEIGHT);
    egui_view_scroll_bar_set_font(EGUI_VIEW_OF(&scroll_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_scroll_bar_set_meta_font(EGUI_VIEW_OF(&scroll_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&scroll_bar_compact), 1);
    egui_view_scroll_bar_set_palette(EGUI_VIEW_OF(&scroll_bar_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DDDA), EGUI_COLOR_HEX(0x17302A),
                                     EGUI_COLOR_HEX(0x57756C), EGUI_COLOR_HEX(0x0D9488), EGUI_COLOR_HEX(0x3BC7B3));
    static egui_view_api_t scroll_bar_compact_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&scroll_bar_compact), &scroll_bar_compact_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    egui_view_override_api_on_key(EGUI_VIEW_OF(&scroll_bar_compact), &scroll_bar_compact_api, consume_preview_key);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&scroll_bar_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&scroll_bar_compact));

    egui_view_scroll_bar_init(EGUI_VIEW_OF(&scroll_bar_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_bar_read_only), SCROLL_BAR_PREVIEW_WIDTH, SCROLL_BAR_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&scroll_bar_read_only), 8, 0, 0, 0);
    egui_view_scroll_bar_set_font(EGUI_VIEW_OF(&scroll_bar_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_scroll_bar_set_meta_font(EGUI_VIEW_OF(&scroll_bar_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&scroll_bar_read_only), 1);
    egui_view_scroll_bar_set_read_only_mode(EGUI_VIEW_OF(&scroll_bar_read_only), 1);
    egui_view_scroll_bar_set_palette(EGUI_VIEW_OF(&scroll_bar_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xDBE2E8), EGUI_COLOR_HEX(0x536474),
                                     EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0x9AA7B5), EGUI_COLOR_HEX(0xA7B6C4));
    static egui_view_api_t scroll_bar_read_only_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&scroll_bar_read_only), &scroll_bar_read_only_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    egui_view_override_api_on_key(EGUI_VIEW_OF(&scroll_bar_read_only), &scroll_bar_read_only_api, consume_preview_key);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&scroll_bar_read_only), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&scroll_bar_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_snapshot(EGUI_VIEW_OF(&scroll_bar_read_only), &read_only_snapshot);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&scroll_bar_primary));
#endif
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
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&scroll_bar_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_BAR_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_BAR_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_BAR_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_BAR_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_PLUS);
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_BAR_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_BAR_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_END);
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_BAR_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_BAR_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_primary_snapshot(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&scroll_bar_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_BAR_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_BAR_RECORD_FRAME_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_BAR_RECORD_WAIT);
        return true;
    case 11:
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
