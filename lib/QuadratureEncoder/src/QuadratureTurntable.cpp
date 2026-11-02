#include "QuadratureTurntable.h"

void QuadratureTurntable::begin(const Config& config) {
    _config = config;
    if (_config.thresholdCounts == 0) _config.thresholdCounts = 1;
    if (_config.unitsPerRevolution == 0) _config.unitsPerRevolution = 1;

    _direction = 0;
    _accumSign = 0;
    _accum = 0;
    _lastMoveUs = 0;
    _lastActiveUs = 0;
    _moving = false;
    _countInRev = 0;
    _position = 0;
}

void QuadratureTurntable::update(int32_t delta, uint32_t nowUs) {
    // A pause longer than the hold ends the current movement. Tracked with a flag
    // so a single count after a very long idle (where the 32-bit microsecond
    // difference has wrapped) can't combine with stale accumulated movement.
    if (_moving && ((uint32_t)(nowUs - _lastMoveUs) >= _config.holdUs)) {
        _moving = false;
        _accum = 0;
        _accumSign = 0;
    }

    if (delta != 0) {
        int8_t sign = (delta > 0) ? 1 : -1;
        uint32_t magnitude = (delta > 0) ? (uint32_t)delta : (uint32_t)(-(int64_t)delta);

        // reversing starts a new accumulation in the other direction
        if (sign != _accumSign) {
            _accumSign = sign;
            _accum = 0;
        }
        _accum = (_accum > UINT32_MAX - magnitude) ? UINT32_MAX : (_accum + magnitude);
        _lastMoveUs = nowUs;
        _moving = true;

        if (_accum >= _config.thresholdCounts) {
            _direction = sign;      // fires, flips, or refreshes the hold
            _lastActiveUs = nowUs;
        }

        if (_config.countsPerRevolution) {
            int64_t next = (int64_t)_countInRev + delta;
            int64_t rev = (int64_t)_config.countsPerRevolution;
            next %= rev;
            if (next < 0) next += rev;
            _countInRev = (uint32_t)next;
            _position = (uint16_t)(((uint64_t)_countInRev * _config.unitsPerRevolution) / _config.countsPerRevolution);
        }
    }

    if ((_direction != 0) && ((uint32_t)(nowUs - _lastActiveUs) >= _config.holdUs)) {
        _direction = 0;
    }
}
