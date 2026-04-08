(function() {
    var i18n = window.EmbeddedGUII18n;
    var state = {
        total: null,
        reference: null,
        showcase: null,
        missingManifest: false
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
        manifestMissing: {
            "zh-CN": "未找到 demos.json，统计值暂不可用。先运行 `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --refresh-existing` 或完整构建命令。",
            "en": "demos.json is missing, so live stats are unavailable. Run `python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --refresh-existing` or a full build first."
        },
        placeholderValue: { "zh-CN": "--", "en": "--" }
    };

    var principleCards = [
        {
            title: { "zh-CN": "Reference 优先", "en": "Reference First" },
            body: {
                "zh-CN": "新控件、重构和视觉收口都应优先贴齐 Fluent 2 / WPF UI，可替代的旧控件不再继续并行演化。",
                "en": "New widgets, refactors, and visual consolidation should align with Fluent 2 / WPF UI before anything else."
            }
        },
        {
            title: { "zh-CN": "Showcase 限定", "en": "Showcase Is Limited" },
            body: {
                "zh-CN": "showcase 只用于保留历史探索、行业样例或过渡方案，不再承担主线设计语言的扩张职责。",
                "en": "Showcase is reserved for historical explorations, domain samples, or transitional patterns instead of mainline design growth."
            }
        },
        {
            title: { "zh-CN": "文档必须 UTF-8", "en": "Docs Must Stay UTF-8" },
            body: {
                "zh-CN": "文档和站点源码统一保持 UTF-8；新增 README、脚本和页面都需要通过编码检查，避免再次出现串码或问号占位。",
                "en": "Documentation and site sources must stay in UTF-8. New README files, scripts, and pages should pass encoding checks to avoid mojibake regressions."
            }
        }
    ];

    var workflowItems = [
        {
            title: { "zh-CN": "Reference 编译检查", "en": "Reference Compile Sweep" },
            command: "python scripts/code_compile_check.py --custom-widgets --track reference"
        },
        {
            title: { "zh-CN": "Reference 运行检查", "en": "Reference Runtime Sweep" },
            command: "python scripts/code_runtime_check.py --app HelloCustomWidgets --track reference"
        },
        {
            title: { "zh-CN": "刷新网页 manifest", "en": "Refresh Web Manifest" },
            command: "python scripts/web/wasm_build_demos.py --app HelloCustomWidgets --refresh-existing"
        },
        {
            title: { "zh-CN": "检查文档编码", "en": "Check Documentation Encoding" },
            command: "python scripts/checks/check_docs_encoding.py"
        }
    ];

    var trackCards = [
        {
            tone: "reference",
            title: { "zh-CN": "Reference", "en": "Reference" },
            body: {
                "zh-CN": "主线控件。保留在默认网页目录中，作为 Fluent 2 / WPF UI 对齐基线。",
                "en": "Mainline widgets. Kept in the default web catalog as the Fluent 2 / WPF UI baseline."
            }
        },
        {
            tone: "showcase",
            title: { "zh-CN": "Showcase", "en": "Showcase" },
            body: {
                "zh-CN": "历史样例与过渡轨道。仍可浏览，但不再作为默认设计语言继续外扩。",
                "en": "Historical samples and transitional entries. Still browseable, but no longer used as the default design language."
            }
        },
        {
            tone: "deprecated",
            title: { "zh-CN": "Deprecated", "en": "Deprecated" },
            body: {
                "zh-CN": "默认不进入网页 manifest；仅在显式构建或回溯审查时保留。",
                "en": "Excluded from the default web manifest and kept only for explicit rebuilds or historical review."
            }
        }
    ];

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

    function renderStaticText() {
        document.title = t("pageTitle");
        document.getElementById("landing-eyebrow").textContent = t("landingEyebrow");
        document.getElementById("landing-title").textContent = t("landingTitle");
        document.getElementById("landing-description").textContent = t("landingDescription");
        document.getElementById("action-reference").textContent = t("actionReference");
        document.getElementById("action-showcase").textContent = t("actionShowcase");
        document.getElementById("action-manifest").textContent = t("actionManifest");
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
        document.getElementById("tracks-note").textContent = state.missingManifest ? t("manifestMissing") : t("tracksNote");
    }

    function renderStats() {
        document.getElementById("stat-total-value").textContent = fmtNumber(state.total);
        document.getElementById("stat-reference-value").textContent = fmtNumber(state.reference);
        document.getElementById("stat-showcase-value").textContent = fmtNumber(state.showcase);
    }

    function renderPrinciples() {
        document.getElementById("principles-grid").innerHTML = principleCards.map(function(card) {
            return [
                '<article class="landing-card">',
                '<h3>' + esc(i18n.format(card.title)) + '</h3>',
                '<p>' + esc(i18n.format(card.body)) + '</p>',
                '</article>'
            ].join("");
        }).join("");
    }

    function renderWorkflow() {
        document.getElementById("workflow-list").innerHTML = workflowItems.map(function(item) {
            return [
                '<article class="command-item">',
                '<h3>' + esc(i18n.format(item.title)) + '</h3>',
                '<code>' + esc(item.command) + '</code>',
                '</article>'
            ].join("");
        }).join("");
    }

    function renderTracks() {
        document.getElementById("tracks-grid").innerHTML = trackCards.map(function(card) {
            return [
                '<article class="landing-card tone-' + esc(card.tone) + '">',
                '<h3>' + esc(i18n.format(card.title)) + '</h3>',
                '<p>' + esc(i18n.format(card.body)) + '</p>',
                '</article>'
            ].join("");
        }).join("");
    }

    function render() {
        renderStaticText();
        renderStats();
        renderPrinciples();
        renderWorkflow();
        renderTracks();
    }

    function loadManifest() {
        fetch("demos/demos.json").then(function(response) {
            if (!response.ok) {
                throw new Error("Missing demos.json");
            }
            return response.json();
        }).then(function(entries) {
            var widgets = entries.filter(function(entry) {
                return entry.app === "HelloCustomWidgets";
            });
            state.total = widgets.length;
            state.reference = widgets.filter(function(entry) { return entry.track === "reference"; }).length;
            state.showcase = widgets.filter(function(entry) { return entry.track === "showcase"; }).length;
            state.missingManifest = false;
            render();
        }).catch(function() {
            state.total = null;
            state.reference = null;
            state.showcase = null;
            state.missingManifest = true;
            render();
        });
    }

    i18n.bindLanguageToggle(document);
    render();
    document.addEventListener("embeddedgui:languagechange", render);
    loadManifest();
})();
