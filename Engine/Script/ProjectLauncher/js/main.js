import { WebViewCommunicator } from './WebViewCommunicator.js';
import { Popup } from './Popup.js';
import { PanelNav } from './PanelNav.js';
import { ProjectList } from './ProjectList.js';
import { Project } from './Project.js';

class App {
    constructor() {
        this.communicator = new WebViewCommunicator();
        this.popup = new Popup();
        this.panelNav = new PanelNav();
        this.projectList = new ProjectList();
        this.project = new Project(this.communicator, this.popup, this.projectList);
    }

    init() {
        this.panelNav.init();
        this.project.init();
        this.bindWindowEvents();
        this.bindCardClick();
        this.sendReady();
    }

    bindWindowEvents() {
        document.addEventListener('mousedown', (e) => {
            if (e.button === 0) {
                const excludeTags = ['INPUT', 'TEXTAREA', 'BUTTON', 'SELECT', 'A'];
                if (excludeTags.includes(e.target.tagName)) {
                    return;
                }
                this.communicator.sendStartDrag();
            }
        });

        document.addEventListener('contextmenu', (e) => {
            e.preventDefault();
        });
    }

    bindCardClick() {
        const cardsContainer = document.getElementById('cards');
        if (cardsContainer) {
            cardsContainer.addEventListener('click', (e) => {
                const card = e.target.closest('.card-content');
                if (card) {
                    const context = this.projectList.getCardData(card);
                    this.project.open(context);
                }
            });
        }
    }

    sendReady() {
        setTimeout(() => {
            this.communicator.sendReady();
        }, 100);

        setTimeout(() => {
            this.communicator.sendScanProject();
        }, 200);
    }

    close() {
        this.communicator.sendClose();
    }
}

const app = new App();

document.addEventListener('DOMContentLoaded', () => {
    app.init();
});

window.new_project = () => app.project.create();
window.browse = () => app.project.browse();
window.Close = () => app.close();