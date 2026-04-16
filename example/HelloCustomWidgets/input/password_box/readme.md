# password_box 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`PasswordBox`
- 当前保留形态：`masked`、`revealed`、`compact`、`read only`
- 当前保留交互：主区保留标准密码字段与 reveal 切换；底部 `compact / read only` 统一收口为静态 preview 对照
- 当前移除内容：页面级 guide、状态说明、preview 清焦桥接、录制阶段 preview 状态轮换、额外键盘驱动轨道和 showcase 化页面 chrome

## 1. 为什么需要这个控件
`password_box` 用于承载 Wi-Fi 密码、部署密钥、管理员口令这类单值秘密字段。它需要保留遮罩文本、reveal 切换和标准表单输入语义，不能退化成普通 `text_box`，也不能被多值输入类控件替代。

## 2. 为什么现有控件不够用
- `text_box` 是通用文本输入，不具备密码遮罩和 reveal 入口。
- `token_input` 面向多值编辑，不适合单条秘密字段。
- `auto_suggest_box` 更偏建议输入，不承担安全字段语义。
- 当前主线仍需要一版与 `Fluent 2 / WPF UI PasswordBox` 语义对齐的 `PasswordBox` reference。

## 3. 当前页面结构
- 标题：`Password Box`
- 主区：一个可切换 `masked / revealed` 的主 `password_box`
- 底部：一行并排的两个静态 preview
- 左侧 preview：`compact`，固定显示 `7429`
- 右侧 preview：`read only`，固定显示 `fleet-admin`

目录：
- `example/HelloCustomWidgets/input/password_box/`

## 4. 主区 reference 快照
主区录制轨道只保留 3 组程序化快照，不再在录制阶段对 preview 发送输入，也不再依赖 preview 帮主区清焦：

1. `Wi-Fi passphrase`
   文本：`studio-24`
   状态：`masked`
2. `Wi-Fi passphrase`
   文本：`studio-24`
   状态：`revealed`
3. `Deploy secret`
   文本：`release-key-7`
   状态：`masked`

底部 preview 在整条轨道中始终保持不变：

1. `compact`
   placeholder：`Quick PIN`
   文本：`7429`
2. `read only`
   placeholder：`Read only`
   文本：`fleet-admin`

## 5. 视觉与布局规格
- 画布：`480 x 480`
- 根布局：`224 x 154`
- 主控件：`196 x 70`
- 底部 preview 行：`216 x 44`
- 单个 preview：`106 x 44`
- 页面结构：标题 -> 主 `password_box` -> 底部 `compact / read only`
- 风格约束：浅灰 page panel、白色主表面、低噪音边框、清晰的遮罩文本与 reveal 图标层级，不回退到页面级状态卡片

## 6. 状态矩阵
| 状态 | 主控件 | Compact preview | Read only preview |
| --- | --- | --- | --- |
| 默认显示 | `studio-24` 遮罩态 | `7429` | `fleet-admin` |
| 快照 2 | `studio-24` 明文态 | 保持不变 | 保持不变 |
| 快照 3 | `release-key-7` 遮罩态 | 保持不变 | 保持不变 |
| 录制最终稳定帧 | `release-key-7` 遮罩态 | 保持不变 | 保持不变 |
| 静态 preview 对照 | 否 | 是 | 是 |

## 7. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态
2. 抓取首帧
3. 切到 `Wi-Fi passphrase / revealed`
4. 抓取第二组主区快照
5. 切到 `Deploy secret / masked`
6. 抓取第三组主区快照
7. 等待并抓取最终稳定帧

说明：
- 录制阶段不再发送 `Backspace`、数字键、`Tab`、`Space`
- 录制阶段不再切换 `compact` preview 到其他文本
- 底部 preview 统一通过 `egui_view_password_box_override_static_preview_api()` 吞掉 `touch / key`
- preview 只负责静态 reference 对照，不再承担清焦或页面状态桥接职责

## 8. 单元测试口径
`example/HelloUnitTest/test/test_password_box.c` 当前覆盖两部分：

