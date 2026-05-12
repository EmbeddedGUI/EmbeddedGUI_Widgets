#include "egui.h"
#include "egui_view_canvas.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define CANVAS_ROOT_WIDTH             224
#define CANVAS_ROOT_HEIGHT            240
#define CANVAS_PRIMARY_PANEL_WIDTH    196
#define CANVAS_PRIMARY_PANEL_HEIGHT   120
#define CANVAS_PRIMARY_CANVAS_WIDTH   176
#define CANVAS_PRIMARY_CANVAS_HEIGHT  64
#define CANVAS_PREVIEW_PANEL_WIDTH    104
#define CANVAS_PREVIEW_PANEL_HEIGHT   76
#define CANVAS_PREVIEW_CANVAS_WIDTH   84
#define CANVAS_PREVIEW_CANVAS_HEIGHT  30
#define CANVAS_BOTTOM_ROW_WIDTH       216
#define CANVAS_BOTTOM_ROW_HEIGHT      76
#define CANVAS_ITEM_CAPACITY          4
#define CANVAS_PINNED_ITEM_COUNT      2
#define CANVAS_COMPACT_ITEM_COUNT     3
#define CANVAS_RECORD_WAIT            90
#define CANVAS_RECORD_FRAME_WAIT      170
#define CANVAS_RECORD_FINAL_WAIT      280
#define CANVAS_DEFAULT_SNAPSHOT       0

#define PRIMARY_SNAPSHOT_COUNT        ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef enum
{
    CANVAS_ITEM_TONE_NEUTRAL = 0,
    CANVAS_ITEM_TONE_ACCENT,
    CANVAS_ITEM_TONE_WARM,
} canvas_item_tone_t;

typedef struct
{
    const char *title;
    canvas_item_tone_t tone;
    egui_dim_t x;
    egui_dim_t y;
    egui_dim_t width;
    egui_dim_t height;
} canvas_item_t;

typedef struct
{
    const char *heading;
    const char *note;
    uint8_t use_compact_style;
    uint8_t item_count;
    canvas_item_t items[CANVAS_ITEM_CAPACITY];
} canvas_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_group_t primary_canvas;
static egui_view_linearlayout_t primary_cards[CANVAS_ITEM_CAPACITY];
static egui_view_label_t primary_card_titles[CANVAS_ITEM_CAPACITY];
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t pinned_panel;
static egui_view_label_t pinned_heading_label;
static egui_view_group_t pinned_preview_canvas;
static egui_view_linearlayout_t pinned_cards[CANVAS_PINNED_ITEM_COUNT];
static egui_view_label_t pinned_card_titles[CANVAS_PINNED_ITEM_COUNT];
static egui_view_label_t pinned_note_label;
static egui_view_linearlayout_t compact_panel;
static egui_view_label_t compact_heading_label;
static egui_view_group_t compact_preview_canvas;
static egui_view_linearlayout_t compact_cards[CANVAS_COMPACT_ITEM_COUNT];
static egui_view_label_t compact_card_titles[CANVAS_COMPACT_ITEM_COUNT];
static egui_view_label_t compact_note_label;
static egui_view_api_t pinned_preview_api;
static egui_view_api_t compact_preview_api;
static uint8_t ui_ready;
static char primary_heading_text[24];
static char primary_note_text[72];
static char primary_card_title_text[CANVAS_ITEM_CAPACITY][16];
static char pinned_heading_text[16];
static char pinned_note_text[24];
static char pinned_card_title_text[CANVAS_PINNED_ITEM_COUNT][8];
static char compact_heading_text[16];
static char compact_note_text[24];
static char compact_card_title_text[CANVAS_COMPACT_ITEM_COUNT][8];

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, HCW_COLOR_PAGE_BG, EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, HCW_COLOR_SURFACE, EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_canvas_standard_param, HCW_COLOR_PANEL, EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_canvas_standard_params, &bg_canvas_standard_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_canvas_standard, &bg_canvas_standard_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_canvas_compact_param, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_canvas_compact_params, &bg_canvas_compact_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_canvas_compact, &bg_canvas_compact_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_neutral_param, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_neutral_params, &bg_card_neutral_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_neutral, &bg_card_neutral_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_accent_param, HCW_COLOR_PRIMARY_TINT, EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_accent_params, &bg_card_accent_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_accent, &bg_card_accent_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_warm_param, HCW_COLOR_WARNING_SOFT, EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_warm_params, &bg_card_warm_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_warm, &bg_card_warm_params);

