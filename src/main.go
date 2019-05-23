package main

import (
	"time"

	"github.com/sirupsen/logrus"
	"gobot.io/x/gobot/drivers/aio"

	"gobot.io/x/gobot"
	"gobot.io/x/gobot/drivers/gpio"
	"gobot.io/x/gobot/platforms/firmata"
)

type ServerConf struct {
	PumperPinNum int
}

func main() {
	logrus.SetLevel(logrus.DebugLevel)
	firmMetaAdaptor := firmata.NewTCPAdaptor("172.26.67.2:3030")
	led := gpio.NewLedDriver(firmMetaAdaptor, "2")

	cfgStore := NewConfMgrServer(1000)
	pumper := NewHold4TimePumper(led, cfgStore)
	mulListener := NewMulWLListener(cfgStore, pumper)
	A0Pin := aio.NewAnalogSensorDriver(firmMetaAdaptor, "3", 100*time.Microsecond)
	waterMonitor := NewWaterMonitor(cfgStore, A0Pin, mulListener)
	work := func() {
		A0Pin.Start()
		waterMonitor.StartWatchRoutine()
		//gobot.Every(1*time.Second, func() {
		//	pumper.TurnOn(cfgStore.GetConf().TurnOnce)
		//})
	}

	robot := gobot.NewRobot("bot",
		[]gobot.Connection{firmMetaAdaptor},
		[]gobot.Device{led},
		work,
	)

	robot.Start()
}
