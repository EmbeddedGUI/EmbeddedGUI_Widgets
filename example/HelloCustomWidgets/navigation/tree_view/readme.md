# TreeView 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`
- 对应组件名：`TreeView`
- 当前保留形态：`Controls open`、`Docs open`、`Resources open`、`Settings open`、`compact`、`read only`
- 当前保留交互：主区保留层级缩进、分支展开、selection、`Up / Down / Home / End` 键盘导航、same-target release，以及 `current_snapshot / current_index / font / meta_font / palette / compact / read_only` setter 统一清理 `pressed`；底部 `compact / read only` preview 继续通过静态 preview API 吞掉 `touch / key` 并保持状态不变，不触发 `on_selection_changed`
- 当前移除内容：页面级 `guide`、状态文案、section label、双列预览壳、可点击 preview 卡、preview 清主控件 focus 的桥接逻辑、第二条 `compact` 预览轨道、重描边和强高亮引导条，以及旧版 finalize README 章节顺序
- EGUI 适配说明：继续复用仓库内 `tree_view` 基础实现，本轮只收口 `reference` 页面结构、静态 preview、README、单测与验收口径，不修改 `sdk/EmbeddedGUI`

## 1. 为什么需要这个控件
`tree_view` 用来表达标准层级导航和资源浏览语义，适合文件树、设置分类、目录结构和知识树这类“父子关系可见”的场景。

## 2. 为什么现有控件不够
- `nav_panel` 更接近平铺导航，不表达递进层级。
- `breadcrumb_bar` 只表达当前位置，不保留兄弟节点和分支展开。
- `menu_flyout` 是临时命令列表，不适合持续浏览结构。
- `tab_view` 和 `tab_strip` 负责工作区切换，不负责树状层级。

## 3. 当前页面结构
- 标题：`Tree View`
- 主区：一个主 `tree_view`
- 底部：两个真正静态的 preview
- 左侧 preview：`compact`，固定显示 `Library branch`
- 右侧 preview：`read only`，固定显示 `Static preview`
- 页面结构统一收口为：标题 -> 主 `tree_view` -> 底部 `compact / read only`

目录：
- `example/HelloCustomWidgets/navigation/tree_view/`

## 4. 主区 reference 快照
主区录制轨道只保留四组主区状态与最终稳定帧：

1. `Controls open`
   默认主树，选中 `Tree View`
2. `Docs open`
   切到文档分支，选中 `API`
3. `Resources open`
   切到 warning tone 分支，选中 `Tokens`
4. `Settings open`
   切到设置分支，选中 `Themes`
5. `Controls open`
   回到默认主树，作为最终稳定帧

底部 preview 在整条录制轨道中保持固定：

1. `compact`
   `Library branch`
   紧凑静态对照，固定 `compact_mode`
2. `read only`
   `Static preview`
   只读静态对照，固定 `compact_mode + read_only_mode`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根容器尺寸：`224 x 236`
- 主控件尺寸：`198 x 116`
- 底部对照行尺寸：`216 x 80`
- 单个 preview 尺寸：`104 x 80`
- 页面结构：标题 -> 主 `tree_view` -> 底部 `compact / read only`
- 页面风格：浅灰 `page panel`、白色树卡、轻边框、低噪音引导线，以及克制的 caption / footer / meta pill 层级

