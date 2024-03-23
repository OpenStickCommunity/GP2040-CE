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

## Combination Mode DDI Override

This replaces whatever the gamepad has for output with a non-zero DDI output. SOCD history is maintained. Cross-axis
behavior is interesting here, allowing some down-to-left/right tricks.

| # | Input           | Neutral | Up Prio   | First Input Wins | Last Input Wins |
| - | --------------- | ------- | --------- | ---------------- | --------------- |
| 1 | Core Left       | Left    | Left      | Left             | Left            |
| 2 | Core Right      | Neutral | Neutral   | Left             | Right           |
| 3 | DDI Down        | Down    | Down      | Down             | Down            |
| 4 | DDI Up          | Neutral | Up        | Down             | Up              |
| 5 | xx DDI Up       | Down    | Down      | Down             | Down            |
| 6 | xx DDI Down     | Neutral | Neutral   | Left             | Right           |
| 7 | xx Core Right   | Left    | Left      | Left             | Left            |
