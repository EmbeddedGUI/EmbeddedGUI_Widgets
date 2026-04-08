# Persona Group

## 参考来源

- 参考设计体系：`Fluent 2`
- 参考开源库：`WPF UI`
- 对应组件语义：`AvatarGroup`

## 定位

`persona_group` 是当前主线人员分组 reference 控件，用来统一替代旧的叠层头像和装饰性成员展示方案。它同时覆盖标准态、紧凑态和只读态，并把成员名称、职责、在线状态和焦点成员语义收拢到同一套视觉语言里。

## 当前示例内容

- 主卡在多组团队快照之间切换，并根据焦点成员更新状态文案，例如 `Design review: Lena Live`。
- `Compact` 预览提供 `Team / Ops` 两组压缩编排，用来验证小尺寸场景。
- `Read Only` 预览使用归档/只读数据集，检验禁用态下的人员信息可读性。
- 焦点成员变化会驱动状态文字和强调色同步变化。

## 现状判断

- 轨道：`reference`
- 它是人员头像相关控件的主线基线，后续涉及协作成员、审核责任人、团队概览等场景都应优先向它靠齐。
- 旧的 `decoration/avatar_stack` 只保留历史展示价值，不再作为主线扩展对象。

## 使用建议

- 适合团队列表、审阅责任链、任务面板和状态概览页面。
- 若要新增样式或行为，应继续保持 Fluent 2 / WPF UI 的浅色、低噪声、语义优先原则。

## 构建与验证

```bash
make all APP=HelloCustomWidgets APP_SUB=display/persona_group PORT=pc
python scripts/code_runtime_check.py --app HelloCustomWidgets --app-sub display/persona_group --timeout 10 --keep-screenshots
```
