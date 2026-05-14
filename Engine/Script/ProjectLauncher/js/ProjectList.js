export class ProjectList {
    constructor() {
        this.cardsContainer = document.getElementById('cards');
    }

    clear() {
        if (this.cardsContainer) this.cardsContainer.innerHTML = '';
    }

    addCard(project) {
        if (!this.cardsContainer) return;
        const cardHTML = `
            <div class="card-container">
                <div class="card">
                    <div class="card-content" data-name="${project.name}" data-version="${project.version}" data-path="${project.path}">
                        ${project.name}<br>Version: ${project.version}
                    </div>
                </div>
            </div>
        `;
        this.cardsContainer.innerHTML += cardHTML;
    }

    addCards(projects) {
        projects.forEach(project => this.addCard(project));
    }

    getCardData(cardElement) {
        return {
            name: cardElement.dataset.name,
            version: cardElement.dataset.version,
            path: cardElement.dataset.path
        };
    }
}