## 6. 状态矩阵
| 状态 / 区域 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| `Controls open` | 是 | 否 | 否 |
| `Docs open` | 是 | 否 | 否 |
| `Resources open` | 是 | 否 | 否 |
| `Settings open` | 是 | 否 | 否 |
| `compact_mode` | 否 | 是 | 是 |
| `read_only_mode` | 否 | 否 | 是 |
| `keyboard navigation` | 是 | 否 | 否 |
| 静态 preview 吞掉 `touch / key` 且状态不变 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_tree_view.c` 当前覆盖 `8` 条用例：

1. `set_snapshots()` 覆盖 snapshot 数量钳制、空数据重置，以及 `current_snapshot / current_index / pressed` 清理。
2. `current_snapshot / current_index / font / meta_font / palette / compact / read_only` setter 覆盖 guard、默认字体回落、选择保持与 `pressed` 清理。
3. snapshot 切换与 index clamp 覆盖跨 snapshot 选中项收敛和越界 index 钳制。
4. 触摸交互覆盖 same-target release：`DOWN(A) -> MOVE(B) -> UP(B)` 不提交，回到 `A` 后 `UP(A)` 才触发 selection；`ACTION_CANCEL` 只清理 `pressed`。
5. 键盘 `Up / Down / Home / End` 继续驱动 selection 变化。
6. `compact_mode` 切换后会清理残留 `pressed`，但保留选择行为。
7. `read_only_mode / !enable` guard 会清理 `pressed` 并忽略后续 `touch / key` 输入，恢复后继续允许 selection 变化。
8. 静态 preview 用例验证“consumes input and keeps state”，固定校验 `current_snapshot / current_index / compact_mode` 不变，且不会触发 `on_selection_changed`。

补充说明：
- 主区 same-target release 继续遵守非拖拽控件口径：只有回到初始命中项再 `UP` 才能提交。
- 底部 `compact / read only` preview 统一通过 `egui_view_tree_view_override_static_preview_api()` 吞掉输入，只承担静态 reference 对照职责。
- 预览输入只清理残留 `pressed`，不改 `current_snapshot / current_index`，也不触发 `on_selection_changed`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 当前 `reference` 轨道顺序如下：

1. 还原默认 `Controls open` 和底部两个静态 preview，请求首帧并等待 `TREE_VIEW_RECORD_FRAME_WAIT`。
2. 切到 `Docs open`，等待 `TREE_VIEW_RECORD_WAIT`。
3. 请求第二帧并等待 `TREE_VIEW_RECORD_FRAME_WAIT`。
4. 切到 `Resources open`，等待 `TREE_VIEW_RECORD_WAIT`。
5. 请求第三帧并等待 `TREE_VIEW_RECORD_FRAME_WAIT`。
6. 切到 `Settings open`，等待 `TREE_VIEW_RECORD_WAIT`。
7. 请求第四帧并等待 `TREE_VIEW_RECORD_FRAME_WAIT`。
8. 回到默认 `Controls open`，同步底部 preview 固定状态并等待 `TREE_VIEW_RECORD_WAIT`。
9. 请求最终稳定帧，并继续等待 `TREE_VIEW_RECORD_FINAL_WAIT`。

说明：
- 所有截图请求统一走 `request_page_snapshot()`，先布局，再刷新，再申请录制。
- 底部 `compact / read only` preview 在整条轨道中不承担任何状态切换职责。
- 主区变化只来自 `Controls open / Docs open / Resources open / Settings open` 四组 snapshot，以及最终回到默认 `Controls open` 的稳定帧。
- `test.c` 已对齐统一的 `ui_ready + layout_page + request_page_snapshot` 收口模板：初始化阶段在 root view 挂载前后各重放一次默认态与 preview，确保录制首尾都走同一条显式布局路径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/tree_view PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tree_view --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category navigation --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/tree_view
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_tree_view
```

## 10. 验收重点
- 主控件和底部 `compact / read only` preview 都必须完整可见，不能黑屏、白屏或裁切。
- 层级缩进、展开箭头、引导线和右侧 meta pill 需要稳定对齐，但不能回到旧版重描边风格。
- 选中行必须清晰，但不能压过树本身的层级信息。
- `DOWN(A) -> MOVE(B) -> UP(B)` 不能误提交，只有回到 `A` 才能提交。
- `ACTION_CANCEL` 只能清理 `pressed`，不能误改 `current_index` 或误触发监听器。
- `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态，也不触发 `on_selection_changed`。
- 最终稳定帧必须显式回到默认 `Controls open`。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_navigation_tree_view/default`
- 本轮复核结果：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Controls`、`Docs`、`Resources` 和 `Settings`
  - 主区 RGB 差分边界为 `(44, 92) - (436, 254)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 254` 裁切底部 preview 后，preview 区唯一哈希数为 `1`
  - 最终稳定帧显式回到默认 `Controls open`

## 12. 与现有控件的边界
- 相比 `nav_panel`：这里强调树层级，而不是平铺导航。
- 相比 `breadcrumb_bar`：这里强调可见兄弟节点与分支展开。
- 相比 `menu_flyout`：这里是持续浏览结构，不是临时命令列表。
- 相比 `tab_view`：这里表达层级资源树，不是工作区容器。

## 13. 本轮保留与删减
- 保留的主区状态：`Controls open`、`Docs open`、`Resources open`、`Settings open`
- 保留的底部对照：`compact`、`read only`
- 保留的交互与实现约束：selection、keyboard navigation、same-target release、`current_snapshot / current_index / font / meta_font / palette / compact / read_only` setter 清理 `pressed`、static preview 对照
- 删减的旧桥接与旧装饰：页面级 `guide`、状态文案、section label、双列预览壳、可点击 preview 卡、preview 清主控件 focus 的桥接逻辑、第二条 `compact` 预览轨道、重描边和强高亮引导条、旧版 finalize README 章节顺序

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=navigation/tree_view PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核，总计 `845 / 845`，其中 `tree_view` suite `8 / 8`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=12 custom_skipped_allowlist=1`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tree_view --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_navigation_tree_view/default`
  - 共捕获 `11` 帧
- navigation 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category navigation --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category navigation --track reference --bits64`
  - navigation `13 / 13` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub navigation/tree_view`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_navigation_tree_view`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1799 colors=161`
- 截图复核结论：
  - 共捕获 `11` 帧
  - 主区共出现 `4` 组唯一状态，对应 `Controls`、`Docs`、`Resources` 和 `Settings`
  - 主区 RGB 差分边界为 `(44, 92) - (436, 254)`
  - 遮罩主区变化边界后主区外唯一哈希数为 `1`
  - 按 `y >= 254` 裁切底部 preview 后 preview 区唯一哈希数为 `1`
  - 结论：最终稳定帧已回到默认 `Controls open`，底部 `compact / read only` preview 全程保持静态
