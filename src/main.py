from machine import ADC
from machine import Pin
from machine import Timer
import utime

pump_control_pin = Pin(32, Pin.OUT)
water_sensor_pin = Pin(10)


def get_water_num():
    return water_sensor_pin.value()

def check_water_low(cfg, water_num):
    return cfg.get_water_low_num() > water_num

def handle_water_low(pumperMgr):
    print('water low')
    pumperMgr.enable_pumper()

def check_water():
    water_num = get_water_num()
    is_water_low = check_water_low(cfg, water_num)
    if is_water_low:
        handle_water_low(pumperMgr)

class Conf(object):
    def __init__(self, remote_addr=''):
        self.__check_water_interval = 1000
        self.__water_low_num = 100
    
    def get_water_low_num(self):
        return self.__water_low_num

    def get_check_water_interval(self):
        return self.__check_water_interval
    
    def update_from_remote(self):
        pass

    def start_update_interval(self):
        pass
		

class Pumper(object):
    def __init__(self, pin_num = 2, on_timer_id=57):
        self.__pin = Pin(pin_num, Pin.OUT)
        self.__time_to_on = 0
        self.__on_timer_id = on_timer_id
        self.__on_timer = None

    def is_on(self):
        return self.__time_to_on > 0
    
    def on(self):
        print('pumper: on')
        self.__pin.on()
    
    def off(self):
        print('pumper: off')
        self.__pin.off()
        self.__time_to_on = 0
        self.__on_timer.deinit()
        self.__on_timer = None
    
    def on_for_milliseconds(self, milliseconds=1000):
        if self.__time_to_on > milliseconds:
            print('pumper: already on until ', self.__time_to_on)
            return
        self.__time_to_on = milliseconds
        print('pumper: delay on until ', self.__time_to_on)
        self.on()
        if self.__on_timer is not None:
            self.__on_timer.deinit()
        self.__on_timer = Timer(self.__on_timer_id).init(period=self.__time_to_on, mode=Timer.ONE_SHOT, callback=self.off)


class PumperMgr(object):
    def __init__(self, pumper, on_once=1000, protect_once=10000):
        self.__pumper = pumper
        self.__pumper_protect_time = 0
        self.__pumper_on_time_once = on_once
        self.__pumper_protect_time_once = protect_once
    
    def enable_pumper(self):
        now = utime.time()
        if now < self.__pumper_protect_time:
            print('PumperMgr: in protect time, pass.', now, self.__pumper_protect_time)
            return
        self.__pumper.on_for_milliseconds(self.__pumper_on_time_once)
        self.__pumper_protect_time = now + self.__pumper_protect_time_once / 1000
        print('PumperMgr: now protect time is %d' % (self.__pumper_protect_time))




cfg = None
pumperMgr = None


def main():
    global cfg 
    cfg = Conf()
    global pumperMgr 
    pumper = Pumper()
    pumperMgr = PumperMgr(pumper)
    #Timer(1).init(period=cfg.get_check_water_interval(), mode=Timer.PERIODIC, callback=check_water)
    while True:
      check_water()
      utime.sleep(5)

try:
    main()
except Exception as e:
    import sys
    sys.print_exception(e)


