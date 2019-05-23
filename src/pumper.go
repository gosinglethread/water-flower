package main

import (
	"time"

	"github.com/sirupsen/logrus"

	"gobot.io/x/gobot/drivers/gpio"
)

type Pumper interface {
	TurnOn(turnOnDuration time.Duration) (isAlreadyOn bool, err error)
	TurnOff() (isAlreadyOff bool, err error)
}

type Hold4TimePumper struct {
	cfgStore  ConfStore
	hold2Time time.Time
	pin       *gpio.LedDriver
	stopTimer *time.Timer
}

func NewHold4TimePumper(pin *gpio.LedDriver, cfg ConfStore) *Hold4TimePumper {
	return &Hold4TimePumper{
		pin:       pin,
		hold2Time: time.Now(),
		cfgStore:  cfg,
	}
}

func (hp *Hold4TimePumper) TurnOn(turnOnDuration time.Duration) (isAlreadyOn bool, err error) {
	logrus.Debug("turn on for:", turnOnDuration)
	if time.Now().Before(hp.hold2Time) {
		logrus.Debug("in hold time. skip")
		return
	}
	isAlreadyOn = hp.isOn()
	if !isAlreadyOn {
		hp.pin.On()
	}
	if hp.stopTimer != nil {
		hp.stopTimer.Stop()
	}
	hp.stopTimer = time.AfterFunc(turnOnDuration, func() {
		hp.TurnOff()
	})
	hp.hold2Time = time.Now().Add(hp.cfgStore.GetConf().HoldOnce)
	logrus.Debugf("hold to:%s", hp.hold2Time.Format("2006-01-02 15:04:05"))
	return
}

func (hp *Hold4TimePumper) isOn() bool {
	return hp.pin.State()
}

func (hp *Hold4TimePumper) TurnOff() (isAlreadyOff bool, err error) {
	logrus.Debugf("turn off")
	isAlreadyOff = !hp.isOn()
	if hp.stopTimer != nil {
		hp.stopTimer.Stop()
		hp.stopTimer = nil
	}
	if !isAlreadyOff {
		hp.pin.Off()
	}
	return
}
