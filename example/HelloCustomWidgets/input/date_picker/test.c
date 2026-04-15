#include "egui.h"
#include "egui_view_date_picker.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define DATE_PICKER_ROOT_WIDTH            224
#define DATE_PICKER_ROOT_HEIGHT           264
#define DATE_PICKER_PRIMARY_WIDTH         196
#define DATE_PICKER_PRIMARY_OPEN_HEIGHT   180
#define DATE_PICKER_PRIMARY_CLOSED_HEIGHT 82
#define DATE_PICKER_PREVIEW_WIDTH         104
#define DATE_PICKER_PREVIEW_HEIGHT        48
#define DATE_PICKER_BOTTOM_ROW_WIDTH      216
#define DATE_PICKER_BOTTOM_ROW_HEIGHT     48
#define DATE_PICKER_RECORD_WAIT           100
#define DATE_PICKER_RECORD_FRAME_WAIT     170
#define DATE_PICKER_RECORD_FINAL_WAIT     360
#define DATE_PICKER_DEFAULT_SNAPSHOT      0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct date_picker_snapshot date_picker_snapshot_t;
struct date_picker_snapshot
{
    uint16_t year;
    uint16_t panel_year;
    uint8_t month;
    uint8_t panel_month;
    uint8_t day;
    uint8_t opened;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_date_picker_t picker_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_date_picker_t picker_compact;
static egui_view_date_picker_t picker_read_only;
static egui_view_api_t picker_compact_api;
static egui_view_api_t picker_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Date Picker";

static const date_picker_snapshot_t primary_snapshots[] = {
        {2026, 2026, 3, 3, 18, 1},
        {2026, 2026, 3, 4, 18, 1},
        {2026, 2026, 4, 4, 2, 1},
};

static const date_picker_snapshot_t compact_snapshot = {2026, 2026, 3, 3, 18, 0};
static const date_picker_snapshot_t read_only_snapshot = {2026, 2026, 4, 4, 5, 0};

static void layout_page(void);
static void focus_primary_widget(void);

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
        if (!point_in_view_work_region(EGUI_VIEW_OF(&picker_primary), event->location.x, event->location.y))
        {
            egui_view_clear_focus(EGUI_VIEW_OF(&picker_primary));
        }
    }
#else
    EGUI_UNUSED(event);
#endif
    return 1;
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

static void sync_primary_layout(void)
{
    uint8_t opened = egui_view_date_picker_get_opened(EGUI_VIEW_OF(&picker_primary));

    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), opened ? EGUI_ALIGN_HCENTER : (EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER));
    egui_view_set_size(EGUI_VIEW_OF(&picker_primary), DATE_PICKER_PRIMARY_WIDTH, opened ? DATE_PICKER_PRIMARY_OPEN_HEIGHT : DATE_PICKER_PRIMARY_CLOSED_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&picker_primary), 0, 0, 0, 6);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_snapshot_to_picker(egui_view_t *view, const date_picker_snapshot_t *snapshot)
{
    egui_view_date_picker_set_date(view, snapshot->year, snapshot->month, snapshot->day);
    egui_view_date_picker_set_display_month(view, snapshot->panel_year, snapshot->panel_month);
    egui_view_date_picker_set_opened(view, snapshot->opened);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot_to_picker(EGUI_VIEW_OF(&picker_primary), &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT]);
    sync_primary_layout();
    if (ui_ready)
    {
        focus_primary_widget();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(DATE_PICKER_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    apply_snapshot_to_picker(EGUI_VIEW_OF(&picker_compact), &compact_snapshot);
    apply_snapshot_to_picker(EGUI_VIEW_OF(&picker_read_only), &read_only_snapshot);

    if (ui_ready)
    {
        layout_page();
    }
}

static void on_primary_open_changed(egui_view_t *self, uint8_t opened)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(opened);
    sync_primary_layout();
}

