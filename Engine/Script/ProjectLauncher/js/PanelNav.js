export class PanelNav {
    constructor() {
        this.navBtns = null;
        this.panels = null;
    }

    init() {
        this.navBtns = document.querySelectorAll('.nav-btn[data-panel]');
        this.panels = document.querySelectorAll('.panel-card');
        this.bindEvents();
    }

    bindEvents() {
        this.navBtns.forEach(btn => {
            btn.addEventListener('click', () => this.switchPanel(btn.dataset.panel));
        });
    }

    switchPanel(panelName) {
        this.navBtns.forEach(b => b.classList.remove('active'));
        this.panels.forEach(p => p.classList.remove('active'));

        const activeBtn = document.querySelector(`.nav-btn[data-panel="${panelName}"]`);
        if (activeBtn) activeBtn.classList.add('active');

        const panelId = `panel-${panelName}`;
        const activePanel = document.getElementById(panelId);
        if (activePanel) activePanel.classList.add('active');
    }
}