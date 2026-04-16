#include "egui.h"
#include "egui_view_selector_bar.h"
#include "uicode.h"
#include "demo_scaffold.h"
#include "resource/egui_icon_material_symbols.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SELECTOR_BAR_ROOT_WIDTH            224
#define SELECTOR_BAR_ROOT_HEIGHT           206
#define SELECTOR_BAR_PANEL_WIDTH           196
#define SELECTOR_BAR_PANEL_HEIGHT          104
#define SELECTOR_BAR_PRIMARY_WIDTH         180
#define SELECTOR_BAR_PRIMARY_HEIGHT        54
#define SELECTOR_BAR_PREVIEW_PANEL_WIDTH   104
#define SELECTOR_BAR_PREVIEW_PANEL_HEIGHT  72
#define SELECTOR_BAR_PREVIEW_WIDTH         84
#define SELECTOR_BAR_PREVIEW_HEIGHT        42
#define SELECTOR_BAR_BOTTOM_ROW_WIDTH      216
#define SELECTOR_BAR_BOTTOM_ROW_HEIGHT     72
#define SELECTOR_BAR_RECORD_WAIT           90
#define SELECTOR_BAR_RECORD_FRAME_WAIT     170
#define SELECTOR_BAR_RECORD_FINAL_WAIT     520

typedef struct
{
    const char *heading;
    const char *note;
    egui_color_t accent_color;
    uint8_t selected_index;
} selector_bar_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_selector_bar_t primary_bar;
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_panel;
static egui_view_label_t compact_heading_label;
static egui_view_selector_bar_t compact_bar;
static egui_view_linearlayout_t icon_only_panel;
static egui_view_label_t icon_only_heading_label;
static egui_view_selector_bar_t icon_only_bar;
static egui_view_api_t compact_bar_api;
static egui_view_api_t icon_only_bar_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

static const char *title_text = "SelectorBar";
static const char *primary_items[] = {"Recent", "Search", "Saved"};
static const char *primary_icons[] = {EGUI_ICON_MS_SCHEDULE, EGUI_ICON_MS_SEARCH, EGUI_ICON_MS_FAVORITE};
static const char *compact_items[] = {"Home", "Find"};
static const char *compact_icons[] = {EGUI_ICON_MS_HOME, EGUI_ICON_MS_SEARCH};
static const char *icon_only_icons[] = {EGUI_ICON_MS_HOME, EGUI_ICON_MS_SEARCH, EGUI_ICON_MS_SETTINGS};

static const selector_bar_snapshot_t primary_snapshots[] = {
        {
                "Recent activity",
                "A lightweight top-level switch keeps a small set of destinations visible.",
                EGUI_COLOR_HEX(0x0F6CBD),
                0,
        },
        {
                "Quick search",
                "Icon and label stay paired without inflating the navigation chrome.",
                EGUI_COLOR_HEX(0xA15C00),
                1,
        },
        {
                "Pinned collection",
                "SelectorBar stays lighter than a full tab shell for page-to-page hops.",
                EGUI_COLOR_HEX(0x0F7B45),
                2,
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

static void init_panel(egui_view_linearlayout_t *panel, egui_dim_t width, egui_dim_t height, egui_background_t *background, uint8_t align)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(panel));
    egui_view_set_size(EGUI_VIEW_OF(panel), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(panel), align);
    egui_view_set_background(EGUI_VIEW_OF(panel), background);
    egui_view_set_padding(EGUI_VIEW_OF(panel), 8, 8, 8, 8);
}

static void apply_primary_snapshot(uint8_t index)
{
    const selector_bar_snapshot_t *snapshot = &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)];

    egui_view_label_set_text(EGUI_VIEW_OF(&primary_heading_label), snapshot->heading);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_note_label), snapshot->note);
    egui_view_selector_bar_set_palette(EGUI_VIEW_OF(&primary_bar), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD3DBE3), EGUI_COLOR_HEX(0x17212C),
                                       EGUI_COLOR_HEX(0x6C7A88), snapshot->accent_color);
    egui_view_selector_bar_set_current_index(EGUI_VIEW_OF(&primary_bar), snapshot->selected_index);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_compact_state(void)
{
    egui_view_selector_bar_set_palette(EGUI_VIEW_OF(&compact_bar), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD8E0E8), EGUI_COLOR_HEX(0x22303C),
                                       EGUI_COLOR_HEX(0x6E7C89), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_selector_bar_set_compact_mode(EGUI_VIEW_OF(&compact_bar), 1);
    egui_view_selector_bar_set_current_index(EGUI_VIEW_OF(&compact_bar), 0);
}

