# segmented_control 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`SegmentedControl`
- 本次保留状态：`standard`、`compact`、`read only`、`focused`
- 删除效果：页面级 guide、状态回显、section divider、外部 preview 标签、场景化轮播入口、复杂 hover 动画、Acrylic 和图标化装饰段
- EGUI 适配说明：复用仓库已有 `segmented_control` 核心交互，在 `480 x 480` 页面里优先保证选中胶囊、焦点 ring 和底部双预览对照稳定；底部 `compact / read only` 统一通过 `hcw_segmented_control_override_static_preview_api()` 固定为静态 preview，不再承担 preview dismiss 桥接

## 1. 为什么需要这个控件

`segmented_control` 用来表达同一组互斥选项之间的轻量切换，适合视图模式、时间范围、过滤级别、密度选择这类页内局部状态切换。它比 tab 更轻，比 radio 更紧凑，是 Fluent / WPF UI 主线里明确存在的标准输入控件。

## 2. 为什么现有控件不够用

- `tab_strip` 更偏整页导航，不适合页内局部过滤
- `toggle_button` 与 `split_button` 是单个动作控件，不是互斥选项组
- `radio_button` 强调表单语义，不适合横向胶囊式切换
- 当前 reference 主线需要一版更接近 `Fluent 2 / WPF UI` 的标准 `SegmentedControl`

因此这里继续保留 `segmented_control`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `segmented_control`，保留真实触摸切换与焦点状态
- 左下 `compact` 预览展示紧凑尺寸下的轻量 reference
- 右下 `read only` 预览展示静态只读对照
- 主控件保留 `Left / Right / Up / Down / Home / End / Tab` 键盘闭环
- 底部 `compact / read only` 预览统一通过 `hcw_segmented_control_override_static_preview_api()` 吞掉 touch / key，全程保持静态 reference，不再承担 preview dismiss 桥接
- 示例页只保留标题、主 `segmented_control` 和底部 `compact / read only` 双预览，不再保留 guide、状态回显和外部标签

目录：

- `example/HelloCustomWidgets/input/segmented_control/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 112`
- 页面结构：标题 -> 主 `segmented_control` -> `compact / read only` 双预览
- 主控件：`196 x 38`
- 底部双预览容器：`216 x 30`
- `compact` 预览：`104 x 30`
- `read only` 预览：`104 x 30`
- 视觉规则：
  - 使用浅灰白 page panel + 白底低噪音表面
  - 主控件保留标准胶囊选中块、外边框和焦点 ring
  - `compact` 预览只收紧圆角、padding 和分段间距，不改变控件语义
  - `read only` 预览保留选中项表达，但不承担真实交互

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 112` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Segmented Control` | 页面标题 |
| `control_primary` | `egui_view_segmented_control_t` | `196 x 38` | `Overview / Team / Usage / Access` | 标准主控件 |
| `control_compact` | `egui_view_segmented_control_t` | `104 x 30` | `Day / Week` | 紧凑静态预览 |
| `control_read_only` | `egui_view_segmented_control_t` | `104 x 30` | `Off / Auto / Lock` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `Overview / Team / Usage / Access`，选中 `Team` | `Day / Week`，选中 `Day` | `Off / Auto / Lock`，选中 `Auto` |
| 触摸切换 | 切到第三段 `Usage` | 静态不变 | 静态不变 |
| 键盘末项态 | 切到 `Live / Pending / History`，按 `End` 后选中 `History` | 静态不变 | 静态不变 |
| 键盘首项态 | 切到 `Day / Week / Month / Year`，按 `Home` 后选中 `Day` | 静态不变 | 静态不变 |
| 最终稳定帧 | 回到默认 `Overview / Team / Usage / Access`，选中 `Team` | `Day / Week`，选中 `Day` | `Off / Auto / Lock`，选中 `Auto` |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 重放默认主控件与底部 `compact / read only` preview，显式请求主控件 focus，并抓取默认态截图
2. 触摸主控件第三段，验证真实切换反馈后抓取第二帧
3. 程序化切换主控件到 `Live / Pending / History` 组，并通过 `End` 跳到最后一项，再抓取第三帧
4. 程序化切换主控件到 `Day / Week / Month / Year` 组，并通过 `Home` 回到首项，再抓取第四帧
5. 显式恢复默认 `Overview / Team / Usage / Access` 组和主控件 focus，最后抓取最终稳定帧

