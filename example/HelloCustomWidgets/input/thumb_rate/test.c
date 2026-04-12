#include <string.h>

#include "egui.h"
#include "egui_view_thumb_rate.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define THUMB_RATE_ROOT_WIDTH        224
#define THUMB_RATE_ROOT_HEIGHT       176
#define THUMB_RATE_PANEL_WIDTH       196
#define THUMB_RATE_PANEL_HEIGHT      96
#define THUMB_RATE_PRIMARY_WIDTH     172
#define THUMB_RATE_PRIMARY_HEIGHT    52
#define THUMB_RATE_PREVIEW_WIDTH     96
#define THUMB_RATE_PREVIEW_HEIGHT    36
#define THUMB_RATE_BOTTOM_ROW_WIDTH  200
#define THUMB_RATE_BOTTOM_ROW_HEIGHT 36
#define THUMB_RATE_RECORD_WAIT       90
#define THUMB_RATE_RECORD_FRAME_WAIT 170

typedef struct thumb_rate_snapshot thumb_rate_snapshot_t;
struct thumb_rate_snapshot
{
    const char *heading;
    const char *neutral_note;
    const char *liked_note;
    const char *disliked_note;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t heading_label;
static egui_view_thumb_rate_t primary_rate;
static egui_view_label_t note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_thumb_rate_t compact_preview;
static egui_view_api_t compact_preview_api;
static egui_view_thumb_rate_t read_only_preview;
static egui_view_api_t read_only_preview_api;
static uint8_t current_snapshot_index = 0;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

static const char *title_text = "ThumbRate";

static const thumb_rate_snapshot_t primary_snapshots[] = {
        {
                "Release note helpful?",
                "No vote recorded yet.",
                "Marked as helpful.",
                "Marked as not useful.",
        },
        {
                "Setup article helpful?",
                "Feedback is still pending.",
                "Setup guidance looks useful.",
                "Setup guidance needs work.",
        },
};

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color,
                            uint8_t align)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void clear_primary_focus(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&primary_rate));
#endif
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        clear_primary_focus();
    }
    return 1;
}

static void sync_primary_note(void)
{
    const thumb_rate_snapshot_t *snapshot = &primary_snapshots[current_snapshot_index];
    uint8_t state = egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&primary_rate));
    const char *text = snapshot->neutral_note;

    if (state == EGUI_VIEW_THUMB_RATE_STATE_LIKED)
    {
        text = snapshot->liked_note;
    }
    else if (state == EGUI_VIEW_THUMB_RATE_STATE_DISLIKED)
    {
        text = snapshot->disliked_note;
    }
    egui_view_label_set_text(EGUI_VIEW_OF(&note_label), text);
}

static void on_primary_rate_changed(egui_view_t *self, uint8_t state, uint8_t part)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(state);
    EGUI_UNUSED(part);
    sync_primary_note();
}

static void apply_primary_snapshot(uint8_t index)
{
    current_snapshot_index = index % EGUI_ARRAY_SIZE(primary_snapshots);
    egui_view_label_set_text(EGUI_VIEW_OF(&heading_label), primary_snapshots[current_snapshot_index].heading);
    egui_view_thumb_rate_set_state(EGUI_VIEW_OF(&primary_rate), EGUI_VIEW_THUMB_RATE_STATE_NONE);
    egui_view_thumb_rate_set_current_part(EGUI_VIEW_OF(&primary_rate), EGUI_VIEW_THUMB_RATE_PART_LIKE);
    sync_primary_note();
}

