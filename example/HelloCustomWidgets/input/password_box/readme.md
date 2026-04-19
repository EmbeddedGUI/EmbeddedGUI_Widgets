# password_box 自定义控件设计说明

## 参考来源
- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件：`PasswordBox`
- 当前保留形态：`Wi-Fi passphrase / masked`、`Wi-Fi passphrase / revealed`、`Deploy secret / masked`、`compact`、`read only`
- 当前保留交互：主区保留标准密码字段、reveal 切换、`Tab / Space / Enter / Escape` part 闭环与静态 preview 对照
- 当前移除内容：页面级 guide、状态说明、preview 清焦桥接、录制阶段 preview 状态轮换、额外键盘驱动轨道、showcase 化页面 chrome，以及旧版 finalize README 章节结构
- EGUI 适配说明：目录和 demo 继续使用 `input/password_box`，底层仍复用仓库内现有 `egui_view_password_box` 实现；本轮只收口 README、reference 录制说明、static preview 语义与验收记录，不修改 `sdk/EmbeddedGUI`

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
| 录制最终稳定帧 | 回到默认 `studio-24` 遮罩态 | 保持不变 | 保持不变 |
| 静态 preview 吞掉 `touch / key` 且不改状态 | 否 | 是 | 是 |

## 7. 交互语义与单测口径
`example/HelloUnitTest/test/test_password_box.c` 当前覆盖 `10` 条用例：

1. `Tab` 在有文本时循环到 reveal。
   覆盖 field 与 reveal part 的基础闭环。
2. 空文本或只读时 `Tab` 保持在 field。
   覆盖空值与只读状态下的导航 guard。
3. 键盘 reveal 切换。
   覆盖 `Space` 打开 reveal 与 `Escape` 关闭 reveal。
4. setter 清理 `pressed`。
   覆盖 `set_text()`、`clear()`、`set_current_part()`、`set_revealed()`、`set_palette()`、`set_compact_mode()`、`set_read_only_mode()` 对 `pressed_part / is_pressed` 的清理。
5. 插入与退格遵守光标位置。
   覆盖字段编辑、插入与 `Backspace`。
6. 前删与导航。
   覆盖 `Delete`、光标移动以及 reveal part 导航。
7. `touch` 触发 reveal。
   覆盖 reveal 部位 same-target release 切换。
8. read only 忽略变更。
   覆盖只读模式下的文本编辑拒绝与 reveal 禁止。
9. compact、read only 与 disabled guard 清理 `pressed`。
   覆盖三类 guard 下的 `touch / key` 输入拒绝与按压态回收。
10. static preview 吞掉输入且保持状态不变。
   固定校验 `region_screen / on_changed / font / meta_font / icon_font / label / helper / placeholder / surface_color / border_color / text_color / muted_text_color / accent_color / text / masked_text / text_len / cursor_pos / current_part / pressed_part / compact_mode / read_only_mode / revealed / cursor_visible / scroll_offset_x / alpha / enable` 不变，并要求 `is_pressed == false`、`changed_count == 0`、`changed_revealed == 0xFF`、`changed_part == EGUI_VIEW_PASSWORD_BOX_PART_NONE`、`changed_text == ""`。

说明：
- 主区真实交互继续保留标准密码字段、reveal part、文本编辑与键盘 `Tab / Space / Enter / Escape` 闭环。
- setter、模式切换、read only / disabled guard 和 static preview 都统一要求先清理残留 `pressed_part / is_pressed`，再处理后续状态。
- 底部 `compact / read only` preview 统一通过 `egui_view_password_box_override_static_preview_api()` 吞掉 `touch / key`，只承担静态 reference 对照，不改 `text / masked_text / current_part / revealed / compact_mode / read_only_mode / region_screen / palette`。

## 8. 录制动作设计
`egui_port_get_recording_action()` 已收口为静态 preview 工作流：

1. 应用主区默认快照和底部 preview 固定状态并抓取首帧，等待 `PASSWORD_BOX_RECORD_FRAME_WAIT`。
2. 切到 `Wi-Fi passphrase / revealed`，等待 `PASSWORD_BOX_RECORD_WAIT`。
3. 抓取第二组主区快照，等待 `PASSWORD_BOX_RECORD_FRAME_WAIT`。
4. 切到 `Deploy secret / masked`，等待 `PASSWORD_BOX_RECORD_WAIT`。
5. 抓取第三组主区快照，等待 `PASSWORD_BOX_RECORD_FRAME_WAIT`。
6. 回到默认 `Wi-Fi passphrase / masked`，等待 `PASSWORD_BOX_RECORD_FINAL_WAIT`。
7. 抓取最终稳定帧，等待 `PASSWORD_BOX_RECORD_FINAL_WAIT`。

