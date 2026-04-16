#include "egui.h"
#include "egui_view_tag.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TAG_ROOT_WIDTH        224
#define TAG_ROOT_HEIGHT       140
#define TAG_PRIMARY_WIDTH     148
#define TAG_PRIMARY_HEIGHT    28
#define TAG_PREVIEW_WIDTH     88
#define TAG_PREVIEW_HEIGHT    24
#define TAG_BOTTOM_ROW_WIDTH  184
#define TAG_BOTTOM_ROW_HEIGHT 24
#define TAG_RECORD_WAIT       90
#define TAG_RECORD_FRAME_WAIT 170
#define TAG_RECORD_FINAL_WAIT 280
#define TAG_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct tag_snapshot tag_snapshot_t;
struct tag_snapshot
{
    const char *text;
    const char *secondary;
    const char *status_label;
    egui_color_t status_label_color;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t secondary_color;
    egui_color_t accent_color;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_tag_t primary_tag;
static egui_view_label_t primary_status_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_tag_t compact_tag;
static egui_view_tag_t read_only_tag;
static egui_view_api_t compact_api;
static egui_view_api_t read_only_api;
static uint8_t primary_snapshot_index;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Tag";

static const tag_snapshot_t primary_snapshots[] = {
        {
                "Assigned",
                "Today",
                "Assigned / standard",
                EGUI_COLOR_HEX(0x51606F),
                EGUI_COLOR_HEX(0xFFFFFF),
                EGUI_COLOR_HEX(0xD5DDE6),
                EGUI_COLOR_HEX(0x1F2A35),
                EGUI_COLOR_HEX(0x637283),
                EGUI_COLOR_HEX(0x0F6CBD),
        },
        {
                "Needs review",
                "2 files",
                "Needs review / warm",
                EGUI_COLOR_HEX(0xA15C00),
                EGUI_COLOR_HEX(0xFFFBF5),
                EGUI_COLOR_HEX(0xE4D7C7),
                EGUI_COLOR_HEX(0x332611),
                EGUI_COLOR_HEX(0x8C6E52),
                EGUI_COLOR_HEX(0xA15C00),
        },
        {
                "Pinned",
                "Release",
                "Pinned / calm",
                EGUI_COLOR_HEX(0x0F7B45),
                EGUI_COLOR_HEX(0xF7FBF8),
                EGUI_COLOR_HEX(0xD1E1D7),
                EGUI_COLOR_HEX(0x1D3327),
                EGUI_COLOR_HEX(0x5F7D69),
                EGUI_COLOR_HEX(0x0F7B45),
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
    const tag_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    primary_snapshot_index = (uint8_t)(index % PRIMARY_SNAPSHOT_COUNT);
    egui_view_tag_apply_standard_style(EGUI_VIEW_OF(&primary_tag));
    egui_view_tag_set_text(EGUI_VIEW_OF(&primary_tag), snapshot->text);
    egui_view_tag_set_secondary_text(EGUI_VIEW_OF(&primary_tag), snapshot->secondary);
    egui_view_tag_set_dismissible(EGUI_VIEW_OF(&primary_tag), 1);
    egui_view_tag_set_palette(EGUI_VIEW_OF(&primary_tag), snapshot->surface_color, snapshot->border_color, snapshot->text_color, snapshot->secondary_color,
                              snapshot->accent_color);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_status_label), snapshot->status_label);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_status_label), snapshot->status_label_color, EGUI_ALPHA_100);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(TAG_DEFAULT_SNAPSHOT);
}

static void apply_compact_state(void)
{
    egui_view_tag_apply_compact_style(EGUI_VIEW_OF(&compact_tag));
    egui_view_tag_set_text(EGUI_VIEW_OF(&compact_tag), "Compact");
    egui_view_tag_set_secondary_text(EGUI_VIEW_OF(&compact_tag), "Preview");
    egui_view_tag_set_dismissible(EGUI_VIEW_OF(&compact_tag), 1);
}

static void apply_read_only_state(void)
{
    egui_view_tag_apply_read_only_style(EGUI_VIEW_OF(&read_only_tag));
    egui_view_tag_set_text(EGUI_VIEW_OF(&read_only_tag), "System");
    egui_view_tag_set_secondary_text(EGUI_VIEW_OF(&read_only_tag), "Locked");
    egui_view_tag_set_dismissible(EGUI_VIEW_OF(&read_only_tag), 1);
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

static void on_primary_tag_dismiss(egui_view_t *self)
{
    EGUI_UNUSED(self);
    apply_primary_snapshot((uint8_t)(primary_snapshot_index + 1));
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TAG_ROOT_WIDTH, TAG_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, TAG_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_tag_init(EGUI_VIEW_OF(&primary_tag));
    egui_view_set_size(EGUI_VIEW_OF(&primary_tag), TAG_PRIMARY_WIDTH, TAG_PRIMARY_HEIGHT);
    egui_view_tag_set_font(EGUI_VIEW_OF(&primary_tag), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_tag_set_secondary_font(EGUI_VIEW_OF(&primary_tag), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tag_set_on_dismiss_listener(EGUI_VIEW_OF(&primary_tag), on_primary_tag_dismiss);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_tag), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_tag));

    init_text_label(&primary_status_label, TAG_ROOT_WIDTH, 12, "Assigned / standard", (const egui_font_t *)&egui_res_font_montserrat_10_4,
                    EGUI_COLOR_HEX(0x51606F), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_status_label), 0, 0, 0, 14);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_status_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TAG_BOTTOM_ROW_WIDTH, TAG_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_tag_init(EGUI_VIEW_OF(&compact_tag));
    egui_view_set_size(EGUI_VIEW_OF(&compact_tag), TAG_PREVIEW_WIDTH, TAG_PREVIEW_HEIGHT);
    egui_view_tag_set_font(EGUI_VIEW_OF(&compact_tag), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tag_set_secondary_font(EGUI_VIEW_OF(&compact_tag), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tag_override_static_preview_api(EGUI_VIEW_OF(&compact_tag), &compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_tag), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_tag));

    egui_view_tag_init(EGUI_VIEW_OF(&read_only_tag));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_tag), TAG_PREVIEW_WIDTH, TAG_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_tag), 8, 0, 0, 0);
    egui_view_tag_set_font(EGUI_VIEW_OF(&read_only_tag), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tag_set_secondary_font(EGUI_VIEW_OF(&read_only_tag), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tag_override_static_preview_api(EGUI_VIEW_OF(&read_only_tag), &read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_tag), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_tag));

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
        EGUI_SIM_SET_WAIT(p_action, TAG_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, TAG_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TAG_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, TAG_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TAG_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TAG_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
