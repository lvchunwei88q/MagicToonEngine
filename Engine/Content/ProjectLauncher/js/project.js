let engine_version = "0.0.0.0";

function isValidPath(str) {
    if (!str || typeof str !== 'string') return false;
    
    str = str.trim();
    if (!str) return false;
    
    // 检查非法字符
    const illegalChars = /[<>"|?*]/;
    if (illegalChars.test(str)) return false;
    
    // 检查是否为路径格式
    const pathRegex = /^([a-zA-Z]:[\\\/]|\\\\|\.{1,2}[\\\/]|\/[^\/])/;
    return pathRegex.test(str);
}

function new_project() {
    const path = document.getElementById("projectPath").value.trim();
    const name = document.getElementById("projectName").value.trim();
    if (!path) {
        popup.show('请输入项目路径', { 
            type: 'warning',
        });
        return;
    }
    if (!isValidPath(path)) {
        popup.show('请输入正确项目路径', { 
            type: 'warning',
        });
        return;
    }
    if (!name) {
        popup.show('请输入项目名称', { 
            type: 'warning',
        });
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
                    version: element.version,
                    path: element.path
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

function bind_project_event() {
    window.chrome.webview.addEventListener('message', (event) => {
        const data = event.data;
        
        if (data.action === 'project_create_success') {
            popup.show('项目创建成功', { 
                type: 'success',
                title: '完成' ,
                duration: 2500
            });
            document.getElementById("projectPath").value = "";
            document.getElementById("projectName").value = "";
        }else if(data.action === 'project_refresh'){
            popup.show('项目出现变动', { 
                type: 'warning',
                title: '注意' ,
                duration: 5000
            });
        }else if(data.action === 'project_existence'){
            popup.show('项目创建失败，已存在相同的项目！', { 
                type: 'error',
                title: '注意' ,
                duration: 5000
            });
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

// open project
function open_project(context){
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage(JSON.stringify({
            action: "web_open_project",
            path:context.path,
            name:context.name,
        }));
    }
}