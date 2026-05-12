#include <string.h>

#include "egui.h"
#include "egui_view_swipe_control.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif


#define SWIPE_CONTROL_ROOT_WIDTH        224
#define SWIPE_CONTROL_ROOT_HEIGHT       222
#define SWIPE_CONTROL_PRIMARY_WIDTH     196
#define SWIPE_CONTROL_PRIMARY_HEIGHT    118
#define SWIPE_CONTROL_PREVIEW_WIDTH     104
#define SWIPE_CONTROL_PREVIEW_HEIGHT    64
#define SWIPE_CONTROL_BOTTOM_ROW_WIDTH  216
#define SWIPE_CONTROL_BOTTOM_ROW_HEIGHT 64
#define SWIPE_CONTROL_RECORD_WAIT       110
#define SWIPE_CONTROL_RECORD_FRAME_WAIT 150
#define SWIPE_CONTROL_RECORD_FINAL_WAIT 280

typedef struct swipe_control_track swipe_control_track_t;
struct swipe_control_track
{
    const char *title;
    const char *helper;
    const egui_view_swipe_control_item_t *item;
    const egui_view_swipe_control_action_t *start_action;
    const egui_view_swipe_control_action_t *end_action;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_swipe_control_t swipe_control_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_swipe_control_t swipe_control_compact;
static egui_view_swipe_control_t swipe_control_read_only;
static egui_view_api_t swipe_control_compact_api;
static egui_view_api_t swipe_control_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, HCW_COLOR_PAGE_BG, EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Swipe Control";

static const egui_view_swipe_control_item_t inbox_item = {
        "Mail", "Invoice follow-up", "Reveal quick actions without leaving the row.", "Due today", HCW_COLOR_SURFACE, HCW_COLOR_PRIMARY};
static const egui_view_swipe_control_action_t inbox_start_action = {"Pin", "Keep", HCW_COLOR_PRIMARY, HCW_COLOR_SURFACE};
static const egui_view_swipe_control_action_t inbox_end_action = {"Delete", "Remove", HCW_COLOR_DANGER, HCW_COLOR_SURFACE};

static const egui_view_swipe_control_item_t planner_item = {
        "Plan", "Planner sync", "One row, two sides, one calm reveal model.", "Board ready", HCW_COLOR_SURFACE, HCW_COLOR_PRIMARY};
static const egui_view_swipe_control_action_t planner_start_action = {"Flag", "Review", HCW_COLOR_PRIMARY, HCW_COLOR_SURFACE};
static const egui_view_swipe_control_action_t planner_end_action = {"Archive", "Store", HCW_COLOR_WARNING, HCW_COLOR_SURFACE};

static const egui_view_swipe_control_item_t review_item = {
        "Build", "Renderer check", "Keys and swipe gestures share the same state.", "Waiting QA", HCW_COLOR_SURFACE, HCW_COLOR_SUCCESS};
static const egui_view_swipe_control_action_t review_start_action = {"Done", "Close", HCW_COLOR_SUCCESS, HCW_COLOR_SURFACE};
static const egui_view_swipe_control_action_t review_end_action = {"Snooze", "Later", HCW_COLOR_PRIMARY, HCW_COLOR_SURFACE};

static const egui_view_swipe_control_item_t compact_mail_item = {"Mini", "Pocket", "", "", HCW_COLOR_SURFACE, HCW_COLOR_PRIMARY};
static const egui_view_swipe_control_action_t compact_mail_start_action = {"Pin", "", HCW_COLOR_PRIMARY, HCW_COLOR_SURFACE};
static const egui_view_swipe_control_action_t compact_mail_end_action = {"Delete", "", HCW_COLOR_DANGER, HCW_COLOR_SURFACE};

static const egui_view_swipe_control_item_t compact_queue_item = {"Mini", "Queue", "", "", HCW_COLOR_WARNING_SOFT, HCW_COLOR_WARNING};
static const egui_view_swipe_control_action_t compact_queue_start_action = {"Flag", "", HCW_COLOR_PRIMARY, HCW_COLOR_SURFACE};
static const egui_view_swipe_control_action_t compact_queue_end_action = {"Archive", "", HCW_COLOR_WARNING, HCW_COLOR_SURFACE};

static const egui_view_swipe_control_item_t read_only_item = {"Lock", "Locked row", "", "", HCW_COLOR_PANEL, HCW_COLOR_TEXT};
static const egui_view_swipe_control_action_t read_only_start_action = {"Pin", "", HCW_COLOR_TEXT, HCW_COLOR_SURFACE};
static const egui_view_swipe_control_action_t read_only_end_action = {"Delete", "", HCW_COLOR_BORDER_STRONG, HCW_COLOR_SURFACE};

static const swipe_control_track_t primary_tracks[] = {
        {"Inbox", "Swipe right to pin or left to delete.", &inbox_item, &inbox_start_action, &inbox_end_action},
        {"Planner", "One row keeps the same reveal structure.", &planner_item, &planner_start_action, &planner_end_action},
        {"Review", "Keyboard and touch share one reveal state.", &review_item, &review_start_action, &review_end_action},
};

static const swipe_control_track_t compact_tracks[] = {
        {"", "", &compact_mail_item, &compact_mail_start_action, &compact_mail_end_action},
        {"", "", &compact_queue_item, &compact_queue_start_action, &compact_queue_end_action},
};

static const swipe_control_track_t read_only_track = {"", "", &read_only_item, &read_only_start_action, &read_only_end_action};

static void layout_page(void);

static void apply_track(egui_view_t *view, const swipe_control_track_t *track)
{
    egui_view_swipe_control_set_title(view, track->title == NULL ? "" : track->title);
    egui_view_swipe_control_set_helper(view, track->helper == NULL ? "" : track->helper);
    egui_view_swipe_control_set_item(view, track->item);
    egui_view_swipe_control_set_actions(view, track->start_action, track->end_action);
    egui_view_swipe_control_set_reveal_state(view, EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE);
    egui_view_swipe_control_set_current_part(view, EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE);
}

static void apply_primary_track(uint8_t index)
{
    apply_track(EGUI_VIEW_OF(&swipe_control_primary), &primary_tracks[index % EGUI_ARRAY_SIZE(primary_tracks)]);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_compact_track(uint8_t index)
{
    apply_track(EGUI_VIEW_OF(&swipe_control_compact), &compact_tracks[index % EGUI_ARRAY_SIZE(compact_tracks)]);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_track(0);
}

static void apply_read_only_preview_state(void)
{
    apply_track(EGUI_VIEW_OF(&swipe_control_read_only), &read_only_track);
}

static void apply_preview_states(void)
{
    apply_compact_track(0);
    apply_read_only_preview_state();
    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
}

static void layout_page(void)
{
    layout_local_views();
    egui_core_layout_childs_user_root_view(uicode_get_core(), EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

static void focus_primary_swipe_control(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&swipe_control_primary));
#endif
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    focus_primary_swipe_control();
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&swipe_control_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&swipe_control_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&swipe_control_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&swipe_control_primary), &event);
    if (ui_ready)
    {
        layout_page();
    }
}

