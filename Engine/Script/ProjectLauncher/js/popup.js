export class Popup {
    constructor() {
        this.popup = null;
        this.timeoutId = null;
        this.initStyles();
    }

    initStyles() {
        if (document.getElementById('simple-popup-styles')) return;

        const style = document.createElement('style');
        style.id = 'simple-popup-styles';
        style.textContent = `
            .simple-popup-overlay{position:fixed;top:0;left:0;width:100vw;height:100vh;background:rgba(0,0,0,0);z-index:10000;display:flex;justify-content:center;align-items:center;pointer-events:none}.simple-popup-content{background:rgba(30,30,40,0.95);border:1px solid rgba(255,255,255,0.1);border-radius:8px;padding:16px 20px;box-shadow:0 10px 25px -5px rgba(0,0,0,0.3);max-width:min(90vw,400px);min-width:280px;position:relative;color:#c8c8d8;font-family:system-ui,sans-serif;pointer-events:auto}.simple-popup-header{display:flex;align-items:center;gap:8px;margin-bottom:12px;padding-right:24px}.simple-popup-icon{width:22px;height:22px;flex-shrink:0}.simple-popup-title{font-size:16px;font-weight:600;margin:0;color:#e0e0f0;line-height:1.4}.simple-popup-body{font-size:14px;line-height:1.5;color:#8888a0;word-break:break-word}.simple-popup-close{position:absolute;top:12px;right:12px;background:transparent;border:none;border-radius:4px;width:24px;height:24px;display:flex;align-items:center;justify-content:center;font-size:18px;cursor:pointer;color:#8888a0;padding:0;line-height:1;transition:all .2s}.simple-popup-close:hover{background:rgba(255,255,255,0.1);color:#e0e0f0}.simple-popup-progress{position:absolute;bottom:0;left:0;height:3px;background:#7864dc;border-radius:0 0 0 8px;animation:progress-shrink linear forwards}@keyframes progress-shrink{from{width:100%}to{width:0%}}.simple-popup-content.info .simple-popup-icon{color:#7864dc}.simple-popup-content.info .simple-popup-progress{background:#7864dc}.simple-popup-content.warning .simple-popup-icon{color:#F59E0B}.simple-popup-content.warning .simple-popup-progress{background:#F59E0B}.simple-popup-content.error .simple-popup-icon{color:#EF4444}.simple-popup-content.error .simple-popup-progress{background:#EF4444}.simple-popup-content.success .simple-popup-icon{color:#10B981}.simple-popup-content.success .simple-popup-progress{background:#10B981}@media (max-width:768px){.simple-popup-content{margin:16px;max-width:calc(100vw - 32px);padding:14px 16px}}
        `;
        document.head.appendChild(style);
    }

    show(content, options = {}) {
        this.close();

        const config = {
            type: options.type || 'info',
            title: options.title || this.getDefaultTitle(options.type),
            duration: options.duration !== undefined ? options.duration : 3000,
            x: options.x || null,
            y: options.y || null,
            width: options.width || null,
            className: options.className || ''
        };

        this.createPopup(content, config);
    }

    getDefaultTitle(type) {
        const titles = { info: '提示', warning: '警告', error: '错误', success: '成功' };
        return titles[type] || '提示';
    }

    getIconSVG(type) {
        const icons = {
            info: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><line x1="12" y1="16" x2="12" y2="12"/><circle cx="12" cy="8" r="1"/></svg>`,
            warning: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 7v6M12 17h.01"/><path d="M12 22C6.477 22 2 17.523 2 12S6.477 2 12 2s10 4.477 10 10-4.477 10-10 10z"/></svg>`,
            error: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg>`,
            success: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"/><polyline points="22 4 12 14.01 9 11.01"/></svg>`
        };
        return icons[type] || icons.info;
    }

    createPopup(content, options) {
        const overlay = document.createElement('div');
        overlay.className = 'simple-popup-overlay';

        const contentContainer = document.createElement('div');
        contentContainer.className = `simple-popup-content ${options.type} ${options.className}`;

        if (options.width) contentContainer.style.width = options.width;

        let popupHTML = `
            <div class="simple-popup-header">
                <span class="simple-popup-icon">${this.getIconSVG(options.type)}</span>
                <h3 class="simple-popup-title">${options.title}</h3>
            </div>
            <div class="simple-popup-body"></div>
        `;

        contentContainer.innerHTML = popupHTML;

        const bodyContainer = contentContainer.querySelector('.simple-popup-body');
        if (typeof content === 'string') {
            bodyContainer.innerHTML = content;
        } else if (content instanceof HTMLElement) {
            bodyContainer.appendChild(content);
        }

        const closeBtn = document.createElement('button');
        closeBtn.className = 'simple-popup-close';
        closeBtn.innerHTML = '×';
        closeBtn.setAttribute('aria-label', '关闭');
        closeBtn.onclick = () => this.close();
        contentContainer.appendChild(closeBtn);

        if (options.duration > 0) {
            const progress = document.createElement('div');
            progress.className = 'simple-popup-progress';
            progress.style.animationDuration = `${options.duration}ms`;
            contentContainer.appendChild(progress);

            this.timeoutId = setTimeout(() => this.close(), options.duration);
        }

        if (options.x !== null && options.y !== null) {
            contentContainer.style.position = 'absolute';
            contentContainer.style.left = `${options.x}px`;
            contentContainer.style.top = `${options.y}px`;
            contentContainer.style.transform = 'none';
            contentContainer.style.margin = '0';
        } else {
            contentContainer.style.position = 'fixed';
            contentContainer.style.left = '50%';
            contentContainer.style.top = '5%';
            contentContainer.style.transform = 'translateX(-50%)';
            contentContainer.style.margin = '0';
        }

        overlay.appendChild(contentContainer);
        document.body.appendChild(overlay);

        this.popup = { overlay, timeoutId: this.timeoutId };
    }

    close() {
        if (this.timeoutId) {
            clearTimeout(this.timeoutId);
            this.timeoutId = null;
        }
        if (this.popup) {
            const { overlay } = this.popup;
            if (overlay && overlay.parentNode) {
                overlay.parentNode.removeChild(overlay);
            }
            this.popup = null;
        }
    }

    isOpen() { return this.popup !== null; }
}