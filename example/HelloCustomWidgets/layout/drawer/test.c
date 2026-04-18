#include "egui.h"
#include "egui_view_drawer.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define DRAWER_ROOT_W             224
#define DRAWER_ROOT_H             216
#define DRAWER_PRIMARY_W          196
#define DRAWER_PRIMARY_H          112
#define DRAWER_PREVIEW_W          104
#define DRAWER_PREVIEW_H          72
#define DRAWER_BOTTOM_W           216
#define DRAWER_BOTTOM_H           72
#define DRAWER_RECORD_WAIT        90
#define DRAWER_RECORD_FRAME_WAIT  170
#define DRAWER_RECORD_FINAL_WAIT  280
#define DRAWER_DEFAULT_SNAPSHOT   0

#define PRIMARY_SNAPSHOT_COUNT    ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct
{
    const char *eyebrow;
    const char *title;
    const char *body_primary;
    const char *body_secondary;
    const char *footer;
    const char *tag;
    uint8_t anchor;
    uint8_t presentation_mode;
    uint8_t open;
} drawer_demo_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_drawer_t primary_drawer;
static egui_view_linearlayout_t bottom_row;
static egui_view_drawer_t compact_drawer;
static egui_view_drawer_t read_only_drawer;
static egui_view_api_t compact_api;
static egui_view_api_t read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Drawer";

static const drawer_demo_snapshot_t primary_snapshots[] = {
        {"F", "Filters", "Inline rail.", "Context stays.", "Inline", "I",
         EGUI_VIEW_DRAWER_ANCHOR_START, EGUI_VIEW_DRAWER_MODE_INLINE, 1},
        {"R", "Review", "Overlay rail.", "Light veil.", "Overlay", "O",
         EGUI_VIEW_DRAWER_ANCHOR_END, EGUI_VIEW_DRAWER_MODE_OVERLAY, 1},
        {"A", "Archive", "Toggle only.", "", "Closed", "C", EGUI_VIEW_DRAWER_ANCHOR_END, EGUI_VIEW_DRAWER_MODE_INLINE, 0},
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

static void apply_snapshot(egui_view_t *view, const drawer_demo_snapshot_t *snapshot)
{
    egui_view_drawer_set_eyebrow(view, snapshot->eyebrow);
    egui_view_drawer_set_title(view, snapshot->title);
    egui_view_drawer_set_body_primary(view, snapshot->body_primary);
    egui_view_drawer_set_body_secondary(view, snapshot->body_secondary);
    egui_view_drawer_set_footer(view, snapshot->footer);
    egui_view_drawer_set_tag(view, snapshot->tag);
    egui_view_drawer_set_anchor(view, snapshot->anchor);
    egui_view_drawer_set_presentation_mode(view, snapshot->presentation_mode);
    egui_view_drawer_set_open(view, snapshot->open);
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

static void apply_primary_state(uint8_t index)
{
    apply_snapshot(EGUI_VIEW_OF(&primary_drawer), &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT]);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_state(DRAWER_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    static const drawer_demo_snapshot_t compact_snapshot = {
            "C", "Quick", "Open rail.", "", "Preview", "",
            EGUI_VIEW_DRAWER_ANCHOR_START, EGUI_VIEW_DRAWER_MODE_OVERLAY, 1};
    static const drawer_demo_snapshot_t read_only_snapshot = {
            "RO", "Read", "Muted shell.", "", "Static", "",
            EGUI_VIEW_DRAWER_ANCHOR_END, EGUI_VIEW_DRAWER_MODE_INLINE, 1};

    apply_snapshot(EGUI_VIEW_OF(&compact_drawer), &compact_snapshot);
    egui_view_drawer_set_compact_mode(EGUI_VIEW_OF(&compact_drawer), 1);

    apply_snapshot(EGUI_VIEW_OF(&read_only_drawer), &read_only_snapshot);
    egui_view_drawer_set_compact_mode(EGUI_VIEW_OF(&read_only_drawer), 1);
    egui_view_drawer_set_read_only_mode(EGUI_VIEW_OF(&read_only_drawer), 1);
    if (ui_ready)
    {
        layout_page();
    }
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), DRAWER_ROOT_W, DRAWER_ROOT_H);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, DRAWER_ROOT_W, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_drawer_init(EGUI_VIEW_OF(&primary_drawer));
    egui_view_set_size(EGUI_VIEW_OF(&primary_drawer), DRAWER_PRIMARY_W, DRAWER_PRIMARY_H);
    egui_view_drawer_set_font(EGUI_VIEW_OF(&primary_drawer), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_drawer_set_meta_font(EGUI_VIEW_OF(&primary_drawer), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_drawer), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_drawer));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), DRAWER_BOTTOM_W, DRAWER_BOTTOM_H);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_drawer_init(EGUI_VIEW_OF(&compact_drawer));
    egui_view_set_size(EGUI_VIEW_OF(&compact_drawer), DRAWER_PREVIEW_W, DRAWER_PREVIEW_H);
    egui_view_drawer_set_font(EGUI_VIEW_OF(&compact_drawer), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_drawer_set_meta_font(EGUI_VIEW_OF(&compact_drawer), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_drawer_override_static_preview_api(EGUI_VIEW_OF(&compact_drawer), &compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_drawer), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_drawer));

    egui_view_drawer_init(EGUI_VIEW_OF(&read_only_drawer));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_drawer), DRAWER_PREVIEW_W, DRAWER_PREVIEW_H);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_drawer), 8, 0, 0, 0);
    egui_view_drawer_set_font(EGUI_VIEW_OF(&read_only_drawer), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_drawer_set_meta_font(EGUI_VIEW_OF(&read_only_drawer), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_drawer_override_static_preview_api(EGUI_VIEW_OF(&read_only_drawer), &read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_drawer), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_drawer));

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
        EGUI_SIM_SET_WAIT(p_action, DRAWER_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_state(1);
        }
        EGUI_SIM_SET_WAIT(p_action, DRAWER_RECORD_FINAL_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DRAWER_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_state(2);
        }
        EGUI_SIM_SET_WAIT(p_action, DRAWER_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DRAWER_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, DRAWER_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DRAWER_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
