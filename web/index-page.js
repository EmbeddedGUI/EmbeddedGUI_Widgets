(function() {
    var i18n = window.EmbeddedGUII18n;
    var state = {
        manifest: {
            total: null,
            reference: null,
            showcase: null,
            missing: true
        },
        policy: {
            total: null,
            defaultWebTotal: null,
            deprecated: null,
            replacementCount: null,
            categoryCount: null,
            categories: [],
            replacements: [],
            missing: true
        }
    };

    var categoryLabels = {
        input: { "zh-CN": "输入", "en": "Input" },
        layout: { "zh-CN": "布局", "en": "Layout" },
        navigation: { "zh-CN": "导航", "en": "Navigation" },
        display: { "zh-CN": "显示", "en": "Display" },
        feedback: { "zh-CN": "反馈", "en": "Feedback" },
        decoration: { "zh-CN": "装饰", "en": "Decoration" },
        chart: { "zh-CN": "图表", "en": "Chart" },
        media: { "zh-CN": "媒体", "en": "Media" },
        misc: { "zh-CN": "其他", "en": "Misc" }
    };

    var text = {
        pageTitle: { "zh-CN": "EmbeddedGUI Widgets", "en": "EmbeddedGUI Widgets" },
        landingEyebrow: { "zh-CN": "Fluent 2 / WPF UI", "en": "Fluent 2 / WPF UI" },
        landingTitle: { "zh-CN": "HelloCustomWidgets 统一入口", "en": "HelloCustomWidgets Main Entry" },
        landingDescription: {
            "zh-CN": "整个站点已经收口到 Fluent 2 / WPF UI 参考体系。Reference 作为主线保留，showcase 仅作为过渡与历史样例，不再继续扩散成新的默认风格。",
            "en": "The site now converges on Fluent 2 / WPF UI. Reference widgets remain on the mainline, while showcase entries are kept only as transitional and historical samples."
        },
        actionReference: { "zh-CN": "打开 Reference 目录", "en": "Open Reference Catalog" },
        actionShowcase: { "zh-CN": "查看 Showcase 轨道", "en": "Open Showcase Track" },
        actionManifest: { "zh-CN": "查看 demos.json", "en": "Open demos.json" },
        actionPolicy: { "zh-CN": "查看 catalog-policy.json", "en": "Open catalog-policy.json" },
        statTotalLabel: { "zh-CN": "站点目录", "en": "Web Catalog" },
        statTotalDescription: { "zh-CN": "当前网页包内可直接打开的 HelloCustomWidgets 条目。", "en": "HelloCustomWidgets entries currently available in the web bundle." },
        statReferenceLabel: { "zh-CN": "Reference 主线", "en": "Reference Mainline" },
        statReferenceDescription: { "zh-CN": "后续视觉和交互统一默认以这一轨道为基准。", "en": "Future visual and interaction work should default to this track." },
        statShowcaseLabel: { "zh-CN": "Showcase 保留", "en": "Showcase Archive" },
        statShowcaseDescription: { "zh-CN": "只保留展示价值，不再作为主线控件继续扩张。", "en": "Kept for demonstration value, not for further mainline expansion." },
        principlesKicker: { "zh-CN": "Mainline Rules", "en": "Mainline Rules" },
        principlesTitle: { "zh-CN": "统一风格的三个约束", "en": "Three Rules for a Unified Style" },
        workflowKicker: { "zh-CN": "Workflow", "en": "Workflow" },
        workflowTitle: { "zh-CN": "日常维护入口", "en": "Daily Maintenance Entry Points" },
        tracksKicker: { "zh-CN": "Track Policy", "en": "Track Policy" },
        tracksTitle: { "zh-CN": "Catalog 轨道说明", "en": "Catalog Track Policy" },
        tracksNote: {
            "zh-CN": "默认网页 manifest 不再暴露 deprecated 条目；如需回看旧控件，请显式使用构建脚本切换到对应轨道。",
            "en": "The default web manifest no longer exposes deprecated entries. Use the build scripts explicitly if you need to revisit old widgets."
        },
        tracksNotePruned: {
            "zh-CN": "当前仓库已完成 deprecated 目录清退，默认保留的只有 Reference 与 Showcase 两条轨道。",
            "en": "Deprecated directories have been pruned from the repository. Only Reference and Showcase remain in the retained set."
        },
        manifestMissing: {
            "zh-CN": "未找到 demos.json，首页中的 web 统计暂不可用。先运行 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --refresh-existing` 或完整构建命令。",
            "en": "demos.json is missing, so web bundle stats are unavailable. Run `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --refresh-existing` or a full build first."
        },
        policyMissing: {
            "zh-CN": "未找到 catalog-policy.json，catalog 规则摘要暂不可用。先运行 `python scripts/sync_widget_catalog.py`。",
            "en": "catalog-policy.json is missing, so policy summary is unavailable. Run `python scripts/sync_widget_catalog.py` first."
        },
        catalogNoteSummary: {
            "zh-CN": "完整 catalog 共 {total} 个控件，其中 {defaultWebTotal} 个仍进入默认网页目录，{deprecated} 个保持 deprecated，不再默认发布。",
            "en": "The full catalog contains {total} widgets. {defaultWebTotal} still ship in the default web catalog, while {deprecated} stay deprecated and unpublished by default."
        },
        placeholderValue: { "zh-CN": "--", "en": "--" },
        principlesReferenceTitle: { "zh-CN": "Reference 优先", "en": "Reference First" },
        principlesReferenceBody: {
            "zh-CN": "新控件、重构和视觉收口都应优先贴齐 Fluent 2 / WPF UI，可替代的旧控件不再继续并行演化。",
            "en": "New widgets, refactors, and visual consolidation should align with Fluent 2 / WPF UI before anything else."
        },
        principlesShowcaseTitle: { "zh-CN": "Showcase 限定", "en": "Showcase Is Limited" },
        principlesShowcaseBody: {
            "zh-CN": "showcase 只用于保留历史探索、行业样例或过渡方案，不再承担主线设计语言的扩张职责。",
            "en": "Showcase is reserved for historical explorations, domain samples, or transitional patterns instead of mainline design growth."
        },
        principlesDocsTitle: { "zh-CN": "文档必须 UTF-8", "en": "Docs Must Stay UTF-8" },
        principlesDocsBody: {
            "zh-CN": "文档和站点源码统一保持 UTF-8；新增 README、脚本和页面都需要通过编码检查，避免再次出现串码或问号占位。",
            "en": "Documentation and site sources must stay in UTF-8. New README files, scripts, and pages should pass encoding checks to avoid mojibake regressions."
        },
        workflowCompileTitle: { "zh-CN": "Reference 编译检查", "en": "Reference Compile Sweep" },
        workflowRuntimeTitle: { "zh-CN": "Reference 运行检查", "en": "Reference Runtime Sweep" },
        workflowManifestTitle: { "zh-CN": "刷新网页 manifest", "en": "Refresh Web Manifest" },
        workflowPolicyTitle: { "zh-CN": "同步 catalog policy", "en": "Sync Catalog Policy" },
        trackReferenceBody: {
            "zh-CN": "主线控件。保留在默认网页目录中，作为 Fluent 2 / WPF UI 对齐基线。",
            "en": "Mainline widgets. Kept in the default web catalog as the Fluent 2 / WPF UI baseline."
        },
        trackShowcaseBody: {
            "zh-CN": "历史样例与过渡轨道。仍可浏览，但不再作为默认设计语言继续外扩。",
            "en": "Historical samples and transitional entries. Still browseable, but no longer used as the default design language."
        },
        trackDeprecatedBody: {
            "zh-CN": "默认不进入网页 manifest；仅在显式构建或回溯审查时保留。",
            "en": "Excluded from the default web manifest and kept only for explicit rebuilds or historical review."
        },
        catalogKicker: { "zh-CN": "Catalog Policy", "en": "Catalog Policy" },
        catalogTitle: { "zh-CN": "完整 catalog 摘要", "en": "Full Catalog Summary" },
        policyTotalLabel: { "zh-CN": "完整总量", "en": "Full Catalog" },
        policyTotalDescription: { "zh-CN": "HelloCustomWidgets 源目录中的全部控件，不受默认网页 manifest 过滤。", "en": "All widgets in the HelloCustomWidgets source tree, before default web manifest filtering." },
        policyDeprecatedLabel: { "zh-CN": "Deprecated", "en": "Deprecated" },
        policyDeprecatedDescription: { "zh-CN": "已退出主线，只在显式回溯或兼容核对时保留。", "en": "No longer on the mainline and kept only for explicit review or compatibility checks." },
        policyReplacementLabel: { "zh-CN": "Replacement 路径", "en": "Replacement Paths" },
        policyReplacementDescription: { "zh-CN": "已经明确指定迁移目标的过渡控件数量。", "en": "Transitional widgets that already have explicit migration targets." },
        policyCategoryLabel: { "zh-CN": "分类数", "en": "Categories" },
        policyCategoryDescription: { "zh-CN": "当前 catalog 覆盖的控件分类维度。", "en": "Widget categories currently covered by the catalog." },
        categorySummaryLabel: { "zh-CN": "分类覆盖", "en": "Category Coverage" },
        categoryEmpty: { "zh-CN": "当前没有可展示的分类摘要。", "en": "No category summary is available right now." },
        replacementSummaryLabel: { "zh-CN": "推荐替代关系", "en": "Recommended Replacements" },
        replacementEmpty: { "zh-CN": "当前没有 replacement 关系。", "en": "There are no replacement relationships right now." },
        categoryBreakdown: {
            "zh-CN": "共 {total} 个 · R {reference} / S {showcase} / D {deprecated}",
            "en": "{total} total · R {reference} / S {showcase} / D {deprecated}"
        }
    };

    function t(key, params) {
        return i18n.format(text[key], params);
    }

    function esc(value) {
        return String(value || "")
            .replace(/&/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/"/g, "&quot;")
            .replace(/'/g, "&#39;");
    }

    function fmtNumber(value) {
        if (value === null || value === undefined) {
            return t("placeholderValue");
        }
        return String(value);
    }

    function fmtCategory(categoryId) {
        return i18n.format(categoryLabels[categoryId] || { "zh-CN": categoryId, "en": categoryId });
    }

    function demoQuery(widgetId) {
        return "custom.html?demo=" + encodeURIComponent(widgetId);
    }

    function renderStaticText() {
        document.title = t("pageTitle");
        document.getElementById("landing-eyebrow").textContent = t("landingEyebrow");
        document.getElementById("landing-title").textContent = t("landingTitle");
        document.getElementById("landing-description").textContent = t("landingDescription");
        document.getElementById("action-reference").textContent = t("actionReference");
        document.getElementById("action-showcase").textContent = t("actionShowcase");
        document.getElementById("action-manifest").textContent = t("actionManifest");
        document.getElementById("action-policy").textContent = t("actionPolicy");
        document.getElementById("stat-total-label").textContent = t("statTotalLabel");
        document.getElementById("stat-total-description").textContent = t("statTotalDescription");
        document.getElementById("stat-reference-label").textContent = t("statReferenceLabel");
        document.getElementById("stat-reference-description").textContent = t("statReferenceDescription");
        document.getElementById("stat-showcase-label").textContent = t("statShowcaseLabel");
        document.getElementById("stat-showcase-description").textContent = t("statShowcaseDescription");
        document.getElementById("principles-kicker").textContent = t("principlesKicker");
        document.getElementById("principles-title").textContent = t("principlesTitle");
        document.getElementById("workflow-kicker").textContent = t("workflowKicker");
        document.getElementById("workflow-title").textContent = t("workflowTitle");
        document.getElementById("tracks-kicker").textContent = t("tracksKicker");
        document.getElementById("tracks-title").textContent = t("tracksTitle");
        document.getElementById("tracks-note").textContent = state.manifest.missing
            ? t("manifestMissing")
            : (state.policy.deprecated === 0 ? t("tracksNotePruned") : t("tracksNote"));
        document.getElementById("catalog-kicker").textContent = t("catalogKicker");
        document.getElementById("catalog-title").textContent = t("catalogTitle");
        document.getElementById("category-summary-label").textContent = t("categorySummaryLabel");
        document.getElementById("replacement-summary-label").textContent = t("replacementSummaryLabel");
        document.getElementById("catalog-note").textContent = state.policy.missing
            ? t("policyMissing")
            : t("catalogNoteSummary", {
                total: fmtNumber(state.policy.total),
                defaultWebTotal: fmtNumber(state.policy.defaultWebTotal),
                deprecated: fmtNumber(state.policy.deprecated)
            });
    }

    function renderManifestStats() {
        document.getElementById("stat-total-value").textContent = fmtNumber(state.manifest.total);
        document.getElementById("stat-reference-value").textContent = fmtNumber(state.manifest.reference);
        document.getElementById("stat-showcase-value").textContent = fmtNumber(state.manifest.showcase);
    }

    function renderPrinciples() {
        var cards = [
            { title: t("principlesReferenceTitle"), body: t("principlesReferenceBody") },
            { title: t("principlesShowcaseTitle"), body: t("principlesShowcaseBody") },
            { title: t("principlesDocsTitle"), body: t("principlesDocsBody") }
        ];
        document.getElementById("principles-grid").innerHTML = cards.map(function(card) {
            return [
                '<article class="landing-card">',
                '<h3>' + esc(card.title) + '</h3>',
                '<p>' + esc(card.body) + '</p>',
                '</article>'
            ].join("");
        }).join("");
    }

    function renderWorkflow() {
        var items = [
            { title: t("workflowCompileTitle"), command: "python scripts/code_compile_check.py --custom-widgets --track reference" },
            { title: t("workflowRuntimeTitle"), command: "python scripts/code_runtime_check.py --app HelloCustomWidgets --track reference" },
            { title: t("workflowManifestTitle"), command: "python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --refresh-existing" },
            { title: t("workflowPolicyTitle"), command: "python scripts/sync_widget_catalog.py" }
        ];
        document.getElementById("workflow-list").innerHTML = items.map(function(item) {
            return [
                '<article class="command-item">',
                '<h3>' + esc(item.title) + '</h3>',
                '<code>' + esc(item.command) + '</code>',
                '</article>'
            ].join("");
        }).join("");
    }

    function renderTracks() {
        var cards = [
            { tone: "reference", title: "Reference", body: t("trackReferenceBody") },
            { tone: "showcase", title: "Showcase", body: t("trackShowcaseBody") }
        ];
        if (state.policy.deprecated !== 0) {
            cards.push({ tone: "deprecated", title: "Deprecated", body: t("trackDeprecatedBody") });
        }
        document.getElementById("tracks-grid").innerHTML = cards.map(function(card) {
            return [
                '<article class="landing-card tone-' + esc(card.tone) + '">',
                '<h3>' + esc(card.title) + '</h3>',
                '<p>' + esc(card.body) + '</p>',
                '</article>'
            ].join("");
        }).join("");
    }

    function renderPolicySummary() {
        var stats = [
            {
                label: t("policyTotalLabel"),
                value: fmtNumber(state.policy.total),
                description: t("policyTotalDescription")
            },
            {
                label: t("policyDeprecatedLabel"),
                value: fmtNumber(state.policy.deprecated),
                description: t("policyDeprecatedDescription")
            },
            {
                label: t("policyReplacementLabel"),
                value: fmtNumber(state.policy.replacementCount),
                description: t("policyReplacementDescription")
            },
            {
                label: t("policyCategoryLabel"),
                value: fmtNumber(state.policy.categoryCount),
                description: t("policyCategoryDescription")
            }
        ];
        document.getElementById("catalog-stats-grid").innerHTML = stats.map(function(item) {
            return [
                '<article class="landing-stat-card compact">',
                '<div class="meta-label">' + esc(item.label) + '</div>',
                '<div class="landing-stat-value">' + esc(item.value) + '</div>',
                '<p>' + esc(item.description) + '</p>',
                '</article>'
            ].join("");
        }).join("");

        if (!state.policy.categories.length) {
            document.getElementById("category-summary-list").innerHTML = '<p class="landing-note replacement-empty">' + esc(t("categoryEmpty")) + '</p>';
        } else {
            document.getElementById("category-summary-list").innerHTML = state.policy.categories.map(function(category) {
                return [
                    '<div class="category-pill">',
                    '<strong>' + esc(fmtCategory(category.id)) + '</strong>',
                    '<span>' + esc(t("categoryBreakdown", category)) + '</span>',
                    '</div>'
                ].join("");
            }).join("");
        }

        if (!state.policy.replacements.length) {
            document.getElementById("replacement-summary-list").innerHTML = '<p class="landing-note replacement-empty">' + esc(t("replacementEmpty")) + '</p>';
            return;
        }

        document.getElementById("replacement-summary-list").innerHTML = state.policy.replacements.map(function(item) {
            return [
                '<a class="replacement-item" href="' + esc(demoQuery(item.target)) + '">',
                '<span class="replacement-source">' + esc(item.source) + '</span>',
                '<span class="replacement-arrow">→</span>',
                '<span class="replacement-target">' + esc(item.target) + '</span>',
                '</a>'
            ].join("");
        }).join("");
    }

    function render() {
        renderStaticText();
        renderManifestStats();
        renderPrinciples();
        renderWorkflow();
        renderTracks();
        renderPolicySummary();
    }

    function loadManifest() {
        return fetch("demos/demos.json").then(function(response) {
            if (!response.ok) {
                throw new Error("Missing demos.json");
            }
            return response.json();
        }).then(function(entries) {
            var widgets = entries.filter(function(entry) {
                return entry.app === "HelloCustomWidgets";
            });
            state.manifest.total = widgets.length;
            state.manifest.reference = widgets.filter(function(entry) { return entry.track === "reference"; }).length;
            state.manifest.showcase = widgets.filter(function(entry) { return entry.track === "showcase"; }).length;
            state.manifest.missing = false;
        }).catch(function() {
            state.manifest.total = null;
            state.manifest.reference = null;
            state.manifest.showcase = null;
            state.manifest.missing = true;
        });
    }

    function loadPolicy() {
        return fetch("catalog-policy.json").then(function(response) {
            if (!response.ok) {
                throw new Error("Missing catalog-policy.json");
            }
            return response.json();
        }).then(function(summary) {
            state.policy.total = summary.total;
            state.policy.defaultWebTotal = summary.defaultWebTotal;
            state.policy.deprecated = summary.tracks ? summary.tracks.deprecated : null;
            state.policy.replacementCount = summary.replacements ? summary.replacements.length : 0;
            state.policy.categoryCount = summary.categories ? summary.categories.length : 0;
            state.policy.categories = summary.categories || [];
            state.policy.replacements = summary.replacements || [];
            state.policy.missing = false;
        }).catch(function() {
            state.policy.total = null;
            state.policy.defaultWebTotal = null;
            state.policy.deprecated = null;
            state.policy.replacementCount = null;
            state.policy.categoryCount = null;
            state.policy.categories = [];
            state.policy.replacements = [];
            state.policy.missing = true;
        });
    }

    i18n.bindLanguageToggle(document);
    render();
    document.addEventListener("embeddedgui:languagechange", render);
    Promise.all([loadManifest(), loadPolicy()]).finally(render);
})();
