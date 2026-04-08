# Avatar Stack

## 定位

`avatar_stack` 是较早的展示型人员叠层控件，强调重叠头像、焦点成员和紧凑卡片。它保留了主卡、`Compact`、`Locked` 三种状态，但已经不再作为主线方案维护。

## 当前示例内容

- 主卡在两组成员快照之间切换，示例成员包括 `Alice / Blake / Cindy / Dean` 与 `Emma / Finn / Gray / Hana`。
- `Compact` 预览展示三人缩略叠层，并跟随焦点成员切换不同强调色。
- `Locked` 预览保留只读外观，用来验证禁用态下的头像堆叠可读性。
- 页面点击主卡或 `Compact` 区域后，会在 `Core A / Core B / Compact A / Compact B` 状态之间轮换。

## 现状判断

- 轨道：`showcase`
- 替代项：`display/persona_group`
- `persona_group` 已经承接人员分组、状态、紧凑态和只读态语义，且更贴近 Fluent 2 / WPF UI 的统一风格。

## 使用建议

- 如果只是保留历史叠层头像效果，可以继续查看此 demo。
- 如果要做正式的人员展示、协作列表或状态编组，应该直接使用 `display/persona_group`。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=decoration/avatar_stack PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub decoration/avatar_stack --timeout 10 --keep-screenshots
```
