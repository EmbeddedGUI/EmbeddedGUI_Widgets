# flip_view 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`WinUI FlipView`
- 对应组件名：`FlipView`
- 本次保留状态：`standard`、`previous / surface / next`、`compact`、`read only`
- 删除效果：页面级 `guide`、状态文案、section divider、preview label、故事化 hero 文案、过重阴影和高强调按钮
- EGUI 适配说明：继续复用仓库内 `flip_view` 基础实现，本轮只收口 `reference` 页结构、示例文案和绘制强度，不改 `sdk/EmbeddedGUI`；`items / current index / current part / compact / read only / view disabled` 切换共享同一套 `pressed` 清理语义

## 1. 为什么需要这个控件
`flip_view` 用来表达“当前只保留一张主卡，但允许按顺序浏览前后内容”的标准单卡轮播语义。它适合精选摘要、运营看板、记录回看、步骤卡片这类一次只希望用户聚焦当前页的场景。

## 2. 为什么现有控件不够用
- `coverflow_strip` 强调中心卡和侧卡透视层级，不是标准 `FlipView` 的单卡浏览语义。
- `pips_pager` 强调页码位置反馈，本身不承担主内容展示。
- `tab_view` 和 `tab_strip` 强调多页切换，不适合“保持单页承载内容、按顺序前后翻页”的场景。

因此这里继续保留 `flip_view`，但示例页必须回到统一的 `Fluent / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主状态：一张主卡承载当前内容，左右 overlay 按钮负责顺序翻页。
- `compact` 预览：保留同一控件语义，只压缩层级和辅助文本，用于验证小尺寸收口。
- `read only` 预览：保留卡面结构，但冻结翻页操作，用于验证禁用边界。
- 页面只保留标题、主 `flip_view` 和底部 `compact / read only` 双预览，不再保留 guide、状态栏、section label、preview label 这类页面级 chrome。

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 224`
- 主控件尺寸：`196 x 122`
- 底部对照行尺寸：`216 x 64`
- `compact` 预览：`104 x 64`
- `read only` 预览：`104 x 64`
- 页面结构：标题 -> 主 `flip_view` -> `compact / read only`
- 样式约束：
  - 外层面板保持浅底、轻边框、低噪音阴影
  - 主卡只保留 eyebrow pill、counter pill、标题、描述、footer 这些控件内层信息
  - previous / next 按钮继续保留，但按钮填充、描边和图标强调度压低
  - 不保留透视侧卡、故事化大标题、强装饰色块或页面级说明条

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
| `read only` | `Snapshot` | 禁用对照 | 箭头和内容保留，但不响应输入 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主轨道、`compact` 轨道和 `read only` 预览后输出默认帧。
2. 发送 `Right` 键，验证主控件翻到下一页。
3. 发送 `End` 键，验证尾页边界和 next 禁用态。
4. 程序化切换主轨道到 `Operations`，验证主内容和标题同步变化。
5. 程序化切换 `compact` 轨道，验证底部静态对照同步更新。

## 8. 编译、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/flip_view PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/flip_view --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 不能黑屏、白屏、主卡缺失或预览裁切
- 主卡、overlay 按钮、`compact / read only` 对照都必须完整可见
- eyebrow、counter、chevron、短标题和 footer 的左右留白必须稳定
- 翻页、尾页禁用、轨道切换后，截图中必须能明确看出内容和边界状态变化
- `items / current index / current part / compact / read only / view disabled` 切换后不能残留 previous / surface / next 的 `pressed` 高亮或下压位移渲染
- `compact / read_only_mode / !enable` 不仅要忽略后续 touch / key 输入，还要在收到新输入时先清理残留 `pressed` 状态

## 9. 已知限制与后续方向
- 当前用纯绘制卡片模拟 `FlipView`，不加载真实图片资源。
- 当前不做 hover 渐显 / 渐隐动画，只保留静态 overlay 语义。
- 当前不做自动轮播、缩放模式和图片裁剪语义，后续如需图片浏览器方向再扩展。

## 10. 与现有控件的边界
- 相比 `coverflow_strip`：这里不展示左右透视侧卡，只保留单卡 surface。
- 相比 `pips_pager`：这里展示的是当前页内容本体，页码只是次级信息。
- 相比 `tab_view`：这里是连续翻页浏览，不是多页签切换。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`WinUI FlipView`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`FlipView`
- 本次保留核心状态：
  - `previous / surface / next`
  - `standard`
  - `compact`
  - `read only`
- 本次保留交互：
  - 键盘 `Left / Right / Home / End / Tab / Enter / Space / Plus / Minus / Escape`
  - 触摸 `previous / next / surface`

## 13. 相比参考原型删掉的效果或装饰
- 删掉页面级 `guide`、状态条、section divider、preview label 和点击标签切换场景。
- 删掉故事化 hero 文案和过强配色表达，统一改成低噪音业务摘要文案。
- 删掉高强调阴影、厚边框和更像 showcase 的按钮强调。
- 删掉真实图片加载、自动轮播和 hover 动画，只保留核心控件语义。

## 14. EGUI 适配时的简化点与约束
- 不引入图片资源，使用纯色卡面 + 文本层级模拟内容页。
- 通过 `current_part` + `current_index` 统一键盘与触摸状态机。
- `compact` 和 `read only` 直接在同一控件里切换绘制分支，避免拆分多套实现。
- 页面空白区和底部预览不再承担场景切换职责，只保留 reference 对照和失焦收口语义。
