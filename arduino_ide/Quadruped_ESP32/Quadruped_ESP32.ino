#include <ESP32Servo.h>
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
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>QUADRUPED CONTROLLER</title>
  <style>
    body { background-color: #0d1117; color: #58a6ff; font-family: 'Segoe UI', sans-serif; text-align: center; margin: 0; display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh; touch-action: manipulation; user-select: none; -webkit-user-select: none; }
    h2 { margin: 0 0 20px 0; text-transform: uppercase; letter-spacing: 2px; text-shadow: 0 0 10px rgba(88, 166, 255, 0.4); }
    .container { max-width: 400px; padding: 20px; }
    
    .d-pad { display: grid; grid-template-columns: repeat(3, 80px); grid-template-rows: repeat(3, 80px); gap: 10px; margin-bottom: 30px; justify-content: center; }
    .spacer { pointer-events: none; }
    
    .btn { 
      background: #21262d; border: 2px solid #30363d; border-radius: 12px; color: #c9d1d9; 
      font-size: 28px; width: 100%; height: 100%; cursor: pointer; transition: all 0.1s; 
      box-shadow: 0 4px 0 #161b22; display: flex; align-items: center; justify-content: center;
      -webkit-tap-highlight-color: transparent;
    }
    .btn:active, .btn.active { 
      background: #1f6feb; border-color: #1f6feb; color: #fff; transform: translateY(4px); box-shadow: 0 0 0 transparent; 
    }
    
    .actions-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; width: 100%; max-width: 300px; }
    .btn-act { 
      background: #238636; border: none; padding: 12px 5px; border-radius: 6px; color: white; 
      font-weight: bold; font-size: 14px; box-shadow: 0 3px 0 #1a6328; cursor: pointer;
    }
    .btn-act:active { transform: translateY(3px); box-shadow: none; }
    .btn-stop { background: #da3633; box-shadow: 0 3px 0 #9e2320; }

    p { font-size: 12px; color: #8b949e; margin-top: 25px; }
  </style>
</head>
<body>
  <h2>KAME ROBOT</h2>
  
  <div class="d-pad">
    <div class="spacer"></div>
    <div class="btn" ontouchstart="start(1)" onmousedown="start(1)" ontouchend="stop()" onmouseup="stop()">&#9650;</div>
    <div class="spacer"></div>
    
    <div class="btn" ontouchstart="start(3)" onmousedown="start(3)" ontouchend="stop()" onmouseup="stop()">&#9664;</div>
    <div class="btn btn-stop" onclick="stop()">&#9632;</div>
    <div class="btn" ontouchstart="start(4)" onmousedown="start(4)" ontouchend="stop()" onmouseup="stop()">&#9654;</div>
    
    <div class="spacer"></div>
    <div class="btn" ontouchstart="start(2)" onmousedown="start(2)" ontouchend="stop()" onmouseup="stop()">&#9660;</div>
    <div class="spacer"></div>
  </div>

  <div class="actions-grid">
    <button class="btn-act" onclick="act(6)">Heart</button>
    <button class="btn-act" onclick="act(7)">Fire</button>
    <button class="btn-act" onclick="act(8)">Jump</button>
    <button class="btn-act" onclick="act(9)">Hello</button>
    <button class="btn-act" onclick="act(10)">Punch</button>
    <button class="btn-act" onclick="act(11)">Dance</button>
  </div>
  
  <p>DevPool Robot Control</p>

<script>
  // Chặn menu chuột phải để trải nghiệm như App
  window.oncontextmenu = function(e) { e.preventDefault(); e.stopPropagation(); return false; };

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
    
    // Nếu là lệnh di chuyển (1-4) hoặc dừng (5)
    if (cmd >= 1 && cmd <= 5) {
      if (cmd == 5) {
        currentMove = 0;
        robot.home();
      } else {
        currentMove = cmd; // Lưu trạng thái di chuyển
        isPerformingAction = false;
      }
    }
    // Nếu là lệnh hành động đặc biệt (6 trở đi)
    else if (cmd >= 6) {
      currentMove = 0; // Dừng di chuyển thường
      isPerformingAction = true;
      processSpecialAction(cmd);
      isPerformingAction = false;
    }
  }
  server.send(200, "text/plain", "OK");
}

void processSpecialAction(int cmd) {
    switch (cmd) {
        case 6: robot.pushUp(2, 2000); break; // HEART
        case 7: robot.upDown(4, 250); break;  // FIRE
        case 8: robot.jump(); break;          // SKULL/JUMP
        case 9: robot.hello(); break;         // CROSS/HELLO
        case 10: robot.frontBack(4, 200); break; // PUNCH
        case 11: robot.dance(2, 1000); break;    // MASK/DANCE
    }
    robot.home(); // Về vị trí chuẩn sau khi hành động
}

// ================= SETUP ====================
void setup() {
    Serial.begin(115200);
    
    // Cài đặt WiFi AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    IPAddress ip = WiFi.softAPIP();
    
    Serial.print("AP IP address: ");
    Serial.println(ip);

    // Cài đặt Robot
    robot.init();
    robot.home();
    delay(500);

    // Cài đặt Web Server
    server.on("/", handleRoot);
    server.on("/cmd", handleCommand);
    server.begin();
    Serial.println("Web server started");
}

// ================= LOOP =====================
void loop() {
    server.handleClient();

    // Xử lý di chuyển liên tục khi giữ nút
    if (!isPerformingAction && currentMove != 0) {
        switch (currentMove) {
            case 1: robot.walk(1, 550); break;  // UP
            case 2: robot.walk(-1, 550); break; // DOWN
            case 3: robot.turnL(1, 550); break; // LEFT
            case 4: robot.turnR(1, 550); break; // RIGHT
        }
    } 
    else {
        // Nếu không làm gì thì delay nhẹ để giảm tải
        // Không gọi robot.home() liên tục để tránh giật servo
        delay(10);
    }
}
