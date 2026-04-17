#include <string.h>

#include "egui.h"
#include "egui_view_calendar_view.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define CALENDAR_VIEW_ROOT_WIDTH        224
#define CALENDAR_VIEW_ROOT_HEIGHT       232
#define CALENDAR_VIEW_PRIMARY_WIDTH     196
#define CALENDAR_VIEW_PRIMARY_HEIGHT    144
#define CALENDAR_VIEW_PREVIEW_WIDTH     104
#define CALENDAR_VIEW_PREVIEW_HEIGHT    50
#define CALENDAR_VIEW_BOTTOM_ROW_WIDTH  216
#define CALENDAR_VIEW_BOTTOM_ROW_HEIGHT 50
#define CALENDAR_VIEW_RECORD_WAIT       100
#define CALENDAR_VIEW_RECORD_FRAME_WAIT 150
#define CALENDAR_VIEW_RECORD_FINAL_WAIT 420
#define CALENDAR_VIEW_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct calendar_view_snapshot calendar_view_snapshot_t;
struct calendar_view_snapshot
{
    const char *label;
    const char *helper;
    uint16_t selection_year;
    uint8_t selection_month;
    uint8_t start_day;
    uint8_t end_day;
    uint16_t display_year;
    uint8_t display_month;
    uint16_t today_year;
    uint8_t today_month;
    uint8_t today_day;
    uint8_t current_part;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_calendar_view_t calendar_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_calendar_view_t calendar_compact;
static egui_view_calendar_view_t calendar_read_only;
static egui_view_api_t calendar_compact_api;
static egui_view_api_t calendar_read_only_api;
static uint8_t ui_ready;

static void layout_page(void);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Calendar View";

static const calendar_view_snapshot_t primary_snapshots[] = {
        {"Booking window", "Enter anchors a range, +/- browses months", 2026, 3, 9, 13, 2026, 3, 2026, 3, 15, EGUI_VIEW_CALENDAR_VIEW_PART_GRID},
        {"Release freeze", "Preview stays static below", 2026, 11, 3, 7, 2026, 11, 2026, 11, 5, EGUI_VIEW_CALENDAR_VIEW_PART_GRID},
};

static const calendar_view_snapshot_t compact_snapshots[] = {
        {"", "", 2026, 5, 5, 8, 2026, 5, 2026, 5, 6, EGUI_VIEW_CALENDAR_VIEW_PART_GRID},
};

static const calendar_view_snapshot_t read_only_snapshot = {"", "", 2026, 7, 18, 22, 2026, 7, 2026, 7, 20, EGUI_VIEW_CALENDAR_VIEW_PART_GRID};

static void dismiss_primary_focus(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&calendar_primary));
#endif
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static uint8_t point_in_view_work_region(egui_view_t *view, egui_dim_t x, egui_dim_t y)
{
    egui_region_t region;

    egui_view_get_work_region(view, &region);
    return egui_region_pt_in_rect(&region, x, y) ? 1 : 0;
}
#endif

static int dismiss_primary_focus_on_down(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        if (!point_in_view_work_region(EGUI_VIEW_OF(&calendar_primary), event->location.x, event->location.y))
        {
            dismiss_primary_focus();
        }
    }
#else
    EGUI_UNUSED(event);
#endif
    return 1;
}

static void apply_snapshot(egui_view_t *view, const calendar_view_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return;
    }

    egui_view_calendar_view_set_label(view, snapshot->label != NULL ? snapshot->label : "");
    egui_view_calendar_view_set_helper(view, snapshot->helper != NULL ? snapshot->helper : "");
    egui_view_calendar_view_set_today(view, snapshot->today_year, snapshot->today_month, snapshot->today_day);
    egui_view_calendar_view_set_display_month(view, snapshot->display_year, snapshot->display_month);
    egui_view_calendar_view_set_range(view, snapshot->selection_year, snapshot->selection_month, snapshot->start_day, snapshot->end_day);
    egui_view_calendar_view_set_current_part(view, snapshot->current_part);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot(EGUI_VIEW_OF(&calendar_primary), &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT]);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(CALENDAR_VIEW_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    apply_snapshot(EGUI_VIEW_OF(&calendar_compact), &compact_snapshots[0]);
    apply_snapshot(EGUI_VIEW_OF(&calendar_read_only), &read_only_snapshot);
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
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

static void focus_primary_calendar_view(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&calendar_primary));
#endif
}

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    layout_page();
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}

