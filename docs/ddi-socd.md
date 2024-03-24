# Dual Directional Input + SOCD Modes

Documenting results for future reference. Below examples are holding the buttons sequentially, unless otherwise noted by
"xx", which means the button is released.

## Basic SOCD Example

I only documented Left/Right tests here, because Up/Down either follows the same behavior (Neutral/First/Last) on the
other axis, or it has its own far more explicit Up priority that isn't worth distinguishing in this doc.

| # | Input           | Up Priority/Neutral | First Input Wins | Last Input Wins |
| - | --------------- | ------------------- | ---------------- | --------------- |
| 1 | Core Left       | Left                | Left             | Left            |
| 2 | Core Right      | Neutral             | Left             | Right           |
| 3 | xx Core Left    | Right               | Right            | Right           |
| 4 | Core Left       | Neutral             | Right            | Left            |
| 5 | xx Core Right   | Left                | Left             | Left            |

## Combination Mode Mixed

All Combination Mode Mixed examples apply equally to Combination Mode None when the DDI output is the same LS/DP/RS
option as the core gamepad's.

I only documented Left/Right tests here, because Up/Down either follows the same behavior (Neutral/First/Last) on the
other axis, or it has its own far more explicit Up priority that isn't worth distinguishing in this doc.

This mode seems to be useful for having two sets of inputs contribute to any one of LS/DP/RS at the same time, but is
distinguished from two sets of core gamepad mappings by the fact that this allows for two layers of SOCD cleaning to be
applied, essentially `SOCD( SOCD(gamepad) | SOCD(DDI) )` which can allow for some interesting inputs.

| # | Input           | Up Priority/Neutral | First Input Wins | Last Input Wins |
| - | --------------- | ------------------- | ---------------- | --------------- |
| 1 | Core Left       | Left                | Left             | Left            |
| 2 | DDI Right       | Neutral             | Left             | Right           |
| 3 | DDI Left        | Left                | Left             | Left            |
| 4 | xx DDI Left     | Neutral             | Left             | Right           |
| 5 | Core Right      | Right               | Left             | Right           |
| 6 | DDI Left        | Neutral             | Left             | Left            |

| # | Input           | Up Priority/Neutral | First Input Wins | Last Input Wins |
| - | --------------- | ------------------- | ---------------- | --------------- |
| 1 | Core Left       | Left                | Left             | Left            |
| 2 | Core Right      | Neutral             | Left             | Right           |
| 3 | DDI Left        | Left                | Left             | Left            |
| 4 | xx DDI Left     | Neutral             | Left             | Right           |
| 5 | DDI Left        | Left                | Left             | Left            |
| 6 | DDI Right       | Neutral             | Left             | Right           |

| # | Input           | Up Priority/Neutral | First Input Wins | Last Input Wins |
| - | --------------- | ------------------- | ---------------- | --------------- |
| 1 | Core Right      | Right               | Right            | Right           |
| 2 | Core Left       | Neutral             | Right            | Left            |
| 3 | DDI Right       | Right               | Right            | Right           |
| 4 | xx DDI Right    | Neutral             | Right            | Left            |
| 5 | DDI Right       | Right               | Right            | Right           |
| 6 | DDI Left        | Neutral             | Right            | Left            |

| # | Input           | Up Priority/Neutral | First Input Wins | Last Input Wins |
| - | --------------- | ------------------- | ---------------- | --------------- |
| 1 | DDI Left        | Left                | Left             | Left            |
| 2 | DDI Right       | Neutral             | Left             | Right           |
| 3 | Core Left       | Left                | Left             | Left            |
| 4 | xx Core Left    | Neutral             | Left             | Right           |
| 5 | Core Left       | Left                | Left             | Left            |
| 6 | Core Right      | Neutral             | Left             | Right           |

## Combination Mode Gamepad Override

This ignores the DDI output when the core gamepad has any output. SOCD history is maintained. Cross-axis
behavior is interesting here, allowing some down-to-left/right tricks.

This makes Tekken Shadow Step easy, apparently.

| # | Input           | Neutral   | Up Prio   | First Input Wins | Last Input Wins |
| - | --------------- | --------- | --------- | ---------------- | --------------- |
| 1 | DDI Down        | Down      | Down      | Down             | Down            |
| 2 | Core Left       | Left      | Left      | Left             | Left            |
| 3 | Core Down       | Down Left | Down Left | Down Left        | Down Left       |
| 4 | xx DDI Down     | Down Left | Down Left | Down Left        | Down Left       |
| 5 | xx Core Down    | Left      | Left      | Left             | Left            |
| 6 | xx Core Left    | Neutral   | Neutral   | Neutral          | Neutral         |
| 7 | DDI Down        | Down      | Down      | Down             | Down            |
| 8 | DDI Left        | Down Left | Down Left | Down Left        | Down Left       |
| 9 | Core Up         | Up        | Up        | Up               | Up              |

| # | Input           | Neutral | Up Prio   | First Input Wins | Last Input Wins |
| - | --------------- | ------- | --------- | ---------------- | --------------- |
| 1 | Core Left       | Left    | Left      | Left             | Left            |
| 2 | DDI Down        | Left    | Left      | Left             | Left            |
| 3 | DDI Up          | Left    | Left      | Left             | Left            |
| 4 | xx DDI Up       | Left    | Left      | Left             | Left            |
| 5 | xx DDI Down     | Left    | Left      | Left             | Left            |
| 6 | Core Right      | Neutral | Neutral   | Left             | Right           |

| # | Input           | Neutral | Up Prio   | First Input Wins | Last Input Wins |
| - | --------------- | ------- | --------- | ---------------- | --------------- |
| 1 | DDI Left        | Left    | Left      | Left             | Left            |
| 2 | DDI Right       | Neutral | Neutral   | Left             | Right           |
| 3 | Core Down       | Down    | Down      | Down             | Down            |
| 4 | Core Up         | Neutral | Up        | Down             | Up              |
| 5 | xx Core Up      | Down    | Down      | Down             | Down            |
| 6 | xx Core Down    | Neutral | Neutral   | Left             | Right           |
| 7 | xx DDI Right    | Left    | Left      | Left             | Left            |

## Combination Mode DDI Override

This replaces whatever the gamepad has for output with a non-zero DDI output. SOCD history is maintained. Cross-axis
behavior is interesting here, allowing some down-to-left/right tricks. (It's the same idea as Gamepad Override, just
swapping which method overrides which.)

| # | Input           | Neutral | Up Prio   | First Input Wins | Last Input Wins |
| - | --------------- | ------- | --------- | ---------------- | --------------- |
| 1 | Core Left       | Left    | Left      | Left             | Left            |
| 2 | Core Right      | Neutral | Neutral   | Left             | Right           |
| 3 | DDI Down        | Down    | Down      | Down             | Down            |
| 4 | DDI Up          | Neutral | Up        | Down             | Up              |
| 5 | xx DDI Up       | Down    | Down      | Down             | Down            |
| 6 | xx DDI Down     | Neutral | Neutral   | Left             | Right           |
| 7 | xx Core Right   | Left    | Left      | Left             | Left            |
