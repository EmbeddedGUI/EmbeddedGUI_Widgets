# HelloCustomWidgets 设计方案

## 当前定位

- `HelloCustomWidgets` 已经收口为 `Reference` 主线控件库，默认对齐 `Fluent 2 / WPF UI`。
- 截至 `2026-04-13`，仓库保留 `90` 个控件目录，覆盖 `input`、`layout`、`navigation`、`display`、`feedback` 五个分类。
- 历史 `showcase` / `deprecated` 轨道以及 `chart`、`media`、`decoration` 等非主线分类已经清退，不再作为新增控件的目标目录。

## 控件边界

- 新控件默认放在 `example/HelloCustomWidgets/<category>/<widget>/`，先在仓库内完成 reference 收口，再决定是否下沉到框架层。
- 允许两类实现：
  - 对标准控件语义的 EGUI 适配，例如 `grid_view`、`scroll_viewer`、`teaching_tip`
  - 对现有基础控件的 reference 封装，例如 `text_block`、`button`、`progress_bar`
- 不继续扩展强行业化、强叙事化或纯装饰导向的示例控件。

## 目录结构

```text
example/HelloCustomWidgets/
├── build.mk
├── create_custom_widget.py
├── demo_scaffold.c
├── demo_scaffold.h
├── widget_catalog.json
├── input/
├── layout/
├── navigation/
├── display/
└── feedback/
```

单个控件目录的最小交付结构：

```text
{category}/{widget}/
├── egui_view_{widget}.h
├── egui_view_{widget}.c
├── test.c
├── readme.md
└── iteration_log/              # 本地验收资料，不纳入 git commit
```

按需补充：

- `resource/`
- `resource/img/`
- `resource/font/`

## 脚手架约束

推荐使用脚本创建新目录骨架：

```bash
python example/HelloCustomWidgets/create_custom_widget.py --category display --name weather_icon
```

脚手架输出应满足当前 workflow 的最低要求：

- 只允许当前五个分类
- 默认生成 `egui_view_<widget>.h/.c`
- 默认生成基于 `demo_scaffold.h` 的 `test.c`
- 默认生成 `readme.md`
- 默认创建本地 `iteration_log/iteration_log.md` 和 `iteration_log/images/`

## 参考分类

| 分类 | 说明 | 当前示例 |
| --- | --- | --- |
| `input` | 输入、编辑、选择和命令触发 | `auto_suggest_box`、`text_box`、`date_picker` |
| `layout` | 容器、滚动、排布和分栏 | `grid_view`、`relative_panel`、`scroll_viewer` |
| `navigation` | 导航、层级和页面切换 | `breadcrumb_bar`、`tab_view`、`tree_view` |
| `display` | 内容展示、图标和信息可视化 | `text_block`、`rich_text_block`、`animated_icon` |
| `feedback` | 进度、提示、弹层和通知 | `message_bar`、`progress_bar`、`teaching_tip` |

## 构建与验收入口

### 单控件脚手架验证

```bash
make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc
python scripts/checks/check_touch_release_semantics.py --scope custom --category input
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/auto_suggest_box --track reference --timeout 10 --keep-screenshots
```

### 分类回归

```bash
python scripts/code_compile_check.py --custom-widgets --category input --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --track reference --bits64
```

### Web / Catalog

```bash
python scripts/sync_widget_catalog.py
python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --track reference --refresh-existing
python scripts/web/web_smoke_check.py --web-root web --manifest web/demos/demos.json --name-filter auto_suggest_box
```

### 文档与编码

```bash
python scripts/checks/check_docs_encoding.py
```

## CI 约束

- `HelloCustomWidgets` 的分类级编译与 runtime 回归只覆盖当前五个 reference 分类。
- 默认网页目录只发布 `reference` 控件，不再把 `showcase` / `deprecated` 当作默认入口。
- `widget_catalog.json` 与 `web/catalog-policy.json` 需要保持同步。
- README、workflow、web 页面和脚本文件必须保持 UTF-8。

## 当前维护流程

1. 先读 `.claude/workflow/widget_acceptance_workflow.md`
2. 再读 `.claude/workflow/widget_progress_tracker.md`
3. 从 tracker 选择当前控件，避免并行开多个进行中项
4. 在控件目录下完成实现、README、`test.c` 和本地 `iteration_log`
5. 依次完成编译、触控语义检查、runtime、截图归档和文档更新
6. 更新 tracker，并为单个控件或单组清理动作单独提交 commit
