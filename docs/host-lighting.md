# Host Lighting

Host Lighting lets software on a connected PC drive a GP2040-CE board's RGB
LEDs in real time over USB, alongside the normal controller function. Game
state, per-button effects, ambient scenes - anything a host application can
compute, it can show on the board's lights, while the on-board animations
take over automatically the moment the host goes quiet.

The feature is fully self-describing: hosts query each board's LED layout,
identity and state over the same interface, so host software needs no
per-board configuration or updates when new boards are released.

## How it works

The add-on is disabled by default, like all GP2040-CE add-ons. Enable it in
the web configurator (`Configuration -> Add-Ons -> Host Lighting`); board makers
can ship it on by defining `HOST_LIGHTING_ENABLED 1` in a board config.

When enabled, supported input modes expose one extra vendor-defined HID
interface next to the regular controller interface. Hosts exchange fixed
64-byte reports on it:

- Lighting commands write into a **staged frame** on the board; a `COMMIT`
  publishes it atomically to the LED render loop, so multi-report frames never
  tear.
- The render loop shows the host frame while it stays fresh. If the host
  releases control - or simply stops talking for a configurable timeout
  (default 2 s) - the board's own animations resume seamlessly. A crashed or
  disconnected host can never leave the lights stuck.
- **Whole-frame** takeover replaces all lighting; **overlay** takeover only
  replaces pixels the host has explicitly staged, compositing host effects
  over the running animations.
- Everything the host needs to know - LED map, colour order, board identity,
  current state - is served by `GET_CAPS` from the board's live configuration,
  including user remaps.

## Supported modes

| Input mode | Lighting interface |
|---|---|
| Generic (DInput) | Yes |
| Keyboard | Yes |
| XInput | Yes - see below |
| PS3/PS4/PS5, Switch, Xbox, other console modes | Never |

**XInput** carries a choice, configurable in the web configurator
(`Host Lighting` section):

- **Auto** (default): the board boots with its stock, console-identical
  identity. A console begins authentication immediately and the board stays
  stock - consoles never see any difference. A PC sends no console
  authentication, so after ~4 seconds the board re-enumerates as a composite
  whose controller interface binds the operating system's own Xbox 360 driver
  (Windows via MS OS descriptors, Linux via the kernel's xpad vendor match),
  with the lighting interface alongside. Hosts should expect the lighting
  device to appear a few seconds after plug-in in this mode.
- **Always on**: the composite identity from boot (PC-only while enabled).
- **Off**: stock XInput identity always; no lighting interface in XInput mode.

With the add-on disabled (the default), every mode presents byte-identical
USB descriptors to stock firmware.

## Discovery

Do not match VID:PID - it varies by input mode and user override. Enumerate
HID devices and match the top-level collection:

- **Usage page `0xFF47`, usage `0x4C`**
- One 64-byte input report and one 64-byte output report, no report IDs
  (Windows buffers are 65 bytes with a leading `0x00` report-ID byte)

Then send `PING` and require the `GPHL` magic and protocol version >= 1.0.
Bind boards persistently by the factory-unique ID from `GET_CAPS` page 0 -
never by device path, which changes with USB ports.

Multiple boards connected at once appear as fully independent lighting
devices - one interface per board, each addressed separately with no
interaction between them.

## Protocol reference

The wire protocol carried on this interface is the Host Lighting Protocol
(HLP) - the `GPHL` magic in `PING` replies and the `hlp-` prefix on the
reference tools refer to it.

All transfers are 64-byte reports. Requests: `[0]=command, [1]=sequence,
[2..]=payload`. Replies echo the command with bit 7 set: `[0]=cmd|0x80,
[1]=sequence, [2]=status, [3..]=payload`. Status: `0` OK, `1` unsupported
command, `2` invalid argument.

### Commands

| Cmd | Name | Payload | Notes |
|---|---|---|---|
*Session and discovery (0x01-0x0F)*
| 0x01 | PING | - | Reply `[3..6]="GPHL", [7..8]=version`; any OK command refreshes the takeover keepalive |
| 0x02 | GET_CAPS | `[2]=page` (+`[3]=start` for page 4) | See pages below |
| 0x03 | SET_MODE | `[2]=takeover (0 whole-frame, 1 overlay), [3..4]=timeout ms LE (0->2000, min 100), [5]=apply board brightness` | Send at connect; settings persist until reboot |

*Frame staging (0x10-0x2F)*
| 0x10 | SET_BUTTONS | `[2]=n (1-15)`, n x `[buttonId,R,G,B]` | Stage by control; reply `[3]=applied, [4]=skipped` |
| 0x11 | SET_RANGE | `[2]=start, [3]=count (1-20)`, count x `[R,G,B]` | Stage raw pixels |
| 0x12 | SET_RANGE_RGBW | `[2]=start, [3]=count (1-15)`, count x `[R,G,B,W]` | Boards without a white channel ignore W |
| 0x13 | FILL | `[2]=scope (0 all, 1 buttons, 2 case, 3 player LEDs), [3..5]=RGB` | Stage a scope |
| 0x14 | CLEAR | - | Reset staged pixels and overlay validity |

*Frame lifecycle (0x30-0x3F)*
| 0x30 | COMMIT | - | Atomically publish the staged frame |
| 0x31 | RELEASE | - | Immediately return to on-board animations |

*Board features (0x40-0x4F)*
| 0x40 | SET_ANIMATION | `[2]=index` | Select the on-board animation; applied live and persisted |

*Privileged management, magic-guarded (0x70-0x7F)*
| 0x7B | SET_INPUT_MODE | `[2]=InputMode, [3..6]="MODE"` | ACKs, saves, reboots into the new mode |
| 0x7C | REBOOT_WEBCONFIG | `[2..5]="WEBC"` | Reboot into the web configurator |
| 0x7F | REBOOT_BOOTSEL | `[2..5]="BOOT"` | Reboot into the UF2 bootloader |

A control may own more than one light: some boards wire two physical buttons
to the same action (a second Up, for example), and each has its own light.
Colouring a control colours every light that belongs to it. The capability
map reports one LED range per control - the first - so a host that needs to
address the additional lights individually can do so by raw index with
`SET_RANGE`.

Button IDs 0-17 follow GP2040-CE's canonical order: Up, Down, Left, Right,
B1-B4, L1, R1, L2, R2, S1, S2, L3, R3, A1, A2. Specials: 24-27 player LEDs
1-4, 28 turbo LED, 29 the whole case range. `SET_INPUT_MODE` values are the
firmware's `InputMode` enum (0 XINPUT, 1 SWITCH, 2 PS3, 3 KEYBOARD, 4 PS4,
5 XBONE, 6 MDMINI, 7 NEOGEO, 8 PCEMINI, 9 EGRET, 10 ASTRO, 11 PSCLASSIC,
12 XBOXORIGINAL, 13 PS5, 14 GENERIC, 15 SWITCH_PRO, 16 P5GENERAL); the config
mode is entered via `REBOOT_WEBCONFIG` instead.

