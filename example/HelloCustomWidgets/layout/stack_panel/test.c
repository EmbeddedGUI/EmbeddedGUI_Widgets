#include "egui.h"
#include "egui_view_stack_panel.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define STACK_PANEL_ROOT_WIDTH            224
#define STACK_PANEL_ROOT_HEIGHT           226
#define STACK_PANEL_PRIMARY_PANEL_WIDTH   196
#define STACK_PANEL_PRIMARY_PANEL_HEIGHT  116
#define STACK_PANEL_PRIMARY_STACK_WIDTH   176
#define STACK_PANEL_PRIMARY_STACK_HEIGHT  64
#define STACK_PANEL_PREVIEW_PANEL_WIDTH   104
#define STACK_PANEL_PREVIEW_PANEL_HEIGHT  74
#define STACK_PANEL_PREVIEW_STACK_WIDTH   84
#define STACK_PANEL_HORIZONTAL_PREVIEW_H  34
#define STACK_PANEL_COMPACT_PREVIEW_H     32
#define STACK_PANEL_BOTTOM_ROW_WIDTH      216
#define STACK_PANEL_BOTTOM_ROW_HEIGHT     74
#define STACK_PANEL_ITEM_CAPACITY         4
#define STACK_PANEL_HORIZONTAL_ITEMS      2
#define STACK_PANEL_COMPACT_ITEMS         2
#define STACK_PANEL_RECORD_WAIT           90
#define STACK_PANEL_RECORD_FRAME_WAIT     170
#define STACK_PANEL_RECORD_FINAL_WAIT     520

typedef enum
{
    STACK_ITEM_TONE_NEUTRAL = 0,
    STACK_ITEM_TONE_ACCENT,
    STACK_ITEM_TONE_WARM,
} stack_item_tone_t;

typedef struct
{
    const char *title;
    stack_item_tone_t tone;
    egui_dim_t width;
    egui_dim_t height;
} stack_item_t;

typedef struct
{
    const char *heading;
    const char *note;
    uint8_t is_horizontal;
    uint8_t use_compact_style;
    uint8_t item_count;
    stack_item_t items[STACK_PANEL_ITEM_CAPACITY];
} stack_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_linearlayout_t primary_stack_panel;
static egui_view_linearlayout_t primary_cards[STACK_PANEL_ITEM_CAPACITY];
static egui_view_label_t primary_card_titles[STACK_PANEL_ITEM_CAPACITY];
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t horizontal_panel;
static egui_view_label_t horizontal_heading_label;
static egui_view_linearlayout_t horizontal_preview_stack;
static egui_view_linearlayout_t horizontal_cards[STACK_PANEL_HORIZONTAL_ITEMS];
static egui_view_label_t horizontal_card_titles[STACK_PANEL_HORIZONTAL_ITEMS];
static egui_view_label_t horizontal_note_label;
static egui_view_linearlayout_t compact_panel;
static egui_view_label_t compact_heading_label;
static egui_view_linearlayout_t compact_preview_stack;
static egui_view_linearlayout_t compact_cards[STACK_PANEL_COMPACT_ITEMS];
static egui_view_label_t compact_card_titles[STACK_PANEL_COMPACT_ITEMS];
static egui_view_label_t compact_note_label;
static egui_view_api_t horizontal_preview_api;
static egui_view_api_t compact_preview_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_neutral_param, EGUI_COLOR_HEX(0xF4F7FA), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_neutral_params, &bg_card_neutral_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_neutral, &bg_card_neutral_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_accent_param, EGUI_COLOR_HEX(0xE8F1FB), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_accent_params, &bg_card_accent_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_accent, &bg_card_accent_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_warm_param, EGUI_COLOR_HEX(0xF9EEE2), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_warm_params, &bg_card_warm_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_warm, &bg_card_warm_params);

static const char *title_text = "StackPanel";

static const stack_snapshot_t primary_snapshots[] = {
        {
                "Review flow",
                "Related notes keep one calm vertical reading order.",
                0,
                0,
                3,
                {
                        {"Overview", STACK_ITEM_TONE_ACCENT, 156, 18},
                        {"Queue", STACK_ITEM_TONE_NEUTRAL, 156, 18},
                        {"Publish", STACK_ITEM_TONE_WARM, 156, 18},
                },
        },
        {
                "Inline tools",
                "Horizontal stacking keeps adjacent actions on one rail.",
                1,
                0,
                3,
                {
                        {"Filter", STACK_ITEM_TONE_NEUTRAL, 52, 24},
                        {"Review", STACK_ITEM_TONE_ACCENT, 56, 24},
                        {"Export", STACK_ITEM_TONE_WARM, 52, 24},
                },
        },
        {
                "Compact notes",
                "A denser stack shortens the lane without changing order.",
                0,
                1,
                4,
                {
                        {"Monday notes", STACK_ITEM_TONE_NEUTRAL, 148, 14},
                        {"Build watch", STACK_ITEM_TONE_ACCENT, 148, 14},
                        {"Risk check", STACK_ITEM_TONE_WARM, 148, 14},
                        {"Docs pass", STACK_ITEM_TONE_NEUTRAL, 148, 14},
                },
        },
};