static const char *title_text = "Canvas";

static uint8_t canvas_has_text(const char *text)
{
    return (text != NULL && text[0] != '\0') ? 1 : 0;
}

static uint8_t canvas_text_len(const char *text)
{
    uint8_t len = 0;

    if (text == NULL)
    {
        return 0;
    }

    while (text[len] != '\0')
    {
        len++;
    }
    return len;
}

static egui_dim_t canvas_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (!canvas_has_text(text) || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, text, 0, 0, &width, &height);
    return width;
}

static void canvas_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
{
    uint8_t copy_length;
    uint8_t index;
    uint8_t length;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (text == NULL || max_chars == 0)
    {
        return;
    }

    length = canvas_text_len(text);
    if (length <= max_chars)
    {
        copy_length = length;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; ++index)
        {
            buffer[index] = text[index];
        }
        buffer[copy_length] = '\0';
        return;
    }

    if (max_chars <= 3)
    {
        copy_length = max_chars;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; ++index)
        {
            buffer[index] = '.';
        }
        buffer[copy_length] = '\0';
        return;
    }

    copy_length = max_chars - 3;
    if (copy_length > buffer_size - 4)
    {
        copy_length = buffer_size - 4;
    }
    for (index = 0; index < copy_length; ++index)
    {
        buffer[index] = text[index];
    }
    buffer[copy_length] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static void canvas_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
                                     egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (!canvas_has_text(text) || max_width <= 0)
    {
        return;
    }

    max_chars = canvas_text_len(text);
    canvas_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = canvas_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)canvas_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }

        max_chars--;
        canvas_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static void canvas_set_fitted_label_text(egui_view_label_t *label, char *buffer, uint8_t buffer_size, const char *text, egui_dim_t fallback_char_width)
{
    egui_dim_t max_width;

    if (label == NULL || buffer == NULL || buffer_size == 0)
    {
        return;
    }

    max_width = EGUI_VIEW_OF(label)->region.size.width;
    canvas_fit_text_to_width(label->font, text, buffer, buffer_size, max_width, fallback_char_width);
    egui_view_label_set_text(EGUI_VIEW_OF(label), buffer);
}

static const canvas_snapshot_t primary_snapshots[] = {
        {
                "Pinned notes",
                "Canvas keeps small overlays anchored to exact positions.",
                0,
                3,
                {
                        {"Scope", CANVAS_ITEM_TONE_ACCENT, 0, 0, 68, 18},
                        {"Review", CANVAS_ITEM_TONE_NEUTRAL, 92, 6, 62, 18},
                        {"Publish", CANVAS_ITEM_TONE_WARM, 42, 36, 78, 18},
                },
        },
        {
                "Status overlay",
                "Absolute offsets let helper cards stay near the target surface.",
                0,
                3,
                {
                        {"Alerts", CANVAS_ITEM_TONE_WARM, 6, 0, 62, 18},
                        {"Ready", CANVAS_ITEM_TONE_ACCENT, 98, 16, 54, 18},
                        {"Notes", CANVAS_ITEM_TONE_NEUTRAL, 24, 34, 66, 18},
                },
        },
        {
                "Compact board",
                "Compact spacing keeps a dense annotation board readable.",
                1,
                4,
                {
                        {"Pin", CANVAS_ITEM_TONE_ACCENT, 0, 0, 50, 14},
                        {"Cue", CANVAS_ITEM_TONE_NEUTRAL, 94, 4, 48, 14},
                        {"Dock", CANVAS_ITEM_TONE_WARM, 20, 24, 56, 14},
                        {"Map", CANVAS_ITEM_TONE_NEUTRAL, 102, 40, 44, 14},
                },
        },
};

