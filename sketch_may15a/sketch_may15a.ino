#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Arduino_JSON.h>

#define BUAD_RATE 115200
#define WATER_PIN A0
#define PUMPER_PIN LED_BUILTIN

String SID = "rlmygtt";
String WiFi_PASSWORD = "12345678";

class Conf {
  public:
  int check_water_interval;
  int water_low;
  int hold_once_ms;
  int turn_on_once_ms;
  int water_event_trigger_step;
  Conf() {
    check_water_interval = 500;
    water_low = 100;
    hold_once_ms = 5000;
    turn_on_once_ms = 2000;
    water_event_trigger_step = 1;
  }
  int get_check_water_interval() {
    return check_water_interval;
  }
  int get_water_low() {
    return water_low;
  }
  int get_hold_once_ms() {
    return hold_once_ms;
  }
  int get_turn_on_once_ms() {
    return turn_on_once_ms;
  }
  String toString() {
    JSONVar t;
    t["check_water_interval"] = check_water_interval;
    t["water_low"] = water_low;
    t["hold_once_ms"] = hold_once_ms;
    t["turn_on_once_ms"] = turn_on_once_ms;
    t["water_event_trigger_step"] = water_event_trigger_step;
    return JSON.stringify(t);
  }
};



class Pumper {
  private:
  int pumper_pin;
  int over_ms;

  public:
  Pumper(int pp) {
    pumper_pin = pp;
    over_ms = -1;
  }
  void turn_on_for_millis(int ms) {
    int stop_ms = millis() + ms;
    if (over_ms > stop_ms) {
      Serial.printf("pump util %d, later than %d, so skip\n", over_ms, stop_ms);
      return;
    }
    over_ms = stop_ms;
    digitalWrite(pumper_pin, HIGH);
    Serial.printf("increase pump time to:%d\n", over_ms);
    return;
  }
  void check_timeup() {
    if (over_ms <= 0) {
      return;
    }
    int ms = millis();
    if (over_ms < ms) {
      Serial.printf("now time is %d, stop pump for %d\n", ms, over_ms);
      digitalWrite(pumper_pin, LOW);
      over_ms = -1;
      return;
    }
    return;
  }
};

class PumperMgr {
  private:
  Pumper* pr;
  int hold_ms;
  int hold_once_ms;
  int turn_on_once_ms;

  public:
  PumperMgr(Pumper* ppr, int hom, int toom) {
    pr = ppr;
    hold_once_ms = hom;
    turn_on_once_ms = toom;
    hold_ms = -1;
  }
  void water_flower() {
    int ms = millis();
    if (ms < hold_ms) {
      return;
    }
    hold_ms = ms + hold_once_ms;
    pr->turn_on_for_millis(turn_on_once_ms);
    Serial.printf("water flower, now hold time is:%d\n", hold_ms);
    return;
  }
};

class WiFiMgr {
  private:
  bool isWiFiConnected;
  String* ssid;
  String* sspasswd;
  public:
  WiFiMgr() {
    isWiFiConnected = false;
//    ssid = pssid;
//    sspasswd = pPasswd;
  }
  bool connectWithRetry(String ssid, String pw, int retryTimes) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pw);
    while ((WiFi.status() != WL_CONNECTED) && retryTimes>0) {
      delay(500);
      Serial.printf("ssid:%s, pass:%s\n", &ssid, &pw);
      retryTimes--;
    }
    isWiFiConnected = (WiFi.status() == WL_CONNECTED);
    return isWiFiConnected;
  }
  void checkWiFi() {
    bool isWiFiConnectedNow = WiFi.status() == WL_CONNECTED;
    if (isWiFiConnected && !isWiFiConnectedNow) {
      //disconnect event
      hanleWiFiDisconnect();
      return;
    }
    if (!isWiFiConnected && isWiFiConnectedNow) {
      hanadleWiFiConnect();
      return;
    }
    //state not changed, pass
    return;
  }
  void hanleWiFiDisconnect() {
    Serial.println("wifi disconnected");
    isWiFiConnected = false;
  }
  void hanadleWiFiConnect() {
    Serial.println("wifi connected");
    isWiFiConnected = true;
  }
};

class WaterMonitor {
  private:
  int water_pin;
  Conf* cfg;
  int lastCheckMilli;
  PumperMgr* prm;

  public:
  WaterMonitor(int wp, Conf* c, PumperMgr* p) {
    water_pin = wp;
    cfg = c;
    prm = p;
    lastCheckMilli = 0;
  }
  bool is_water_low() {
    return getWaterValue() < cfg->water_low;
  }
  void checkWater() {
    if (millis() - lastCheckMilli < cfg->check_water_interval) {
      return;
    }

    if (is_water_low()){
      prm->water_flower();
    }
  }
  int getWaterValue() {
    return analogRead(water_pin);
  }
};

Conf* cfg;
WaterMonitor* waterMonitor;
Pumper* pr;
PumperMgr* prm;
WiFiMgr* wfMgr;
ESP8266WebServer server(80);

void handleGetWater() {
  int waterValue = waterMonitor->getWaterValue();
  Serial.printf("hanlde get water:%d\n", waterValue);
  server.send(200, "text/plain", String(waterValue));
}

void handleConfig() {
  HTTPMethod method = server.method();
  if (method == HTTP_GET) {
    server.send(200, "text/json", cfg->toString());
    return;
  }
  if (server.hasArg("check_water_interval")&& server.arg("check_water_interval").toInt() > 0) {
    cfg->check_water_interval = server.arg("check_water_interval").toInt();
  } 
  if (server.hasArg("water_low")&& server.arg("water_low").toInt() > 0) {
    cfg->water_low = server.arg("water_low").toInt();
  } 
  if (server.hasArg("hold_once_ms")&& server.arg("hold_once_ms").toInt() > 0) {
    cfg->hold_once_ms = server.arg("hold_once_ms").toInt();
  } 
  if (server.hasArg("turn_on_once_ms")&& server.arg("turn_on_once_ms").toInt() > 0) {
    cfg->turn_on_once_ms = server.arg("turn_on_once_ms").toInt();
  } 
  if (server.hasArg("water_event_trigger_step")&& server.arg("water_event_trigger_step").toInt() > 0) {
    cfg->water_event_trigger_step = server.arg("water_event_trigger_step").toInt();
  }
  server.send(200, "text/plain", "");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(BUAD_RATE);
  Serial.printf("set up, buad rate:%d\n", BUAD_RATE);

  pinMode(PUMPER_PIN, OUTPUT);
  
  cfg = new Conf();
  pr = new Pumper(PUMPER_PIN);
  prm = new PumperMgr(pr, cfg->get_hold_once_ms(), cfg->get_turn_on_once_ms());
  waterMonitor = new WaterMonitor(WATER_PIN, cfg, prm);
  wfMgr = new WiFiMgr();
  if (wfMgr->connectWithRetry(SID, WiFi_PASSWORD, 10)) {
    Serial.println("connect WiFi connected, ip:");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("connect WiFi failed");
    return;
  }
  server.on("/sensor/water", handleGetWater);
  server.on("/config", handleConfig);
  server.begin();
}

void loop() {
  server.handleClient();
  pr->check_timeup();
  waterMonitor->checkWater();
}
