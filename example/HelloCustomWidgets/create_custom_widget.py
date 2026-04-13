#!/usr/bin/env python3
"""Create a new HelloCustomWidgets scaffold aligned with the current workflow.

Usage:
    python example/HelloCustomWidgets/create_custom_widget.py --category display --name weather_icon
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from textwrap import dedent

VALID_CATEGORIES = (
    "display",
    "feedback",
    "input",
    "layout",
    "navigation",
)

NAME_PATTERN = re.compile(r"^[a-z][a-z0-9]*(?:_[a-z0-9]+)*$")


def to_display_title(name: str) -> str:
    return " ".join(part.capitalize() for part in name.split("_"))


def ensure_snake_case(name: str) -> None:
    if not NAME_PATTERN.fullmatch(name):
        raise ValueError("widget name must be snake_case and start with a lowercase letter")


def normalize_template(text: str) -> str:
    return dedent(text).strip() + "\n"


def header_template(name: str, name_upper: str) -> str:
    return normalize_template(
        f"""
        #ifndef _EGUI_VIEW_{name_upper}_H_
        #define _EGUI_VIEW_{name_upper}_H_

        #include "egui.h"

        #ifdef __cplusplus
        extern "C" {{
        #endif

        typedef struct egui_view_{name} egui_view_{name}_t;
        struct egui_view_{name}
        {{
            egui_view_linearlayout_t base;
            egui_view_label_t badge_label;
            egui_view_label_t title_label;
            egui_view_label_t summary_label;
            uint8_t compact_mode;
            uint8_t read_only_mode;
        }};

        void egui_view_{name}_init(egui_view_t *self);
        void egui_view_{name}_set_badge(egui_view_t *self, const char *text);
        void egui_view_{name}_set_title(egui_view_t *self, const char *text);
        void egui_view_{name}_set_summary(egui_view_t *self, const char *text);
        void egui_view_{name}_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
        uint8_t egui_view_{name}_get_compact_mode(egui_view_t *self);
        void egui_view_{name}_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
        uint8_t egui_view_{name}_get_read_only_mode(egui_view_t *self);

        #ifdef __cplusplus
        }}
        #endif

        #endif /* _EGUI_VIEW_{name_upper}_H_ */
        """
    )


def source_template(name: str, display_title: str) -> str:
    return normalize_template(
        f"""
        #include "egui_view_{name}.h"

        EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE({name}_bg_standard_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
        EGUI_BACKGROUND_PARAM_INIT({name}_bg_standard_params, &{name}_bg_standard_param, NULL, NULL);
        EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT({name}_bg_standard, &{name}_bg_standard_params);

        EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE({name}_bg_compact_param, EGUI_COLOR_HEX(0xF3FBF8), EGUI_ALPHA_100, 10);
        EGUI_BACKGROUND_PARAM_INIT({name}_bg_compact_params, &{name}_bg_compact_param, NULL, NULL);
        EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT({name}_bg_compact, &{name}_bg_compact_params);

        EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE({name}_bg_read_only_param, EGUI_COLOR_HEX(0xF7F9FB), EGUI_ALPHA_100, 10);
        EGUI_BACKGROUND_PARAM_INIT({name}_bg_read_only_params, &{name}_bg_read_only_param, NULL, NULL);
        EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT({name}_bg_read_only, &{name}_bg_read_only_params);

        static const char *safe_text(const char *text)
        {{
            return text == NULL ? "" : text;
        }}

        static void init_internal_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height)
        {{
            egui_view_label_init(EGUI_VIEW_OF(label));
            egui_view_set_size(EGUI_VIEW_OF(label), width, height);
            egui_view_label_set_align_type(EGUI_VIEW_OF(label), EGUI_ALIGN_CENTER);
        }}

        static void sync_visual_state(egui_view_{name}_t *widget)
        {{
            egui_color_t badge_color = EGUI_COLOR_HEX(0x0F6CBD);
            egui_color_t title_color = EGUI_COLOR_HEX(0x22303F);
            egui_color_t summary_color = EGUI_COLOR_HEX(0x6B7A89);
            egui_background_t *background = EGUI_BG_OF(&{name}_bg_standard);
            egui_dim_t horizontal_padding = widget->compact_mode ? 8 : 10;
            egui_dim_t vertical_padding = widget->compact_mode ? 8 : 10;
            egui_dim_t badge_width = widget->compact_mode ? 108 : 132;
            egui_dim_t title_height = widget->compact_mode ? 12 : 14;
            egui_dim_t summary_height = widget->compact_mode ? 18 : 20;

            if (widget->read_only_mode)
            {{
                background = EGUI_BG_OF(&{name}_bg_read_only);
                badge_color = EGUI_COLOR_HEX(0x8A97A3);
                title_color = EGUI_COLOR_HEX(0x5C6B79);
                summary_color = EGUI_COLOR_HEX(0x8A97A3);
            }}
            else if (widget->compact_mode)
            {{
                background = EGUI_BG_OF(&{name}_bg_compact);
                badge_color = EGUI_COLOR_HEX(0x0D9488);
                title_color = EGUI_COLOR_HEX(0x18312F);
                summary_color = EGUI_COLOR_HEX(0x5B7A73);
            }}

            egui_view_set_background(EGUI_VIEW_OF(widget), background);
            egui_view_set_padding(EGUI_VIEW_OF(widget), horizontal_padding, vertical_padding, horizontal_padding, vertical_padding);

            egui_view_set_size(EGUI_VIEW_OF(&widget->badge_label), badge_width, 10);
            egui_view_set_size(EGUI_VIEW_OF(&widget->title_label), 144, title_height);
            egui_view_set_size(EGUI_VIEW_OF(&widget->summary_label), 144, summary_height);

            egui_view_label_set_font_color(EGUI_VIEW_OF(&widget->badge_label), badge_color, EGUI_ALPHA_100);
            egui_view_label_set_font_color(EGUI_VIEW_OF(&widget->title_label), title_color, EGUI_ALPHA_100);
            egui_view_label_set_font_color(EGUI_VIEW_OF(&widget->summary_label), summary_color, EGUI_ALPHA_100);

            egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(widget));
            egui_view_invalidate(EGUI_VIEW_OF(widget));
        }}

        void egui_view_{name}_set_badge(egui_view_t *self, const char *text)
        {{
            EGUI_LOCAL_INIT_VAR(egui_view_{name}_t, widget);
            egui_view_label_set_text(EGUI_VIEW_OF(&widget->badge_label), safe_text(text));
            egui_view_invalidate(self);
        }}

        void egui_view_{name}_set_title(egui_view_t *self, const char *text)
        {{
            EGUI_LOCAL_INIT_VAR(egui_view_{name}_t, widget);
            egui_view_label_set_text(EGUI_VIEW_OF(&widget->title_label), safe_text(text));
            egui_view_invalidate(self);
        }}

        void egui_view_{name}_set_summary(egui_view_t *self, const char *text)
        {{
            EGUI_LOCAL_INIT_VAR(egui_view_{name}_t, widget);
            egui_view_label_set_text(EGUI_VIEW_OF(&widget->summary_label), safe_text(text));
            egui_view_invalidate(self);
        }}

        void egui_view_{name}_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
        {{
            EGUI_LOCAL_INIT_VAR(egui_view_{name}_t, widget);

            compact_mode = compact_mode ? 1 : 0;
            if (widget->compact_mode == compact_mode)
            {{
                return;
            }}

            widget->compact_mode = compact_mode;
            sync_visual_state(widget);
        }}

        uint8_t egui_view_{name}_get_compact_mode(egui_view_t *self)
        {{
            EGUI_LOCAL_INIT_VAR(egui_view_{name}_t, widget);
            return widget->compact_mode;
        }}

        void egui_view_{name}_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
        {{
            EGUI_LOCAL_INIT_VAR(egui_view_{name}_t, widget);

            read_only_mode = read_only_mode ? 1 : 0;
            if (widget->read_only_mode == read_only_mode)
            {{
                return;
            }}

            widget->read_only_mode = read_only_mode;
            sync_visual_state(widget);
        }}

        uint8_t egui_view_{name}_get_read_only_mode(egui_view_t *self)
        {{
            EGUI_LOCAL_INIT_VAR(egui_view_{name}_t, widget);
            return widget->read_only_mode;
        }}

        void egui_view_{name}_init(egui_view_t *self)
        {{
            EGUI_LOCAL_INIT_VAR(egui_view_{name}_t, widget);

            egui_view_linearlayout_init(self);
            egui_view_linearlayout_set_orientation(self, 0);
            egui_view_linearlayout_set_align_type(self, EGUI_ALIGN_HCENTER);

            init_internal_label(&widget->badge_label, 132, 10);
            egui_view_set_margin(EGUI_VIEW_OF(&widget->badge_label), 0, 0, 0, 4);
            egui_view_group_add_child(self, EGUI_VIEW_OF(&widget->badge_label));

            init_internal_label(&widget->title_label, 144, 14);
            egui_view_set_margin(EGUI_VIEW_OF(&widget->title_label), 0, 0, 0, 4);
            egui_view_group_add_child(self, EGUI_VIEW_OF(&widget->title_label));

            init_internal_label(&widget->summary_label, 144, 20);
            egui_view_group_add_child(self, EGUI_VIEW_OF(&widget->summary_label));

            widget->compact_mode = 0;
            widget->read_only_mode = 0;

            egui_view_{name}_set_badge(self, "DRAFT");
            egui_view_{name}_set_title(self, "{display_title}");
            egui_view_{name}_set_summary(self, "Reference shell placeholder.");
            sync_visual_state(widget);
        }}
        """
    )


def test_template(name: str, name_upper: str, display_title: str) -> str:
    return normalize_template(
        f"""
        #include "egui.h"
        #include "egui_view_{name}.h"
        #include "uicode.h"
        #include "demo_scaffold.h"

        #if EGUI_CONFIG_RECORDING_TEST
        #include "core/egui_input_simulator.h"
        #endif

        #define {name_upper}_ROOT_WIDTH        224
        #define {name_upper}_ROOT_HEIGHT       180
        #define {name_upper}_PRIMARY_WIDTH     180
        #define {name_upper}_PRIMARY_HEIGHT    88
        #define {name_upper}_RECORD_FRAME_WAIT 220

        static egui_view_linearlayout_t root_layout;
        static egui_view_label_t title_label;
        static egui_view_{name}_t widget_primary;

        EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
        EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
        EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

        static const char *title_text = "{display_title}";

        void test_init_ui(void)
        {{
            egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
            egui_view_set_size(EGUI_VIEW_OF(&root_layout), {name_upper}_ROOT_WIDTH, {name_upper}_ROOT_HEIGHT);
            egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
            egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
            egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

            egui_view_label_init(EGUI_VIEW_OF(&title_label));
            egui_view_set_size(EGUI_VIEW_OF(&title_label), {name_upper}_ROOT_WIDTH, 18);
            egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
            egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
            egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
            egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
            egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
            egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

            egui_view_{name}_init(EGUI_VIEW_OF(&widget_primary));
            egui_view_set_size(EGUI_VIEW_OF(&widget_primary), {name_upper}_PRIMARY_WIDTH, {name_upper}_PRIMARY_HEIGHT);
            egui_view_{name}_set_badge(EGUI_VIEW_OF(&widget_primary), "DRAFT");
            egui_view_{name}_set_title(EGUI_VIEW_OF(&widget_primary), "Replace with the real widget");
            egui_view_{name}_set_summary(EGUI_VIEW_OF(&widget_primary), "Scaffold aligned with the reference workflow.");
            egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&widget_primary));

            hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

            egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&widget_primary));
            egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

            egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
            egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
        }}

        #if EGUI_CONFIG_RECORDING_TEST
        bool egui_port_get_recording_action(int action_index, egui_sim_action_t *p_action)
        {{
            static int last_action = -1;
            int first_call = action_index != last_action;

            last_action = action_index;

            switch (action_index)
            {{
            case 0:
                if (first_call)
                {{
                    recording_request_snapshot();
                }}
                EGUI_SIM_SET_WAIT(p_action, {name_upper}_RECORD_FRAME_WAIT);
                return true;
            default:
                return false;
            }}
        }}
        #endif
        """
    )


def readme_template(category: str, name: str, display_title: str) -> str:
    return normalize_template(
        f"""
        # {display_title}

        ## 1. 为什么需要这个控件
        - 待补充：说明 `{display_title}` 在 Fluent 2 / WPF UI 主线里的语义和保留价值。

        ## 2. 为什么现有控件不够用
        - 待补充：说明当前 `reference` 主线里哪些控件无法直接覆盖目标场景。

        ## 3. 目标场景与示例概览
        - 目标分类：`{category}`
        - 目标目录：`example/HelloCustomWidgets/{category}/{name}/`
        - 待补充：主示例、紧凑态、只读态和其它必要状态。

        ## 4. 视觉与布局规格
        - 待补充：记录尺寸、留白、层级、焦点、禁用态和选中态规则。

        ## 5. 控件清单与状态矩阵
        | 项目 | 说明 |
        | --- | --- |
        | 主控件 | 待补充 |
        | 预览态 | 待补充 |
        | 交互状态 | 待补充 |

        ## 6. 录制动作设计
        - 待补充：列出 recording test 需要覆盖的关键操作与截图节点。

        ## 7. 编译 / Runtime / 截图验收标准
        - 单控件编译：`make all APP=HelloCustomWidgets APP_SUB={category}/{name} PORT=pc`
        - 触控语义：`python scripts/checks/check_touch_release_semantics.py --scope custom --category {category}`
        - 单控件 runtime：`python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub {category}/{name} --track reference --timeout 10 --keep-screenshots`

        ## 8. 参考设计体系与开源母本
        - 待补充：记录 Fluent / WinUI / WPF 对应链接和母本依据。

        ## 9. 对应的 Fluent / WPF UI 组件名
        - 待补充：写明标准控件名。

        ## 10. 保留的核心状态与删掉的装饰效果
        - 待补充：说明必须保留的状态语义，以及主动删除的非主线装饰。

        ## 11. EGUI 适配时的简化点与限制
        - 待补充：说明像素预算、字体、输入能力、性能边界和已知限制。

        ## 本地验收资料
        - `iteration_log/` 仅用于本地验收，不纳入 git commit。
        """
    )


def iteration_log_template(display_title: str) -> str:
    return normalize_template(
        f"""
        # {display_title} Iteration Log

        > 本目录仅用于本地验收，禁止提交到 git。

        ## 目标
        - 待补充：记录本轮收口目标、参考语义和验收边界。

        ## 记录约定
        - 每一轮至少记录：目标、代码改动、编译结果、runtime 结果、截图路径和结论。
        - 从 `iter_01` 开始顺序追加，不要预填虚假结果。
        - 截图建议放到 `images/iter_xx/`。

        ## 迭代记录
        | 轮次 | 日期 | 目标 | 代码改动 | 编译 | Runtime | 截图 | 结论 |
        | --- | --- | --- | --- | --- | --- | --- | --- |
        """
    )


def write_file(path: Path, content: str) -> None:
    path.write_text(content, encoding="utf-8")


def create_widget_scaffold(category: str, name: str) -> Path:
    ensure_snake_case(name)

    repo_root = Path(__file__).resolve().parents[2]
    widget_dir = repo_root / "example" / "HelloCustomWidgets" / category / name
    iteration_dir = widget_dir / "iteration_log"
    images_dir = iteration_dir / "images"
    name_upper = name.upper()
    display_title = to_display_title(name)

    if widget_dir.exists():
        raise FileExistsError(f"{widget_dir} already exists")

    images_dir.mkdir(parents=True, exist_ok=False)

    write_file(widget_dir / f"egui_view_{name}.h", header_template(name, name_upper))
    write_file(widget_dir / f"egui_view_{name}.c", source_template(name, display_title))
    write_file(widget_dir / "test.c", test_template(name, name_upper, display_title))
    write_file(widget_dir / "readme.md", readme_template(category, name, display_title))
    write_file(iteration_dir / "iteration_log.md", iteration_log_template(display_title))

    return widget_dir


def main() -> int:
    parser = argparse.ArgumentParser(description="Create a new HelloCustomWidgets scaffold")
    parser.add_argument("--category", required=True, choices=VALID_CATEGORIES, help="Widget category")
    parser.add_argument("--name", required=True, help="Widget name in snake_case, for example weather_icon")
    args = parser.parse_args()

    try:
        widget_dir = create_widget_scaffold(args.category, args.name)
    except ValueError as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1
    except FileExistsError as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    repo_root = Path(__file__).resolve().parents[2]
    relative_widget_dir = widget_dir.relative_to(repo_root)
    name = args.name

    print(f"Created widget scaffold: {relative_widget_dir}/")
    print(f"  egui_view_{name}.h")
    print(f"  egui_view_{name}.c")
    print("  test.c")
    print("  readme.md")
    print("  iteration_log/iteration_log.md")
    print("  iteration_log/images/")
    print()
    print("Next steps:")
    print(f"  make all APP=HelloCustomWidgets APP_SUB={args.category}/{name} PORT=pc")
    print(f"  python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub {args.category}/{name} --track reference --timeout 10 --keep-screenshots")
    print("  Update readme.md and iteration_log/iteration_log.md before acceptance")
    print("  Keep iteration_log/ local-only and out of git commits")
    return 0


if __name__ == "__main__":
    sys.exit(main())
