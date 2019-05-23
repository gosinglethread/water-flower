package main

import (
	"time"

	"github.com/sirupsen/logrus"

	"gobot.io/x/gobot/drivers/aio"
)

type WaterLowListener interface {
	HandleWaterLow(minValue int, nowValue int)
}

type WaterMonitor struct {
	cfgStore ConfStore
	pin      *aio.AnalogSensorDriver
	ls       []WaterLowListener
}

func NewWaterMonitor(cfgStore ConfStore, pin *aio.AnalogSensorDriver, ls ...WaterLowListener) *WaterMonitor {
	return &WaterMonitor{
		cfgStore: cfgStore,
		pin:      pin,
		ls:       ls,
	}
}

func (wm *WaterMonitor) StartWatchRoutine() {
	go wm.Watch()
}

func (wm *WaterMonitor) Watch() {
	for {
		wm.checkOnce()
		time.Sleep(wm.cfgStore.GetConf().CheckInterval)
	}
}
func (wm *WaterMonitor) checkOnce() {
	wl, err := wm.pin.Read()
	if err != nil {
		logrus.Debugf("read water value failed:%v", err)
		return
	}
	cfgWL := wm.cfgStore.GetConf().WaterLow
	if wl < cfgWL {
		wm.handleWaterLow(cfgWL, wl)
	} else {
		logrus.Debugf("water:%d fine", wl)
	}
}

func (wm *WaterMonitor) handleWaterLow(minWL, wl int) {
	for _, l := range wm.ls {
		l.HandleWaterLow(minWL, wl)
	}
}
