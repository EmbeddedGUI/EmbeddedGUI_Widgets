---
name: build-and-debug
description: Use when building EmbeddedGUI Widgets demos, diagnosing compile errors, or debugging runtime issues on the PC simulator
---

# Build & Debug Skill

EmbeddedGUI Widgets 仓库的构建、检查和常见问题排查指南。

## 构建模型

本仓库不是 SDK 根目录。仓库根 `Makefile` 是一层转发器，实际构建逻辑来自 `sdk/EmbeddedGUI/Makefile`：

```
Makefile                         ← widgets 仓库入口，转发到 SDK
├── sdk/EmbeddedGUI/              ← 固定版本 SDK 子模块，默认不要修改
├── example/HelloCustomWidgets/   ← 按 category/widget 组织的 widget demo
├── example/HelloUnitTest/        ← 自定义 widget 单测入口
├── scripts/                      ← widgets 仓库检查、运行、发布辅助脚本
└── output/                       ← 本仓库本地构建输出
```

构建时会把本仓库的 `example/` 作为应用根传给 SDK，并把输出写回本仓库的 `output/`。

## Make 命令速查

```bash
# 构建一个 widget demo
make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc

# 运行一个 widget demo
make run APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc

# 构建 HelloUnitTest 测试程序
make all APP=HelloUnitTest PORT=pc_test

# 清理本地输出
make clean

# 生成资源（有 resource/ 目录时 make all 会自动触发）
make resource APP=HelloCustomWidgets APP_SUB=display/image_icon PORT=pc

# 强制重新生成资源（忽略缓存）
make resource_refresh APP=HelloCustomWidgets APP_SUB=display/image_icon PORT=pc
```

### 关键参数

| 参数 | 说明 | 示例 |
|------|------|------|
| `APP` | 应用名 | `HelloCustomWidgets`, `HelloUnitTest` |
| `APP_SUB` | widget 子目录 | `input/button`, `layout/grid` |
| `PORT` | 构建端口 | `pc`, `pc_test` |
| `BITS` | 位宽（可选） | `64` |
| `COMPILE_OPT_LEVEL` | 优化级别 | `-O0`, `-O2` |
| `USER_CFLAGS` | 自定义编译标志 | `-DEGUI_CONFIG_SCREEN_WIDTH=320` |

## 编译检查

```bash
# 检查一个 widget 分类，先做 touch release 语义审计，再批量编译
python scripts/code_compile_check.py --custom-widgets --category input --bits64

# 只构建并运行 HelloUnitTest
python scripts/code_compile_check.py --unit-tests-only --bits64

# 编译所有 reference widget demo 并运行单测
python scripts/code_compile_check.py --full-check --bits64
```

特点：

- 默认使用快速编译标志（`-O0`，无调试符号）。
- widget demo 通过 catalog 选择，默认跟随当前仓库策略检查 reference track。
- `--custom-widgets` 会先调用 `scripts/checks/check_touch_release_semantics.py --scope custom`。
- `--unit-tests-only` 使用 `HelloUnitTest` 的 `pc_test` 端口构建并运行测试程序。

## 运行时验证

```bash
# 验证一个 widget demo，并保留截图
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/auto_suggest_box --timeout 10 --keep-screenshots

# 验证一个分类
python scripts/code_runtime_check.py --app HelloCustomWidgets --category input --bits64
```

截图输出位于 `runtime_check_output/`。若要人工核对关键帧，优先查看对应 demo 目录下的 `frame_*.png`。

## 发布前一键检查

```bash
# 完整发布前检查
python scripts/release_check.py

# 跳过 WASM 时会自动跳过依赖的 web smoke、render gallery 和 generated web artifact 检查
python scripts/release_check.py --skip wasm

# 只检查一个分类的 native 流程
python scripts/release_check.py --category input --skip wasm

# 失败后继续跑后续步骤，便于一次性收集问题
python scripts/release_check.py --keep-going
```

