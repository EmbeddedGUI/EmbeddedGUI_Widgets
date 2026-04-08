# Scene Crossfader

## 定位

`scene_crossfader` 是专业演出/音频场景里的历史 demo，用来展示双源混合、场景切换和跨通道平衡。它与 `Fluent 2 / WPF UI` 的通用输入体系距离较远，因此已经被归入清退轨道。

## 当前示例内容

- 组件内置 `Aurora Mix`、`Drift Swap`、`Pulse Hold`、`Night Cut` 四组状态。
- 每组状态都包含模式名、场景说明、`A/B` 比例、左右源标签和当前混合位置。
- 示例更偏一次性完整演示，而不是可复用的主线控件样式。

## 现状判断

- 轨道：`deprecated`
- 默认网页构建不会继续发布该控件。
- 如果未来仍有场景混合需求，应重新评估是否值得保留为领域专用组件，而不是延续这份旧示例。

## 使用建议

- 仅用于保留历史行业 demo、回看旧交互或验证自定义绘制。
- 不建议继续投入主线维护成本。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=input/scene_crossfader PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub input/scene_crossfader --timeout 10 --keep-screenshots
```