### Command round-trip

A command and its reply typically complete in about **1 ms** - one USB frame,
as expected for a 1 ms interrupt endpoint in each direction. The round trip
is bounded by the host's polling, not by board processing, so it can stretch
to several milliseconds when the host machine is under load. Set host
timeouts generously (tens of milliseconds) rather than near the typical
figure, to avoid false failures.

### Replies while streaming

Every command is answered, and replies stay reliable at streaming rates: a
46-LED board driven at 60 fps - three staging reports plus a `COMMIT` every
frame - acknowledges 300 of 300 commits with none lost.

Because commands can be pipelined, replies arrive interleaved: the
acknowledgements for a frame's staging reports land while the host is still
waiting for that frame's `COMMIT` reply. **Match each reply by its command
byte and sequence number, holding a small queue of recent replies.** A host
that keeps only the newest reply in a single slot will discard
acknowledgements it is still waiting for and stall until its own timeout,
losing frame rate to failures that never happened on the wire.

The firmware skips a reply if the interrupt IN endpoint is still busy, so a
reply is best-effort by design; skips are rare in practice even at full
streaming rates. Either way the command executes before the reply is
attempted, so a missing acknowledgement never means a lost frame.

### GET_CAPS pages

Pages are ordered so hosts only ever read forward: fetch page 0 once, poll
page 1 cheaply, and read later pages when something changed. The LED-map
fingerprint appears in pages 1 and 2, so a fetched map self-certifies against
the state that prompted the fetch.

**Page 0 - identity** (static): `[3]` caps format (2), `[4..11]`
factory-unique board ID, then two NUL-terminated strings: board label,
firmware version.

**Page 1 - runtime state** (the poll target): `[3]` current InputMode,
`[4]` profile number, `[5]` brightness step, `[6]` host-assigned player
(the XInput slot granted by the OS; 0 = none), `[7..10]` LED-map fingerprint,
`[11]` current animation index.

