const express = require("express");
const http = require("http");
const WebSocket = require("ws");
const mqtt = require("mqtt");
const path = require("path");

const MQTT_HOST = "mqtt://54.242.249.140:1883"; // TROCAR PELO IP DA VM
const MQTT_TOPIC = "jogo/placar";

const app = express();
const server = http.createServer(app);

app.use(express.static(path.join(__dirname, "../public")));

const wss = new WebSocket.Server({ server });

wss.on("connection", ws => {
    console.log("Cliente conectado ao WebSocket!");
    ws.send(JSON.stringify({ TimeA: 0, TimeB: 0 }));
});

const client = mqtt.connect(MQTT_HOST);

client.on("connect", () => {
    console.log("Conectado ao MQTT!");
    client.subscribe(MQTT_TOPIC, err => {
        if (!err) console.log(`Inscrito no tópico: ${MQTT_TOPIC}`);
    });
});

client.on("message", (topic, message) => {
    const msg = message.toString();
    console.log("Mensagem MQTT:", msg);

    wss.clients.forEach(ws => {
        if (ws.readyState === WebSocket.OPEN) {
            ws.send(msg);
        }
    });
});

const PORT = 3000;
server.listen(PORT, () => {
    console.log(`Servidor rodando em http://localhost:${PORT}`);
});