static void apply_icon_only_state(void)
{
    egui_view_selector_bar_set_palette(EGUI_VIEW_OF(&icon_only_bar), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xDCE3EA), EGUI_COLOR_HEX(0x21303D),
                                       EGUI_COLOR_HEX(0x7A8895), EGUI_COLOR_HEX(0x0F7B45));
    egui_view_selector_bar_set_compact_mode(EGUI_VIEW_OF(&icon_only_bar), 1);
    egui_view_selector_bar_set_current_index(EGUI_VIEW_OF(&icon_only_bar), 1);
}

static void apply_preview_states(void)
{
    apply_compact_state();
    apply_icon_only_state();
    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&icon_only_panel));
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
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SELECTOR_BAR_ROOT_WIDTH, SELECTOR_BAR_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, SELECTOR_BAR_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, SELECTOR_BAR_PANEL_WIDTH, SELECTOR_BAR_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, "Recent activity", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x51606F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 5);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    egui_view_selector_bar_init(EGUI_VIEW_OF(&primary_bar));
    egui_view_set_size(EGUI_VIEW_OF(&primary_bar), SELECTOR_BAR_PRIMARY_WIDTH, SELECTOR_BAR_PRIMARY_HEIGHT);
    egui_view_selector_bar_set_items(EGUI_VIEW_OF(&primary_bar), primary_items, primary_icons, 3);
    egui_view_selector_bar_set_font(EGUI_VIEW_OF(&primary_bar), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_selector_bar_set_icon_font(EGUI_VIEW_OF(&primary_bar), EGUI_FONT_ICON_MS_20);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_bar), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_bar));

    init_text_label(&primary_note_label, 176, 18, "A lightweight top-level switch keeps a small set of destinations visible.",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SELECTOR_BAR_BOTTOM_ROW_WIDTH, SELECTOR_BAR_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&compact_panel, SELECTOR_BAR_PREVIEW_PANEL_WIDTH, SELECTOR_BAR_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_panel));

    init_text_label(&compact_heading_label, 84, 12, "Compact", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_heading_label));

    egui_view_selector_bar_init(EGUI_VIEW_OF(&compact_bar));
    egui_view_set_size(EGUI_VIEW_OF(&compact_bar), SELECTOR_BAR_PREVIEW_WIDTH, SELECTOR_BAR_PREVIEW_HEIGHT);
    egui_view_selector_bar_set_items(EGUI_VIEW_OF(&compact_bar), compact_items, compact_icons, 2);
    egui_view_selector_bar_set_font(EGUI_VIEW_OF(&compact_bar), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_selector_bar_set_icon_font(EGUI_VIEW_OF(&compact_bar), EGUI_FONT_ICON_MS_16);
    egui_view_selector_bar_set_compact_mode(EGUI_VIEW_OF(&compact_bar), 1);
    egui_view_selector_bar_override_static_preview_api(EGUI_VIEW_OF(&compact_bar), &compact_bar_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_bar), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_bar));

    init_panel(&icon_only_panel, SELECTOR_BAR_PREVIEW_PANEL_WIDTH, SELECTOR_BAR_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&icon_only_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&icon_only_panel));

    init_text_label(&icon_only_heading_label, 84, 12, "Icon only", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&icon_only_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&icon_only_panel), EGUI_VIEW_OF(&icon_only_heading_label));

    egui_view_selector_bar_init(EGUI_VIEW_OF(&icon_only_bar));
    egui_view_set_size(EGUI_VIEW_OF(&icon_only_bar), SELECTOR_BAR_PREVIEW_WIDTH, SELECTOR_BAR_PREVIEW_HEIGHT);
    egui_view_selector_bar_set_items(EGUI_VIEW_OF(&icon_only_bar), NULL, icon_only_icons, 3);
    egui_view_selector_bar_set_icon_font(EGUI_VIEW_OF(&icon_only_bar), EGUI_FONT_ICON_MS_16);
    egui_view_selector_bar_set_compact_mode(EGUI_VIEW_OF(&icon_only_bar), 1);
    egui_view_selector_bar_override_static_preview_api(EGUI_VIEW_OF(&icon_only_bar), &icon_only_bar_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&icon_only_bar), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&icon_only_panel), EGUI_VIEW_OF(&icon_only_bar));

    apply_primary_snapshot(0);
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_snapshot(0);
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
            apply_primary_snapshot(0);
            apply_preview_states();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SELECTOR_BAR_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, SELECTOR_BAR_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SELECTOR_BAR_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, SELECTOR_BAR_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SELECTOR_BAR_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(0);
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, SELECTOR_BAR_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SELECTOR_BAR_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
