function new_project() {
    const path = document.getElementById("projectPath").value.trim();
    if (!path) {
        alert("请输入项目路径");
        return;
    }
    // 通过 WebView2 的 chrome.webview 接口发送消息到 C++
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage(JSON.stringify({
            action: "create_projnew_projectct",
            name: "MyGame",
            path: path
        }));
    }
}