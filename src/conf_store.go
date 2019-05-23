package main

import (
	"encoding/json"
	"net/http"
	"strconv"
	"time"

	"github.com/sirupsen/logrus"
)

type Conf struct {
	CheckInterval         time.Duration `json:"check_interval"`
	WaterLow              int           `json:"water_low"`
	HoldOnce              time.Duration `json:"hold_once"`
	TurnOnce              time.Duration `json:"turn_once"`
	WaterEventTriggerStep int
}

type ConfStore interface {
	GetConf() (conf Conf)
}

type ConfMgrServer struct {
	c                *Conf
	minCheckInterval int
}

func NewConfMgrServer(minCheckInterval int) *ConfMgrServer {
	return &ConfMgrServer{
		minCheckInterval: minCheckInterval,
		c: &Conf{
			CheckInterval:         1000 * time.Millisecond,
			WaterEventTriggerStep: 1,
			HoldOnce:              5000 * time.Millisecond,
			TurnOnce:              2000 * time.Millisecond,
			WaterLow:              1,
		},
	}
}

func (cs *ConfMgrServer) GetConf() (conf Conf) {
	conf.WaterLow = cs.c.WaterLow
	conf.CheckInterval = cs.c.CheckInterval
	conf.HoldOnce = cs.c.HoldOnce
	conf.TurnOnce = cs.c.TurnOnce
	conf.WaterEventTriggerStep = cs.c.WaterEventTriggerStep
	return
}

func (cs *ConfMgrServer) handleUpdateConf(w http.ResponseWriter, req *http.Request) {
	ciStr := req.FormValue("check_interval")
	if ciStr != "" {
		ci, err := strconv.Atoi(ciStr)
		if err == nil && ci >= cs.minCheckInterval {
			cs.c.CheckInterval = time.Duration(ci) * time.Millisecond
		} else {
			logrus.Error("check interval:%s not int, or to small", ciStr)
			w.WriteHeader(http.StatusBadRequest)
			return
		}
	}
	wlStr := req.FormValue("water_low")
	if wlStr != "" {
		wl, err := strconv.Atoi(wlStr)
		if err == nil {
			cs.c.WaterLow = wl
		} else {
			logrus.Error("water low:%s not int", wlStr)
			w.WriteHeader(http.StatusBadRequest)
			return
		}
	}
	hoStr := req.FormValue("hold_once")
	if hoStr != "" {
		ho, err := strconv.Atoi(hoStr)
		if err == nil {
			cs.c.HoldOnce = time.Duration(ho) * time.Millisecond
		} else {
			logrus.Error("hold once:%s not int", ho)
			w.WriteHeader(http.StatusBadRequest)
			return
		}
	}
	tooStr := req.FormValue("turn_on_once")
	if tooStr != "" {
		too, err := strconv.Atoi(tooStr)
		if err == nil {
			cs.c.TurnOnce = time.Duration(too) * time.Millisecond
		} else {
			logrus.Error("turn on once not int", tooStr)
			w.WriteHeader(http.StatusBadRequest)
			return
		}
	}
	tsStr := req.FormValue("water_event_trigger_step")
	if tsStr != "" {
		ts, err := strconv.Atoi(tsStr)
		if err == nil {
			cs.c.WaterEventTriggerStep = ts
		} else {
			logrus.Error("water event trigger step:%s not int", ts)
			w.WriteHeader(http.StatusBadRequest)
			return
		}
	}
	w.WriteHeader(http.StatusOK)
}

func (cs *ConfMgrServer) handleGetConf(w http.ResponseWriter, req *http.Request) {
	err := json.NewEncoder(w).Encode(cs.c)
	if err != nil {
		logrus.Error("marshal conf failed:", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}
}
