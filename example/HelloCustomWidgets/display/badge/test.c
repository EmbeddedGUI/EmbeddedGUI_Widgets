#include "egui.h"
#include "egui_view_badge.h"
#include "uicode.h"
#include "demo_scaffold.h"

#include "../../../../sdk/EmbeddedGUI/src/resource/egui_icon_material_symbols.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define BADGE_ROOT_WIDTH        224
#define BADGE_ROOT_HEIGHT       112
#define BADGE_PRIMARY_WIDTH     126
#define BADGE_PRIMARY_HEIGHT    28
#define BADGE_PREVIEW_WIDTH     88
#define BADGE_PREVIEW_HEIGHT    24
#define BADGE_BOTTOM_ROW_WIDTH  184
#define BADGE_BOTTOM_ROW_HEIGHT 24
#define BADGE_RECORD_WAIT       90
#define BADGE_RECORD_FRAME_WAIT 170
#define BADGE_RECORD_FINAL_WAIT 280
#define BADGE_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef enum badge_style badge_style_t;
enum badge_style
{
    BADGE_STYLE_FILLED = 0,
    BADGE_STYLE_OUTLINE,
    BADGE_STYLE_SUBTLE,
};

typedef struct badge_snapshot badge_snapshot_t;
struct badge_snapshot
{
    const char *text;
    const char *icon;
    badge_style_t style;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t accent_color;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_badge_t primary_badge;
static egui_view_linearlayout_t bottom_row;
static egui_view_badge_t compact_badge;
static egui_view_badge_t read_only_badge;
static egui_view_api_t compact_api;
static egui_view_api_t read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Badge";

static const badge_snapshot_t primary_snapshots[] = {
        {
                "Verified",
                EGUI_ICON_MS_DONE,
                BADGE_STYLE_FILLED,
                EGUI_COLOR_HEX(0x0F7B45),
                EGUI_COLOR_HEX(0x0F7B45),
                EGUI_COLOR_WHITE,
                EGUI_COLOR_WHITE,
        },
        {
                "Preview",
                EGUI_ICON_MS_INFO,
                BADGE_STYLE_OUTLINE,
                EGUI_COLOR_HEX(0xFFFFFF),
                EGUI_COLOR_HEX(0xB7CBE7),
                EGUI_COLOR_HEX(0x0F548C),
                EGUI_COLOR_HEX(0x0F6CBD),
        },
        {
                "Needs review",
                EGUI_ICON_MS_WARNING,
                BADGE_STYLE_SUBTLE,
                EGUI_COLOR_HEX(0xFFF6EE),
                EGUI_COLOR_HEX(0xF2D4B2),
                EGUI_COLOR_HEX(0x8F4C11),
                EGUI_COLOR_HEX(0xB95A00),
        },
};

static void layout_page(void);

static void apply_snapshot(egui_view_badge_t *badge, const badge_snapshot_t *snapshot)
{
    switch (snapshot->style)
    {
    case BADGE_STYLE_FILLED:
        egui_view_badge_apply_filled_style(EGUI_VIEW_OF(badge));
        break;
    case BADGE_STYLE_OUTLINE:
        egui_view_badge_apply_outline_style(EGUI_VIEW_OF(badge));
        break;
    default:
        egui_view_badge_apply_subtle_style(EGUI_VIEW_OF(badge));
        break;
    }

    egui_view_badge_set_text(EGUI_VIEW_OF(badge), snapshot->text);
    egui_view_badge_set_icon(EGUI_VIEW_OF(badge), snapshot->icon);
    egui_view_badge_set_palette(EGUI_VIEW_OF(badge), snapshot->surface_color, snapshot->border_color, snapshot->text_color, snapshot->accent_color);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot(&primary_badge, &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT]);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(BADGE_DEFAULT_SNAPSHOT);
}

static void apply_compact_state(void)
{
    egui_view_badge_apply_outline_style(EGUI_VIEW_OF(&compact_badge));
    egui_view_badge_set_text(EGUI_VIEW_OF(&compact_badge), "Beta");
    egui_view_badge_set_icon(EGUI_VIEW_OF(&compact_badge), EGUI_ICON_MS_INFO);
    egui_view_badge_set_palette(EGUI_VIEW_OF(&compact_badge), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xC6D5E8), EGUI_COLOR_HEX(0x124B78),
                                EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_badge_set_compact_mode(EGUI_VIEW_OF(&compact_badge), 1);
}

static void apply_read_only_state(void)
{
    egui_view_badge_apply_read_only_style(EGUI_VIEW_OF(&read_only_badge));
    egui_view_badge_set_text(EGUI_VIEW_OF(&read_only_badge), "Archived");
    egui_view_badge_set_icon(EGUI_VIEW_OF(&read_only_badge), NULL);
}

static void apply_preview_states(void)
{
    apply_compact_state();
    apply_read_only_state();

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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), BADGE_ROOT_WIDTH, BADGE_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), BADGE_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_badge_init(EGUI_VIEW_OF(&primary_badge));
    egui_view_set_size(EGUI_VIEW_OF(&primary_badge), BADGE_PRIMARY_WIDTH, BADGE_PRIMARY_HEIGHT);
    egui_view_badge_set_font(EGUI_VIEW_OF(&primary_badge), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_badge_set_icon_font(EGUI_VIEW_OF(&primary_badge), EGUI_FONT_ICON_MS_16);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_badge), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_badge), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_badge));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), BADGE_BOTTOM_ROW_WIDTH, BADGE_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_badge_init(EGUI_VIEW_OF(&compact_badge));
    egui_view_set_size(EGUI_VIEW_OF(&compact_badge), BADGE_PREVIEW_WIDTH, BADGE_PREVIEW_HEIGHT);
    egui_view_badge_set_font(EGUI_VIEW_OF(&compact_badge), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_badge_set_icon_font(EGUI_VIEW_OF(&compact_badge), EGUI_FONT_ICON_MS_16);
    egui_view_badge_override_static_preview_api(EGUI_VIEW_OF(&compact_badge), &compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_badge), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_badge));

    egui_view_badge_init(EGUI_VIEW_OF(&read_only_badge));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_badge), BADGE_PREVIEW_WIDTH, BADGE_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_badge), 8, 0, 0, 0);
    egui_view_badge_set_font(EGUI_VIEW_OF(&read_only_badge), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_badge_set_icon_font(EGUI_VIEW_OF(&read_only_badge), EGUI_FONT_ICON_MS_16);
    egui_view_badge_override_static_preview_api(EGUI_VIEW_OF(&read_only_badge), &read_only_api);
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
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
