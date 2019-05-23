package main

import "github.com/sirupsen/logrus"

type MulWLListener struct {
	pumper   Pumper
	cfgStore ConfStore
}

func NewMulWLListener(cfgStore ConfStore, pumper Pumper) *MulWLListener {
	return &MulWLListener{
		pumper:   pumper,
		cfgStore: cfgStore,
	}
}

func (ml *MulWLListener) HandleWaterLow(minValue int, nowValue int) {
	logrus.Debugf("water low: %d < %d", nowValue, minValue)
	ao, err := ml.pumper.TurnOn(ml.cfgStore.GetConf().TurnOnce)
	if err != nil {
		logrus.Errorf("turn on pumper failed:%v", err)
		return
	}
	logrus.Debugf("pumper is already on ? %v", ao)
}
