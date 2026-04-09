# token_input 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：web token/tag input 常见表单语义
- 对应组件名：`TokenInput`
- 本次保留状态：`standard`、`compact`、`read only`、`input focus`、`token add/remove`
- 删除效果：页面级 guide / 状态文案 / standard label / section divider / preview label、标签点击切换、夸张阴影、拖拽重排、复杂 IME 联动
- EGUI 适配说明：继续复用仓库里的 `token_input` 基础实现，本轮重点收口 reference 页面结构、统一浅色 palette，并保留最核心的 token 提交、删除和只读对照闭环

## 1. 为什么需要这个控件

`token_input` 用来表达“一个字段里编辑多个离散值”的标准表单语义，例如收件人、标签、设备分组和筛选条件。它比普通 `textinput` 更接近真实业务里的多值输入，也比纯展示型 `chips` 更强调可编辑表单控件。

## 2. 为什么现有控件不够用

- `textinput` 只覆盖单段文本，不负责多 token 提交、删除和焦点切换
- `chips` 更偏展示或点击，不承担输入位和编辑语义
- `auto_suggest_box` 更偏建议选择，不覆盖“已提交 token + 当前输入”的混合状态
- 当前主线需要一版更接近 `Fluent 2 / WPF UI TokenInput` 的标准 reference 页面

因此这里继续保留 `token_input`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `token_input`，覆盖 token 列表、输入位和删除 affordance
- 左下 `compact` 预览展示紧凑对照态
- 右下 `read only` 预览展示只读静态对照态
- 示例页只保留标题、主 `token_input` 和底部 `compact / read only` 双预览
- 录制动作只依赖真实 token 区域点击和键盘输入，不再依赖 guide、状态行或标签点击

组件目录：

- `example/HelloCustomWidgets/input/token_input/`

## 4. 视觉与布局规格

- 页面尺寸：`480 x 480`
- 根布局：`224 x 180`
- 页面结构：标题 -> 主 `token_input` -> `compact / read only` 双预览
- 主控件尺寸：`196 x 92`
- 底部双预览行：`216 x 48`
- `compact` 预览：`104 x 48`
- `read only` 预览：`104 x 48`
- 视觉约束：
  - 保持浅色 page panel、白色输入面和轻边框
  - token 胶囊维持低饱和浅色填充，不做 showcase 式多色 badge 堆叠
  - 焦点和删除 affordance 保持轻量强调，不做悬浮夸张动效
  - `compact` 只收紧密度与字体，不改变语义
  - `read only` 保留结果呈现，但不承担交互职责

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 180` | enabled | 页面根容器 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Token Input` | 页面标题 |
| `editor_primary` | `egui_view_token_input_t` | `196 x 92` | `Alice / Ops / QA` | 主 token 输入控件 |
| `editor_compact` | `egui_view_token_input_t` | `104 x 48` | `AL / BO` | 紧凑静态预览 |
| `editor_read_only` | `egui_view_token_input_t` | `104 x 48` | `Audit / Ops / QA / Net / Sys` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认态 | `Alice / Ops / QA` | `AL / BO` | 固定 token 集 |
| 输入焦点 | 点击输入位回到尾部输入区 | 不响应 | 不响应 |
| 键盘提交 | `N / E / T / Enter` 追加新 token | 不响应 | 不响应 |
| remove 删除 | 点击 token remove 图标删除最后一项 | 不响应 | 不响应 |
| 主 snapshot 轮换 | 收件人 -> 标签 | 保持 | 保持 |
| 紧凑 snapshot 轮换 | 保持 | `AL / BO` -> `UI / QA / OPS / SYS / NET` | 保持 |
| 只读弱化 | 不适用 | 不适用 | 保留静态只读对照 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主 snapshot、`compact` snapshot 和只读对照，并请求主控件焦点
2. 请求第一页默认截图
3. 点击主控件输入位，验证焦点进入
4. 请求第二页输入焦点截图
5. 发送 `N / E / T / Enter`，提交一个新 token
6. 请求第三页新增 token 截图
7. 点击新增 token 的 remove 图标
8. 请求第四页删除结果截图
9. 程序化切换主 snapshot 到标签场景
10. 请求第五页 snapshot 切换截图
11. 程序化切换 `compact` 预览到第二个静态快照
12. 请求最终对照截图

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/token_input PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/token_input --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主控件和底部双预览必须完整可见
- token 文本、输入位和 remove affordance 都要清晰可辨
- 主控件必须像标准表单 `TokenInput`，而不是带外部状态桥接的展示卡片
- `compact` 与 `read only` 必须是静态对照，不再承担标签切换职责
- 页面中不再出现 guide、状态文案、standard label、section divider 和外部 preview label

## 9. 已知限制与后续方向

- 当前版本不做拖拽重排
- 当前不做 IME 候选联动或复杂粘贴策略
- 当前仍以固定 snapshot 演示为主，不接动态建议源
- 后续如果要继续下沉到框架层，再评估与 `textinput` / `chips` / `autocomplete` 的共用边界

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `textinput`：这里是多 token 编辑器，而不是单值文本框
- 相比 `chips`：这里保留输入位和编辑语义，不是纯展示 chip 列表
- 相比 `auto_suggest_box`：这里强调“已提交 token + 当前输入”的混合状态
- 相比 `segmented_control`：这里不是互斥切换，而是可累积的多值输入

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：web token/tag input 常见表单模式

## 12. 对应组件名与保留核心状态

- 对应组件名：`TokenInput`
- 本次保留核心状态：
  - `standard`
  - `compact`
  - `read only`
  - `input focus`
  - `token add/remove`

## 13. 相比参考原型删除的效果或装饰

- 不做页面级 guide、状态回显、standard label、section divider 和 preview label
- 不做标签点击轮换和外部状态桥接
- 不做拖拽重排、复杂 IME 联动和桌面端夸张阴影
- 不做 showcase 式多色 token 和额外装饰层

## 14. EGUI 适配时的简化点与约束

- 直接复用 `token_input` 基础结构，避免新增重复基础控件
- 通过示例页 palette 统一 Reference Track 颜色和尺寸
- 用真实 token 区域点击和键盘输入完成录制，不再依赖页面 chrome
- 先完成示例级 `TokenInput` reference，再决定是否继续扩展更复杂的输入策略
