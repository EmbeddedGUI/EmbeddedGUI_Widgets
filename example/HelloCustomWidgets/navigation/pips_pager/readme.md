# Pips Pager

## 参考来源

- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件语义：`PipsPager`

## 定位

`pips_pager` 是当前主线 reference 导航控件，用于离散页码切换、短轨道预览和只读分页展示。它把分页标题、辅助说明、当前页状态和紧凑态统一到同一套浅色 Fluent 风格中。

## 当前示例内容

- 主卡内置 `Onboarding`、`Gallery`、`Report deck` 三组分页快照，分别验证不同总页数和可见 pips 数量。
- `Compact` 预览提供两组缩略分页轨道，用来检查小尺寸下当前页的可辨识度。
- `Read only` 预览用于验证禁用态下的页码信息仍能清楚表达。
- 引导标签和紧凑标签都可触发快照切换，录制脚本还会模拟主卡页码变化。

## 现状判断

- 轨道：`reference`
- 这是分页导航的主线基线，后续如需离散页码轨道、引导步骤分页或轻量内容浏览，都应优先向它靠齐。
- 视觉方向已经统一到浅色卡面、低噪声边框和明确的当前页强调。

## 使用建议

- 适合 onboarding、媒体分页、卡片轮播和短页数内容浏览。
- 后续若继续扩展，应保持 `Fluent 2 / WPF UI` 的语义一致性，而不是回到过度装饰的 showcase 风格。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/pips_pager PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/pips_pager --timeout 10 --keep-screenshots
```