static const canvas_snapshot_t pinned_preview_snapshot = {
        "Pinned",
        "Static preview.",
        0,
        2,
        {
                {"A", CANVAS_ITEM_TONE_NEUTRAL, 0, 2, 28, 12},
                {"B", CANVAS_ITEM_TONE_ACCENT, 42, 8, 28, 12},
        },
};

static const canvas_snapshot_t compact_preview_snapshot = {
        "Compact",
        "Quiet board.",
        1,
        3,
        {
                {"Pin", CANVAS_ITEM_TONE_NEUTRAL, 0, 0, 26, 10},
                {"Cue", CANVAS_ITEM_TONE_WARM, 32, 8, 26, 10},
                {"Map", CANVAS_ITEM_TONE_ACCENT, 12, 16, 30, 10},
        },
};

static egui_background_t *canvas_card_get_background(canvas_item_tone_t tone)
{
    switch (tone)
    {
    case CANVAS_ITEM_TONE_ACCENT:
        return EGUI_BG_OF(&bg_card_accent);
    case CANVAS_ITEM_TONE_WARM:
        return EGUI_BG_OF(&bg_card_warm);
    default:
        return EGUI_BG_OF(&bg_card_neutral);
    }
}

static egui_color_t canvas_card_get_title_color(canvas_item_tone_t tone)
{
    switch (tone)
    {
    case CANVAS_ITEM_TONE_ACCENT:
        return HCW_COLOR_PRIMARY;
    case CANVAS_ITEM_TONE_WARM:
        return HCW_COLOR_WARNING_DARK;
    default:
        return HCW_COLOR_TEXT;
    }
}

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color,
                            uint8_t align_type)
{
    egui_view_label_init(EGUI_VIEW_OF(label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align_type);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void init_panel(egui_view_linearlayout_t *panel, egui_dim_t width, egui_dim_t height, egui_background_t *background, uint8_t align_type)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(panel), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(panel), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(panel), align_type);
    egui_view_set_background(EGUI_VIEW_OF(panel), background);
    egui_view_set_padding(EGUI_VIEW_OF(panel), 10, 10, 10, 10);
}

static void init_card(egui_view_linearlayout_t *card, egui_view_label_t *title_value, egui_dim_t width, egui_dim_t height)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(card), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(card), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(card), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(card), EGUI_ALIGN_CENTER);
    egui_view_set_padding(EGUI_VIEW_OF(card), 4, 4, 2, 2);
    egui_view_set_background(EGUI_VIEW_OF(card), EGUI_BG_OF(&bg_card_neutral));

    init_text_label(title_value, width - 8, height - 4, "", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_TEXT, EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(card), EGUI_VIEW_OF(title_value));
}

static void set_card_state(egui_view_linearlayout_t *card, egui_view_label_t *title_value, char *title_buffer, uint8_t title_buffer_size,
                           const canvas_item_t *item, uint8_t visible)
{
    uint8_t thin_card;

    if (!visible)
    {
        egui_view_set_gone(EGUI_VIEW_OF(card), 1);
        return;
    }

    thin_card = item->height <= 10 ? 1 : 0;
    egui_view_set_gone(EGUI_VIEW_OF(card), 0);
    egui_view_set_size(EGUI_VIEW_OF(card), item->width, item->height);
    egui_view_set_background(EGUI_VIEW_OF(card), canvas_card_get_background(item->tone));
    egui_view_set_padding(EGUI_VIEW_OF(card), thin_card ? 3 : 4, thin_card ? 3 : 4, thin_card ? 1 : 2, thin_card ? 1 : 2);
    egui_view_set_size(EGUI_VIEW_OF(title_value), item->width - (thin_card ? 6 : 8), item->height - (thin_card ? 2 : 4));
    canvas_set_fitted_label_text(title_value, title_buffer, title_buffer_size, item->title, 4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(title_value), canvas_card_get_title_color(item->tone), EGUI_ALPHA_100);
    hcw_canvas_set_child_origin(EGUI_VIEW_OF(card), item->x, item->y);
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(card));
}

