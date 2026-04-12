#include "egui.h"
#include "egui_view_animated_icon.h"
#include "uicode.h"
#include "demo_scaffold.h"
#include "resource/egui_icon_material_symbols.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define ANIMATED_ICON_ROOT_WIDTH        224
#define ANIMATED_ICON_ROOT_HEIGHT       220
#define ANIMATED_ICON_PANEL_WIDTH       196
#define ANIMATED_ICON_PANEL_HEIGHT      118
#define ANIMATED_ICON_PRIMARY_SIZE      60
#define ANIMATED_ICON_PREVIEW_WIDTH     104
#define ANIMATED_ICON_PREVIEW_HEIGHT    72
#define ANIMATED_ICON_PREVIEW_SIZE      28
#define ANIMATED_ICON_BOTTOM_ROW_WIDTH  216
#define ANIMATED_ICON_BOTTOM_ROW_HEIGHT 72
#define ANIMATED_ICON_RECORD_WAIT       90
#define ANIMATED_ICON_RECORD_FRAME_WAIT 170
#define ANIMATED_ICON_RECORD_ANIM_WAIT  260

typedef struct
{
    const char *heading;
    const char *state_name;
    egui_color_t color;
    const char *note;
} animated_icon_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_animated_icon_t primary_icon;
static egui_view_label_t primary_state_label;
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t chevron_panel;
static egui_view_label_t chevron_heading_label;
static egui_view_animated_icon_t chevron_icon;
static egui_view_label_t chevron_body_label;
static egui_view_linearlayout_t fallback_panel;
static egui_view_label_t fallback_heading_label;
static egui_view_animated_icon_t fallback_icon;
static egui_view_label_t fallback_body_label;
static egui_view_api_t chevron_icon_api;
static egui_view_api_t fallback_icon_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

static const char *title_text = "AnimatedIcon";

static const animated_icon_snapshot_t primary_snapshots[] = {
        {
                "Back / Normal",
                "Normal",
                EGUI_COLOR_HEX(0x0F6CBD),
                "Named states drive the icon without changing layout.",
        },
        {
                "Back / PointerOver",
                "PointerOver",
                EGUI_COLOR_HEX(0x0F6CBD),
                "A state change plays a short motion instead of swapping bitmaps.",
        },
        {
                "Back / Pressed",
                "Pressed",
                EGUI_COLOR_HEX(0x0F6CBD),
                "Pressed tightens the stroke and lands on the final marker.",
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
    const animated_icon_snapshot_t *snapshot = &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)];

    egui_view_label_set_text(EGUI_VIEW_OF(&primary_heading_label), snapshot->heading);
    egui_view_animated_icon_set_source(EGUI_VIEW_OF(&primary_icon), egui_view_animated_icon_get_back_source());
    egui_view_animated_icon_set_palette(EGUI_VIEW_OF(&primary_icon), snapshot->color);
    egui_view_animated_icon_set_animation_enabled(EGUI_VIEW_OF(&primary_icon), 1);
    egui_view_animated_icon_set_state(EGUI_VIEW_OF(&primary_icon), snapshot->state_name);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_state_label), snapshot->state_name);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_state_label), snapshot->color, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_note_label), snapshot->note);
}

