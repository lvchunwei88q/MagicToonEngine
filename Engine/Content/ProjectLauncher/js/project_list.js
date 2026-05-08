const card_context = {
    name: "",
    version: ""
};

function new_project_card(card_context){
    document.getElementById("cards").innerHTML += 
    `
        <div class="card-container">
            <div id="card">
                <div class="card-content">
                ${card_context.name}<br>Version: ${card_context.version}
                </div>
            </div>
        </div>
    `;
}