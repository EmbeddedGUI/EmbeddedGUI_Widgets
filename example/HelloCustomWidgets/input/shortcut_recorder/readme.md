# shortcut_recorder 自定义控件设计说明

## 参考来源

- 参考设计系统：`Fluent 2`
- 参考开源库：`WPF UI`
- 次级补充参考：`ModernWpf`
- 对应组件名：`Keyboard shortcut field / accelerator editor`
- 本次保留状态：`standard`、`listening`、`preset apply`、`compact`、`read only`
- 删除效果：页面级 guide、状态回显、section divider、外部 preview 标签、场景化轮播入口、系统级冲突检测、Alt/Win 修饰键、复杂 hover 动画和桌面级焦点环过渡
- EGUI 适配说明：保留快捷键录入字段、监听态、preset 列表与只读摘要，在 `480 x 480` 页面里优先保证 token 可读性、状态对比和底部双预览稳定

## 1. 为什么需要这个控件

`shortcut_recorder` 用来表达标准快捷键录入语义，适合命令面板热键、全局搜索入口、操作加速键和工作区定制设置页。它强调“进入监听态后捕获组合键”，不是普通文本输入，也不是离散按钮集合。

## 2. 为什么现有控件不够用

- `textinput` 关注自由文本输入，不表达快捷键监听态
- `token_input` 强调多值 token 编辑，不表达单个组合键绑定
- `command_bar` 和 `command_palette` 负责触发命令，不负责持久化快捷键录入
- 当前 reference 主线需要一版更接近 `Fluent 2 / WPF UI` 的标准快捷键录入控件

因此这里继续保留 `shortcut_recorder`，但示例页必须回到统一的 reference 结构。

## 3. 目标场景与示例概览

- 主区域展示标准 `shortcut_recorder`，覆盖默认绑定、监听态、捕获组合键、preset 应用和清空绑定
- 左下 `compact` 预览展示紧凑尺寸下的快捷键摘要
- 右下 `read only` 预览展示锁定态摘要
- 主控件保留字段、preset、clear 的真实触摸 / 键盘闭环
- 示例页只保留标题、主 `shortcut_recorder` 和底部 `compact / read only` 双预览，不再保留 guide、状态回显和外部标签

目录：

- `example/HelloCustomWidgets/input/shortcut_recorder/`

## 4. 视觉与布局规格

- 画布：`480 x 480`
- 根布局：`224 x 264`
- 页面结构：标题 -> 主 `shortcut_recorder` -> `compact / read only` 双预览
- 主控件：`194 x 138`
- 底部双预览容器：`218 x 82`
- `compact` 预览：`106 x 82`
- `read only` 预览：`106 x 82`
- 视觉规则：
  - 使用浅灰白 page panel + 低噪音白色表面
  - 主控件保留快捷键 token、监听状态 pill、preset 行和 clear 动作
  - `compact` 预览保留录入摘要，但收敛为轻量对照
  - `read only` 预览保留锁定态摘要，不承担真实交互

## 5. 控件清单

| 变量名 | 类型 | 尺寸 (W x H) | 初始状态 | 用途 |
| --- | --- | ---: | --- | --- |
| `root_layout` | `egui_view_linearlayout_t` | `224 x 264` | enabled | 页面根布局 |
| `title_label` | `egui_view_label_t` | `224 x 18` | `Shortcut Recorder` | 页面标题 |
| `recorder_primary` | `egui_view_shortcut_recorder_t` | `194 x 138` | `Ctrl + K` | 标准主控件 |
| `recorder_compact` | `egui_view_shortcut_recorder_t` | `106 x 82` | `Ctrl + Shift + P` | 紧凑静态预览 |
| `recorder_read_only` | `egui_view_shortcut_recorder_t` | `106 x 82` | `Ctrl + S` | 只读静态预览 |

## 6. 状态覆盖矩阵

| 状态 / 区域 | 主控件 | Compact | Read only |
| --- | --- | --- | --- |
| 默认绑定 | 是 | 是 | 是 |
| listening | 是 | 否 | 否 |
| 捕获新绑定 | 是 | 否 | 否 |
| preset apply | 是 | 否 | 否 |
| clear | 是 | 否 | 否 |
| 静态对照 | 否 | 是 | 是 |

## 7. `egui_port_get_recording_action()` 录制动作设计

1. 应用默认主控件与 `compact` 预览状态
2. 请求第一页截图
3. 触摸主控件字段区域，进入监听态
4. 请求第二页截图
5. 通过 `Ctrl + Shift + P` 捕获新绑定
6. 请求第三页截图
7. 通过 `Tab + End + Enter` 切到最后一个 preset 并应用 `Ctrl + 1`
8. 请求第四页截图
9. 通过 `Tab + Enter` 清空绑定，同时把 `compact` 预览切到第二组静态对照
10. 请求最终截图并保留收尾等待

## 8. 编译、runtime、截图验收标准

```bash
make all APP=HelloCustomWidgets APP_SUB=input/shortcut_recorder PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/shortcut_recorder --track reference --timeout 10 --keep-screenshots
make all APP=HelloUnitTest PORT=pc_test
output\main.exe
python scripts/checks/check_docs_encoding.py
```

验收重点：

- 主控件字段、preset 行、clear 动作和底部双预览必须完整可见，不能被裁切
- `Ctrl` / `Shift` / `Key` token、监听状态 pill 和边框层级必须清晰可辨
- 主控件触摸与键盘切换必须都能工作，且不再依赖外部 guide / label 点击
- 页面中不再出现 guide、状态回显、section divider 和外部 preview 标签
- `compact` 与 `read only` 必须保持 Fluent / WPF UI 的低噪音浅色 reference

## 9. 已知限制与后续方向

- 当前只覆盖 `Ctrl` / `Shift` 两种修饰键，不扩展到 `Alt` / `Win`
- 当前不做系统级冲突检测、保留键校验和真实持久化写入
- 当前 preset 为固定数组，不接真实用户配置源
- 若后续要沉入框架层，再单独评估更完整的快捷键编辑器边界

## 10. 与现有控件的重叠分析与差异化边界

- 相比 `textinput`：本控件表达组合键捕获，不做自由文本编辑
- 相比 `token_input`：本控件只处理单个快捷键绑定，不做多 token 管理
- 相比 `command_bar`：本控件负责录入和保存快捷键，不负责触发命令
- 相比 `toggle_button` / `drop_down_button`：本控件是输入字段，不是单个命令触发器

## 11. 参考设计系统与开源母本

- 参考设计系统：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充参考：`ModernWpf`

## 12. 对应组件名，以及本次保留的核心状态

- 对应组件名：`Keyboard shortcut field / accelerator editor`
- 本次保留状态：
  - `standard`
  - `listening`
  - `preset apply`
  - `compact`
  - `read only`

## 13. 相比参考原型删除了哪些效果或装饰

- 不做页面级 guide、状态回显、section divider 和外部 preview 标签
- 不做系统级冲突提示、命令搜索联动和上下文帮助弹层
- 不做 `Alt` / `Win` / 多段 chord 组合录入
- 不做桌面级 hover、复杂焦点动画和全局快捷键注册

## 14. EGUI 适配时的简化点与约束

- 使用固定 preset 数组和固定录制脚本，优先保证示例稳定可复现
- 用轻量 token pill 和状态 pill 表达快捷键录入，不引入额外图像资源
- 底部 `compact` 与 `read only` 固定为静态对照，不再承担额外交互
- 先完成示例级审阅稳定性，再决定是否抽象到框架公共层
