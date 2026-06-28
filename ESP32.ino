#include <WiFi.h>

const char* ssid = "WIFINAME";
const char* password = "WIFIPASSWORD";

WiFiServer server(80);

// TX only: GPIO17 -> Arduino D12
HardwareSerial ArduinoSerial(1);

const char webpage[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Robot Controller</title>

<style>
button {
    width: 150px;
    height: 80px;
    font-size: 24px;
    margin: 10px;
}
body {
    text-align: center;
    font-family: Arial;
}
</style>

</head>
<body>

<h1>Robot Controller</h1>

<button
onmousedown="sendCmd('F')"
onmouseup="sendCmd('S')"
ontouchstart="sendCmd('F')"
ontouchend="sendCmd('S')">
Forward
</button>

<br>

<button
onmousedown="sendCmd('L')"
onmouseup="sendCmd('S')"
ontouchstart="sendCmd('L')"
ontouchend="sendCmd('S')">
Left
</button>

<button
onmousedown="sendCmd('R')"
onmouseup="sendCmd('S')"
ontouchstart="sendCmd('R')"
ontouchend="sendCmd('S')">
Right
</button>

<br>

<button
onmousedown="sendCmd('B')"
onmouseup="sendCmd('S')"
ontouchstart="sendCmd('B')"
ontouchend="sendCmd('S')">
Backward
</button>

<br><br>

<button onclick="sendCmd('S')">
STOP
</button>

<script>
function sendCmd(cmd) {
    fetch('/' + cmd)
        .catch(err => console.log(err));
}
</script>

</body>
</html>
)rawliteral";

void setup() {

    Serial.begin(115200);

    ArduinoSerial.begin(9600, SERIAL_8N1, -1, 17);

    Serial.println("Connecting to WiFi...");

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.begin();

    Serial.println("Server started!");
}

void loop() {

    WiFiClient client = server.available();

    if (!client) return;

    client.setTimeout(10);

    String request = client.readStringUntil('\r');

    Serial.println(request);

    if (request.indexOf("GET /F") >= 0) {
        ArduinoSerial.write('F');
        Serial.println("Sent F");
    }
    else if (request.indexOf("GET /B") >= 0) {
        ArduinoSerial.write('B');
        Serial.println("Sent B");
    }
    else if (request.indexOf("GET /L") >= 0) {
        ArduinoSerial.write('L');
        Serial.println("Sent L");
    }
    else if (request.indexOf("GET /R") >= 0) {
        ArduinoSerial.write('R');
        Serial.println("Sent R");
    }
    else if (request.indexOf("GET /S") >= 0) {
        ArduinoSerial.write('S');
        Serial.println("Sent S");
    }

    while (client.available()) {
        client.read();
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();

    if (request.indexOf("GET / ") >= 0) {
        client.println(webpage);
    }
    else {
        client.println("OK");
    }

    client.stop();
}
