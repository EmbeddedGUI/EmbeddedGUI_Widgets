# Widget Acceptance Workflow

本工作流适用于 `HelloCustomWidgets` 下所有仍要继续维护的自定义控件：

- `example/HelloCustomWidgets/<category>/<widget>/`

当前仓库已经收口到 `Fluent 2 / WPF UI` 主线。后续无论是新增控件、重构旧控件，还是继续验收现有控件，都必须先满足下面两条前置约束：

1. 优先对齐 `Fluent 2 / WPF UI` 的标准组件语义、状态和视觉语言。
2. 如果某个控件无法解释为主线标准控件、且没有清晰差异化价值，就不要保留，直接删除。

## 固定规则

- 单控件推进：同一时刻只允许一个控件处于进行中状态。
- 单控件闭环：开始某个控件后，默认连续执行到验收、更新 tracker、单独提交 commit 为止。
- 单控件提交：一个控件一次 commit，不混入下一个控件的改动。
- 30 轮迭代门槛：每个控件至少记录 30 轮有效迭代，且最新一轮必须是 `PASS`。
- 视觉统一优先：任何高噪音、强行业化、强叙事化、强装饰化表达，都不能压过 Fluent 主线语义。
- 文档统一 UTF-8：README、计划、workflow、web 文案都必须保持 UTF-8，不允许乱码留仓。
- `iteration_log/` 仅作本地审阅证据，不纳入 git commit。

## Step -1：先读 tracker

开始新一轮工作前，必须先读取并同步：

- `.claude/workflow/widget_progress_tracker.md`

规则：

- 如果 “当前进行中” 不为空，优先继续当前控件，不切题。
- 只有当前控件完成验收，或明确移入“已搁置 / 待恢复”，才能切换到下一个控件。
- 一旦选定控件，立即把控件名、分类、开始日期和目标写入 tracker。

## Step 0：选择控件

选择原则：

- 只从当前保留的 `reference` 主线中继续维护，或新增明确属于 `Fluent 2 / WPF UI` 的标准控件。
- 优先处理仍不统一、视觉噪音偏高、状态表达不标准、文档不完整或实现重复的控件。
- 对于无法继续证明主线价值的控件，优先删除，而不是继续补丁式保留。
- 控件命名统一使用小写下划线，如 `auto_suggest_box`、`teaching_tip`。
- 分类目录统一使用语义化英文，如 `input`、`layout`、`navigation`、`display`、`feedback`。

## Step 1：补齐设计与文档

目标目录：

- `example/HelloCustomWidgets/<category>/<widget>/`

至少需要：

- `readme.md`
- `iteration_log/iteration_log.md`
- `iteration_log/images/`

要求：

- `readme.md` 必须是 UTF-8 中文。
- `readme.md` 与 `test.c`、实现文件保持同步。
- `iteration_log/iteration_log.md` 必须真实记录每轮迭代，不允许只口头说明。

`readme.md` 至少包含：

1. 为什么需要这个控件。
2. 为什么现有控件不够用。
3. 目标场景与示例概览。
4. 视觉与布局规格。
5. 控件清单与状态矩阵。
6. 录制动作设计。
7. 编译 / runtime / 截图验收标准。
8. 参考设计体系与开源母本。
9. 对应的 Fluent / WPF UI 组件名。
10. 保留的核心状态与删掉的装饰效果。
11. EGUI 适配时的简化点与限制。

## Step 2：实现控件

在控件目录下实现：

- `egui_view_<widget>.h`
- `egui_view_<widget>.c`
- `test.c`

按需增加：

- `resource/`
- `resource/img/`
- `resource/font/`

说明：

- 默认先做 `HelloCustomWidgets` 版本，不直接下沉到 `src/widget/`。
- 只有控件在当前仓库里稳定、且确实有复用价值时，才再讨论是否升级为框架层控件。

## Step 3：编译验证

先执行：

```bash
make all APP=HelloCustomWidgets APP_SUB=<category>/<widget> PORT=pc
```

要求：

- 编译必须通过。
- 若控件依赖资源，资源路径必须与子目录保持一致。
- 编译失败时，先修复编译，再进入 runtime。

## Step 3.5：触控释放语义审计

执行：

