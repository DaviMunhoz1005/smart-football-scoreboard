const placarA = document.getElementById("placarA");
const placarB = document.getElementById("placarB");

const ws = new WebSocket(`ws://${window.location.hostname}:3000`);

ws.onopen = () => console.log("Conectado ao WebSocket!");

ws.onmessage = (event) => {
    try {
        const data = JSON.parse(event.data);
        placarA.textContent = data.TimeA;
        placarB.textContent = data.TimeB;
    } catch (e) {
        console.error("Erro ao processar mensagem WS:", e);
    }
};

ws.onerror = (err) => console.error("Erro WebSocket:", err);
