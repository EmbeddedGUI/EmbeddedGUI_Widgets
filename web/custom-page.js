(function() {
    var i18n = window.EmbeddedGUII18n;
    var state = {
        allDemos: [],
        demoMap: {},
        activeTrack: "reference",
        requestedTrack: "",
        query: "",
        selectedName: "",
        initialTarget: ""
    };

    var categoryOrder = ["input", "layout", "navigation", "display", "feedback", "decoration", "chart", "media", "misc"];
    var text = {
        pageTitle: { "zh-CN": "EmbeddedGUI HelloCustomWidgets", "en": "EmbeddedGUI HelloCustomWidgets" },
        sidebarTitle: { "zh-CN": "HelloCustomWidgets 控件目录", "en": "HelloCustomWidgets Catalog" },
        sidebarDescription: {
            "zh-CN": "项目主线统一收口到 Fluent 2 / WPF UI。左侧默认按 reference 基线浏览；如本地 manifest 仍包含历史轨道，可通过显式参数回看。",
            "en": "The project now converges on Fluent 2 / WPF UI. The sidebar defaults to the reference baseline, while historical tracks can still be revisited explicitly in local manifests."
        },
        sidebarDescriptionReferenceOnly: {
            "zh-CN": "项目主线统一收口到 Fluent 2 / WPF UI。当前仓库与默认网页目录只保留 reference 控件。",
            "en": "The project now converges on Fluent 2 / WPF UI. Both the repository and the default web catalog keep only reference widgets."
        },
        linkHome: { "zh-CN": "站点入口", "en": "Site entry" },
        linkJson: { "zh-CN": "demos.json", "en": "demos.json" },
        trackLabel: { "zh-CN": "显示轨道", "en": "Display track" },
        searchPlaceholder: { "zh-CN": "搜索控件、分类或参考组件", "en": "Search widgets, categories, or reference components" },
        defaultEyebrow: { "zh-CN": "Reference 优先目录", "en": "Reference-first catalog" },
        defaultTitle: { "zh-CN": "选择一个控件", "en": "Select a widget" },
        defaultDescription: {
            "zh-CN": "左侧列表展示当前网页包内可用的 HelloCustomWidgets，并优先围绕 reference 主线组织。",
            "en": "The sidebar lists the HelloCustomWidgets demos available in the current web bundle and prioritizes the reference mainline."
        },
        defaultDescriptionReferenceOnly: {
            "zh-CN": "左侧列表只保留当前网页包内可用的 HelloCustomWidgets reference 控件。",
            "en": "The sidebar only lists HelloCustomWidgets reference widgets available in the current web bundle."
        },
        emptyTitle: { "zh-CN": "当前筛选下没有控件", "en": "No widgets match the current filters" },
        emptyDescription: { "zh-CN": "可以切换轨道、清空搜索，或重新执行 `python scripts/web/wasm_build_demos.py`。", "en": "Switch track, clear the search, or rerun `python scripts/web/wasm_build_demos.py`." },
        missingCatalogTitle: { "zh-CN": "缺少 demos.json", "en": "Missing demos.json" },
        missingCatalogDescription: { "zh-CN": "这个页面依赖 `web/demos/demos.json`。请先运行 `python scripts/web/wasm_build_demos.py`。", "en": "This page depends on `web/demos/demos.json`. Run `python scripts/web/wasm_build_demos.py` first." },
        previewLabel: { "zh-CN": "实时预览", "en": "Live preview" },
        docLabel: { "zh-CN": "README", "en": "README" },
        docBundled: { "zh-CN": "随 demo 一起生成", "en": "Bundled with demo" },
        docMissing: { "zh-CN": "当前条目没有 README", "en": "README is missing for this entry" },
        openDemo: { "zh-CN": "打开示例", "en": "Open demo" },
        openReadme: { "zh-CN": "打开 README", "en": "Open README" },
        openReplacement: { "zh-CN": "打开替代控件", "en": "Open replacement" },
        metaWidgetId: { "zh-CN": "控件 ID", "en": "Widget ID" },
        metaTrack: { "zh-CN": "轨道", "en": "Track" },
        metaVisibility: { "zh-CN": "可见性", "en": "Visibility" },
        metaReference: { "zh-CN": "参考体系", "en": "Reference system" },
        metaReplacement: { "zh-CN": "替代项", "en": "Replacement" },
        metaNone: { "zh-CN": "无", "en": "None" },
        summaryTemplate: { "zh-CN": "分类：{category}。轨道：{track}。", "en": "Category: {category}. Track: {track}." },
        referenceSummary: { "zh-CN": "对齐 {system} / {library} 的 {component}", "en": "Aligned with {component} from {system} / {library}" },
        referenceNotice: { "zh-CN": "这是当前主线 reference 控件，后续视觉和交互统一以它为基线。", "en": "This is a mainline reference widget. Future visual and interaction work should use it as the baseline." },
        showcaseNotice: { "zh-CN": "这是历史 showcase 条目，只建议用于回看或迁移参考。", "en": "This is a historical showcase entry and should only be used for review or migration reference." },
        deprecatedNotice: { "zh-CN": "这是已清退条目，仅建议用于兼容核对或历史回看。", "en": "This entry has been deprecated and should only be kept for compatibility checks or historical review." },
        replacementNotice: { "zh-CN": "建议改用 `{replacement}`，后续设计与维护以替代项为准。", "en": "Use `{replacement}` instead. Future design and maintenance should follow the replacement entry." },
        countReference: { "zh-CN": "Reference {count}", "en": "Reference {count}" },
        countShowcase: { "zh-CN": "Showcase {count}", "en": "Showcase {count}" },
        countVisible: { "zh-CN": "当前 {count}", "en": "Visible {count}" },
        sidebarEmpty: { "zh-CN": "当前轨道没有可显示条目。", "en": "No entries are available on the current track." }
    };

    var labels = {
        categories: {
            input: { "zh-CN": "输入", "en": "Input" },
            layout: { "zh-CN": "布局", "en": "Layout" },
            navigation: { "zh-CN": "导航", "en": "Navigation" },
            display: { "zh-CN": "显示", "en": "Display" },
            feedback: { "zh-CN": "反馈", "en": "Feedback" },
            decoration: { "zh-CN": "装饰", "en": "Decoration" },
            chart: { "zh-CN": "图表", "en": "Chart" },
            media: { "zh-CN": "媒体", "en": "Media" },
            misc: { "zh-CN": "其他", "en": "Misc" }
        },
        tracks: {
            reference: { "zh-CN": "Reference", "en": "Reference" },
            showcase: { "zh-CN": "Showcase", "en": "Showcase" },
            deprecated: { "zh-CN": "Deprecated", "en": "Deprecated" },
            all: { "zh-CN": "全部", "en": "All" }
        },
        visibility: {
            public: { "zh-CN": "公开", "en": "Public" },
            internal: { "zh-CN": "内部", "en": "Internal" },
            hidden: { "zh-CN": "隐藏", "en": "Hidden" }
        }
    };

    function t(key, params) { return i18n.format(text[key], params); }
    function fmtWords(value) { return String(value || "").replace(/[_-]+/g, " ").replace(/\b\w/g, function(match) { return match.toUpperCase(); }); }
    function esc(value) { return String(value || "").replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;").replace(/'/g, "&#39;"); }
    function normalize(value) { return String(value || "").toLowerCase().replace(/[_/.-]+/g, " ").replace(/\s+/g, " ").trim(); }
    function fmtCategory(value) { var key = String(value || "misc").toLowerCase(); return i18n.format(labels.categories[key] || { "zh-CN": fmtWords(key), "en": fmtWords(key) }); }
    function fmtTrack(value) { var key = String(value || "reference").toLowerCase(); return i18n.format(labels.tracks[key] || labels.tracks.reference); }
    function fmtVisibility(value) { var key = String(value || "public").toLowerCase(); return i18n.format(labels.visibility[key] || { "zh-CN": fmtWords(key), "en": fmtWords(key) }); }
    function demoName(widgetId) { return "HelloCustomWidgets_" + String(widgetId || "").replace(/\//g, "_"); }
    function demoPath(demo) { return "demos/" + demo.name + "/" + demo.app + ".html"; }
    function frameHeight(demo) { var height = Number(demo && demo.height); return !Number.isFinite(height) || height <= 0 ? 460 : Math.max(420, height + 48); }
    function compareCategory(a, b) { var ia = categoryOrder.indexOf(a); var ib = categoryOrder.indexOf(b); if (ia === -1 && ib === -1) return a.localeCompare(b); if (ia === -1) return 1; if (ib === -1) return -1; return ia - ib; }

    function normalizeTrack(value) {
        var track = String(value || "").toLowerCase();
        return ["reference", "showcase", "deprecated", "all"].indexOf(track) !== -1 ? track : "reference";
    }

    function parseMeta(demo) {
        var widgetId = demo.widgetId || demo.appSub || "";
        var parts = widgetId.split("/", 2);
        return {
            widgetId: widgetId,
            category: parts[0] || "misc",
            widget: parts[1] || widgetId || demo.name
        };
    }

    function currentDemos() {
        return state.allDemos.filter(function(demo) {
            var trackOk = state.activeTrack === "all" || demo.track === state.activeTrack;
            return trackOk && (!state.query || demo.searchText.indexOf(state.query) !== -1);
        });
    }

    function referenceSummary(demo) {
        if (!demo.referenceSystem || !demo.referenceLibrary || !demo.referenceComponent) {
            return t("metaNone");
        }
        return t("referenceSummary", {
            system: demo.referenceSystem,
            library: demo.referenceLibrary,
            component: demo.referenceComponent
        });
    }

    function showcaseCount() {
        return state.allDemos.filter(function(demo) { return demo.track === "showcase"; }).length;
    }

    function setStaticText() {
        var showcaseTotal = showcaseCount();
        document.title = t("pageTitle");
        document.getElementById("sidebar-title").textContent = t("sidebarTitle");
        document.getElementById("sidebar-description").textContent = showcaseTotal === 0 ? t("sidebarDescriptionReferenceOnly") : t("sidebarDescription");
        document.getElementById("link-home").textContent = t("linkHome");
        document.getElementById("link-json").textContent = t("linkJson");
        document.getElementById("track-label").textContent = t("trackLabel");
        document.getElementById("search-input").placeholder = t("searchPlaceholder");
        document.getElementById("content-title").textContent = t("defaultTitle");
        document.getElementById("content-description").textContent = showcaseTotal === 0 ? t("defaultDescriptionReferenceOnly") : t("defaultDescription");
        document.getElementById("empty-title").textContent = t("emptyTitle");
        document.getElementById("empty-description").textContent = t("emptyDescription");
        document.getElementById("preview-label").textContent = t("previewLabel");
        document.getElementById("doc-label").textContent = t("docLabel");

        document.querySelectorAll("[data-track]").forEach(function(button) {
            var track = button.getAttribute("data-track");
            button.textContent = fmtTrack(track);
            button.classList.toggle("hidden", track === "showcase" && showcaseTotal === 0);
            button.classList.toggle("active", track === state.activeTrack);
        });
    }

    function renderSidebar(visible) {
        var sidebar = document.getElementById("sidebar-list");
        var showcaseTotal = showcaseCount();
        var stats = [
            '<span class="stat-badge">' + esc(t("countReference", { count: state.allDemos.filter(function(demo) { return demo.track === "reference"; }).length })) + "</span>"
        ];

        if (showcaseTotal !== 0) {
            stats.push('<span class="stat-badge">' + esc(t("countShowcase", { count: showcaseTotal })) + "</span>");
        }
        stats.push('<span class="stat-badge accent">' + esc(t("countVisible", { count: visible.length })) + "</span>");
        document.getElementById("sidebar-stats").innerHTML = stats.join("");

        if (!visible.length) {
            sidebar.innerHTML = '<div class="sidebar-empty">' + esc(t("sidebarEmpty")) + "</div>";
            return;
        }

        var groups = {};
        visible.forEach(function(demo) {
            groups[demo.meta.category] = groups[demo.meta.category] || [];
            groups[demo.meta.category].push(demo);
        });

        var html = [];
        Object.keys(groups).sort(compareCategory).forEach(function(category) {
            var items = groups[category].slice().sort(function(a, b) {
                return fmtWords(a.meta.widget).localeCompare(fmtWords(b.meta.widget));
            });

            html.push('<section class="category-block"><div class="category-heading"><span>' + esc(fmtCategory(category)) + "</span><span>" + items.length + '</span></div><div class="nav-list">');
            items.forEach(function(demo) {
                html.push([
                    '<a href="#' + esc(demo.name) + '" class="nav-item' + (demo.name === state.selectedName ? " active" : "") + '" data-demo-name="' + esc(demo.name) + '">',
                    '<div class="nav-main"><strong>' + esc(fmtWords(demo.meta.widget)) + '</strong><span class="track-chip track-' + esc(demo.track) + '">' + esc(fmtTrack(demo.track)) + "</span></div>",
                    '<div class="nav-meta">' + esc(fmtCategory(category)) + (demo.replacement ? '<span class="nav-replacement">' + esc(demo.replacement) + "</span>" : "") + "</div>",
                    "</a>"
                ].join(""));
            });
            html.push("</div></section>");
        });

        sidebar.innerHTML = html.join("");
    }

    function ensureSelection(visible) {
        if (!visible.length) {
            state.selectedName = "";
            return;
        }
        if (visible.some(function(demo) { return demo.name === state.selectedName; })) {
            return;
        }
        if (state.initialTarget && state.demoMap[state.initialTarget] && visible.some(function(demo) { return demo.name === state.initialTarget; })) {
            state.selectedName = state.initialTarget;
            state.initialTarget = "";
            return;
        }
        state.selectedName = visible[0].name;
    }

    function renderSelection(visible, missingCatalog) {
        var empty = document.getElementById("empty-state");
        var preview = document.getElementById("preview-panel");
        var docShell = document.getElementById("doc-shell");
        var title = document.getElementById("content-title");
        var desc = document.getElementById("content-description");
        var eyebrow = document.getElementById("hero-eyebrow");
        var metaGrid = document.getElementById("meta-grid");
        var notices = document.getElementById("notice-stack");
        var actionRow = document.getElementById("action-row");

        if (missingCatalog || !visible.length || !state.selectedName || !state.demoMap[state.selectedName]) {
            empty.classList.remove("hidden");
            preview.classList.add("hidden");
            docShell.classList.add("hidden");
            title.textContent = missingCatalog ? t("missingCatalogTitle") : t("emptyTitle");
            desc.textContent = missingCatalog ? t("missingCatalogDescription") : t("emptyDescription");
            eyebrow.textContent = t("defaultEyebrow");
            metaGrid.innerHTML = "";
            notices.innerHTML = "";
            actionRow.innerHTML = "";
            document.getElementById("empty-title").textContent = title.textContent;
            document.getElementById("empty-description").textContent = desc.textContent;
            return;
        }

        var demo = state.demoMap[state.selectedName];
        empty.classList.add("hidden");
        preview.classList.remove("hidden");
        docShell.classList.remove("hidden");
        eyebrow.textContent = fmtCategory(demo.meta.category) + " / " + fmtTrack(demo.track);
        title.textContent = fmtWords(demo.meta.widget);
        desc.textContent = t("summaryTemplate", {
            category: fmtCategory(demo.meta.category),
            track: fmtTrack(demo.track)
        });
        metaGrid.innerHTML = [
            ["metaWidgetId", demo.widgetId || demo.meta.widgetId || demo.meta.widget],
            ["metaTrack", fmtTrack(demo.track)],
            ["metaVisibility", fmtVisibility(demo.visibility)],
            ["metaReference", referenceSummary(demo)],
            ["metaReplacement", demo.replacement || t("metaNone")]
        ].map(function(pair) {
            return '<div class="meta-card"><div class="meta-label">' + esc(t(pair[0])) + '</div><div class="meta-value">' + esc(pair[1]) + "</div></div>";
        }).join("");

        notices.innerHTML = [
            demo.track === "reference" ? '<div class="notice notice-positive">' + esc(t("referenceNotice")) + "</div>" : "",
            demo.track === "showcase" ? '<div class="notice notice-warning">' + esc(t("showcaseNotice")) + "</div>" : "",
            demo.track === "deprecated" ? '<div class="notice notice-warning">' + esc(t("deprecatedNotice")) + "</div>" : "",
            demo.replacement ? '<div class="notice notice-neutral">' + esc(t("replacementNotice", { replacement: demo.replacement })) + "</div>" : ""
        ].join("");

        actionRow.innerHTML = [
            '<a class="button-link" href="' + esc(demoPath(demo)) + '" target="_blank" rel="noreferrer">' + esc(t("openDemo")) + "</a>",
            demo.doc ? '<a class="button-link secondary" href="' + esc(demo.doc) + '" target="_blank" rel="noreferrer">' + esc(t("openReadme")) + "</a>" : "",
            demo.replacement && state.demoMap[demoName(demo.replacement)] ? '<button type="button" class="button-link ghost" data-target-demo="' + esc(demoName(demo.replacement)) + '">' + esc(t("openReplacement")) + "</button>" : ""
        ].join("");

        document.getElementById("preview-size").textContent = (demo.width || "?") + " x " + (demo.height || "?");
        document.getElementById("doc-status").textContent = demo.doc ? t("docBundled") : t("docMissing");

        var frame = document.getElementById("demo-frame");
        frame.style.height = frameHeight(demo) + "px";
        frame.src = demoPath(demo);
        frame.title = fmtWords(demo.meta.widget);

        var docPanel = document.getElementById("doc-panel");
        if (demo.doc) {
            renderDoc(docPanel, demo.doc);
        } else {
            docPanel.style.display = "block";
            docPanel.innerHTML = '<p class="doc-empty">' + esc(t("docMissing")) + "</p>";
        }

        try {
            var url = new URL(window.location.href);
            if (demo.widgetId) {
                url.searchParams.set("demo", demo.widgetId);
            }
            history.replaceState(null, "", url.pathname + url.search + "#" + demo.name);
        } catch (error) {
            // Ignore URL update failures.
        }
    }

    function render(missingCatalog) {
        setStaticText();
        var visible = currentDemos();
        ensureSelection(visible);
        renderSidebar(visible);
        renderSelection(visible, missingCatalog);
    }

    function initData(entries) {
        state.allDemos = entries.filter(function(entry) {
            return entry.app === "HelloCustomWidgets";
        }).map(function(entry) {
            entry.meta = parseMeta(entry);
            entry.widgetId = entry.widgetId || entry.meta.widgetId;
            entry.track = entry.track || "reference";
            entry.visibility = entry.visibility || "public";
            entry.searchText = normalize([
                entry.name,
                entry.widgetId,
                entry.meta.category,
                entry.meta.widget,
                entry.referenceComponent,
                entry.referenceLibrary,
                entry.replacement
            ].join(" "));
            return entry;
        }).sort(function(a, b) {
            return a.name.localeCompare(b.name);
        });

        state.demoMap = {};
        state.allDemos.forEach(function(demo) {
            state.demoMap[demo.name] = demo;
        });

        var hasShowcase = state.allDemos.some(function(demo) { return demo.track === "showcase"; });
        var resolvedRequestedTrack = state.requestedTrack;
        if (!hasShowcase && resolvedRequestedTrack === "showcase") {
            resolvedRequestedTrack = "reference";
        }
        if (!hasShowcase && state.activeTrack === "showcase") {
            state.activeTrack = "reference";
        }

        if (state.initialTarget && state.demoMap[state.initialTarget]) {
            state.selectedName = state.initialTarget;
            state.activeTrack = resolvedRequestedTrack || (state.demoMap[state.initialTarget].track === "deprecated" ? "all" : state.demoMap[state.initialTarget].track);
        } else if (state.allDemos.length) {
            var firstReference = state.allDemos.find(function(demo) { return demo.track === "reference"; });
            state.selectedName = (firstReference || state.allDemos[0]).name;
        }

        render(false);
    }

    document.getElementById("track-switches").addEventListener("click", function(event) {
        var button = event.target.closest("[data-track]");
        if (!button) {
            return;
        }
        state.activeTrack = button.getAttribute("data-track");
        render(false);
    });

    document.getElementById("search-input").addEventListener("input", function(event) {
        state.query = normalize(event.target.value);
        render(false);
    });

    document.getElementById("sidebar-list").addEventListener("click", function(event) {
        var link = event.target.closest("[data-demo-name]");
        if (!link) {
            return;
        }
        event.preventDefault();
        state.selectedName = link.getAttribute("data-demo-name");
        render(false);
    });

    document.getElementById("action-row").addEventListener("click", function(event) {
        var button = event.target.closest("[data-target-demo]");
        if (!button) {
            return;
        }
        state.selectedName = button.getAttribute("data-target-demo");
        state.activeTrack = state.demoMap[state.selectedName] && state.demoMap[state.selectedName].track === "deprecated"
            ? "all"
            : (state.demoMap[state.selectedName] ? state.demoMap[state.selectedName].track : "reference");
        render(false);
    });

    document.addEventListener("embeddedgui:languagechange", function() {
        render(false);
    });

    try {
        var params = new URLSearchParams(window.location.search);
        var requested = params.get("demo") || params.get("app");
        var requestedTrack = params.get("track");
        state.initialTarget = requested
            ? (requested.indexOf("HelloCustomWidgets_") === 0 ? requested : demoName(requested))
            : (window.location.hash ? window.location.hash.slice(1) : "");
        if (requestedTrack) {
            state.requestedTrack = normalizeTrack(requestedTrack);
            state.activeTrack = state.requestedTrack;
        }
    } catch (error) {
        state.initialTarget = window.location.hash ? window.location.hash.slice(1) : "";
    }

    i18n.bindLanguageToggle(document);
    render(false);
    fetch("demos/demos.json").then(function(response) {
        if (!response.ok) {
            throw new Error("Missing demos.json");
        }
        return response.json();
    }).then(initData).catch(function() {
        render(true);
    });
})();
