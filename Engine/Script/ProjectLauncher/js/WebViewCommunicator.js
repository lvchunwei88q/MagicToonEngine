export class WebViewCommunicator {
    constructor() {
        this.listeners = new Map();
        this.isReady = false;
        this.init();
    }

    init() {
        if (this.isWebViewAvailable()) {
            window.chrome.webview.addEventListener('message', this.handleMessage.bind(this));
            this.isReady = true;
        }
    }

    isWebViewAvailable() {
        return window.chrome && window.chrome.webview;
    }

    send(action, data = {}) {
        if (!this.isWebViewAvailable()) {
            console.warn('WebView is not available');
            return false;
        }

        const message = { action, ...data };

        try {
            window.chrome.webview.postMessage(JSON.stringify(message));
            return true;
        } catch (error) {
            console.error('Failed to send message:', error);
            return false;
        }
    }

    on(action, callback) {
        if (!this.listeners.has(action)) {
            this.listeners.set(action, []);
        }
        this.listeners.get(action).push(callback);
    }

    off(action, callback) {
        const callbacks = this.listeners.get(action);
        if (callbacks) {
            const index = callbacks.indexOf(callback);
            if (index > -1) {
                callbacks.splice(index, 1);
            }
        }
    }

    handleMessage(event) {
        try {
            const data = typeof event.data === 'string' ? JSON.parse(event.data) : event.data;
            const action = data.action;

            if (this.listeners.has(action)) {
                const callbacks = this.listeners.get(action);
                callbacks.forEach(callback => {
                    try {
                        callback(data);
                    } catch (error) {
                        console.error('Error in listener callback:', error);
                    }
                });
            }
        } catch (error) {
            console.error('Failed to parse message:', error);
        }
    }

    sendReady() { return this.send('web_ready'); }
    sendScanProject() { return this.send('web_scan_project'); }
    sendClose() { return this.send('web_close'); }
    sendBrowse() { return this.send('web_browse'); }
    sendCreateProject(name, path) { return this.send('create_projnew_projectct', { name, path }); }
    sendOpenProject(name, path) { return this.send('web_open_project', { name, path }); }
    sendStartDrag() { return this.send('start_drag'); }
}