```bash
python scripts/checks/check_touch_release_semantics.py --scope custom --category <category>
```

要求：

- 非拖拽型控件不能在 `ACTION_MOVE` 改写点击目标。
- 非拖拽型控件只能在 `DOWN` 命中的同一目标上 `UP` 提交。
- 至少覆盖以下语义：
  - `DOWN(A) -> MOVE(B) -> UP(B)` 不提交
  - `DOWN(A) -> MOVE(B) -> MOVE(A) -> UP(A)` 才提交
- 如果控件本身就是拖拽 / 连续交互例外，必须在检查脚本 allowlist 中登记，并在提交说明里写明原因。

## Step 4：Runtime 验证

执行：

```bash
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub <category>/<widget> --track reference --timeout 10 --keep-screenshots
```

要求：

- 不能黑屏、白屏或主体缺失。
- 控件主体必须完整可见，不能被裁切。
- 关键文本、图标、边框、选中态、hover / pressed / disabled / focused 等状态必须可辨认。
- 视觉要符合 Fluent 主线：浅色、低噪音、标准层级、合理留白。
- 若日志出现 `[RUNTIME_CHECK_FAIL]`，必须先修复再重跑。

## Step 4.5：归档 iteration_log

runtime 通过后，立即把关键截图复制到：

- `example/HelloCustomWidgets/<category>/<widget>/iteration_log/images/iter_xx/`

并更新：

- `example/HelloCustomWidgets/<category>/<widget>/iteration_log/iteration_log.md`

要求：

- 每轮至少归档 1 张关键截图。
- 交互型控件建议归档 2 到 3 张，覆盖初始态、交互态、结果态。
- `iteration_log.md` 必须记录目标、代码改动、编译结果、runtime 结果、视觉结论、交互结论和最终判定。

## Step 5：记录 30 轮迭代

每轮迭代都必须完整执行以下闭环：

1. 改代码。
2. 编译。
3. 跑触控语义检查。
4. 跑 runtime。
5. 看截图，确认视觉与交互是否还符合 Fluent / WPF UI 主线。
6. 复制关键截图到 `iteration_log/images/iter_xx/`。
7. 更新 `iteration_log/iteration_log.md`。

如果同一轮修改影响了同分类多个控件，单控件通过后，再补一次分类级 runtime：

```bash
python scripts/code_runtime_check.py --app HelloCustomWidgets --category <category> --track reference --bits64
```

## Step 6：验收收口

控件可收口前，必须同时满足：

- `readme.md` 完整且与实现一致。
- `iteration_log/iteration_log.md` 已记录至少 30 轮迭代。
- `make all APP=HelloCustomWidgets APP_SUB=<category>/<widget> PORT=pc` 通过。
- `check_touch_release_semantics.py` 通过。
- `code_runtime_check.py` 通过。
- 关键截图已归档到 `iteration_log/`。
- 该控件仍符合 `Fluent 2 / WPF UI` 主线，不属于应删除的非主线控件。

## Step 7：更新 tracker 并提交

完成后立即更新：

- `.claude/workflow/widget_progress_tracker.md`

更新规则：

- 从“当前进行中”移除该控件。
- 在“Reference 主线控件清单”中更新状态、备注或完成日期。
- 如果控件被判定不再保留，也要在 tracker 中记录删除原因，而不是静默消失。

随后执行：

- 如有需要，先运行 `python scripts/code_format.py`
- 单独创建一次 commit
- 提交内容只围绕这一个控件或这一组明确的清理动作

提交信息示例：

```bash
git commit -m "refactor: polish auto_suggest_box reference widget"
git commit -m "refactor: remove non-mainline custom widget"
```

## 当前最小交付物

每个仍在维护的控件至少应包含：

- `example/HelloCustomWidgets/<category>/<widget>/readme.md`
- `example/HelloCustomWidgets/<category>/<widget>/egui_view_<widget>.h`
- `example/HelloCustomWidgets/<category>/<widget>/egui_view_<widget>.c`
- `example/HelloCustomWidgets/<category>/<widget>/test.c`

本地审阅证据：

- `example/HelloCustomWidgets/<category>/<widget>/iteration_log/iteration_log.md`
- `example/HelloCustomWidgets/<category>/<widget>/iteration_log/images/`