static const stack_snapshot_t horizontal_preview_snapshot = {
        "Horizontal",
        "Static preview.",
        1,
        0,
        2,
        {
                {"A", STACK_ITEM_TONE_NEUTRAL, 34, 18},
                {"B", STACK_ITEM_TONE_ACCENT, 34, 18},
        },
};

static const stack_snapshot_t compact_preview_snapshot = {
        "Compact",
        "Quiet stack.",
        0,
        1,
        2,
        {
                {"Brief", STACK_ITEM_TONE_NEUTRAL, 70, 12},
                {"Queue", STACK_ITEM_TONE_WARM, 70, 12},
        },
};

static egui_background_t *stack_card_get_background(stack_item_tone_t tone)
{
    switch (tone)
    {
    case STACK_ITEM_TONE_ACCENT:
        return EGUI_BG_OF(&bg_card_accent);
    case STACK_ITEM_TONE_WARM:
        return EGUI_BG_OF(&bg_card_warm);
    default:
        return EGUI_BG_OF(&bg_card_neutral);
    }
}

static egui_color_t stack_card_get_title_color(stack_item_tone_t tone)
{
    switch (tone)
    {
    case STACK_ITEM_TONE_ACCENT:
        return EGUI_COLOR_HEX(0x0F5EA6);
    case STACK_ITEM_TONE_WARM:
        return EGUI_COLOR_HEX(0x8C4A00);
    default:
        return EGUI_COLOR_HEX(0x21303F);
    }
}

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color,
                            uint8_t align_type)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align_type);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void init_panel(egui_view_linearlayout_t *panel, egui_dim_t width, egui_dim_t height, egui_background_t *background, uint8_t align_type)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(panel));
    egui_view_set_size(EGUI_VIEW_OF(panel), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(panel), align_type);
    egui_view_set_background(EGUI_VIEW_OF(panel), background);
    egui_view_set_padding(EGUI_VIEW_OF(panel), 10, 10, 10, 10);
}

static void init_card(egui_view_linearlayout_t *card, egui_view_label_t *title_value, egui_dim_t width, egui_dim_t height)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(card));
    egui_view_set_size(EGUI_VIEW_OF(card), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(card), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(card), EGUI_ALIGN_CENTER);
    egui_view_set_padding(EGUI_VIEW_OF(card), 4, 2, 4, 2);
    egui_view_set_background(EGUI_VIEW_OF(card), EGUI_BG_OF(&bg_card_neutral));

    init_text_label(title_value, width - 8, height - 4, "", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x21303F), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(card), EGUI_VIEW_OF(title_value));
}

static void set_card_state(egui_view_linearlayout_t *card, egui_view_label_t *title_value, const stack_item_t *item, uint8_t visible)
{
    if (!visible)
    {
        egui_view_set_gone(EGUI_VIEW_OF(card), 1);
        return;
    }

    egui_view_set_gone(EGUI_VIEW_OF(card), 0);
    egui_view_set_size(EGUI_VIEW_OF(card), item->width, item->height);
    egui_view_set_background(EGUI_VIEW_OF(card), stack_card_get_background(item->tone));
    egui_view_set_size(EGUI_VIEW_OF(title_value), item->width - 8, item->height - 4);
    egui_view_label_set_text(EGUI_VIEW_OF(title_value), item->title);
    egui_view_label_set_font_color(EGUI_VIEW_OF(title_value), stack_card_get_title_color(item->tone), EGUI_ALPHA_100);
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(card));
}

static void apply_stack_style_for_snapshot(egui_view_t *stack_panel, const stack_snapshot_t *snapshot)
{
    if (snapshot->is_horizontal)
    {
        hcw_stack_panel_apply_horizontal_style(stack_panel);
    }
    else if (snapshot->use_compact_style)
    {
        hcw_stack_panel_apply_compact_style(stack_panel);
    }
    else
    {
        hcw_stack_panel_apply_standard_style(stack_panel);
    }
}

static void apply_snapshot_to_stack_panel(egui_view_t *stack_panel, egui_view_linearlayout_t *cards, egui_view_label_t *titles, uint8_t capacity,
                                          const stack_snapshot_t *snapshot)
{
    uint8_t index;

    apply_stack_style_for_snapshot(stack_panel, snapshot);
    for (index = 0; index < capacity; index++)
    {
        uint8_t visible = index < snapshot->item_count;
        set_card_state(&cards[index], &titles[index], &snapshot->items[index], visible);
    }
    hcw_stack_panel_layout_childs(stack_panel);
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&horizontal_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
}

#if EGUI_CONFIG_RECORDING_TEST
static void refresh_root_layout(void)
{
    layout_local_views();
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}
#endif

