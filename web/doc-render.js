function getDocLoadingText() {
    if (window.EmbeddedGUII18n) {
        return window.EmbeddedGUII18n.format({
            "zh-CN": "加载中...",
            "en": "Loading..."
        });
    }
    return "Loading...";
}

function fetchUtf8Text(docPath) {
    return fetch(docPath)
        .then(function(response) {
            if (!response.ok) {
                throw new Error("Not found");
            }
            return response.arrayBuffer();
        })
        .then(function(buffer) {
            return new TextDecoder("utf-8").decode(buffer);
        });
}

function renderDoc(container, docPath) {
    if (!docPath) {
        container.innerHTML = "";
        container.style.display = "none";
        return;
    }

    container.style.display = "block";
    container.innerHTML = '<p class="doc-loading">' + getDocLoadingText() + "</p>";
    fetchUtf8Text(docPath)
        .then(function(markdown) {
            container.innerHTML = marked.parse(markdown);
        })
        .catch(function() {
            container.innerHTML = "";
            container.style.display = "none";
        });
}

function renderDocSummary(container, docPath) {
    if (!docPath) {
        container.innerHTML = "";
        container.style.display = "none";
        return;
    }

    container.style.display = "block";
    fetchUtf8Text(docPath)
        .then(function(markdown) {
            var lines = markdown.split("\n");
            var paragraph = "";
            for (var index = 0; index < lines.length; index += 1) {
                var line = lines[index].trim();
                if (!line || line.charAt(0) === "#" || line.charAt(0) === "-" || line.charAt(0) === "|") {
                    continue;
                }
                paragraph = line;
                break;
            }
            if (paragraph) {
                container.innerHTML = "<p>" + paragraph + "</p>";
            } else {
                container.innerHTML = "";
                container.style.display = "none";
            }
        })
        .catch(function() {
            container.innerHTML = "";
            container.style.display = "none";
        });
}
