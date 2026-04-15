#include "egui.h"
#include "egui_view_info_badge.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define INFO_BADGE_ROOT_WIDTH        224
#define INFO_BADGE_ROOT_HEIGHT       144
#define INFO_BADGE_ROW_WIDTH         176
#define INFO_BADGE_ROW_HEIGHT        24
#define INFO_BADGE_BOTTOM_ROW_WIDTH  60
#define INFO_BADGE_BOTTOM_ROW_HEIGHT 20
#define INFO_BADGE_RECORD_WAIT       90
#define INFO_BADGE_RECORD_FRAME_WAIT 170
#define INFO_BADGE_RECORD_FINAL_WAIT 280
#define INFO_BADGE_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct info_badge_snapshot info_badge_snapshot_t;
struct info_badge_snapshot
{
    const char *count_label;
    uint16_t count_value;
    egui_color_t count_color;
    const char *icon_label;
    const char *icon_text;
    egui_color_t icon_color;
    const char *dot_label;
    egui_color_t dot_color;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t count_row;
static egui_view_label_t count_row_label;
static egui_view_notification_badge_t count_badge;
static egui_view_linearlayout_t icon_row;
static egui_view_label_t icon_row_label;
static egui_view_notification_badge_t icon_badge;
static egui_view_linearlayout_t dot_row;
static egui_view_label_t dot_row_label;
static egui_view_notification_badge_t dot_badge;
static egui_view_linearlayout_t bottom_row;
static egui_view_notification_badge_t overflow_badge;
static egui_view_notification_badge_t attention_badge;
static egui_view_api_t overflow_badge_api;
static egui_view_api_t attention_badge_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "InfoBadge";

static const info_badge_snapshot_t primary_snapshots[] = {
        {
                "Inbox updates",
                18,
                EGUI_COLOR_HEX(0xC42B1C),
                "Policy note",
                EGUI_ICON_MS_INFO,
                EGUI_COLOR_HEX(0x0F6CBD),
                "Review pending",
                EGUI_COLOR_HEX(0xC42B1C),
        },
        {
                "Build blockers",
                7,
                EGUI_COLOR_HEX(0xC42B1C),
                "QA warning",
                EGUI_ICON_MS_WARNING,
                EGUI_COLOR_HEX(0xB95A00),
                "Deployment paused",
                EGUI_COLOR_HEX(0xC42B1C),
        },
        {
                "Finished checks",
                2,
                EGUI_COLOR_HEX(0x0F7B45),
                "Published",
                EGUI_ICON_MS_DONE,
                EGUI_COLOR_HEX(0x0F7B45),
                "Watch list",
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

static void init_row(egui_view_linearlayout_t *row)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(row));
    egui_view_set_size(EGUI_VIEW_OF(row), INFO_BADGE_ROW_WIDTH, INFO_BADGE_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(row), EGUI_ALIGN_VCENTER);
}

static void apply_primary_snapshot(uint8_t index)
{
    const info_badge_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    egui_view_label_set_text(EGUI_VIEW_OF(&count_row_label), snapshot->count_label);
    egui_view_label_set_text(EGUI_VIEW_OF(&icon_row_label), snapshot->icon_label);
    egui_view_label_set_text(EGUI_VIEW_OF(&dot_row_label), snapshot->dot_label);

    hcw_info_badge_apply_count_style(EGUI_VIEW_OF(&count_badge));
    hcw_info_badge_set_count(EGUI_VIEW_OF(&count_badge), snapshot->count_value);
    hcw_info_badge_set_palette(EGUI_VIEW_OF(&count_badge), snapshot->count_color, EGUI_COLOR_WHITE);

    hcw_info_badge_apply_icon_style(EGUI_VIEW_OF(&icon_badge));
    hcw_info_badge_set_icon(EGUI_VIEW_OF(&icon_badge), snapshot->icon_text);
    hcw_info_badge_set_palette(EGUI_VIEW_OF(&icon_badge), snapshot->icon_color, EGUI_COLOR_WHITE);

    hcw_info_badge_apply_attention_style(EGUI_VIEW_OF(&dot_badge));
    hcw_info_badge_set_palette(EGUI_VIEW_OF(&dot_badge), snapshot->dot_color, EGUI_COLOR_WHITE);

    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(INFO_BADGE_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    hcw_info_badge_apply_count_style(EGUI_VIEW_OF(&overflow_badge));
    hcw_info_badge_set_count(EGUI_VIEW_OF(&overflow_badge), 128);
    hcw_info_badge_set_palette(EGUI_VIEW_OF(&overflow_badge), EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_WHITE);
    egui_view_notification_badge_set_max_display(EGUI_VIEW_OF(&overflow_badge), 99);

    hcw_info_badge_apply_attention_style(EGUI_VIEW_OF(&attention_badge));
    hcw_info_badge_set_palette(EGUI_VIEW_OF(&attention_badge), EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_WHITE);

    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&count_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&icon_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&dot_row));
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), INFO_BADGE_ROOT_WIDTH, INFO_BADGE_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), INFO_BADGE_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_row(&count_row);
    egui_view_set_margin(EGUI_VIEW_OF(&count_row), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&count_row));

    init_text_label(&count_row_label, 124, 12, "Inbox updates", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&count_row), EGUI_VIEW_OF(&count_row_label));

    egui_view_notification_badge_init(EGUI_VIEW_OF(&count_badge));
    egui_view_set_size(EGUI_VIEW_OF(&count_badge), 34, 20);
    egui_view_set_margin(EGUI_VIEW_OF(&count_badge), 18, 0, 0, 0);
    egui_view_notification_badge_set_font(EGUI_VIEW_OF(&count_badge), (const egui_font_t *)&egui_res_font_montserrat_8_4);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&count_badge), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&count_row), EGUI_VIEW_OF(&count_badge));

    init_row(&icon_row);
    egui_view_set_margin(EGUI_VIEW_OF(&icon_row), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&icon_row));

    init_text_label(&icon_row_label, 124, 12, "Policy note", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&icon_row), EGUI_VIEW_OF(&icon_row_label));

    egui_view_notification_badge_init(EGUI_VIEW_OF(&icon_badge));
    egui_view_set_size(EGUI_VIEW_OF(&icon_badge), 20, 20);
    egui_view_set_margin(EGUI_VIEW_OF(&icon_badge), 32, 0, 0, 0);
    egui_view_notification_badge_set_icon_font(EGUI_VIEW_OF(&icon_badge), EGUI_FONT_ICON_MS_16);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&icon_badge), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&icon_row), EGUI_VIEW_OF(&icon_badge));

    init_row(&dot_row);
    egui_view_set_margin(EGUI_VIEW_OF(&dot_row), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&dot_row));

    init_text_label(&dot_row_label, 124, 12, "Review pending", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&dot_row), EGUI_VIEW_OF(&dot_row_label));

    egui_view_notification_badge_init(EGUI_VIEW_OF(&dot_badge));
    egui_view_set_size(EGUI_VIEW_OF(&dot_badge), 12, 12);
    egui_view_set_margin(EGUI_VIEW_OF(&dot_badge), 40, 0, 0, 0);
    egui_view_notification_badge_set_icon_font(EGUI_VIEW_OF(&dot_badge), EGUI_FONT_ICON_MS_16);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&dot_badge), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&dot_row), EGUI_VIEW_OF(&dot_badge));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), INFO_BADGE_BOTTOM_ROW_WIDTH, INFO_BADGE_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_notification_badge_init(EGUI_VIEW_OF(&overflow_badge));
    egui_view_set_size(EGUI_VIEW_OF(&overflow_badge), 40, 20);
    egui_view_notification_badge_set_font(EGUI_VIEW_OF(&overflow_badge), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_info_badge_override_static_preview_api(EGUI_VIEW_OF(&overflow_badge), &overflow_badge_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&overflow_badge), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&overflow_badge));

    egui_view_notification_badge_init(EGUI_VIEW_OF(&attention_badge));
    egui_view_set_size(EGUI_VIEW_OF(&attention_badge), 12, 12);
    egui_view_set_margin(EGUI_VIEW_OF(&attention_badge), 8, 0, 0, 0);
    egui_view_notification_badge_set_icon_font(EGUI_VIEW_OF(&attention_badge), EGUI_FONT_ICON_MS_16);
    hcw_info_badge_override_static_preview_api(EGUI_VIEW_OF(&attention_badge), &attention_badge_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&attention_badge), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&attention_badge));

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
        EGUI_SIM_SET_WAIT(p_action, INFO_BADGE_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BADGE_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BADGE_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BADGE_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BADGE_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, INFO_BADGE_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BADGE_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
