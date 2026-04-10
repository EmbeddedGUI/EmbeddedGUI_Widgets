# flip_view 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`WinUI FlipView`
- 对应组件名称：`FlipView`
- 本次保留状态：`standard`、`previous / surface / next`、`compact`、`read only`
- 删除效果：页面级 `guide`、状态说明条、`section divider`、`preview label`、故事化 hero 文案、过强阴影和高对比强调按钮
- EGUI 适配说明：继续复用仓库内 `flip_view` 基础实现，本轮重点收口 `same-target release`、`static preview`、`pressed` 清理、preview 点击后的 focus 收尾和交互后的最终稳定帧，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`flip_view` 用来表达“当前只保留一张主卡片，但允许用户按顺序浏览前后内容”的标准单卡翻页语义。它适合精选摘要、运营看板、记录回看、步骤卡片这类一次只希望用户聚焦当前页的场景。

## 2. 为什么现有控件不够用
- `coverflow_strip` 强调中心卡和侧卡透视层级，不是标准 `FlipView` 的单卡浏览语义。
- `pips_pager` 侧重页码位置反馈，本身不承载主内容。
- `tab_view` 和 `tab_strip` 强调多页签切换，不适合“保留单页内容并按顺序前后翻页”的场景。

因此这里继续保留 `flip_view`，但示例页只保留统一的 `Fluent / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主状态：一张主卡片承载当前内容，左右 overlay 按钮负责顺序翻页。
- `compact` 预览：保留同一控件语义，只压缩层级和辅助文本，用于验证小尺寸收口。
- `read only` 预览：保留卡片结构，但冻结翻页操作，用于验证只读边界。
- 页面只保留标题、主 `flip_view` 和底部 `compact / read only` 双预览，不再保留 guide、状态栏、section label、preview label 这类页面级 chrome。

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`196 x 122`
- 底部对照行尺寸：`216 x 64`
- `compact` 预览尺寸：`104 x 64`
- `read only` 预览尺寸：`104 x 64`
- 页面结构：标题 -> 主 `flip_view` -> `compact / read only`
- 样式约束：
  - 外层面板保持浅底、轻边框、低噪音阴影。
  - 主卡只保留 `eyebrow pill`、`counter pill`、标题、描述和 `footer` 这些控件内层信息。
  - `previous / next` 按钮继续保留，但填充、描边和图标强调度压低。
  - 不保留透视侧卡、故事化大标题、强装饰色块或页面级说明栏。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `flip_view_primary` | `egui_view_flip_view_t` | `196 x 122` | `Highlights / index=1` | 主 `FlipView` |
| `flip_view_compact` | `egui_view_flip_view_t` | `104 x 64` | `Compact / index=0` | 紧凑静态对照 |
| `flip_view_read_only` | `egui_view_flip_view_t` | `104 x 64` | `Read only / index=1` | 只读静态对照 |
| `primary_tracks` | `flip_view_track_t[3]` | - | `Highlights` | 主控件轨道切换数据 |
| `compact_tracks` | `flip_view_track_t[2]` | - | `Compact` | 紧凑预览轨道切换数据 |
| `read_only_track` | `flip_view_track_t` | - | `Read only` | 只读预览固定数据 |

## 6. 状态覆盖矩阵

| 区域 / 轨道 | Snapshot | 关键状态 | 说明 |
| --- | --- | --- | --- |
| 主控件 `Highlights` | `Shipping board` | 默认态 | 初始页停在中间项，前后翻页都可用 |
| 主控件 `Highlights` | `Archive handoff` | 尾页边界 | `End` 后 next 按钮禁用 |
| 主控件 `Operations` | `Planning board` | 主轨道切换 | 验证标题、卡面内容与配色同步变化 |
| `compact` | `Snapshot` / `Archive` | 小尺寸收口 | helper 隐藏，布局更紧凑 |
| `read only` | `Snapshot` | 只读对照 | 箭头和内容保留，但不响应输入 |

## 7. 交互语义与预览收口
- 主控件保留真实 `touch / key` 交互，并按 non-dragging 控件收口到 `same-target release`：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- `set_font()`、`set_meta_font()`、`set_title()`、`set_helper()`、`set_palette()`、`set_items()`、`set_current_index()`、`set_current_part()`、`set_compact_mode()`、`set_read_only_mode()` 统一先清理残留 `pressed_part / is_pressed`。
- `compact / read only / !enable` 收到新输入时必须先清理残留 `pressed`，然后拒绝后续交互。
- 底部两个 preview 统一通过 `egui_view_flip_view_override_static_preview_api()` 收口：
  - 吞掉 `touch / key`
  - 只负责清理残留 `pressed`
  - 不修改 `current_index / current_part`
  - 不触发 `on_changed`
- preview 点击时只清主控件 focus，不改 preview 自身状态，也不承担页面切换职责。

## 8. 录屏动作设计
`egui_port_get_recording_action()` 的录屏顺序如下：
1. 重置主轨道、`compact` 轨道和 `read only` 预览后输出默认帧。
2. 发送 `Right` 键，验证主控件翻到下一页。
3. 发送 `End` 键，验证尾页边界和 next 禁用态。
4. 程序化切换主轨道到 `Operations`，验证主内容和标题同步变化。
5. 程序化切换 `compact` 轨道，并让主控件获得 focus。
6. 点击 `compact` preview，验证 preview 只负责清主控件 focus。
7. 追加 preview 点击后的收尾帧和最终稳定帧，确认没有残留 `pressed`、黑白屏、裁切或整屏污染。

## 9. 编译、运行时、单测与文档检查
```bash
make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
output\main.exe

