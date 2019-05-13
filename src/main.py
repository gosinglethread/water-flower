# encoding:utf8
from machine import ADC
from machine import Pin
from machine import Timer

pump_control_pin = Pin(11, Pin.OUT)
water_sensor_pin = Pin(10)

class Conf(object):
    def __init__(self, *args):
        super(Conf, self).__init__(*args))
        self.__check_water_interval = 1
        self.__water_low_num = 100
    
    def get_water_low_num():
        return self.__water_low_num

    def get_check_water_interval():
        return self.__check_water_interval


global_conf = Conf()

def get_conf():
    return global_conf

def get_remote_conf():
    return Conf()



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
    Timer(-1).init(period=milliseconds, Timer.ONE_SHOT, dis_power_pump)

def dis_power_pump():
    pump_control_pin.off()

def check_water():
    cfg = get_conf()
    water_num = get_water_num()
    is_water_low = check_water_low(cfg, water_num)
    if is_water_low:
        handle_water_low()

def update_global_conf():
    global_conf = get_remote_conf

def main():
    global_conf = get_remote_conf()
    Timer(1).init(period=global_conf.get_check_water_interval, mode=Timer.PERIOD, callback=check_water)
    Timer(2).init(period=global_conf.get_check_water_interval, mode=Timer.PERIOD, callback=update_global_conf)