static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    focus_primary_calendar_view();
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&calendar_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&calendar_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&calendar_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&calendar_primary), &event);
    if (ui_ready)
    {
        layout_page();
    }
}
#endif

void test_init_ui(void)
{
    ui_ready = 0;
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), CALENDAR_VIEW_ROOT_WIDTH, CALENDAR_VIEW_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));
    static egui_view_api_t root_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&root_layout), &root_touch_api, dismiss_primary_focus_on_down);

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), CALENDAR_VIEW_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_calendar_view_init(EGUI_VIEW_OF(&calendar_primary));
    egui_view_set_size(EGUI_VIEW_OF(&calendar_primary), CALENDAR_VIEW_PRIMARY_WIDTH, CALENDAR_VIEW_PRIMARY_HEIGHT);
    egui_view_calendar_view_set_font(EGUI_VIEW_OF(&calendar_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_calendar_view_set_meta_font(EGUI_VIEW_OF(&calendar_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_calendar_view_set_first_day_of_week(EGUI_VIEW_OF(&calendar_primary), 1);
    egui_view_calendar_view_set_palette(EGUI_VIEW_OF(&calendar_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                        EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_margin(EGUI_VIEW_OF(&calendar_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&calendar_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), CALENDAR_VIEW_BOTTOM_ROW_WIDTH, CALENDAR_VIEW_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_calendar_view_init(EGUI_VIEW_OF(&calendar_compact));
    egui_view_set_size(EGUI_VIEW_OF(&calendar_compact), CALENDAR_VIEW_PREVIEW_WIDTH, CALENDAR_VIEW_PREVIEW_HEIGHT);
    egui_view_calendar_view_set_font(EGUI_VIEW_OF(&calendar_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_calendar_view_set_meta_font(EGUI_VIEW_OF(&calendar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_calendar_view_set_first_day_of_week(EGUI_VIEW_OF(&calendar_compact), 1);
    egui_view_calendar_view_set_compact_mode(EGUI_VIEW_OF(&calendar_compact), 1);
    egui_view_calendar_view_set_palette(EGUI_VIEW_OF(&calendar_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                        EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_calendar_view_override_static_preview_api(EGUI_VIEW_OF(&calendar_compact), &calendar_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&calendar_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&calendar_compact));

    egui_view_calendar_view_init(EGUI_VIEW_OF(&calendar_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&calendar_read_only), CALENDAR_VIEW_PREVIEW_WIDTH, CALENDAR_VIEW_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&calendar_read_only), 8, 0, 0, 0);
    egui_view_calendar_view_set_font(EGUI_VIEW_OF(&calendar_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_calendar_view_set_meta_font(EGUI_VIEW_OF(&calendar_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_calendar_view_set_first_day_of_week(EGUI_VIEW_OF(&calendar_read_only), 1);
    egui_view_calendar_view_set_compact_mode(EGUI_VIEW_OF(&calendar_read_only), 1);
    egui_view_calendar_view_set_read_only_mode(EGUI_VIEW_OF(&calendar_read_only), 1);
    egui_view_calendar_view_set_palette(EGUI_VIEW_OF(&calendar_read_only), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x6B7A89),
                                        EGUI_COLOR_HEX(0x7A8796), EGUI_COLOR_HEX(0x7A8796), EGUI_COLOR_HEX(0x7A8796));
    egui_view_calendar_view_override_static_preview_api(EGUI_VIEW_OF(&calendar_read_only), &calendar_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&calendar_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&calendar_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
    focus_primary_calendar_view();
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
            focus_primary_calendar_view();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CALENDAR_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_ENTER);
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, CALENDAR_VIEW_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CALENDAR_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_ENTER);
            apply_primary_key(EGUI_KEY_CODE_PLUS);
        }
        EGUI_SIM_SET_WAIT(p_action, CALENDAR_VIEW_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CALENDAR_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(1);
            focus_primary_calendar_view();
        }
        EGUI_SIM_SET_WAIT(p_action, CALENDAR_VIEW_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CALENDAR_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 7:
        EGUI_SIM_SET_WAIT(p_action, CALENDAR_VIEW_RECORD_FINAL_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CALENDAR_VIEW_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