static void apply_canvas_style_for_snapshot(egui_view_t *canvas, const canvas_snapshot_t *snapshot)
{
    if (snapshot->use_compact_style)
    {
        hcw_canvas_apply_compact_style(canvas);
        egui_view_set_background(canvas, EGUI_BG_OF(&bg_canvas_compact));
    }
    else
    {
        hcw_canvas_apply_standard_style(canvas);
        egui_view_set_background(canvas, EGUI_BG_OF(&bg_canvas_standard));
    }
}

static void apply_snapshot_to_canvas(egui_view_t *canvas, egui_view_linearlayout_t *cards, egui_view_label_t *titles, char *card_title_text,
                                     uint8_t title_text_size, uint8_t capacity, const canvas_snapshot_t *snapshot)
{
    uint8_t index;

    apply_canvas_style_for_snapshot(canvas, snapshot);
    for (index = 0; index < capacity; index++)
    {
        uint8_t visible = index < snapshot->item_count;
        set_card_state(&cards[index], &titles[index], &card_title_text[index * title_text_size], title_text_size, &snapshot->items[index], visible);
    }
    hcw_canvas_layout_childs(canvas);
}

static void layout_local_views(void)
{
    hcw_canvas_layout_childs(EGUI_VIEW_OF(&primary_canvas));
    hcw_canvas_layout_childs(EGUI_VIEW_OF(&pinned_preview_canvas));
    hcw_canvas_layout_childs(EGUI_VIEW_OF(&compact_preview_canvas));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&pinned_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
}

