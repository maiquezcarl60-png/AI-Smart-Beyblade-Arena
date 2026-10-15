#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char* WIFI_SSID   = "MSI 4242";
const char* WIFI_PASS   = "111111111";
const char* LAPTOP      = "192.168.137.1";
const int   LAPTOP_PORT = 5000;

#define PIN_START 17
#define PIN_RESET 16
#define PIN_BUZZ  23
#define PIN_GREEN 4
#define PIN_BLUE  19
#define PIN_RED   18

LiquidCrystal_I2C lcd(0x27, 16, 2);

WebServer server(80);

int lastStart = HIGH, lastReset = HIGH;

unsigned long lastPress = 0;

void beep(int n, int ms = 120) {

  for (int i = 0; i < n; i++) {

    digitalWrite(PIN_BUZZ, HIGH); delay(ms);

    digitalWrite(PIN_BUZZ, LOW);  delay(ms);

  }

}

void longBeep(int ms = 600) {

  digitalWrite(PIN_BUZZ, HIGH); delay(ms);
  digitalWrite(PIN_BUZZ, LOW);
}

void leds(bool r, bool b, bool g) {

  digitalWrite(PIN_RED, r);
  digitalWrite(PIN_BLUE, b);
  digitalWrite(PIN_GREEN, g);
}

void show(const String& l1, const String& l2 = "") {

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(l1);
  lcd.setCursor(0, 1); lcd.print(l2);
}


void sendToLaptop(const char* command) {

  Serial.print("Sending to laptop: ");
  Serial.println(command);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("  -> FAILED: WiFi not connected!");
    return;
  }

  HTTPClient http;
  String url = String("http://") + LAPTOP + ":" + LAPTOP_PORT + "/api/esp32/command";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(String("{\"command\"😕"") + command + "\"}");
  Serial.print("  -> HTTP response code: ");
  Serial.println(code);
  http.end();
}
// ---- command handler ----------------------------------------------------
void handleCmd() {
  String c = server.arg("c");
  Serial.print("Received command from laptop: ");
  Serial.println(c);
  // --- COUNTDOWN:3 / COUNTDOWN:2 / COUNTDOWN:1 ---

  if (c.startsWith("COUNTDOWN:")) {
    String num = c.substring(10);          // "3", "2", "1"
    show("GET READY!", "   " + num + "...");
    leds(0, 0, 0);
    beep(1, 80);                           // short beep per count
  // --- GO --
  } else if (c == "GO") {
    show(">>> GO! <<<", "BATTLE START");
    leds(0, 0, 0);
    longBeep(500);                         // long beep = GO signal
  // --- BATTLE (running) ---
  } else if (c == "BATTLE") {
    show("BATTLE", "IN PROGRESS...");
    leds(0, 0, 0);
  // --- WINNER --
  } else if (c == "WINNER:1") {

    show("AKIRA  WINS!", ">>>> P1 <<<<");
    leds(1, 0, 0);
    beep(3, 150);

  } else if (c == "WINNER:2") {
    show("RYU    WINS!", ">>>> P2 <<<<");
    leds(0, 1, 0);
    beep(3, 150);

  // --- TIE ---

  } else if (c == "TIE") {

    show("* TIE *", "DRAW MATCH");

    leds(1, 1, 0);

    beep(2, 70);


  // --- STOP ---

  } else if (c == "STOP") {

    show("MATCH", "STOPPED");

    leds(0, 0, 0);


  // --- RESET / READY ---

  } else if (c == "RESET" || c == "READY") {

    show("AI BEYBLADE", "READY");

    leds(0, 0, 1);


  // --- PING (heartbeat) ---

  } else if (c == "PING") {

    // silent -> just reply OK

  }


  server.send(200, "text/plain", "OK");

}


// ---- setup ----------------------------------------------------------

void setup() {

  Serial.begin(115200);

  delay(500);

  Serial.println();

  Serial.println("=== AI SMART BEYBLADE ARENA - ESP32 BOOTING ===");


  pinMode(PIN_START, INPUT_PULLUP);

  pinMode(PIN_RESET, INPUT_PULLUP);

  pinMode(PIN_BUZZ,  OUTPUT);

  pinMode(PIN_GREEN, OUTPUT);

  pinMode(PIN_BLUE,  OUTPUT);

  pinMode(PIN_RED,   OUTPUT);


  lcd.init();

  lcd.backlight();

  show("AI BEYBLADE", "CONNECTING...");


  Serial.print("Connecting to WiFi: ");

  Serial.println(WIFI_SSID);


  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED) {

    delay(300);

    Serial.print(".");

    attempts++;

    if (attempts > 100) {

      Serial.println();

      Serial.println("WiFi FAILED to connect after 30 seconds!");

      Serial.println("Check WIFI_SSID / WIFI_PASS / hotspot is ON.");

      show("WIFI FAILED!", "CHECK HOTSPOT");

      attempts = 0;

    }

  }


  Serial.println();

  Serial.println("WiFi connected!");

  Serial.print("ESP32 IP address: ");

  Serial.println(WiFi.localIP());


  server.on("/cmd", handleCmd);

  server.begin();

  Serial.println("Web server started!");

  Serial.println("Waiting for commands on /cmd?c=...");

  Serial.println("================================================");


  show("AI BEYBLADE", "READY");

  leds(0, 0, 1);

  beep(2, 100);   // startup beeps = connected

}


// ---- loop ----------------------------------------------------------

void loop() {

  server.handleClient();


  int s = digitalRead(PIN_START);

  int r = digitalRead(PIN_RESET);


  if (millis() - lastPress > 300) {

    if (lastStart == HIGH && s == LOW) {

      Serial.println("START button pressed!");

      sendToLaptop("START");

      lastPress = millis();

    }

    if (lastReset == HIGH && r == LOW) {

      Serial.println("RESET button pressed!");

      sendToLaptop("RESET");

      lastPress = millis();

    }

  }


  lastStart = s;

  lastReset = r;

}
