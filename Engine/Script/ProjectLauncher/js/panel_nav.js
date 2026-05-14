function bind_panel_nav_event(){
    // 导航切换
    const navBtns = document.querySelectorAll('.nav-btn[data-panel]');
    const panels = document.querySelectorAll('.panel-card');

    navBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            // 移除所有 active
            navBtns.forEach(b => b.classList.remove('active'));
            panels.forEach(p => p.classList.remove('active'));

            // 激活当前
            btn.classList.add('active');
            const panelId = 'panel-' + btn.dataset.panel;
            document.getElementById(panelId).classList.add('active');
        });
    });
}