---
name: runtime-verification
description: Use when verifying HelloCustomWidgets code changes compile and render correctly, or when diagnosing rendering issues like black screens, missing widgets, or layout problems
---

# Runtime Verification Skill

Widget 源码、录制动作、资源或运行路径修改后，应执行运行验证。只通过编译不算完成；纯文档或 CI 文本改动可用对应轻量检查替代。

## 推荐流程

1. **单个 demo 编译**
   ```bash
   make all APP=HelloCustomWidgets APP_SUB={CATEGORY}/{WIDGET} PORT=pc
   ```

2. **单测检查**
   ```bash
   python scripts/code_compile_check.py --unit-tests-only --bits64
   ```

3. **单个 demo 运行时检查**
   ```bash
   python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub {CATEGORY}/{WIDGET} --timeout 10 --keep-screenshots
   ```
   要求最终输出 `ALL PASSED`。

4. **截图确认**
   检查 `runtime_check_output/HelloCustomWidgets_{CATEGORY}_{WIDGET}/default/frame_*.png`：
   - 不能黑屏或空白
   - 控件可见，布局合理
   - 文字和图标可读
   - 交互后的 pressed/selected/focus 状态没有残留污染

## 分类回归

改动影响共享 helper、分类公共模式、`uicode_disp0.*` 接线或多个 widget 时，运行分类回归：

```bash
python scripts/code_compile_check.py --custom-widgets --category {CATEGORY} --bits64
python scripts/code_runtime_check.py --app HelloCustomWidgets --category {CATEGORY} --bits64
```

`code_runtime_check.py` 会把 `APP_SUB` 里的 `/` 和 `\` 统一替换为 `_`，所以 `input/auto_suggest_box` 的截图目录是：

```
runtime_check_output/HelloCustomWidgets_input_auto_suggest_box/default/
```

## 全量或发布前检查

```bash
# 全量 HelloCustomWidgets runtime 验证
python scripts/code_runtime_check.py --full-check --bits64

# 发布前一键检查；跳过 wasm 时会自动跳过依赖 web 阶段
python scripts/release_check.py --skip wasm

# 单分类 native 收口
python scripts/release_check.py --category input --skip wasm
```

`release_check.py` 会串联 catalog、文档编码、web artifact、touch 语义、编译、单测和 runtime 检查。需要继续收集后续问题时，加 `--keep-going`。

## 多页面和录制动作

- `HelloCustomWidgets` / `HelloUnitTest` 的 app 根入口直接使用 `uicode_disp0.c` / `uicode_disp0.h`。
- 新增录制动作或排查 runtime 时，也按这套多屏入口接线。
- 如果录制动作里通过 `uicode_switch_page()` 切页，必须逐帧确认每页都有正确渲染。
- 录制动作应稳定复现同一状态，避免依赖真实时间随机分支。

## 手动多分辨率验证

本仓库默认不做多分辨率验证。只有用户明确要求时，才执行下面这套手动流程。

`code_runtime_check.py` 当前没有 `--multi-resolution` 参数。需要手动改分辨率编译并录制：

```bash
make all APP=HelloCustomWidgets APP_SUB={CATEGORY}/{WIDGET} PORT=pc USER_CFLAGS="-DEGUI_CONFIG_SCREEN_WIDTH=320 -DEGUI_CONFIG_SCREEN_HEIGHT=240"
output\main.exe output\app_egui_resource_merge.bin --record runtime_check_output/HelloCustomWidgets_{CATEGORY}_{WIDGET}/manual_320x240 2 30 --speed 1
```

可按同样方式测试 `240x320`、`320x320`、`480x320`。

## 失败处理

1. 编译失败：先修复编译错误，再重跑同一 demo。
2. 运行崩溃或超时：排查空指针、越界、死循环和录制动作坐标。
3. 渲染异常：排查布局参数、父子裁剪、资源生成、颜色和透明度设置。
4. 截图不稳定：固定动画起点、录制动作和 snapshot settle 参数。
5. 修复后，重复对应检查直到通过。

## 常见渲染异常对照

| 症状 | 常见原因 | 修复方向 |
|------|----------|----------|
| 黑屏 | `APP_SUB` 选择错误、init/渲染流程未触发、PFB 配置不合理 | 检查 demo 目录、`uicode_disp0.c` 接线和 `app_egui_config.h` |
| 图标空白 | 字符不在字体子集，或 alpha 图未设置渲染色 | 更新 `*_text.txt` / 字体资源，或设置 `egui_view_image_set_image_color()` |
| 文本乱码或方框 | 字符未进字体子集 | 更新字符集并重新生成资源 |
| 子控件不可见 | 父容器裁剪、尺寸不足或层级顺序错误 | 调整父容器尺寸、子控件位置和 Z/order |
| 动画不动 | 动画未启动、定时器未推进或录制时钟未覆盖 | 检查 `egui_animation_start()`、timer 和 runtime 参数 |
| 交互状态残留 | release 语义或 pressed 状态清理不完整 | 运行 touch release 语义检查并核对交互后截图 |

## 文件参考

| 文件 | 说明 |
|------|------|
| `scripts/code_runtime_check.py` | 运行时验证主脚本 |
| `scripts/code_compile_check.py` | 编译和单测检查入口 |
| `scripts/checks/check_touch_release_semantics.py` | touch release 语义审计 |
| `sdk/EmbeddedGUI/porting/pc/sdl_port.c` | SDK PC 端录制与截图输出机制 |
| `runtime_check_output/` | 本地运行截图输出目录 |