static void apply_preview_states(void)
{
    egui_view_animated_icon_set_source(EGUI_VIEW_OF(&chevron_icon), egui_view_animated_icon_get_chevron_down_small_source());
    egui_view_animated_icon_apply_subtle_style(EGUI_VIEW_OF(&chevron_icon));
    egui_view_animated_icon_set_animation_enabled(EGUI_VIEW_OF(&chevron_icon), 0);
    egui_view_animated_icon_set_state(EGUI_VIEW_OF(&chevron_icon), "Pressed");

    egui_view_animated_icon_set_source(EGUI_VIEW_OF(&fallback_icon), NULL);
    egui_view_animated_icon_apply_accent_style(EGUI_VIEW_OF(&fallback_icon));
    egui_view_animated_icon_set_fallback_glyph(EGUI_VIEW_OF(&fallback_icon), EGUI_ICON_MS_SETTINGS);
    egui_view_animated_icon_set_animation_enabled(EGUI_VIEW_OF(&fallback_icon), 0);
    egui_view_animated_icon_set_state(EGUI_VIEW_OF(&fallback_icon), "Normal");
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), ANIMATED_ICON_ROOT_WIDTH, ANIMATED_ICON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, ANIMATED_ICON_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, ANIMATED_ICON_PANEL_WIDTH, ANIMATED_ICON_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, "Back / Normal", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x51606F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    egui_view_animated_icon_init(EGUI_VIEW_OF(&primary_icon));
    egui_view_set_size(EGUI_VIEW_OF(&primary_icon), ANIMATED_ICON_PRIMARY_SIZE, ANIMATED_ICON_PRIMARY_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_icon), 0, 0, 0, 5);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_icon));

    init_text_label(&primary_state_label, 176, 12, "Normal", (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_COLOR_HEX(0x0F6CBD),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_state_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_state_label));

    init_text_label(&primary_note_label, 176, 18, "Named states drive the icon without changing layout.",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), ANIMATED_ICON_BOTTOM_ROW_WIDTH, ANIMATED_ICON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&chevron_panel, ANIMATED_ICON_PREVIEW_WIDTH, ANIMATED_ICON_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&chevron_panel));

    init_text_label(&chevron_heading_label, 84, 12, "Chevron", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&chevron_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&chevron_panel), EGUI_VIEW_OF(&chevron_heading_label));

    egui_view_animated_icon_init(EGUI_VIEW_OF(&chevron_icon));
    egui_view_set_size(EGUI_VIEW_OF(&chevron_icon), ANIMATED_ICON_PREVIEW_SIZE, ANIMATED_ICON_PREVIEW_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&chevron_icon), 0, 0, 0, 4);
    egui_view_animated_icon_override_static_preview_api(EGUI_VIEW_OF(&chevron_icon), &chevron_icon_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&chevron_panel), EGUI_VIEW_OF(&chevron_icon));

    init_text_label(&chevron_body_label, 84, 12, "Pressed", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&chevron_panel), EGUI_VIEW_OF(&chevron_body_label));

    init_panel(&fallback_panel, ANIMATED_ICON_PREVIEW_WIDTH, ANIMATED_ICON_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&fallback_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&fallback_panel));

    init_text_label(&fallback_heading_label, 84, 12, "Fallback", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&fallback_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&fallback_panel), EGUI_VIEW_OF(&fallback_heading_label));

    egui_view_animated_icon_init(EGUI_VIEW_OF(&fallback_icon));
    egui_view_set_size(EGUI_VIEW_OF(&fallback_icon), ANIMATED_ICON_PREVIEW_SIZE, ANIMATED_ICON_PREVIEW_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&fallback_icon), 0, 0, 0, 4);
    egui_view_animated_icon_override_static_preview_api(EGUI_VIEW_OF(&fallback_icon), &fallback_icon_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&fallback_panel), EGUI_VIEW_OF(&fallback_icon));

    init_text_label(&fallback_body_label, 84, 12, "Settings", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&fallback_panel), EGUI_VIEW_OF(&fallback_body_label));

    apply_primary_snapshot(0);
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&chevron_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&fallback_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
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
        }
        EGUI_SIM_SET_WAIT(p_action, ANIMATED_ICON_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ANIMATED_ICON_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, ANIMATED_ICON_RECORD_ANIM_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ANIMATED_ICON_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, ANIMATED_ICON_RECORD_ANIM_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ANIMATED_ICON_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(0);
        }
        EGUI_SIM_SET_WAIT(p_action, ANIMATED_ICON_RECORD_ANIM_WAIT);
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