**Page 2 - LED map** (cache until the fingerprint changes): `[3]`
ledsPerButton, `[4]` colour format (0 GRB, 1 RGB, 2 GRBW, 3 RGBW), `[5]`
layout enum, `[6]` total LEDs, `[7]` brightness maximum, `[8..43]` 18 x
`[firstLED, count]` per button ID (0xFF = unmapped), `[44..47]` player LED
indexes, `[48]` turbo index, `[49..50]` case `[first, count]`, `[51..54]`
fingerprint.

**Page 3 - animations**: `[3]` current index, `[4]` count. Pairs with
`SET_ANIMATION`. The count is what the board actually offers, which varies
by board and by how the user has configured it - do not assume a fixed
number. `SET_ANIMATION` rejects an index at or above the reported count
with an invalid-argument status.

**Page 4 - per-light positions** (request `[3]=start entry`): `[3]` total,
`[4]` count in reply, then count x `[firstLED, gridX, gridY]`. Boards whose
render pipeline has no per-light position data report zero entries; hosts
fall back to the layout enum.

## Typical host flow

1. Discover by usage page; `PING`; require version >= 1.0.
2. `GET_CAPS` 0 (bind by unique ID), 1 (state), 2 (map), optionally 3 and 4.
3. `SET_MODE` with the desired takeover, timeout and brightness policy.
4. Stream: stage changes (`SET_BUTTONS`/`SET_RANGE`/`FILL`) and `COMMIT` once
   per frame; send `PING` on quiet frames to hold the takeover.
5. Poll page 1 periodically; if the fingerprint changes, re-read page 2.
6. `RELEASE` on shutdown - or simply stop; the timeout restores animations.

## Compatibility and versioning

- The protocol version is reported by `PING` (major, minor). From 1.0,
  command IDs and existing payload layouts are frozen. A minor version adds
  things - new commands in reserved slots, new capability pages, new optional
  trailing payload fields; only a major version may change existing
  behaviour.
- Hosts send unused payload bytes as zero; the board ignores unrecognized
  trailing payload bytes. New optional fields can therefore be appended to
  existing commands without breaking older hosts.
- Unknown commands are rejected with status `1` (unsupported); unknown
  capability pages with status `2` (invalid argument). Hosts should treat
  both as "not supported by this firmware", not as errors. Capability pages
  5 and above are reserved.
- Command IDs are grouped by function with room to grow: `0x01-0x0F` session
  and discovery, `0x10-0x2F` frame staging, `0x30-0x3F` frame lifecycle,
  `0x40-0x4F` board features, `0x70-0x7F` privileged management. New commands
  are assigned within the matching range; `0x00` is never a command and bit 7
  is reserved for the reply flag.
- The wire format addresses up to 255 LEDs (all indexes are single bytes);
  the current limit of 100 tracks the render pipeline's ceiling, so raising
  the pipeline limit needs no protocol change.

## Performance

The LED render loop runs at 100 Hz, which is the effective ceiling for
visible updates; 60 fps host streaming is recommended.

Input latency was measured on an RP2040 by instrumenting the input loop with
a counter and comparing against an identically instrumented build of stock
firmware on the same board:

| Firmware | Input sampling interval |
|---|---|
| Stock upstream | 16.70 us |
| Host Lighting, idle | 16.32 us |
| Host Lighting, 60 fps streaming | 16.51 us |
| Host Lighting, 100 fps streaming | 16.62 us |

Driving the lights costs about 0.2 us of sampling interval at 60 fps and
0.3 us at 100 fps - roughly 0.03 % of the 1 ms USB polling budget - with zero
missed commands across sustained runs. The cost does not scale with board
size: a 46-LED board, which needs four USB commands per frame instead of two,
measured the same 0.19 us at 60 fps and 0.29 us at 100 fps. The absolute figures sit slightly
below stock here, which is a binary-layout effect in the flash cache rather
than a real speed-up; the meaningful result is that no configuration measured
slower than stock. Merely enabling the add-on, with no host connected, costs
nothing measurable. With the add-on disabled the lighting code never runs at
all.

## Host implementations

**MESH** supports Host Lighting out of the box: boards are discovered
automatically, per-button LED maps are seeded from `GET_CAPS`, and game
state drives per-control lighting with the board's own animations as the
idle fallback. Any application that can read and write HID reports can
implement the protocol; small Python reference clients covering discovery,
capability decoding, streaming and board management accompany the pull
request that introduced this feature.

## Notes

- Takeover settings persist until the board reboots; hosts should always send
  `SET_MODE` at connect rather than assuming defaults.
- Switching input modes with `SET_INPUT_MODE` re-enumerates the device; modes
  without the lighting interface can only be left via the web configurator or
  boot-time button holds.
- `SET_RANGE_RGBW` may be sent to any board; the white component only renders
  on GRBW/RGBW strips (see page 2's colour format).