`release_check.py` 串联 catalog、文档编码、已提交 web artifact、touch 语义、编译、单测、运行时验证、WASM 构建、浏览器 smoke、render gallery 和生成物一致性检查。

## 常见编译错误诊断

| 错误信息 | 原因 | 修复 |
|----------|------|------|
| `undefined reference to 'egui_res_image_xxx'` | 资源未生成或配置不匹配 | 运行 `make resource_refresh ...`，检查资源配置和生成头文件 |
| `undefined reference to 'egui_res_font_xxx'` | 字体资源缺失 | 检查字体配置，确认字体文件或生成的 C 文件存在 |
| `conflicting types for 'xxx'` | 头文件声明与实现不一致，或旧 obj 残留 | `make clean` 后重新编译 |
| `SDL2.dll not found` | SDL2 库缺失或运行目录不对 | 先重新 `make all ...`，再通过 `make run ...` 或 `scripts/run_app.py` 启动 |
| `implicit declaration of function` | 缺少 `#include` 或函数签名变更 | 添加正确头文件引用，避免隐式声明 |
| `multiple definition of 'xxx'` | 全局变量或函数在头文件定义而非声明 | 头文件用 `extern` 声明，在 `.c` 文件中定义 |
| `No rule to make target` | `build.mk` 或资源生成产物路径错误 | 检查 `EGUI_CODE_SRC`、资源文件名和 `APP_SUB` |

## 构建系统要点

- `HelloCustomWidgets` 和 `HelloUnitTest` 的 app 根入口直接使用 `uicode_disp0.c` / `uicode_disp0.h`。
- 切换 widget demo 时通常不需要手动清理；检查脚本会使用独立输出目录和 obj 后缀隔离批量构建。
- 新增 runtime recording hook 时也按 `uicode_disp0.h` 这套多屏入口接线。
- `iteration_log/` 只用于本地验收记录，不要提交。

## 调试策略

### PC 模拟器调试

```bash
# 编译带调试符号的单个 widget demo
make all APP=HelloCustomWidgets APP_SUB=input/auto_suggest_box PORT=pc COMPILE_DEBUG=-g COMPILE_OPT_LEVEL=-O0

# GDB 调试
gdb output/main.exe
```

### 日志调试

框架提供 `EGUI_LOG_INF/WRN/ERR` 宏（定义在 SDK 的 `egui_log.h`）：

```c
#include "egui_log.h"
EGUI_LOG_INF("value: %d\n", some_value);
```

### 常见运行时问题

| 症状 | 可能原因 | 排查方向 |
|------|----------|----------|
| 启动即崩溃 | 空指针、未初始化控件、事件回调状态无效 | 检查 init 调用顺序和静态状态 |
| 黑屏无渲染 | `APP_SUB` 选择错误、draw 函数未注册、PFB 配置错误 | 确认 demo 目录、`uicode_disp0.c` 接线和 `app_egui_config.h` |
| 触控无响应 | 事件未注册、控件不可见、release 语义不符合仓库约定 | 运行 touch release 语义检查，并核对 test recording |
| 截图不稳定 | 录制动作或动画状态未固定 | 固定 recording 动作、避免依赖真实时间随机分支 |

## 文件参考

| 文件 | 说明 |
|------|------|
| `Makefile` | widgets 仓库构建转发入口 |
| `sdk/EmbeddedGUI/Makefile` | SDK 实际构建入口 |
| `example/HelloCustomWidgets/` | widget demo 源码、资源和文档 |
| `example/HelloUnitTest/` | custom widget 单测 harness |
| `scripts/code_compile_check.py` | 批量编译检查和单测入口 |
| `scripts/code_runtime_check.py` | 运行时截图验证入口 |
| `scripts/checks/check_touch_release_semantics.py` | widget touch release 语义检查 |
| `scripts/checks/check_docs_encoding.py` | 文档 UTF-8 与明显乱码检查 |
| `scripts/checks/check_web_artifacts.py` | web demo 与 render gallery artifact 一致性检查 |
| `scripts/release_check.py` | 发布前多步骤一键检查 |
