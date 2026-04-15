#include "egui.h"
#include "egui_view_counter_badge.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define COUNTER_BADGE_ROOT_WIDTH        224
#define COUNTER_BADGE_ROOT_HEIGHT       148
#define COUNTER_BADGE_PRIMARY_WIDTH     48
#define COUNTER_BADGE_PRIMARY_HEIGHT    24
#define COUNTER_BADGE_PREVIEW_LEFT_W    18
#define COUNTER_BADGE_PREVIEW_RIGHT_W   22
#define COUNTER_BADGE_PREVIEW_HEIGHT    16
#define COUNTER_BADGE_BOTTOM_ROW_WIDTH  48
#define COUNTER_BADGE_BOTTOM_ROW_HEIGHT 16
#define COUNTER_BADGE_RECORD_WAIT       90
#define COUNTER_BADGE_RECORD_FRAME_WAIT 170
#define COUNTER_BADGE_RECORD_FINAL_WAIT 280
#define COUNTER_BADGE_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct counter_badge_snapshot counter_badge_snapshot_t;
struct counter_badge_snapshot
{
    uint16_t count;
    uint8_t max_display;
    uint8_t dot_mode;
    egui_color_t badge_color;
    const char *status_label;
    egui_color_t status_label_color;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_counter_badge_t primary_badge;
static egui_view_label_t primary_status_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_counter_badge_t compact_badge;
static egui_view_counter_badge_t read_only_badge;
static egui_view_api_t compact_api;
static egui_view_api_t read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "CounterBadge";

static const counter_badge_snapshot_t primary_snapshots[] = {
        {
                7,
                99,
                0,
                EGUI_COLOR_HEX(0xC42B1C),
                "Inbox queue / 7",
                EGUI_COLOR_HEX(0xC42B1C),
        },
        {
                128,
                99,
                0,
                EGUI_COLOR_HEX(0xC42B1C),
                "Escalation queue / 99+",
                EGUI_COLOR_HEX(0xC42B1C),
        },
        {
                1,
                99,
                1,
                EGUI_COLOR_HEX(0x0F6CBD),
                "Quiet watch / dot",
                EGUI_COLOR_HEX(0x0F6CBD),
        },
};

static void layout_page(void);

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

static void apply_primary_snapshot(uint8_t index)
{
    const counter_badge_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&primary_badge), snapshot->count);
    egui_view_counter_badge_set_max_display(EGUI_VIEW_OF(&primary_badge), snapshot->max_display);
    egui_view_counter_badge_set_dot_mode(EGUI_VIEW_OF(&primary_badge), snapshot->dot_mode);
    egui_view_counter_badge_set_palette(EGUI_VIEW_OF(&primary_badge), snapshot->badge_color, EGUI_COLOR_WHITE, EGUI_COLOR_WHITE);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_status_label), snapshot->status_label);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_status_label), snapshot->status_label_color, EGUI_ALPHA_100);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(COUNTER_BADGE_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&compact_badge), 4);
    egui_view_counter_badge_set_compact_mode(EGUI_VIEW_OF(&compact_badge), 1);

    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&read_only_badge), 12);
    egui_view_counter_badge_set_compact_mode(EGUI_VIEW_OF(&read_only_badge), 1);
    egui_view_counter_badge_set_read_only_mode(EGUI_VIEW_OF(&read_only_badge), 1);

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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), COUNTER_BADGE_ROOT_WIDTH, COUNTER_BADGE_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, COUNTER_BADGE_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_counter_badge_init(EGUI_VIEW_OF(&primary_badge));
    egui_view_set_size(EGUI_VIEW_OF(&primary_badge), COUNTER_BADGE_PRIMARY_WIDTH, COUNTER_BADGE_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_badge), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_badge), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_badge));

    init_text_label(&primary_status_label, COUNTER_BADGE_ROOT_WIDTH, 12, "Inbox queue / 7", (const egui_font_t *)&egui_res_font_montserrat_10_4,
                    EGUI_COLOR_HEX(0xC42B1C), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_status_label), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_status_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), COUNTER_BADGE_BOTTOM_ROW_WIDTH, COUNTER_BADGE_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_counter_badge_init(EGUI_VIEW_OF(&compact_badge));
    egui_view_set_size(EGUI_VIEW_OF(&compact_badge), COUNTER_BADGE_PREVIEW_LEFT_W, COUNTER_BADGE_PREVIEW_HEIGHT);
    egui_view_counter_badge_override_static_preview_api(EGUI_VIEW_OF(&compact_badge), &compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_badge), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_badge));

    egui_view_counter_badge_init(EGUI_VIEW_OF(&read_only_badge));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_badge), COUNTER_BADGE_PREVIEW_RIGHT_W, COUNTER_BADGE_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_badge), 8, 0, 0, 0);
    egui_view_counter_badge_override_static_preview_api(EGUI_VIEW_OF(&read_only_badge), &read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_badge), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_badge));

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
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COUNTER_BADGE_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
