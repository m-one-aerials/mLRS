//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
// Dynamic Power Control
//********************************************************
#ifndef DYNPOWER_H
#define DYNPOWER_H
#pragma once


#include "hal/hal.h"


// Thresholds
#define DYNPOWER_RSSI_THRESH_UP       15    // RSSI < (sensitivity + UP) -> raise power
#define DYNPOWER_RSSI_THRESH_DN       21    // RSSI > (sensitivity + DN) -> lower power
#define DYNPOWER_LQ_THRESH_DN         95    // only lower power if LQ >= this
#define DYNPOWER_LQ_THRESH_UP         85    // raise power if LQ < this (fallback)
#define DYNPOWER_LQ_BOOST_THRESH_DIFF 20    // sudden LQ drop threshold
#define DYNPOWER_LQ_BOOST_THRESH_MIN  50    // critical LQ level


class tDynPower
{
  public:
    void Init(bool enabled, uint8_t min_idx, uint8_t max_idx)
    {
        _enabled = enabled;
        _min_idx = min_idx;
        _max_idx = max_idx;
        _lq_ma = 100;
    }

    // Called once per second after stats.Update1Hz(), when connected.
    void Tick(int8_t rssi, uint8_t lq, int16_t sensitivity, tRfPower& rfpower)
    {
        if (!_enabled) return;

        // LQ moving average (exponential, K=8)
        int32_t lq_diff = (int32_t)_lq_ma - (int32_t)lq;
        _lq_ma = (uint8_t)(((uint32_t)7 * _lq_ma + (uint32_t)lq + 4) / 8); // +4 for rounding

        // emergency boost
        if (lq_diff >= DYNPOWER_LQ_BOOST_THRESH_DIFF || lq <= DYNPOWER_LQ_BOOST_THRESH_MIN) {
            rfpower.Set(_max_idx);
            return;
        }

        uint8_t current = rfpower.GetCurrentIdx();
        uint8_t start_level = current;

        // RSSI-based adjustment
        int16_t rssi_inc_threshold = sensitivity + DYNPOWER_RSSI_THRESH_UP;
        int16_t rssi_dec_threshold = sensitivity + DYNPOWER_RSSI_THRESH_DN;

        if (rssi < rssi_inc_threshold && current < _max_idx) {
            rfpower.Set(current + 1);
        } else if (rssi > rssi_dec_threshold && _lq_ma >= DYNPOWER_LQ_THRESH_DN && current > _min_idx) {
            rfpower.Set(current - 1);
        }

        // LQ fallback
        if (rfpower.GetCurrentIdx() == start_level && _lq_ma < DYNPOWER_LQ_THRESH_UP && current < _max_idx) {
            rfpower.Set(current + 1);
        }
    }

    // Called when disconnected: set to max power for reconnection
    void SetToMax(tRfPower& rfpower)
    {
        if (!_enabled) return;
        rfpower.Set(_max_idx);
        _lq_ma = 100;
    }

  private:
    bool _enabled;
    uint8_t _min_idx;
    uint8_t _max_idx;
    uint8_t _lq_ma;   // LQ moving average (0-100)
};


#endif // DYNPOWER_H
