# HelloCustomWidgets Plan

> 当前版本改为“参考体系优先”的长期方案说明。`HelloCustomWidgets` 仍然保留现有 showcase 控件，但后续新增控件默认不再走强设定原创路线。

## 当前状态

- `HelloCustomWidgets` 继续使用 `category/widget_name` 的两级 `APP_SUB` 结构
- 截至 `2026-04-08`，仓库中保留 `42` 个控件目录，覆盖 `display`、`feedback`、`input`、`layout`、`navigation` 五个分类
- 当前保留集只剩 `reference=42` 一条主线，后续默认围绕 `Fluent 2 / WPF UI` 标准控件继续维护
- `2026-04-08` 已清退全部 `deprecated` 与 `showcase` 控件目录，不再保留历史演示轨道

## 当前基线

当前基线以 `.claude/workflow/widget_progress_tracker.md` 为准；本文件保留阶段性策略、参考体系和维护约束。

### 主参考体系

- 设计规范：`Fluent 2`
- 开源母本：`WPF UI`
- 次级补充：`ModernWpf`
- 缺口补充：`MahApps.Metro`

### 基本原则

- 新控件先找开源原型，再做 EGUI 适配
- 优先做通用控件，不优先做行业控件
- 优先做未来可能沉入 `src/widget/` 的控件
- 不再继续扩展 demo 风格的自创视觉零件

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

每个新控件的 `readme.md` 顶部必须增加：

- 参考设计系统
- 参考开源库
- 对应组件名
- 保留状态
- 删除效果
- EGUI 简化说明

### 2. 历史白名单（已完成基线）

以下白名单用于说明当时的首批 Reference Track 基线来源，当前条目均已落地；后续选题应以 `.claude/workflow/widget_progress_tracker.md` 中尚未落地的项和最新 acceptance 记录为准：

- `feedback/message_bar`
- `navigation/breadcrumb_bar`
- `feedback/skeleton_loader`
- `navigation/tab_strip`
- `input/number_box`
- `navigation/nav_panel`
- `feedback/toast_stack`
- `display/card_panel`

### 3. 已清退的非主线控件

下面这批控件已从仓库中删除，不再作为后续风格母本，也不再参与默认构建与维护：

- `chart/*`
- `media/*`
- 全部 `showcase` 轨道控件，例如 `pin_cluster`、`status_timeline`、`xy_pad`、`window_snap_grid`、`command_palette`

## 构建入口

- Make 默认入口切换为标准 `reference` 控件，例如：`input/auto_suggest_box`
- CMake 仍支持两级 `APP_SUB`
- 示例：`input/auto_suggest_box`、`feedback/message_bar`、`navigation/tree_view`

## 推荐验证命令

### 构建 / 运行

```bash
make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc
cmake -B build_cmake/HelloCustomWidgets_input_auto_suggest_box -DAPP=HelloCustomWidgets -DAPP_SUB=input/auto_suggest_box -DPORT=pc -G "MinGW Makefiles"
cmake --build build_cmake/HelloCustomWidgets_input_auto_suggest_box -j
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/auto_suggest_box --timeout 10 --keep-screenshots
```

### 编译检查

```bash
python scripts/code_compile_check.py --custom-widgets
```

### Web

```bash
python scripts/web/wasm_build_demos.py
```

## 当前开发流程

1. 先读 `.claude/workflow/widget_acceptance_workflow.md`
2. 再读 `.claude/workflow/widget_progress_tracker.md`
3. 再回看本文件中的“主参考体系”和“存量控件约束”两节
4. 优先从 `.claude/workflow/widget_progress_tracker.md` 中尚未落地的项里选下一个控件
5. 在 `example/HelloCustomWidgets/<category>/<widget>/` 下完成实现、readme、iteration_log 和 runtime 验证
6. 完成后更新 tracker，并单独提交一个 commit

## 维护说明

- `HelloCustomWidgets` 继续承担“样例池 + 探索池”职责
- 当前仓库只保留 **Showcase** 与 **Reference** 两条线，不再混入额外清退轨道目录
- 如果某个新控件未来要沉入框架层，必须优先来自 `Reference Track`
