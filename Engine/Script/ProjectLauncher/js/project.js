export class Project {
    constructor(communicator, popup, projectList) {
        this.communicator = communicator;
        this.popup = popup;
        this.projectList = projectList;
        this.engineVersion = '0.0.0.0';
    }

    init() { this.bindEvents(); }

    isValidPath(str) {
        if (!str || typeof str !== 'string') return false;
        str = str.trim();
        if (!str) return false;
        const illegalChars = /[<>"|?*]/;
        if (illegalChars.test(str)) return false;
        const pathRegex = /^([a-zA-Z]:[\\\/]|\\\\|\.{1,2}[\\\/]|\/[^\/])/;
        return pathRegex.test(str);
    }

    create() {
        const path = document.getElementById('projectPath').value.trim();
        const name = document.getElementById('projectName').value.trim();

        if (!path) {
            this.popup.show('请输入项目路径', { type: 'warning' });
            return;
        }
        if (!this.isValidPath(path)) {
            this.popup.show('请输入正确项目路径', { type: 'warning' });
            return;
        }
        if (!name) {
            this.popup.show('请输入项目名称', { type: 'warning' });
            return;
        }

        this.communicator.sendCreateProject(name, path);
    }

    open(context) {
        this.communicator.sendOpenProject(context.name, context.path);
    }

    browse() { this.communicator.sendBrowse(); }

    bindEvents() {
        this.communicator.on('project_list', (data) => {
            const lists = data.list;
            this.projectList.clear();
            lists.forEach(element => {
                this.projectList.addCard({
                    name: element.name,
                    version: element.version,
                    path: element.path
                });
            });
        });

        this.communicator.on('engine_version', (data) => {
            this.engineVersion = data.version;
            document.querySelectorAll('.version').forEach(element => {
                element.innerHTML = this.engineVersion;
            });
        });

        this.communicator.on('browse_path', (data) => {
            document.getElementById('projectPath').value = data.path;
        });

        this.communicator.on('project_create_success', () => {
            this.popup.show('项目创建成功', { type: 'success', title: '完成', duration: 2500 });
            document.getElementById('projectPath').value = '';
            document.getElementById('projectName').value = '';
        });

        this.communicator.on('project_refresh', () => {
            this.popup.show('项目出现变动', { type: 'warning', title: '注意', duration: 5000 });
        });

        this.communicator.on('project_existence', () => {
            this.popup.show('项目创建失败，已存在相同的项目！', { type: 'error', title: '注意', duration: 5000 });
        });
    }
}