1. 主控件交互与状态清理
   覆盖 `Tab` 闭环、`Space / Enter` reveal 切换、same-target release、只读守卫，以及 `compact / read only / disabled` 分支的 `pressed` 清理。
2. 静态 preview 不变性断言
   通过 `password_box_preview_snapshot_t`、`capture_preview_snapshot()`、`assert_preview_state_unchanged()` 固定校验以下字段：
   `region_screen`、`on_changed`、`font`、`meta_font`、`icon_font`、`label`、`helper`、`placeholder`、`surface_color`、`border_color`、`text_color`、`muted_text_color`、`accent_color`、`text`、`masked_text`、`text_len`、`cursor_pos`、`current_part`、`pressed_part`、`compact_mode`、`read_only_mode`、`revealed`、`cursor_visible`、`scroll_offset_x`、`alpha`、`enable`

同时要求：
- `is_pressed == false`
- `changed_count == 0`
- `changed_revealed == 0xFF`
- `changed_part == EGUI_VIEW_PASSWORD_BOX_PART_NONE`
- `changed_text == ""`

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/password_box PORT=pc

make clean APP=HelloUnitTest PORT=pc_test
make all APP=HelloUnitTest PORT=pc_test
X:\output\main.exe

python scripts/sync_widget_catalog.py
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/checks/check_docs_encoding.py
python scripts/checks/check_widget_catalog.py
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/password_box --track reference --timeout 10 --keep-screenshots
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/password_box
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_password_box
```

## 10. 当前结果
- `HelloCustomWidgets` 单控件编译：PASS
- `HelloUnitTest`：`845 / 845` 通过，其中 `password_box` suite `10 / 10`
- `sync_widget_catalog.py`：PASS（`106` entries）
- `touch release semantics`：PASS（`custom_audited=28`，`custom_skipped_allowlist=5`）
- `docs encoding`：PASS（`134 files`）
- `widget catalog check`：PASS（`106 widgets: reference=106, showcase=0, deprecated=0`）
- 单控件 runtime：PASS（`8` frames captured）
- input 分类 compile/runtime 回归：PASS（`33 / 33`）
- wasm 构建：PASS
- web smoke：`PASS status=Running canvas=480x480 ratio=0.1196 colors=71`

## 11. Runtime 复核结论
复核目录：
- `runtime_check_output/HelloCustomWidgets_input_password_box/default`

复核结果：
- 总帧数：`8`
- 主区 RGB 差分边界：`(54, 160) - (420, 214)`
- 遮罩主区差分边界后，主区外唯一哈希数：`1`
- 按主区差分边界裁切后，主区唯一状态数：`3`
- 按 `y >= 275` 裁切底部 preview 区域后，preview 区唯一哈希数：`1`

目标：
- 主区唯一状态数 = `3`
- 主区外唯一哈希数 = `1`
- 底部 preview 区唯一哈希数 = `1`

## 12. 已知限制
- 当前只覆盖标准密码字段 reference，不扩展 validation、caps lock、密码强度或错误提示。
- 底部 preview 只作为静态 reference，对外不承担交互职责。
- 页面不承载复杂表单流，只验证控件级视觉与交互闭环。

## 13. 与现有控件的边界
- 相比 `text_box`：这里强调秘密字段的遮罩与 reveal 语义，不是通用文本编辑。
- 相比 `token_input`：这里表达单值口令输入，不是多 token 管理。
- 相比 `auto_suggest_box`：这里不涉及建议列表或下拉面板。

## 14. EGUI 适配说明
- 继续复用仓库内已有 `password_box` 实现，不修改 SDK。
- 主区保留标准 `field / reveal` 部位切换与键盘闭环。
- 底部 preview 通过 `egui_view_password_box_override_static_preview_api()` 明确收口为静态 reference。
- 当前优先保证主区 3 组 reference 快照、底部 preview 全程静态，以及 runtime 录制无污染，再评估是否继续上升到框架层公共控件。
