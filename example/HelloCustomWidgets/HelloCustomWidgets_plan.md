# HelloCustomWidgets Plan

> 当前版本已经收口为“Reference 主线优先”的长期维护方案。仓库不再保留 `showcase` / `deprecated` 控件目录，后续新增或重构控件默认都要先对齐 `Fluent 2 / WPF UI`。

## 当前状态

- `HelloCustomWidgets` 继续使用 `category/widget_name` 的两级 `APP_SUB` 结构。
- 截至 `2026-04-21`，`widget_catalog.json` 收录 `106` 个控件条目，覆盖 `input`、`layout`、`navigation`、`display`、`feedback` 五个分类。
- 当前 catalog 保持 `reference=106`、`showcase=0`、`deprecated=0`，默认构建、默认网页目录和后续维护都以这一条主线为准。
- `showcase` 与 `deprecated` 轨道已经清退完成，不再保留历史演示目录。

## 主参考体系

- 设计规范：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充：`ModernWpf`
- 缺口补充：`MahApps.Metro`

## 基本原则

- 新控件先找到稳定参考原型，再做 EGUI 适配。
- 优先维护通用控件，不继续扩展强行业化、强叙事化或强装饰化控件。
- 能被标准 Fluent / WPF UI 语义覆盖的旧控件，不再保留平行实现。
- 所有文档、README、网页源码和策略文件统一使用 UTF-8，避免乱码和历史残留描述。

## 目录结构

```text
example/HelloCustomWidgets/<category>/<widget>/
```

每个控件目录通常包含：

- `egui_view_<widget>.h`
- `egui_view_<widget>.c`
- `test.c`
- `readme.md`
- `iteration_log/`

## 新增控件的额外要求

除既有 workflow 外，新增以下约束：

### 1. 必须有参考来源

每个新控件的 `readme.md` 顶部必须明确写出：

- 参考设计体系
- 参考开源库
- 对应组件名
- 当前保留状态
- 相比参考原型删掉了哪些效果
- EGUI 适配时的简化说明

### 2. 历史首批 reference 基线

以下名单用于说明当前 reference 主线最早的一批稳定样本；后续选题和验收以 `.claude/workflow/widget_progress_tracker.md` 为准：

- `feedback/message_bar`
- `navigation/breadcrumb_bar`
- `feedback/skeleton`
- `navigation/tab_strip`
- `input/number_box`
- `navigation/nav_panel`
- `feedback/toast_stack`
- `display/card_panel`

### 3. 已清退的非主线控件

下面这批控件已经从仓库中删除，不再作为后续风格母本，也不再参与默认构建与维护：

- `chart/*`
- `media/*`
- 全部 `showcase` 轨道控件，例如 `pin_cluster`、`status_timeline`、`xy_pad`、`window_snap_grid`、`command_palette`

## 构建入口

- Make 默认入口切换为标准 `reference` 控件，例如：`input/auto_suggest_box`
- CMake 仍然支持两级 `APP_SUB`
- App 根目录直接使用 `uicode_disp0.c` / `uicode_disp0.h` 作为多屏入口；后续脚手架、文档和 Web 说明都应基于这套结构。
- 推荐示例：`input/auto_suggest_box`、`feedback/message_bar`、`navigation/tree_view`

## 推荐验证命令

### 构建 / 运行

```bash
make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc
cmake -B build_cmake/HelloCustomWidgets_input_auto_suggest_box -DAPP=HelloCustomWidgets -DAPP_SUB=input/auto_suggest_box -DPORT=pc -G "MinGW Makefiles"
cmake --build build_cmake/HelloCustomWidgets_input_auto_suggest_box -j
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/auto_suggest_box --track reference --timeout 10 --keep-screenshots
```

### 编译检查

```bash
python scripts/code_compile_check.py --custom-widgets --track reference
```

### Web

```bash
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --track reference
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --track reference --refresh-existing
python scripts/sync_widget_catalog.py
```

## 当前开发流程

1. 先读 `.claude/workflow/widget_acceptance_workflow.md`
2. 再读 `.claude/workflow/widget_progress_tracker.md`
3. 再回看本文中的“主参考体系”和“已清退的非主线控件”两节
4. 优先从 tracker 中选择仍符合 `Fluent 2 / WPF UI` 主线的控件
5. 在 `example/HelloCustomWidgets/<category>/<widget>/` 下完成实现、README、iteration_log 和 runtime 验证
6. 完成后更新 tracker，并单独提交一个 commit

## 维护说明

- `HelloCustomWidgets` 现在只承担 Reference 样例池与主线控件收口职责。
- 当前仓库只保留 `Reference` 一条线，不再保留 `Showcase` / `Deprecated` 目录。
- 如果某个控件未来要下沉到框架层，也必须优先来自 `Reference` 主线。
