# SymbolIcon 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件名：`SymbolIcon`
- 本次保留语义：只读符号图标显示、统一尺寸约束、颜色样式 helper、静态 preview 输入抑制
- 删减内容：动画、hover 效果、额外装饰边框、场景化 showcase 叙事
- EGUI 适配说明：仓库没有单独的 Fluent Symbols 资源，本控件复用内置 `Material Symbols` 字体子集表达 `SymbolIcon` 语义，不改 SDK

## 1. 为什么需要这个控件？
`SymbolIcon` 适合在导航、状态提示和设置项里承载低噪音图标语义。它比普通文本更紧凑，也比图片资源更轻量，适合作为 reference 主线里的基础显示控件。

## 2. 为什么现有控件不够用？
- `info_badge` 面向徽标和状态点，不是通用图标展示控件。
- `badge_group`、`persona_group` 都是组合型展示，不适合单个符号图标。
- 现有按钮类控件虽然能带 icon，但语义核心是动作触发，不是只读展示。

## 3. 目标场景与示例概览
- 主区域保留一个真实 `SymbolIcon` 参考面板，通过录制切换 `Home / Notifications / Settings` 三组符号与文案。
- 底部左侧保留一个 `subtle` 静态对照。
- 底部右侧保留一个 `accent` 静态对照。
- 页面只保留标题、一个主卡片和底部双 preview，不承担额外交互职责。

目录：
- `example/HelloCustomWidgets/display/symbol_icon/`

## 4. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 196`
- 主面板：`196 x 104`
- 主图标：`64 x 64`
- 底部容器：`216 x 64`
- 单个 preview：`104 x 64`
- 单个 preview 图标：`32 x 32`

视觉原则：
- 保持 Fluent 风格的浅灰页面与白色卡片表面。
- `SymbolIcon` 控件自身只负责居中绘制图标，不承担额外 panel chrome。
- 主状态用冷色强调，subtle 用中性灰，accent 用暖色强调，保证不同语义下的对比度稳定。

## 5. 控件清单
| 变量名 | 类型 | 尺寸 | 用途 |
| --- | --- | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 196` | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | 页面标题 |
| `primary_panel` | `egui_view_linearlayout_t` | `196 x 104` | 主展示面板 |
| `primary_heading_label` | `egui_view_label_t` | `176 x 12` | 主场景标题 |
| `primary_icon` | `egui_view_symbol_icon_t` | `64 x 64` | 主符号图标 |
| `primary_name_label` | `egui_view_label_t` | `176 x 14` | 当前符号名称 |
| `primary_note_label` | `egui_view_label_t` | `176 x 18` | 当前说明文案 |
| `subtle_icon` | `egui_view_symbol_icon_t` | `32 x 32` | 低噪音静态对照 |
| `accent_icon` | `egui_view_symbol_icon_t` | `32 x 32` | 强调态静态对照 |

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Subtle | Accent |
| --- | --- | --- | --- |
| 默认标准色图标 | 是 | 否 | 否 |
| 中性 subtle 图标 | 否 | 是 | 否 |
| 强调 accent 图标 | 录制中切换 | 否 | 是 |
| 只读展示，不提交点击 | 是 | 是 | 是 |
| 静态 preview 吞 `touch / key` | 否 | 是 | 是 |

## 7. 交互语义
- `SymbolIcon` 本轮只保留只读显示语义，不承载 click、toggle 或导航职责。
- 主控件录制轨道通过程序化 setter 切换图标和文案。
- 底部 preview 使用静态 preview API，统一吞掉 `touch / key`，避免误接管焦点或 pressed 状态。

## 8. 本轮收口内容
- 新增 `egui_view_symbol_icon.h/.c`，实现基于图标字体的只读 `SymbolIcon` 控件。
- 补齐 `standard / subtle / accent` 三套样式 helper。
- 补齐 `set_symbol()`、`set_icon_font()`、`set_palette()` 和静态 preview API。
- demo 页面保留一个主 `SymbolIcon` 与两个静态 preview，录制路径覆盖三组图标快照闭环。
- 单测覆盖样式 helper、setter、默认回退与静态 preview 输入抑制。

## 9. 录制动作设计
1. 还原 `Home` 图标初始态。
2. 抓取初始帧。
3. 切到 `Notifications` 图标。
4. 抓取第二帧。
5. 切到 `Settings` 图标。
6. 抓取第三帧。
7. 回到 `Home` 图标。
8. 抓取最终收尾帧。

## 10. 编译、测试与 runtime 验收
```bash
make all APP=HelloCustomWidgets APP_SUB=display/symbol_icon PORT=pc
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category display
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/symbol_icon --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category display --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category display --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub display/symbol_icon
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_display_symbol_icon
```

验收重点：
- 主图标和底部 preview 必须完整可见，不黑屏、不白屏、不裁切。
- `Home / Notifications / Settings` 三组主图标快照必须稳定区分。
- `subtle` 与 `accent` preview 必须保持静态 reference，不响应真实输入。

## 11. 已知限制
- 当前只覆盖最小 `SymbolIcon` 语义，不扩展动画、状态叠层或图标容器组合控件。
- 由于仓库没有单独的 Fluent Symbols 资源，当前使用 `Material Symbols` 子集近似相同语义。
- 当前不提供图标枚举封装，调用方直接传入图标字形字符串。