说明：

- 录制阶段不再切换 `compact` preview
- 底部双 preview 全程通过 `hcw_segmented_control_override_static_preview_api()` 吞掉 touch / key 且不改状态
- `request_page_snapshot()` 统一走 `layout_page() + egui_view_invalidate() + recording_request_snapshot()` 显式布局路径

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/segmented_control PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/segmented_control --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/segmented_control
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_segmented_control
```

## 10. 验收重点
- 主控件和底部双预览必须完整可见，不能被裁切
- 选中胶囊、边框、文字和 focus ring 必须清晰可辨
- 主控件触摸与键盘切换必须都能工作，且不再依赖外部 guide / label 点击
- 主控件必须继续满足 `same-target release`：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，只有回到原命中区后 `UP(A)` 才提交
- 页面中不再出现 guide、状态回显、section divider 和外部 preview 标签
- `compact` 与 `read only` 必须保持 Fluent / WPF UI 的低噪音浅色 reference
- 主控件的 `set_segments / current index / style helper / touch cancel / !enable / key guard` 链路都不能残留 `pressed` 覆盖层
- `Left / Right / Up / Down / Home / End / Tab` 只允许驱动主控件切换；无关按键不能留下残留 `pressed`
- 底部 `compact / read only` 预览必须通过 `hcw_segmented_control_override_static_preview_api()` 吞掉 touch / key 输入，并在收到输入后立即清理残留 `pressed`；录制阶段不再依赖 preview 做状态桥接

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_segmented_control/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 全帧共出现 `5` 组唯一状态，主区哈希分组为 `[0,1,8,9,10] / [2] / [3] / [4,5] / [6,7]`
  - 主区 RGB 差分边界收敛到 `(46, 185) - (433, 237)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 238` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界

- 相比 `tab_strip`：本控件用于局部状态切换，不承担整页导航
- 相比 `radio_button`：本控件是横向胶囊分段，不是表单单选列
- 相比 `toggle_button`：本控件表达互斥分组，不是单一 on/off 动作
- 相比核心层 `src/widget/egui_view_segmented_control`：本目录负责 reference 页面与样式落地，不重复造核心控件

## 13. 本次保留的核心状态与删减项
- 本次保留状态：
  - `standard`
  - `compact`
  - `read only`
  - `focused`
  - `touch switch`
  - `keyboard switch`
- 删减的装饰或桥接：
  - 页面级 guide、状态回显、section divider 和外部 preview 标签
  - 图标段、badge、计数器和桌面级 hover reveal
  - Acrylic、复杂阴影和多层装饰描边
  - 拖拽重排、溢出折叠和复杂自适应动画

## 14. 当前验收结果（2026-04-18）

- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/segmented_control PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - `make all APP=HelloUnitTest PORT=pc_test`
  - `X:\output\main.exe`
  - 本轮按本地 unit 日志复核总计 `845 / 845`，其中 `segmented_control` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/segmented_control --track reference --timeout 10 --keep-screenshots`
  - `11 frames captured -> runtime_check_output/HelloCustomWidgets_input_segmented_control/default`
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/segmented_control`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_segmented_control`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.0855 colors=159`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 全帧共出现 `5` 组唯一状态，主区哈希分组为 `[0,1,8,9,10] / [2] / [3] / [4,5] / [6,7]`
  - 主区 RGB 差分边界为 `(46, 185) - (433, 237)`
  - 遮罩主区边界后，主区外唯一哈希数为 `1`
  - 以 `y >= 238` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 结论：主区覆盖默认态、触摸切换路径、键盘 `End` 末项态与 `Home` 首项态，最终稳定帧已显式回到默认快照；底部 `compact / read only` preview 全程静态
