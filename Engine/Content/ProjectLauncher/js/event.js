function send_to_cpp_ready(){
    setTimeout(()=>{
        if (window.chrome && window.chrome.webview) {
            window.chrome.webview.postMessage(JSON.stringify({
                action: "web_ready"
            }));
        }
    },100);
}

function bind_windows_event() {
    // 监听鼠标左键
    document.addEventListener('mousedown', (e) => {
        if (e.button === 0) { // 左键
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