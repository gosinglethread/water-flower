# encoding:utf8
from machine import ADC
from machine import Pin
from machine import Timer

pump_control_pin = Pin(11, Pin.OUT)
water_sensor_pin = Pin(10)

class Conf(object):
    def __init__(self, remote_addr=''):
        self.__check_water_interval = 1
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
    def __init__(self, pin_num, on_timer_id=57):
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



def get_water_num():
    return water_sensor_pin.read()

def check_water_low(cfg, water_num):
    return cfg.get_water_low_num() > water_num

def handle_water_low():
    print('water low')
    power_pump()

def power_pump():
    pump_control_pin.on()

def power_pump_for_milliseconds(milliseconds=1000):
    power_pump()


def dis_power_pump():
    pump_control_pin.off()

def check_water():
    cfg = Conf()
    water_num = get_water_num()
    is_water_low = check_water_low(cfg, water_num)
    if is_water_low:
        handle_water_low()

def main():
    cfg = Conf()
    Timer(1).init(period=cfg.get_check_water_interval, mode=Timer.PERIOD, callback=check_water)

