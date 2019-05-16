#define BUAD_RATE 115200
#define WATER_PIN A0
#define PUMPER_PIN LED_BUILTIN

class Conf {
  private:
  int check_water_interval;
  int water_low;
  int hold_once_ms;
  int turn_on_once_ms;
  
  public:
  Conf() {
    check_water_interval = 500;
    water_low = 100;
    hold_once_ms = 5000;
    turn_on_once_ms = 2000;
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
};

class WaterMonitor {
  private:
  int min_water_value;
  int water_pin;

  public:
  WaterMonitor(int mwv, int wp) {
    min_water_value = mwv;
    water_pin = wp;
  }
  bool is_water_low() {
    return get_water_value() < min_water_value;
  }
  int get_water_value() {
    return analogRead(water_pin);
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
      Serial.printf("in hold time(%d < %d), skip\n", ms, hold_ms);
      return;
    }
    hold_ms = ms + hold_once_ms;
    pr->turn_on_for_millis(turn_on_once_ms);
    Serial.printf("water flower, now hold time is:%d\n", hold_ms);
    return;
  }
};

Conf* cfg;
WaterMonitor* water_monitor;
Pumper* pr;
PumperMgr* prm;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(BUAD_RATE);
  Serial.printf("set up, buad rate:%d\n", BUAD_RATE);
  pinMode(PUMPER_PIN, OUTPUT);

  cfg = new Conf();
  water_monitor = new WaterMonitor(cfg->get_water_low(), WATER_PIN);
  pr = new Pumper(PUMPER_PIN);
  prm = new PumperMgr(pr, cfg->get_hold_once_ms(), cfg->get_turn_on_once_ms());
}

void loop() {
  pr->check_timeup();
  bool is_water_low = water_monitor->is_water_low();
  if (is_water_low) {
    Serial.println("water low");
    prm->water_flower();
  } else {
    Serial.println("water is fine");
  }
  delay(cfg->get_check_water_interval());
}
