function send_to_cpp_ready(){
    setTimeout(()=>{
        if (window.chrome && window.chrome.webview) {
            window.chrome.webview.postMessage(JSON.stringify({
                action: "web_ready"
            }));
        }
    },100);
    setTimeout(()=>{ // 扫描项目完整性
        if (window.chrome && window.chrome.webview) {
            window.chrome.webview.postMessage(JSON.stringify({
                action: "web_scan_project"
            }));
        }
    },200);
}

function bind_windows_event() {
    // 监听鼠标左键
    document.addEventListener('mousedown', (e) => {
        if (e.button === 0) { // 左键
            const excludeTags = ['INPUT', 'TEXTAREA', 'BUTTON', 'SELECT', 'A'];
            if (excludeTags.includes(e.target.tagName)) {
                return;  // 跳过
            }

            // 通知 C++ 开始拖动
            if (window.chrome && window.chrome.webview) {
                window.chrome.webview.postMessage(JSON.stringify({
                    action: "start_drag"
                }));
            }
        }
    });

    // 阻止默认右键菜单
    document.addEventListener('contextmenu', (e) => {
        e.preventDefault();
    });
}

function Close(){
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage(JSON.stringify({
            action: "web_close"
        }));
    }
}

function bind_project_card_click(){
    document.getElementById('cards').addEventListener('click', (e) => {
        const card = e.target.closest('.card-content');
        if (card) {
            const context = {
                name:card.dataset.name,
                version:card.dataset.version,
                path:card.dataset.path,
            } 
            open_project(context);
        }
    });
}