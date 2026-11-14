#ifndef _QUADRATURE_TURNTABLE_H_
#define _QUADRATURE_TURNTABLE_H_

// QuadratureTurntable: turns encoder count changes into the two things rhythm-game
// turntables are read as, without deciding how they're reported:
//
//   direction() - digital scratch: -1 / 0 / +1. Fires once `thresholdCounts` of
//                 movement in one direction have accumulated (no long travel before
//                 it registers), flips immediately when that much movement happens
//                 the other way, and is held for `holdUs` after the last movement in
//                 the active direction so a scratch reads as one continuous press.
//                 Movement slower than one count per `holdUs` never fires, so a
//                 resting or vibrating platter stays neutral.
//
//   position()  - analog: absolute platter angle, 0 .. unitsPerRevolution-1, wrapping.
//                 Computed from the exact count modulo one revolution, so it never
//                 drifts no matter how long or how fast it's spun.
//
// Pure logic (no hardware access): feed it the count delta and a microsecond
// timestamp every poll, even when the delta is 0, so holds can expire.

#include <stdint.h>

class QuadratureTurntable {
public:
    struct Config {
        uint32_t countsPerRevolution = 0;   // edges per revolution (4 x PPR); 0 = no position
        uint16_t unitsPerRevolution = 256;  // position range, e.g. 256 for an 8-bit axis
        uint32_t thresholdCounts = 1;       // movement needed to fire / reverse (min 1)
        uint32_t holdUs = 100000;           // how long direction stays held after movement
    };

    void begin(const Config& config);

    // delta: signed change in count since the last call. nowUs: free-running
    // microsecond clock (wraps; only differences are used).
    void update(int32_t delta, uint32_t nowUs);

    int8_t direction() const { return _direction; }
    uint16_t position() const { return _position; }

private:
    Config _config;
    int8_t _direction = 0;
    int8_t _accumSign = 0;
    uint32_t _accum = 0;        // movement in _accumSign since the last pause/reversal
    uint32_t _lastMoveUs = 0;   // time of the last nonzero delta
    uint32_t _lastActiveUs = 0; // time of the last movement that (re)fired _direction
    bool _moving = false;       // a movement happened within holdUs
    uint32_t _countInRev = 0;   // count modulo countsPerRevolution
    uint16_t _position = 0;
};

#endif
