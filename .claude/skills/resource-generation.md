---
name: resource-generation
description: Use when adding images, fonts, or icons to HelloCustomWidgets demos, or when resource symbols fail to link
---

# Resource Generation Skill

EmbeddedGUI Widgets 仓库只保留 `HelloCustomWidgets` 相关资源和生成结果。资源生成工具来自 SDK 子模块：`sdk/EmbeddedGUI/scripts/tools/`。

## 当前资源布局

本仓库常见的 per-widget 资源目录：

```
example/HelloCustomWidgets/<category>/<widget>/resource/
├── app_egui_resource_generate.h        # 图片资源声明（部分 widget）
├── egui_icon_<widget>.h                # 图标码点声明（部分 widget）
├── img/
│   └── egui_res_image_*.c              # 已生成图片资源
└── font/
    ├── egui_res_font_*.c               # 已生成字体资源
    └── *_text.txt                      # 图标或字体字符集
```

`example/HelloCustomWidgets/build.mk` 已统一把当前 `APP_SUB` 的 `resource/`、`resource/img/`、`resource/font/` 加入编译和 include 路径。少数 showcase 共享资源目录也在同一个 `build.mk` 中显式列出。

## 常用命令

```bash
# 构建一个带资源的 widget demo
make all APP=HelloCustomWidgets APP_SUB=display/image_icon PORT=pc

# 强制重新生成当前 widget 的资源
make resource_refresh APP=HelloCustomWidgets APP_SUB=display/image_icon PORT=pc

# 只跑资源生成目标
make resource APP=HelloCustomWidgets APP_SUB=display/image_icon PORT=pc

# 资源生成后做一次编译确认
python scripts/code_compile_check.py --custom-widgets --category display --bits64
```

如果需要直接调用 SDK 工具，使用子模块路径：

```bash
python sdk/EmbeddedGUI/scripts/tools/app_resource_generate.py -r example/HelloCustomWidgets/display/image_icon/resource -o output
python sdk/EmbeddedGUI/scripts/tools/img2c.py -i icon.png -n image_icon -f alpha -a 4 -ext 0 -o example/HelloCustomWidgets/display/image_icon/resource/img/egui_res_image_image_icon_alpha_4.c
python sdk/EmbeddedGUI/scripts/tools/ttf2c.py -i icon_font.ttf -n rating_control_icons -t rating_control_icons_text.txt -p 20 -s 4 -o example/HelloCustomWidgets/input/rating_control/resource/font/egui_res_font_rating_control_icons_20_4.c
```

注意：根仓库没有 `scripts/tools/`。不要把 SDK 工具路径写成 `scripts/tools/...`。

## 图片资源

图片资源通常放在 widget 自己的 `resource/img/` 下，并通过 `resource/app_egui_resource_generate.h` 暴露声明。

代码引用示例：

```c
#include "resource/app_egui_resource_generate.h"

egui_view_image_set_image(view, &egui_res_image_image_icon_landscape_rgb565_8);
```

常见格式选择：

| 场景 | format | alpha | 说明 |
|------|--------|-------|------|
| 单色图标 | `alpha` | `4` | 仅存透明度，运行时设置颜色，体积较小 |
| 全彩图片 | `rgb565` | `0` | 16 位色，无透明 |
| 全彩带透明 | `rgb565` | `4` 或 `8` | 保留透明通道 |
| 高质量全彩 | `rgb32` | `8` | 体积最大 |

## 字体和图标资源

图标字体资源通常包括：

- `resource/egui_icon_<widget>.h`：图标码点宏或声明。
- `resource/font/*_text.txt`：要打包进字体的图标字符集。
- `resource/font/egui_res_font_*.c`：生成后的字体 C 文件。

代码引用示例：

```c
#include "resource/egui_icon_rating_control.h"

egui_view_label_set_font(view, &egui_res_font_rating_control_icons_20_4);
```

新增图标或文本后，确认字符已经进入对应 `*_text.txt`，再重新生成字体资源。如果某个 widget 明确维护了完整的 `resource/src/app_resource_config.json` 管线，SDK 的自动提取脚本也可以显式指定 widgets 仓库路径：

```bash
python sdk/EmbeddedGUI/scripts/tools/extract_font_text.py --app HelloCustomWidgets --src-dir example/HelloCustomWidgets/<category>/<widget> --resource-dir example/HelloCustomWidgets/<category>/<widget>/resource/src --dry-run
```

如果当前 widget 没有 `resource/src/app_resource_config.json`，先确认是否应该沿用现有手工生成资源模式；不要为了单个小改动引入新的完整资源管线，除非能同步维护配置、生成物和 build 接线。

## 添加新资源的流程

1. 确认资源是否应归属于单个 widget；默认放到 `example/HelloCustomWidgets/<category>/<widget>/resource/`。
2. 新增或更新源资源、字符集和生成配置时，优先沿用同目录已有结构。
3. 运行 `make resource_refresh APP=HelloCustomWidgets APP_SUB=<category>/<widget> PORT=pc`。
4. 检查生成的 `.c`、`.h` 和 `*_text.txt` 是否符合命名约定。
5. 编译单个 demo，并按风险运行对应分类检查。
6. 如该 demo 已发布到 web，重建或刷新 WASM demo 后运行 web artifact 检查。

## 常见问题排查

| 问题 | 原因 | 修复 |
|------|------|------|
| `undefined reference to 'egui_res_image_xxx'` | 资源 C 文件未生成、未纳入编译或符号名不一致 | 检查 `resource/img/`、头文件声明和 `build.mk` include/src 规则 |
| `undefined reference to 'egui_res_font_xxx'` | 字体 C 文件缺失或命名不匹配 | 检查 `resource/font/` 生成物和代码引用名 |
| 图标显示为空白 | 字符未进入字体子集，或 alpha 图未设置颜色 | 更新 `*_text.txt` 并重新生成；图片图标调用 `egui_view_image_set_image_color()` |
| 文本显示方框 | 字符不在字体资源内 | 补充字符集并重新生成字体 |
| WASM demo 缺资源 | resource 生成物没有被构建系统纳入，或 web 产物未刷新 | 重新 `make all ...`，再运行 `python scripts/web/wasm_build_demos.py --app-sub <category>/<widget>` |
| 生成被缓存跳过 | 旧的资源 merge/bin 或生成物仍存在 | 使用 `make resource_refresh ...` |

## 验证建议

资源变更至少运行：

```bash
make all APP=HelloCustomWidgets APP_SUB=<category>/<widget> PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub <category>/<widget> --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

如果影响 web 发布产物：

```bash
python scripts/web/wasm_build_demos.py --app-sub <category>/<widget>
python scripts/web/web_smoke_check.py --demo HelloCustomWidgets_<category>_<widget>
python scripts/checks/check_web_artifacts.py
```

## 文件参考

| 文件 | 说明 |
|------|------|
| `example/HelloCustomWidgets/build.mk` | per-widget resource 目录接入规则 |
| `sdk/EmbeddedGUI/scripts/tools/app_resource_generate.py` | SDK 资源生成主脚本 |
| `sdk/EmbeddedGUI/scripts/tools/img2c.py` | 图片转 C 资源 |
| `sdk/EmbeddedGUI/scripts/tools/ttf2c.py` | 字体转 C 资源 |
| `sdk/EmbeddedGUI/scripts/tools/extract_font_text.py` | 从 C 源码提取字体字符集 |
| `scripts/code_compile_check.py` | widgets 编译和单测检查 |
| `scripts/code_runtime_check.py` | widgets 运行截图验证 |