static void layout_page(void)
{
    layout_local_views();
    egui_core_layout_childs_user_root_view(uicode_get_core(), EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

static void apply_primary_state(uint8_t index)
{
    const canvas_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    canvas_set_fitted_label_text(&primary_heading_label, primary_heading_text, sizeof(primary_heading_text), snapshot->heading, 4);
    canvas_set_fitted_label_text(&primary_note_label, primary_note_text, sizeof(primary_note_text), snapshot->note, 4);
    apply_snapshot_to_canvas(EGUI_VIEW_OF(&primary_canvas), primary_cards, primary_card_titles, &primary_card_title_text[0][0],
                             sizeof(primary_card_title_text[0]), CANVAS_ITEM_CAPACITY, snapshot);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_state(CANVAS_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    canvas_set_fitted_label_text(&pinned_heading_label, pinned_heading_text, sizeof(pinned_heading_text), pinned_preview_snapshot.heading, 4);
    canvas_set_fitted_label_text(&pinned_note_label, pinned_note_text, sizeof(pinned_note_text), pinned_preview_snapshot.note, 4);
    canvas_set_fitted_label_text(&compact_heading_label, compact_heading_text, sizeof(compact_heading_text), compact_preview_snapshot.heading, 4);
    canvas_set_fitted_label_text(&compact_note_label, compact_note_text, sizeof(compact_note_text), compact_preview_snapshot.note, 4);

    apply_snapshot_to_canvas(EGUI_VIEW_OF(&pinned_preview_canvas), pinned_cards, pinned_card_titles, &pinned_card_title_text[0][0],
                             sizeof(pinned_card_title_text[0]), CANVAS_PINNED_ITEM_COUNT, &pinned_preview_snapshot);
    apply_snapshot_to_canvas(EGUI_VIEW_OF(&compact_preview_canvas), compact_cards, compact_card_titles, &compact_card_title_text[0][0],
                             sizeof(compact_card_title_text[0]), CANVAS_COMPACT_ITEM_COUNT, &compact_preview_snapshot);
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
    uint8_t index;

    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), CANVAS_ROOT_WIDTH, CANVAS_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, CANVAS_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, HCW_COLOR_TEXT,
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 0, 8, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, CANVAS_PRIMARY_PANEL_WIDTH, CANVAS_PRIMARY_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, primary_snapshots[CANVAS_DEFAULT_SNAPSHOT].heading,
                    (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    HCW_COLOR_TEXT_MUTED, EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    egui_view_group_init(EGUI_VIEW_OF(&primary_canvas), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&primary_canvas), CANVAS_PRIMARY_CANVAS_WIDTH, CANVAS_PRIMARY_CANVAS_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_canvas), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_canvas));

    for (index = 0; index < CANVAS_ITEM_CAPACITY; index++)
    {
        init_card(&primary_cards[index], &primary_card_titles[index], 68, 18);
        egui_view_group_add_child(EGUI_VIEW_OF(&primary_canvas), EGUI_VIEW_OF(&primary_cards[index]));
    }

    init_text_label(&primary_note_label, 176, 10, primary_snapshots[CANVAS_DEFAULT_SNAPSHOT].note,
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_TEXT_MUTED, EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), CANVAS_BOTTOM_ROW_WIDTH, CANVAS_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&pinned_panel, CANVAS_PREVIEW_PANEL_WIDTH, CANVAS_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&pinned_panel));

    init_text_label(&pinned_heading_label, 84, 12, "Pinned", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_TEXT, EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&pinned_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&pinned_panel), EGUI_VIEW_OF(&pinned_heading_label));

    egui_view_group_init(EGUI_VIEW_OF(&pinned_preview_canvas), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&pinned_preview_canvas), CANVAS_PREVIEW_CANVAS_WIDTH, CANVAS_PREVIEW_CANVAS_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&pinned_preview_canvas), 0, 0, 0, 4);
    hcw_canvas_override_static_preview_api(EGUI_VIEW_OF(&pinned_preview_canvas), &pinned_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&pinned_preview_canvas), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&pinned_panel), EGUI_VIEW_OF(&pinned_preview_canvas));

    for (index = 0; index < CANVAS_PINNED_ITEM_COUNT; index++)
    {
        init_card(&pinned_cards[index], &pinned_card_titles[index], 28, 12);
        egui_view_group_add_child(EGUI_VIEW_OF(&pinned_preview_canvas), EGUI_VIEW_OF(&pinned_cards[index]));
    }

    init_text_label(&pinned_note_label, 84, 10, "Static preview.", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_TEXT_MUTED,
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&pinned_panel), EGUI_VIEW_OF(&pinned_note_label));

    init_panel(&compact_panel, CANVAS_PREVIEW_PANEL_WIDTH, CANVAS_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_panel));

    init_text_label(&compact_heading_label, 84, 12, "Compact", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_TEXT,
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_heading_label));

    egui_view_group_init(EGUI_VIEW_OF(&compact_preview_canvas), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&compact_preview_canvas), CANVAS_PREVIEW_CANVAS_WIDTH, CANVAS_PREVIEW_CANVAS_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_preview_canvas), 0, 0, 0, 4);
    hcw_canvas_override_static_preview_api(EGUI_VIEW_OF(&compact_preview_canvas), &compact_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_preview_canvas), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_preview_canvas));

    for (index = 0; index < CANVAS_COMPACT_ITEM_COUNT; index++)
    {
        init_card(&compact_cards[index], &compact_card_titles[index], 30, 10);
        egui_view_group_add_child(EGUI_VIEW_OF(&compact_preview_canvas), EGUI_VIEW_OF(&compact_cards[index]));
    }

    init_text_label(&compact_note_label, 84, 10, "Quiet board.", (const egui_font_t *)&egui_res_font_montserrat_8_4, HCW_COLOR_TEXT_MUTED,
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_note_label));

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
        EGUI_SIM_SET_WAIT(p_action, CANVAS_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_state(1);
        }
        EGUI_SIM_SET_WAIT(p_action, CANVAS_RECORD_FINAL_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CANVAS_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_state(2);
        }
        EGUI_SIM_SET_WAIT(p_action, CANVAS_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CANVAS_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, CANVAS_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CANVAS_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
