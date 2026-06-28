#include <WiFi.h>
#include <WebSocketsServer.h>

const char* ssid = "sweethome";
const char* password = "Starlite$1812";

WebSocketsServer webSocket(81);
HardwareSerial ArduinoSerial(1);

const char webpage[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Robot Control</title>
</head>
<body>
<h1>Robot Controller</h1>

<button onclick="sendCmd('F')">Forward</button><br><br>
<button onclick="sendCmd('L')">Left</button>
<button onclick="sendCmd('R')">Right</button><br><br>
<button onclick="sendCmd('B')">Backward</button><br><br>
<button onclick="sendCmd('S')">STOP</button>

<script>
let ws = new WebSocket("ws://" + location.hostname + ":81");

function sendCmd(cmd) {
    ws.send(cmd);
}
</script>

</body>
</html>
)rawliteral";

WiFiServer server(80);

void onWebSocketEvent(uint8_t num,
                      WStype_t type,
                      uint8_t *payload,
                      size_t length) {

    if (type == WStype_TEXT && length > 0) {
        char cmd = payload[0];

        Serial.print("Sending to Arduino: ");
        Serial.println(cmd);

        ArduinoSerial.write(cmd);
    }
}

void setup() {
    Serial.begin(115200);

    ArduinoSerial.begin(9600, SERIAL_8N1, -1, 17);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    server.begin();

    webSocket.begin();
    webSocket.onEvent(onWebSocketEvent);
}

void loop() {
    webSocket.loop();

    WiFiClient client = server.available();

    if (client) {

        while (client.connected() && !client.available()) {
            delay(1);
        }

        while (client.available()) {
            client.read();
        }

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println();
        client.println(webpage);

        client.stop();
    }
}