make clean APP=HelloCustomWidgets APP_SUB=navigation/flip_view PORT=pc
make all APP=HelloCustomWidgets APP_SUB=navigation/flip_view PORT=pc

python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/flip_view --track reference --timeout 10 --keep-screenshots
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 不能黑屏、白屏、主卡缺失或 preview 裁切。
- 主卡、overlay 按钮、底部 `compact / read only` 对照必须完整可见。
- `same-target release`、`cancel`、`compact / read only / disabled guard`、`static preview` 必须全部通过单测。
- preview 点击后的收尾帧和最终稳定帧里，不能残留 `pressed`、focus 污染或整屏异常。
- `Right / Home / End / Plus / Minus / Tab / Enter / Space / Escape` 对主控件的键盘交互不能回归。

## 10. 已知限制与后续方向
- 当前使用纯绘制卡片模拟 `FlipView`，不加载真实图片资源。
- 当前不做 hover 渐显 / 渐隐动画，只保留静态 overlay 语义。
- 当前不做自动轮播、缩放模式和图片裁剪语义，如需图片浏览器方向再扩展。

## 11. 与现有控件的边界
- 相比 `coverflow_strip`：这里不展示左右透视侧卡，只保留单卡 `surface`。
- 相比 `pips_pager`：这里展示的是当前页内容本体，页码只是次级信息。
- 相比 `tab_view`：这里是连续翻页浏览，不是多页签切换。

## 12. 本次保留的核心状态与删减项
- 保留的核心状态：
  - `previous / surface / next`
  - `standard`
  - `compact`
  - `read only`
- 保留的交互：
  - 键盘 `Left / Right / Home / End / Tab / Enter / Space / Plus / Minus / Escape`
  - 触摸 `previous / next / surface`
- 删除的装饰：
  - 页面级 guide、状态条、section divider、preview label
  - 故事化 hero 文案和过强配色表达
  - 高强度阴影、厚边框、过度强调按钮
  - 自动轮播、hover 动画和图片资源加载

## 13. EGUI 适配时的简化点与约束
- 不引入图片资源，使用纯色卡面加文本层级模拟内容页。
- 通过 `current_part + current_index + pressed_part` 统一键盘与触摸状态机。
- `compact` 和 `read only` 直接在同一控件内切换绘制分支，避免拆成多套实现。
- preview 的交互职责收口到控件自己的 `static preview API`，页面层只追加主控件 focus 收尾。
