# Trail Scenes

## 定位

`breadcrumb_trail` 是较早的路径展示控件，主卡、紧凑态和锁定态都带有较强的展示感。它保留了路径卡片和焦点项轮换，但已经不再作为导航主线方案继续推进。

## 当前示例内容

- 主卡在 `Path A / Path B` 两组路径之间切换，并高亮当前焦点项。
- `Compact` 预览提供 `Doc / UI / Run` 与 `Nav / Card / Edit` 两组缩略路径。
- `Locked` 预览用于测试禁用态下的路径结构可读性。
- 页面点击主卡或紧凑区块后，会更新状态文本和强调色。

## 现状判断

- 轨道：`showcase`
- 替代项：`navigation/breadcrumb_bar`
- `breadcrumb_bar` 已经提供更接近 Fluent 2 / WPF UI 的参考语义和浅色主线视觉，因此后续导航路径场景应以替代项为准。

## 使用建议

- 仅适合回看旧的展示稿或保留路径卡片化设计样例。
- 若要做正式页面导航，直接迁移到 `navigation/breadcrumb_bar`。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=navigation/breadcrumb_trail PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub navigation/breadcrumb_trail --timeout 10 --keep-screenshots
```
