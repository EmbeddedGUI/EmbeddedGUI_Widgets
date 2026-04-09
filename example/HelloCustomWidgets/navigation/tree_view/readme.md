# tree_view 设计说明

## 参考来源
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`
- 对应组件名：`TreeView`
- 本次保留状态：`standard`、`branch expanded`、`selection`、`compact`、`read only`
- 删除效果：页面级 `guide`、状态文案、section label、双列预览壳、可点击 preview 卡、重描边、强高亮引导条
- EGUI 适配说明：继续复用仓库内 `tree_view` 基础实现，本轮只收口 `reference` 页面结构、示例快照和绘制强度，不修改 `sdk/EmbeddedGUI`；`snapshot / current selection / compact / read only / view disabled` 切换共享同一套 `pressed` 清理语义

## 1. 为什么需要这个控件
`tree_view` 用来表达标准层级导航和资源浏览语义，适合文件树、设置分类、目录结构和知识树这类“父子关系可见”的场景。

## 2. 为什么现有控件不够用
- `nav_panel` 更接近平铺导航，不表达递进层级。
- `breadcrumb_bar` 只表达当前位置，不保留兄弟节点和分支展开。
- `menu_flyout` 是临时命令列表，不适合持续浏览结构。
- `tab_view` 和 `tab_strip` 负责工作区切换，不负责树状层级。

因此这里继续保留 `tree_view`，但示例页需要回到统一的 `Fluent / WPF UI` reference 结构。

## 3. 目标场景与示例概览
- 主控件：展示标准 `TreeView`，保留分支展开、层级缩进、选中项和 meta 摘要。
- `compact` 预览：保留相同树语义，但压缩为更小尺寸，用于验证小尺寸 reference 收口。
- `read only` 预览：保留冻结态和固定选择，只作为静态对照，不再承担点击或焦点职责，并显式抑制 touch / key 输入。
- 页面只保留标题、主 `tree_view` 和底部 `compact / read only` 双预览，不再保留旧的预览列容器和说明性页面 chrome。

## 4. 视觉与布局规格
- 根容器尺寸：`224 x 236`
- 主控件尺寸：`198 x 116`
- 底部对照行尺寸：`216 x 80`
- `compact` 预览：`104 x 80`
- `read only` 预览：`104 x 80`
- 页面结构：标题 -> 主 `tree_view` -> `compact / read only`
- 样式约束：
  - 保持浅底、白色树卡、轻边框和低噪音引导线。
  - 选中行只保留轻量填充、细边框和窄侧边强调，不压过树层级。
  - caption、footer、meta pill 都保留，但要明显弱于主列表内容。
  - 底部两个 preview 固定为静态 reference 对照，不再参与页面交互闭环。

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `tree_primary` | `egui_view_tree_view_t` | `198 x 116` | `Controls open` | 主 `TreeView` |
| `tree_compact` | `egui_view_tree_view_t` | `104 x 80` | compact | 底部紧凑静态对照 |
| `tree_read_only` | `egui_view_tree_view_t` | `104 x 80` | read only | 底部只读静态对照 |
| `primary_snapshots` | `egui_view_tree_view_snapshot_t[4]` | - | `Controls / Docs / Resources / Settings` | 主控件录制轨道 |
| `compact_snapshots` | `egui_view_tree_view_snapshot_t[2]` | - | `Library / Review` | 紧凑预览程序化切换轨道 |
| `read_only_snapshots` | `egui_view_tree_view_snapshot_t[1]` | - | `Static preview` | 只读预览固定数据 |

## 6. 状态覆盖矩阵
| 区域 / 轨道 | Snapshot | 关键状态 | 说明 |
| --- | --- | --- | --- |
| 主控件 | `Controls open` | 默认分支 | 保留分层缩进和焦点项 |
| 主控件 | `Docs open` | 第二条展开轨道 | 验证选中项与 meta 变化 |
| 主控件 | `Resources open` | 警示 tone | 验证弱高亮和分支层级仍可读 |
| 主控件 | `Settings open` | 最终收尾态 | 验证列表、caption、footer 一致性 |
| `compact` | `Library branch` | 紧凑对照 | 验证小尺寸层级与标题收口 |
| `compact` | `Review branch` | 第二条预览轨道 | 只做程序化切换，不参与交互 |
| `read only` | `Static preview` | 冻结态摘要 | 固定只读，对外禁用触摸和焦点 |

## 7. `egui_port_get_recording_action()` 录制动作设计
1. 重置主控件、`compact` 和 `read only` 到默认 snapshot。
2. 请求默认截图。
3. 切到主控件 `Docs open`。
4. 请求 `Docs` 截图。
5. 切到主控件 `Resources open`。
6. 请求 `Resources` 截图。
7. 程序化切到 `compact / Review branch`。
8. 请求 `compact` 截图。
9. 切到主控件 `Settings open`。
10. 请求最终截图并保留收尾等待。

## 8. 编译、touch、runtime、单测与文档检查
```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/tree_view PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category navigation
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/tree_view --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：
- 主控件和底部 `compact / read only` 预览都必须完整可见，不能裁切。
- 层级缩进、展开箭头、引导线和右侧 meta pill 需要稳定对齐，但不能回到旧版重描边风格。
- 选中行必须清晰，但不能压过树本身的层级信息。
- `compact / read only` 不再响应触摸，也不参与焦点循环；其中 `read only` 还需要清空 pressed 状态并忽略键盘输入。
- `snapshot / current selection / compact / read only / view disabled` 切换后不能残留树行的 `pressed` 高亮或下压位移渲染。
- `read_only_mode / !enable` 不仅要忽略后续 touch / key 输入，还要在收到新输入时先清理残留 `pressed` 状态。
- 触摸释放语义必须继续满足“按下与抬起命中同一目标才提交”。

## 9. 已知限制与后续方向
- 当前仍用固定 `snapshot + item` 数据，不做真实数据源绑定。
- 当前不做拖拽排序、虚拟滚动、复选框树和复杂展开动画。
- 触摸继续只负责切换选中行；分支展开仍通过 snapshot 展示。

## 10. 与现有控件的边界
- 相比 `nav_panel`：这里强调树层级，而不是平铺导航。
- 相比 `breadcrumb_bar`：这里强调可见兄弟节点与分支展开。
- 相比 `menu_flyout`：这里是持续浏览结构，不是临时命令列表。
- 相比 `tab_view`：这里表达层级资源树，不是工作区容器。

## 11. 参考设计系统与开源母本
- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 补充参考：`ModernWpf`

## 12. 对应组件名与本次保留的核心状态
- 对应组件名：`TreeView`
- 本次保留核心状态：
  - `branch expanded`
  - `selection`
  - `compact`
  - `read only`

## 13. 相比参考原型删掉的效果或装饰
- 删掉页面级 guide、状态栏、section label 和旧的双列 preview 包裹结构。
- 删掉可点击 preview 卡与 preview 轨道的外部交互职责。
- 删掉复选框树、拖拽排序、虚拟滚动和复杂展开动画。
- 删掉重描边、厚高亮条和高噪音 caption / footer / meta 胶囊。

## 14. EGUI 适配时的简化点与约束
- 使用固定 `snapshot + item` 数组驱动层级状态，优先保证 reference 稳定。
- 底部 `compact / read only` 固定放在同一行，只承担静态对照职责。
- `read only` 直接使用 `read_only_mode`，避免页面语义与控件实现脱节。
- 当前先作为 `HelloCustomWidgets` reference 示例维护，后续是否下沉框架层再单独评估。
