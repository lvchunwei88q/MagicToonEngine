let engine_version = "0.0.0.0";

function new_project() {
    const path = document.getElementById("projectPath").value.trim();
    const name = document.getElementById("projectName").value.trim();
    if (!path) {
        alert("请输入项目路径");
        return;
    }
    if (!name) {
        alert("请输入项目名称");
        return;
    }

    // 通过 WebView2 的 chrome.webview 接口发送消息到 C++
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage(JSON.stringify({
            action: "create_projnew_projectct",
            name: name,
            path: path
        }));
    }
}

function bind_project_list() {
    window.chrome.webview.addEventListener('message', (event) => {
        const data = event.data;
        
        if (data.action === 'project_list') {
            const lists = data.list;
            claer_project_card();
            for (let index = 0; index < lists.length; index++) {
                const element = lists[index];
                new_project_card({
                    name: element.name,
                    version: element.version
                });
            }
        }
    });
}

function bind_engine_version() {
    window.chrome.webview.addEventListener('message', (event) => {
        const data = event.data;
        
        if (data.action === 'engine_version') {
            const version = data.version;
            engine_version = version;

            document.querySelectorAll(".version").forEach(element => {
                element.innerHTML = engine_version;
            }); 
        }
    });
}

function bind_browse_path() {
    window.chrome.webview.addEventListener('message', (event) => {
        const data = event.data;
        
        if (data.action === 'browse_path') {
            const path = data.path;
            document.getElementById("projectPath").value = path;
        }
    });
}

function browse(){
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage(JSON.stringify({
            action: "web_browse"
        }));
    }
}