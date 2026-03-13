#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "minikame.h"

// ================= WIFI AP =================
const char* ssid = "KAME_ROBOT";
const char* password = "12345678";

// ================= GLOBAL ==================
MiniKame robot;
WebServer server(80);

int currentMove = 0; // 0: Dừng, 1: Tiến, 2: Lùi, 3: Trái, 4: Phải
bool isPerformingAction = false;

// ================= HTML GIAO DIỆN UI =================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no, viewport-fit=cover">
  <title>SPIDER ROBOT CONTROLLER</title>
  <style>
    :root {
      --bg: #0d1117;
      --accent: #58a6ff;
      --accent-glow: rgba(88, 166, 255, 0.3);
      --btn-bg: rgba(33, 38, 45, 0.8);
      --btn-border: #30363d;
      --red: #ff4b4b;
      --green: #238636;
    }

    * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }

    body { 
      background: radial-gradient(circle at center, #161b22 0%, #0d1117 100%);
      color: #c9d1d9;
      font-family: 'Segoe UI', Roboto, Helvetica, sans-serif;
      margin: 0; padding: 0;
      height: 100vh; width: 100vw;
      display: flex; flex-direction: column;
      align-items: center; justify-content: center;
      overflow: hidden;
      user-select: none;
    }

    h1 { 
      font-size: 1.2rem; margin: 10px 0; letter-spacing: 3px; color: var(--accent);
      text-shadow: 0 0 10px var(--accent-glow); text-transform: uppercase;
    }

    /* Layout chính */
    .controller {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 20px;
      width: 100%;
      max-width: 900px;
      padding: 20px;
    }

    /* Chế độ xoay ngang */
    @media (orientation: landscape) {
      .controller {
        flex-direction: row;
        justify-content: space-between;
        padding: 0 40px;
      }
      h1 { position: absolute; top: 10px; width: 100%; text-align: center; }
    }

    /* D-PAD Style */
    .d-pad {
      display: grid;
      grid-template-columns: repeat(3, 70px);
      grid-template-rows: repeat(3, 70px);
      gap: 8px;
    }

    .btn {
      background: var(--btn-bg);
      border: 1px solid var(--btn-border);
      border-radius: 12px;
      display: flex; align-items: center; justify-content: center;
      color: white; font-size: 24px;
      cursor: pointer; transition: 0.1s;
      box-shadow: 0 4px 10px rgba(0,0,0,0.3);
    }

    .btn:active {
      transform: scale(0.95) translateY(2px);
      background: var(--accent);
      box-shadow: 0 0 15px var(--accent-glow);
    }

    .btn-stop { background: var(--red); border-color: #800; font-size: 18px; font-weight: bold; }
    .btn-stop:active { background: #ff0000; }

    /* Actions Style */
    .actions-section {
      display: flex;
      flex-direction: column;
      gap: 10px;
      width: 100%;
      max-width: 320px;
    }

    .actions-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 8px;
    }

    .btn-act {
      background: rgba(48, 54, 61, 0.6);
      border: 1px solid var(--btn-border);
      padding: 12px 5px;
      border-radius: 8px;
      color: #8b949e;
      font-size: 12px; font-weight: bold;
      cursor: pointer; transition: 0.2s;
    }

    .btn-act:active {
      background: var(--green);
      color: white;
      transform: translateY(-2px);
    }

    .spacer { pointer-events: none; }
    
    .status-bar {
      font-size: 10px; color: #484f58; margin-top: 15px;
      text-transform: uppercase; letter-spacing: 1px;
    }
  </style>
</head>
<body>
  <h1>Spider Robot</h1>
  
  <div class="controller">
    <!-- LEFT: MOVEMENT -->
    <div class="d-pad">
      <div class="spacer"></div>
      <div class="btn" ontouchstart="start(1)" onmousedown="start(1)" ontouchend="stop()" onmouseup="stop()">&#9650;</div>
      <div class="spacer"></div>
      
      <div class="btn" ontouchstart="start(3)" onmousedown="start(3)" ontouchend="stop()" onmouseup="stop()">&#9664;</div>
      <div class="btn btn-stop" onclick="stop()">STOP</div>
      <div class="btn" ontouchstart="start(4)" onmousedown="start(4)" ontouchend="stop()" onmouseup="stop()">&#9654;</div>
      
      <div class="spacer"></div>
      <div class="btn" ontouchstart="start(2)" onmousedown="start(2)" ontouchend="stop()" onmouseup="stop()">&#9660;</div>
      <div class="spacer"></div>
    </div>

    <!-- RIGHT: ACTIONS -->
    <div class="actions-section">
      <div class="actions-grid">
        <button class="btn-act" onclick="act(6)">HEART</button>
        <button class="btn-act" onclick="act(7)">FIRE</button>
        <button class="btn-act" onclick="act(8)">JUMP</button>
        <button class="btn-act" onclick="act(9)">HELLO</button>
        <button class="btn-act" onclick="act(10)">PUNCH</button>
        <button class="btn-act" onclick="act(11)">DANCE</button>
      </div>
      <div class="actions-grid">
        <button class="btn-act" onclick="act(12)">LEG FL</button>
        <button class="btn-act" onclick="act(13)">LEG FR</button>
        <button class="btn-act" onclick="act(14)">LEG BL</button>
        <button class="btn-act" onclick="act(15)">LEG BR</button>
      </div>
    </div>
  </div>
  
  <div class="status-bar">Connection: Active | System Ready</div>

<script>
  // Chặn menu chuột phải & zoom
  window.oncontextmenu = function(e) { e.preventDefault(); return false; };
  
  function start(val) { fetch('/cmd?val=' + val); }
  function stop() { fetch('/cmd?val=5'); }
  function act(val) { fetch('/cmd?val=' + val); }
</script>
</body>
</html>
)rawliteral";

// ================= HÀM XỬ LÝ SERVER =================
void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleCommand() {
  if (server.hasArg("val")) {
    int cmd = server.arg("val").toInt();
    
    if (cmd >= 1 && cmd <= 5) {
      if (cmd == 5) {
        currentMove = 0;
        robot.home();
      } else {
        currentMove = cmd; // Lưu trạng thái di chuyển
        isPerformingAction = false;
      }
    }
    else if (cmd >= 6 && cmd <= 15) {
      currentMove = 0;
      isPerformingAction = true;
      processSpecialAction(cmd);
      isPerformingAction = false;
    }
  }
  server.send(200, "text/plain", "OK");
}

void processSpecialAction(int cmd) {
    switch (cmd) {
        case 6: robot.pushUp(2, 2000); break; 
        case 7: robot.upDown(4, 250); break;  
        case 8: robot.jump(); break;          
        case 9: robot.hello(); break;         
        case 10: robot.frontBack(4, 200); break;
        case 11: robot.dance(2, 1000); break;
        case 12: robot.testLeg(0); break; // FL
        case 13: robot.testLeg(1); break; // FR
        case 14: robot.testLeg(2); break; // BL
        case 15: robot.testLeg(3); break; // BR
    }
    robot.home();
}

// ================= PIN CONFIG ================
#define BUZZER_PIN 16 

int melody[] = {
  659, 659, 0, 659,
  0, 523, 659, 0,
  784, 0, 0, 0,
  392, 0, 0, 0
};

int noteDurations[] = {
  150, 150, 150, 150,
  150, 150, 150, 150,
  300, 150, 150, 150,
  300, 150, 150, 150
};

// ================= SETUP ====================
void setup() {
    Serial.begin(115200);
    
    // Khởi tạo buzzer bằng LEDC (Tương thích ESP32 Core V3)
    ledcAttach(BUZZER_PIN, 2000, 8); // Chân, Tần số, Độ phân giải 8-bit

    // Cài đặt WiFi AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    IPAddress ip = WiFi.softAPIP();
    
    Serial.print("AP IP address: ");
    Serial.println(ip);

    // Cài đặt Robot (kèm I2C cho PCA9685)
    robot.init();
    robot.home();
    delay(500);

    // Cài đặt Web Server
    server.on("/", handleRoot);
    server.on("/cmd", handleCommand);
    server.begin();
    Serial.println("Web server started");

    // Phát nhạc khởi động (Super Mario Intro)
    for (int i = 0; i < 16; i++) {
        if (melody[i] > 0) {
            ledcWriteTone(BUZZER_PIN, melody[i]);
            ledcWrite(BUZZER_PIN, 128); // Kích hoạt âm thanh với duty cycle 50%
        } else {
            ledcWrite(BUZZER_PIN, 0); // Tắt âm thanh
        }
        delay(noteDurations[i] * 1.3);
        ledcWrite(BUZZER_PIN, 0); // Ngắt nốt nhạc
    }
}

// ================= LOOP =====================
void loop() {
    server.handleClient();

    if (!isPerformingAction && currentMove != 0) {
        switch (currentMove) {
            case 1: robot.walk(1, 550); break;  // UP
            case 2: robot.walk(-1, 550); break; // DOWN
            case 3: robot.turnL(1, 550); break; // LEFT
            case 4: robot.turnR(1, 550); break; // RIGHT
        }
    } 
    else {
        delay(10);
    }
}