static void focus_primary_widget(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&picker_primary));
#endif
}

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    layout_page();
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), DATE_PICKER_ROOT_WIDTH, DATE_PICKER_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));
    static egui_view_api_t root_layout_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&root_layout), &root_layout_touch_api, dismiss_primary_focus_on_down);

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), DATE_PICKER_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_date_picker_init(EGUI_VIEW_OF(&picker_primary));
    egui_view_set_size(EGUI_VIEW_OF(&picker_primary), DATE_PICKER_PRIMARY_WIDTH, DATE_PICKER_PRIMARY_OPEN_HEIGHT);
    egui_view_date_picker_set_font(EGUI_VIEW_OF(&picker_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_date_picker_set_meta_font(EGUI_VIEW_OF(&picker_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_date_picker_set_label(EGUI_VIEW_OF(&picker_primary), "Ship date");
    egui_view_date_picker_set_helper(EGUI_VIEW_OF(&picker_primary), "Tap day or use +/-");
    egui_view_date_picker_set_today(EGUI_VIEW_OF(&picker_primary), 2026, 3, 15);
    egui_view_date_picker_set_first_day_of_week(EGUI_VIEW_OF(&picker_primary), 1);
    egui_view_date_picker_set_on_open_changed_listener(EGUI_VIEW_OF(&picker_primary), on_primary_open_changed);
    egui_view_date_picker_set_palette(EGUI_VIEW_OF(&picker_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_margin(EGUI_VIEW_OF(&picker_primary), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&picker_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), DATE_PICKER_BOTTOM_ROW_WIDTH, DATE_PICKER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_date_picker_init(EGUI_VIEW_OF(&picker_compact));
    egui_view_set_size(EGUI_VIEW_OF(&picker_compact), DATE_PICKER_PREVIEW_WIDTH, DATE_PICKER_PREVIEW_HEIGHT);
    egui_view_date_picker_set_font(EGUI_VIEW_OF(&picker_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_date_picker_set_meta_font(EGUI_VIEW_OF(&picker_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_date_picker_set_today(EGUI_VIEW_OF(&picker_compact), 2026, 3, 15);
    egui_view_date_picker_set_first_day_of_week(EGUI_VIEW_OF(&picker_compact), 1);
    egui_view_date_picker_set_compact_mode(EGUI_VIEW_OF(&picker_compact), 1);
    egui_view_date_picker_set_palette(EGUI_VIEW_OF(&picker_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_date_picker_override_static_preview_api(EGUI_VIEW_OF(&picker_compact), &picker_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&picker_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&picker_compact));

    egui_view_date_picker_init(EGUI_VIEW_OF(&picker_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&picker_read_only), DATE_PICKER_PREVIEW_WIDTH, DATE_PICKER_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&picker_read_only), 8, 0, 0, 0);
    egui_view_date_picker_set_font(EGUI_VIEW_OF(&picker_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_date_picker_set_meta_font(EGUI_VIEW_OF(&picker_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_date_picker_set_today(EGUI_VIEW_OF(&picker_read_only), 2026, 3, 15);
    egui_view_date_picker_set_first_day_of_week(EGUI_VIEW_OF(&picker_read_only), 1);
    egui_view_date_picker_set_compact_mode(EGUI_VIEW_OF(&picker_read_only), 1);
    egui_view_date_picker_set_read_only_mode(EGUI_VIEW_OF(&picker_read_only), 1);
    egui_view_date_picker_set_palette(EGUI_VIEW_OF(&picker_read_only), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x6B7A89),
                                      EGUI_COLOR_HEX(0x7A8796), EGUI_COLOR_HEX(0x7A8796), EGUI_COLOR_HEX(0x7A8796));
    egui_view_date_picker_override_static_preview_api(EGUI_VIEW_OF(&picker_read_only), &picker_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&picker_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&picker_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
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
            focus_primary_widget();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATE_PICKER_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, DATE_PICKER_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATE_PICKER_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, DATE_PICKER_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATE_PICKER_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, DATE_PICKER_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATE_PICKER_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