static void apply_primary_snapshot(uint8_t index)
{
    const stack_snapshot_t *snapshot = &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)];

    egui_view_label_set_text(EGUI_VIEW_OF(&primary_heading_label), snapshot->heading);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_note_label), snapshot->note);
    apply_snapshot_to_stack_panel(EGUI_VIEW_OF(&primary_stack_panel), primary_cards, primary_card_titles, STACK_PANEL_ITEM_CAPACITY, snapshot);
}

static void apply_preview_states(void)
{
    apply_snapshot_to_stack_panel(EGUI_VIEW_OF(&horizontal_preview_stack), horizontal_cards, horizontal_card_titles, STACK_PANEL_HORIZONTAL_ITEMS,
                                  &horizontal_preview_snapshot);
    apply_snapshot_to_stack_panel(EGUI_VIEW_OF(&compact_preview_stack), compact_cards, compact_card_titles, STACK_PANEL_COMPACT_ITEMS,
                                  &compact_preview_snapshot);
}

void test_init_ui(void)
{
    uint8_t index;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), STACK_PANEL_ROOT_WIDTH, STACK_PANEL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, STACK_PANEL_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, STACK_PANEL_PRIMARY_PANEL_WIDTH, STACK_PANEL_PRIMARY_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, primary_snapshots[0].heading, (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x5E6D7C), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&primary_stack_panel));
    egui_view_set_size(EGUI_VIEW_OF(&primary_stack_panel), STACK_PANEL_PRIMARY_STACK_WIDTH, STACK_PANEL_PRIMARY_STACK_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_stack_panel), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_stack_panel));

    for (index = 0; index < STACK_PANEL_ITEM_CAPACITY; index++)
    {
        init_card(&primary_cards[index], &primary_card_titles[index], 156, 18);
        egui_view_group_add_child(EGUI_VIEW_OF(&primary_stack_panel), EGUI_VIEW_OF(&primary_cards[index]));
    }

    init_text_label(&primary_note_label, 176, 10, primary_snapshots[0].note, (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), STACK_PANEL_BOTTOM_ROW_WIDTH, STACK_PANEL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&horizontal_panel, STACK_PANEL_PREVIEW_PANEL_WIDTH, STACK_PANEL_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&horizontal_panel));

    init_text_label(&horizontal_heading_label, 84, 12, "Horizontal", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&horizontal_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&horizontal_panel), EGUI_VIEW_OF(&horizontal_heading_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&horizontal_preview_stack));
    egui_view_set_size(EGUI_VIEW_OF(&horizontal_preview_stack), STACK_PANEL_PREVIEW_STACK_WIDTH, STACK_PANEL_HORIZONTAL_PREVIEW_H);
    egui_view_set_margin(EGUI_VIEW_OF(&horizontal_preview_stack), 0, 0, 0, 6);
    hcw_stack_panel_override_static_preview_api(EGUI_VIEW_OF(&horizontal_preview_stack), &horizontal_preview_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&horizontal_panel), EGUI_VIEW_OF(&horizontal_preview_stack));

    for (index = 0; index < STACK_PANEL_HORIZONTAL_ITEMS; index++)
    {
        init_card(&horizontal_cards[index], &horizontal_card_titles[index], 34, 18);
        egui_view_group_add_child(EGUI_VIEW_OF(&horizontal_preview_stack), EGUI_VIEW_OF(&horizontal_cards[index]));
    }

    init_text_label(&horizontal_note_label, 84, 10, "Static preview.", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&horizontal_panel), EGUI_VIEW_OF(&horizontal_note_label));

    init_panel(&compact_panel, STACK_PANEL_PREVIEW_PANEL_WIDTH, STACK_PANEL_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_panel));

    init_text_label(&compact_heading_label, 84, 12, "Compact", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_heading_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_preview_stack));
    egui_view_set_size(EGUI_VIEW_OF(&compact_preview_stack), STACK_PANEL_PREVIEW_STACK_WIDTH, STACK_PANEL_COMPACT_PREVIEW_H);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_preview_stack), 0, 0, 0, 6);
    hcw_stack_panel_override_static_preview_api(EGUI_VIEW_OF(&compact_preview_stack), &compact_preview_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_preview_stack));

    for (index = 0; index < STACK_PANEL_COMPACT_ITEMS; index++)
    {
        init_card(&compact_cards[index], &compact_card_titles[index], 70, 12);
        egui_view_group_add_child(EGUI_VIEW_OF(&compact_preview_stack), EGUI_VIEW_OF(&compact_cards[index]));
    }

    init_text_label(&compact_note_label, 84, 10, "Quiet stack.", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_note_label));

    apply_preview_states();
    apply_primary_snapshot(0);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
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
            apply_preview_states();
            apply_primary_snapshot(0);
            refresh_root_layout();
        }
        EGUI_SIM_SET_WAIT(p_action, STACK_PANEL_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, STACK_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
            refresh_root_layout();
        }
        EGUI_SIM_SET_WAIT(p_action, STACK_PANEL_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, STACK_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
            refresh_root_layout();
        }
        EGUI_SIM_SET_WAIT(p_action, STACK_PANEL_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, STACK_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(0);
            refresh_root_layout();
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, STACK_PANEL_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