static void setup_preview_states(void)
{
    egui_view_thumb_rate_set_state(EGUI_VIEW_OF(&compact_preview), EGUI_VIEW_THUMB_RATE_STATE_LIKED);
    egui_view_thumb_rate_set_current_part(EGUI_VIEW_OF(&compact_preview), EGUI_VIEW_THUMB_RATE_PART_LIKE);
    egui_view_thumb_rate_set_state(EGUI_VIEW_OF(&read_only_preview), EGUI_VIEW_THUMB_RATE_STATE_DISLIKED);
    egui_view_thumb_rate_set_current_part(EGUI_VIEW_OF(&read_only_preview), EGUI_VIEW_THUMB_RATE_PART_DISLIKE);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), THUMB_RATE_ROOT_WIDTH, THUMB_RATE_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, THUMB_RATE_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&primary_panel));
    egui_view_set_size(EGUI_VIEW_OF(&primary_panel), THUMB_RATE_PANEL_WIDTH, THUMB_RATE_PANEL_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&primary_panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&primary_panel), EGUI_ALIGN_LEFT);
    egui_view_set_background(EGUI_VIEW_OF(&primary_panel), EGUI_BG_OF(&bg_surface_panel));
    egui_view_set_padding(EGUI_VIEW_OF(&primary_panel), 12, 10, 12, 10);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&heading_label, 172, 12, "Release note helpful?", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x51606F),
                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&heading_label), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&heading_label));

    egui_view_thumb_rate_init(EGUI_VIEW_OF(&primary_rate));
    egui_view_set_size(EGUI_VIEW_OF(&primary_rate), THUMB_RATE_PRIMARY_WIDTH, THUMB_RATE_PRIMARY_HEIGHT);
    egui_view_thumb_rate_apply_standard_style(EGUI_VIEW_OF(&primary_rate));
    egui_view_thumb_rate_set_font(EGUI_VIEW_OF(&primary_rate), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_thumb_rate_set_labels(EGUI_VIEW_OF(&primary_rate), "Like", "Dislike");
    egui_view_thumb_rate_set_on_changed_listener(EGUI_VIEW_OF(&primary_rate), on_primary_rate_changed);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_rate), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_rate));

    init_text_label(&note_label, 172, 14, "No vote recorded yet.", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6C7A88),
                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), THUMB_RATE_BOTTOM_ROW_WIDTH, THUMB_RATE_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_thumb_rate_init(EGUI_VIEW_OF(&compact_preview));
    egui_view_set_size(EGUI_VIEW_OF(&compact_preview), THUMB_RATE_PREVIEW_WIDTH, THUMB_RATE_PREVIEW_HEIGHT);
    egui_view_thumb_rate_apply_compact_style(EGUI_VIEW_OF(&compact_preview));
    egui_view_thumb_rate_set_font(EGUI_VIEW_OF(&compact_preview), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_thumb_rate_set_labels(EGUI_VIEW_OF(&compact_preview), "Like", "Dislike");
    egui_view_thumb_rate_override_static_preview_api(EGUI_VIEW_OF(&compact_preview), &compact_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    compact_preview_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_preview));

    egui_view_thumb_rate_init(EGUI_VIEW_OF(&read_only_preview));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_preview), THUMB_RATE_PREVIEW_WIDTH, THUMB_RATE_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_preview), 8, 0, 0, 0);
    egui_view_thumb_rate_apply_read_only_style(EGUI_VIEW_OF(&read_only_preview));
    egui_view_thumb_rate_set_font(EGUI_VIEW_OF(&read_only_preview), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_thumb_rate_set_labels(EGUI_VIEW_OF(&read_only_preview), "Like", "Dislike");
    egui_view_thumb_rate_override_static_preview_api(EGUI_VIEW_OF(&read_only_preview), &read_only_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    read_only_preview_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_preview));

    apply_primary_snapshot(0);
    setup_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&primary_rate));
#endif
}

#if EGUI_CONFIG_RECORDING_TEST
static void dispatch_primary_key(uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&primary_rate));
#endif
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&primary_rate)->api->dispatch_key_event(EGUI_VIEW_OF(&primary_rate), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&primary_rate)->api->dispatch_key_event(EGUI_VIEW_OF(&primary_rate), &event);
}

static void dispatch_primary_touch_click(uint8_t part)
{
    egui_region_t region;
    egui_motion_event_t event;

    if (!egui_view_thumb_rate_get_part_region(EGUI_VIEW_OF(&primary_rate), part, &region))
    {
        return;
    }

    memset(&event, 0, sizeof(event));
    event.location.x = region.location.x + region.size.width / 2;
    event.location.y = region.location.y + region.size.height / 2;
    event.type = EGUI_MOTION_EVENT_ACTION_DOWN;
    EGUI_VIEW_OF(&primary_rate)->api->dispatch_touch_event(EGUI_VIEW_OF(&primary_rate), &event);
    event.type = EGUI_MOTION_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&primary_rate)->api->dispatch_touch_event(EGUI_VIEW_OF(&primary_rate), &event);
}

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
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&primary_rate));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, THUMB_RATE_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, THUMB_RATE_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            dispatch_primary_touch_click(EGUI_VIEW_THUMB_RATE_PART_LIKE);
        }
        EGUI_SIM_SET_WAIT(p_action, THUMB_RATE_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, THUMB_RATE_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(1);
            dispatch_primary_key(EGUI_KEY_CODE_RIGHT);
            dispatch_primary_key(EGUI_KEY_CODE_SPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, THUMB_RATE_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, THUMB_RATE_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            dispatch_primary_key(EGUI_KEY_CODE_SPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, THUMB_RATE_RECORD_WAIT);
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