说明：
- 录制阶段不再发送 `Backspace`、数字键、`Tab`、`Space` 等真实编辑输入，主区状态只通过 `apply_primary_snapshot()` 与 `apply_primary_default_state()` 切换。
- 录制阶段不再切换 `compact / read only` preview 到其他文本。
- 底部 preview 统一通过 `egui_view_password_box_override_static_preview_api()` 吞掉 `touch / key`。
- `request_page_snapshot()` 统一走 `layout_page() + invalidate + recording_request_snapshot()`，保证 3 组主区快照与最终稳定帧口径一致。
- README 这里按当前 `test.c` 如实保留 `PASSWORD_BOX_RECORD_WAIT / PASSWORD_BOX_RECORD_FRAME_WAIT / PASSWORD_BOX_RECORD_FINAL_WAIT` 三档等待口径。

## 9. 验收命令
```bash
make all APP=HelloCustomWidgets APP_SUB=input/password_box PORT=pc

# 在 X:\ 短路径下执行
make all APP=HelloUnitTest PORT=pc_test

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

## 10. 验收重点
- 主控件和底部 preview 必须完整可见，不能黑屏、白屏或裁切。
- 主区 `Wi-Fi passphrase / masked`、`Wi-Fi passphrase / revealed` 与 `Deploy secret / masked` 三组 reference 快照必须能从截图中稳定区分。
- 主区 field 与 reveal 部位切换、键盘闭环和状态清理链路收口后不能残留 `pressed` 污染。
- 底部 `compact / read only` preview 必须保持静态 reference，对输入只吞不改状态。
- WASM demo 必须能以 `HelloCustomWidgets_input_password_box` 正常加载。

## 11. 截图复核口径
- 检查目录：`runtime_check_output/HelloCustomWidgets_input_password_box/default`
- 本轮复核结果：
  - 共捕获 `9` 帧
  - 全帧共出现 `3` 组唯一状态，主区哈希分组为 `[0,1,6,7,8] / [2,3] / [4,5]`
  - 主区 RGB 差分边界收敛到 `(54, 160) - (420, 214)`
  - 遮罩主区变化边界后，主区外区域唯一哈希数为 `1`
  - 按 `y >= 215` 裁切底部 preview 后，preview 区唯一哈希数为 `1`

## 12. 与现有控件的边界
- 相比 `text_box`：这里强调秘密字段的遮罩与 reveal 语义，不是通用文本编辑。
- 相比 `token_input`：这里表达单值口令输入，不是多 token 管理。
- 相比 `auto_suggest_box`：这里不涉及建议列表或下拉面板。

## 13. 本轮保留与删减
- 保留的主区状态：
  - `Wi-Fi passphrase / masked`
  - `Wi-Fi passphrase / revealed`
  - `Deploy secret / masked`
- 保留的底部对照：
  - `compact`
  - `read only`
- 保留的交互：
  - 标准密码字段编辑
  - reveal 切换
  - 键盘 `Tab / Space / Enter / Escape`
  - setter / 模式切换状态清理
  - 静态 preview 对照
- 删减的旧桥接与旧装饰：
  - 页面级 guide 与复杂表单流
  - preview 清焦桥接
  - 录制阶段真实键盘编辑轨道
  - preview 文本轮换、validation、caps lock、密码强度与错误提示

## 14. 当前验收结果（2026-04-19）
- 单控件编译：`PASS`
  - `make all APP=HelloCustomWidgets APP_SUB=input/password_box PORT=pc`
- `HelloUnitTest`：`日志复核 PASS`
  - 在 `X:\` 短路径下执行 `make all APP=HelloUnitTest PORT=pc_test`
  - 本轮沿用已归档 unit 日志复核总计 `845 / 845`，其中 `password_box` suite `10 / 10`
- catalog / 文档 / 触摸语义：`PASS`
  - `python scripts/sync_widget_catalog.py`
  - `python scripts/checks/check_touch_release_semantics.py --scope custom --category input`
  - `python scripts/checks/check_docs_encoding.py`
  - `python scripts/checks/check_widget_catalog.py`
  - 触摸语义结果：`custom_audited=28 custom_skipped_allowlist=5`
  - 文档编码结果：`134 files`
  - widget catalog 结果：`106 widgets`
- 单控件 runtime：`PASS`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/password_box --track reference --timeout 10 --keep-screenshots`
  - 输出目录：`runtime_check_output/HelloCustomWidgets_input_password_box/default`
  - 共捕获 `9` 帧
- input 分类 compile/runtime 回归：`PASS`
  - `python scripts/code_compile_check.py --custom-widgets --category input --bits64`
  - `python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64`
  - input `33 / 33` 全部通过
- web 链路：`PASS`
  - `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --app-sub input/password_box`
  - `python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --demo HelloCustomWidgets_input_password_box`
  - smoke 结果：`status=Running canvas=480x480 ratio=0.1196 colors=71`
- 截图复核结论：
  - 主区覆盖默认 `Wi-Fi passphrase / masked`、`Wi-Fi passphrase / revealed` 与 `Deploy secret / masked` 三组 reference 快照
  - 最终稳定帧回到默认 `Wi-Fi passphrase / masked`
  - 主区 RGB 差分边界收敛到 `(54, 160) - (420, 214)`
  - 遮罩主区变化边界后主区外保持单哈希，底部 `compact / read only` preview 以 `y >= 215` 裁切后全程保持单哈希静态