static void request_page_snapshot(void)
{
    layout_page();
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}
#endif

void test_init_ui(void)
{
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SWIPE_CONTROL_ROOT_WIDTH, SWIPE_CONTROL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SWIPE_CONTROL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), HCW_COLOR_TEXT, EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_swipe_control_init(EGUI_VIEW_OF(&swipe_control_primary));
    egui_view_set_size(EGUI_VIEW_OF(&swipe_control_primary), SWIPE_CONTROL_PRIMARY_WIDTH, SWIPE_CONTROL_PRIMARY_HEIGHT);
    egui_view_swipe_control_set_font(EGUI_VIEW_OF(&swipe_control_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_swipe_control_set_meta_font(EGUI_VIEW_OF(&swipe_control_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_swipe_control_set_palette(EGUI_VIEW_OF(&swipe_control_primary), HCW_COLOR_SURFACE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_TEXT_STRONG,
                                        HCW_COLOR_TEXT_SOFT, HCW_COLOR_TEXT_SOFT);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&swipe_control_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&swipe_control_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&swipe_control_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SWIPE_CONTROL_BOTTOM_ROW_WIDTH, SWIPE_CONTROL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_swipe_control_init(EGUI_VIEW_OF(&swipe_control_compact));
    egui_view_set_size(EGUI_VIEW_OF(&swipe_control_compact), SWIPE_CONTROL_PREVIEW_WIDTH, SWIPE_CONTROL_PREVIEW_HEIGHT);
    egui_view_swipe_control_set_font(EGUI_VIEW_OF(&swipe_control_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_swipe_control_set_meta_font(EGUI_VIEW_OF(&swipe_control_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_swipe_control_set_compact_mode(EGUI_VIEW_OF(&swipe_control_compact), 1);
    egui_view_swipe_control_set_palette(EGUI_VIEW_OF(&swipe_control_compact), HCW_COLOR_SURFACE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_TEXT_STRONG,
                                        HCW_COLOR_TEXT_SOFT, HCW_COLOR_TEXT_SOFT);
    egui_view_swipe_control_override_static_preview_api(EGUI_VIEW_OF(&swipe_control_compact), &swipe_control_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&swipe_control_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&swipe_control_compact));

    egui_view_swipe_control_init(EGUI_VIEW_OF(&swipe_control_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&swipe_control_read_only), SWIPE_CONTROL_PREVIEW_WIDTH, SWIPE_CONTROL_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&swipe_control_read_only), 8, 0, 0, 0);
    egui_view_swipe_control_set_font(EGUI_VIEW_OF(&swipe_control_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_swipe_control_set_meta_font(EGUI_VIEW_OF(&swipe_control_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_swipe_control_set_compact_mode(EGUI_VIEW_OF(&swipe_control_read_only), 1);
    egui_view_swipe_control_set_read_only_mode(EGUI_VIEW_OF(&swipe_control_read_only), 1);
    egui_view_swipe_control_set_palette(EGUI_VIEW_OF(&swipe_control_read_only), HCW_COLOR_SURFACE, HCW_COLOR_TRACK_STRONG, HCW_COLOR_TEXT,
                                        HCW_COLOR_TEXT_SOFT, HCW_COLOR_TEXT);
    egui_view_swipe_control_override_static_preview_api(EGUI_VIEW_OF(&swipe_control_read_only), &swipe_control_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&swipe_control_read_only), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&swipe_control_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
    focus_primary_swipe_control();
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
            apply_primary_default_state();
            apply_preview_states();
            focus_primary_swipe_control();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_LEFT);
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_track(1);
            focus_primary_swipe_control();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_track(2);
            focus_primary_swipe_control();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            apply_primary_default_state();
            focus_primary_swipe_control();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_FINAL_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
