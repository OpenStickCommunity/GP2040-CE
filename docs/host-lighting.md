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

Then send `PING` and require the `GPHL` magic and protocol version >= v1.0.
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
| 0x02 | GET_CAPS | `[2]=page` (+`[3]=start` for pages 4 and 5) | See pages below |
| 0x03 | SET_MODE | `[2]=takeover (0 whole-frame, 1 overlay), [3..4]=timeout ms LE (0->2000, min 100, max 10000), [5]=apply board brightness` | Send at connect; settings persist until reboot |

*Frame staging (0x10-0x2F)*
| 0x10 | SET_BUTTONS | `[2]=n (1-15)`, n x `[buttonId,R,G,B]` | Stage by control; reply `[3]=applied, [4]=skipped` |
| 0x11 | SET_RANGE | `[2]=start, [3]=count (1-20)`, count x `[R,G,B]` | Stage raw pixels |
| 0x12 | SET_RANGE_RGBW | `[2]=start, [3]=count (1-15)`, count x `[R,G,B,W]` | Boards without a white channel ignore W |
| 0x13 | FILL | `[2]=scope (0 all, 1 buttons, 2 case, 3 player LEDs), [3..5]=RGB` | Stage a scope. The buttons scope covers every button light, including any on the extended controls |
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
How many of them `SET_BUTTONS` colours depends on the board's LED framework.
The LED-refactor pipeline colours every light belonging to the control, so
one entry lights both Up buttons. The classic pipeline colours the single
light its per-control configuration names, and reaches the other under
whichever button ID that configuration parked it on. `FILL`'s buttons scope
reaches every button light on either.

Page 2 reports one LED range per control regardless. A host that needs all of
a control's lights, on any board, reads page 5 - which lists every light and
names its owner - and colours the extras by raw index with `SET_RANGE`.

Button IDs 0-17 follow GP2040-CE's gamepad bit order: Up, Down, Left, Right,
B1-B4, L1, R1, L2, R2, S1, S2, L3, R3, A1, A2. Specials: 24-27 player LEDs
1-4, 28 turbo LED, 29 the whole case range. 18 and 19 name A3 and A4, and
30-41 name E1-E12; page 2 has no slot for those, so they appear only in page 5
and a host learns them from there. 20-23 are permanently unassigned - those
gamepad bits are the dpad in a second encoding, not four more controls.
`0xFF` means no button ID names this light.

The extended IDs are report-only. `SET_BUTTONS` stages 0-17 and 24-29; an entry
naming A3, A4 or an E-button is counted as skipped, exactly as any control the
board has no light for. Colour those lights with `SET_RANGE` instead, using the
first LED and count page 5 gives for the record - which is why the record
carries both.

`SET_INPUT_MODE` values are the firmware's `InputMode` enum (0 XINPUT,
1 SWITCH, 2 PS3, 3 KEYBOARD, 4 PS4,
5 XBONE, 6 MDMINI, 7 NEOGEO, 8 PCEMINI, 9 EGRET, 10 ASTRO, 11 PSCLASSIC,
12 XBOXORIGINAL, 13 PS5, 14 GENERIC, 15 SWITCH_PRO, 16 P5GENERAL); the config
mode is entered via `REBOOT_WEBCONFIG` instead.

### Command round-trip

A command and its reply typically complete in about **2 ms** - one USB frame
out and one back, as expected for a 1 ms interrupt endpoint in each direction.
Measured over 300 pings the median is 2.0 ms on both pipelines and both test
boards.

Set host timeouts in the tens of milliseconds rather than near this figure.

### Replies while streaming

Every command is answered, and replies stay reliable at streaming rates: a
46-LED board driven at 60 fps - three staging reports plus a `COMMIT` every
frame - acknowledges 300 of 300 commits with none lost.

Because commands can be pipelined, replies arrive interleaved: the
acknowledgements for a frame's staging reports land while the host is still
waiting for that frame's `COMMIT` reply. Match each reply by its command
byte and sequence number, holding a small queue of recent replies. A host
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
`[11]` current animation index (0xFF = none selected), `[12..15]` feature
bitmask (bit 0 per-light positions are real, bit 1 the light table returns
entries, remaining bits zero), `[16]` LED framework (0 not reported,
1 classic, 2 LED-refactor), `[17]` animation namespace (0 not reported,
1 built-in effects, 2 stored profiles), `[18]` render rate in Hz
(0 = not stated).

A cleared feature bit is a promise the corresponding page returns nothing, not
merely that it might. Both bits describe the board as it is at the moment of
the read: the light registry is populated on the render core during LED setup,
so a host that enumerates early can legitimately see them clear and should
re-read when the fingerprint changes. The framework byte is diagnostic - it
exists so a support question can be answered in one read. Branch on the feature
bits and the per-record flags, never on it.

Read `[18]` rather than assuming a rate: classic renders at 100 Hz and the
LED-refactor at 40 Hz today, both are free to change, and a host streaming
faster than the board renders simply discards the difference with nothing to
tell it why.

**Page 2 - LED map** (cache until the fingerprint changes): `[3]`
ledsPerButton, `[4]` colour format (0 GRB, 1 RGB, 2 GRBW, 3 RGBW), `[5]`
layout enum, `[6]` LED count as the extent of the mapped range, `[7]`
brightness maximum, `[8..43]` 18 x `[firstLED, count]` per button ID
(0xFF = unmapped), `[44..47]` player LED indexes, `[48]` turbo index,
`[49..50]` case `[first, count]`, `[51..54]` fingerprint.

Page 2 answers "where do I write this control", for the eighteen canonical
controls. It is not an inventory of the board's lights. Absence from page 2
means the control has no slot here, not that no light exists. A board may carry
lights on controls page 2 cannot name, and lights with no owning control at
all. Where the page 1 light-table bit is set, page 5 is authoritative and
page 2's bindings are best-effort - some board configurations assign a
control's LED index to a light that control does not drive, because it is the
only way that pipeline can make the light work at all.

`[6]` is the extent - the highest LED index in use, plus one - which is the
number a host sizes a frame buffer from. It is deliberately not the sum of the
ranges above it, because that sum undercounts by exactly the lights page 2
cannot name.

**Page 3 - animations**: `[3]` current index (0xFF = none selected),
`[4]` count. Pairs with
`SET_ANIMATION`. The count is what the board actually offers, which varies
by board and by how the user has configured it - do not assume a fixed
number. `SET_ANIMATION` rejects an index at or above the reported count
with an invalid-argument status.

**Page 4 - per-light positions** (request `[3]=start entry`): `[3]` total,
`[4]` count in reply, then count x `[firstLED, gridX, gridY]`. Boards whose
render pipeline has no per-light position data report zero entries; hosts
fall back to the layout enum. Page 4 is a projection of page 5's columns and
is kept because it fits 19 entries per read against page 5's 4, so a host that
only wants positions keeps a cheap path.

**Page 5 - light table** (request `[3]=start entry`): `[3]` total records,
`[4]` start entry echoed, `[5]` count in this reply, `[6]` record stride in
bytes, `[7..]` the records, `[60..63]` the LED-map fingerprint.

Each record is 12 bytes:

| off | field | meaning |
|---|---|---|
| +0 | first LED | first LED index on the chain. **Not a unique key** - boards exist with two lights starting at the same index |
| +1 | LED count | LEDs in this light, which may differ from page 2's global `ledsPerButton` |
| +2 | light kind | 0 button, 1 case, 2 turbo, 3-6 player 1-4, `0xFF` unknown |
| +3 | button ID | the owning control, or `0xFF` if none names it |
| +4 | GPIO pin | the pin the control sits on, or `0xFF` when unknown or not applicable |
| +5..6 | GPIO action | the pin's action, signed 16-bit little-endian, verbatim. `0x8000` means no owning action |
| +7 | player index | 0-3, or `0xFF` |
| +8 | case group | the light's slot in the profile's non-button palette. Opaque, not unique, `0xFF` for none |
| +9..10 | grid X, Y | valid only when the position flag is set |
| +11 | flags | bit 0 position is real, bit 1 record came from a per-light table, rest zero. A clear bit claims nothing |

A light's identity is its **record ordinal**, which is why the reply echoes the
start entry. Records appear in the same order as page 4's, so ordinal *n* on
one page is ordinal *n* on the other.

Where a control owns several lights, several records carry the same button ID.
That is the page's purpose, and it needs no special case in a host.

Read the stride from `[6]` rather than assuming 12. A record can never be
widened in place - that would move every record after the first, and nothing
in the reply would let a parser notice - so future per-light fields go on a new
page. The stride exists so that rule is enforced by the format rather than
remembered.

Both flags are positive assertions: a set bit is the board vouching for
something, so a record asserting nothing reads as the weaker case. A board
with no per-light table rebuilds these records from its per-control
configuration and leaves bit 1 clear. Those rows are per-*control*, so a board
wiring two buttons to one action cannot be represented, and each action is
definitional rather than read from a live light. The layout is identical
either way: a board fills in what it knows and writes the sentinels where it
does not, so a host parses one way and reads the flags to learn what is real.
Availability is a property of the board, never of the protocol version.

The fingerprint in the tail lets a host notice the map changing underneath a
walk that takes several reads. If it changes mid-walk, discard the partial
table and start again.

## Typical host flow

1. Discover by usage page; `PING`; require version >= v1.0.
2. `GET_CAPS` 0 (bind by unique ID), 1 (state), 2 (map), optionally 3, and 4
   or 5. Page 5 is the full light inventory; page 2 alone suits a host that
   only colours canonical controls.
3. `SET_MODE` with the desired takeover, timeout and brightness policy. The
   timeout is clamped to the range in the command table; send `PING` to idle
   longer.
4. Stream: stage changes (`SET_BUTTONS`/`SET_RANGE`/`FILL`) and `COMMIT` once
   per frame; send `PING` on quiet frames to hold the takeover.
5. Poll page 1 periodically; if the fingerprint changes, re-read whichever of
   pages 2, 4 and 5 you cache - one fingerprint certifies all three, and a
   change mid-walk means discarding the partial table.
6. `RELEASE` on shutdown - or simply stop; the timeout restores animations.

## Compatibility and versioning

- The protocol version is reported by `PING` (major, minor). Command IDs and
  existing payload layouts are frozen. A minor version adds things - new
  commands in reserved slots, new capability pages, new optional trailing
  payload fields; only a major version may change existing behaviour.
- Hosts send unused payload bytes as zero; the board ignores unrecognized
  trailing payload bytes. Boards zero every unused reply byte and hosts ignore
  unrecognized trailing reply bytes, so new optional fields can be appended to
  existing commands and pages without breaking older hosts. An entry in a
  fixed-stride array is not a trailing field: an entry's width is frozen once
  the page ships, because widening it moves every entry after the first and
  nothing in the reply would let a parser notice. Further per-light fields go
  on a new page.
- The caps format byte (page 0 `[3]`) versions page 0's own layout, not the
  feature set. It stays at 2 for all of v1.x. Feature discovery is the page 1
  bitmask. Hosts must not require equality on it.
- Unknown commands are rejected with status `1` (unsupported); unknown
  capability pages with status `2` (invalid argument). Hosts should treat
  both as "not supported by this firmware", not as errors. Capability pages
  6 and above are reserved.
- Command IDs are grouped by function with room to grow: `0x01-0x0F` session
  and discovery, `0x10-0x2F` frame staging, `0x30-0x3F` frame lifecycle,
  `0x40-0x4F` board features, `0x70-0x7F` privileged management. New commands
  are assigned within the matching range; `0x00` is never a command and bit 7
  is reserved for the reply flag.
- The wire format addresses up to 255 LEDs (all indexes are single bytes);
  the current limit of 100 tracks the render pipeline's ceiling, so raising
  the pipeline limit needs no protocol change.

## Performance

The LED render loop's rate is the effective ceiling for visible updates, and it
differs by LED framework - the classic pipeline renders at 100 Hz, the
LED-refactor pipeline at 40 Hz. **Read the actual rate from page 1 `[18]`
rather than assuming one.** A host streaming faster than the board renders is
not an error and nothing reports it; the extra frames are simply never shown.
60 fps streaming suits a 100 Hz board.

### Input sampling cost

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

Building the light table and computing the map fingerprint happen only when a
host requests a capability page, never on the render path.

### Protocol throughput

Measured on both pipelines and both test boards, streaming whole frames of
`SET_RANGE` plus one `COMMIT`:

| | round-trip median | 60 fps commits | unthrottled |
|---|---|---|---|
| classic, 46 LEDs | 2.00 ms | 300/300, 0 lost | 122 fps |
| classic, 16 LEDs | 2.00 ms | 300/300, 0 lost | 247 fps |
| LED-refactor, 46 LEDs | 2.00 ms | 300/300, 0 lost | 122 fps |
| LED-refactor, 16 LEDs | 2.00 ms | 300/300, 0 lost | 249 fps |

Both pipelines hold a full 60 fps with no lost acknowledgements. Unthrottled
throughput scales with the number of reports a frame needs: four for a 46-LED
board, two for a 16-LED board. Both figures exceed either render rate, so
streaming faster than the rate page 1 `[18]` reports gains nothing.

## Host implementations

**MESH** supports Host Lighting out of the box: boards are discovered
automatically, per-button LED maps are seeded from `GET_CAPS`, and game
state drives per-control lighting with the board's own animations as the
idle fallback. Any application that can read and write HID reports can
implement the protocol; small Python reference clients covering discovery,
capability decoding, streaming and board management live in the
`gp2040ce-binary-tools` repository.

## Notes

- Takeover settings persist until the board reboots; hosts should always send
  `SET_MODE` at connect rather than assuming defaults.
- Switching input modes with `SET_INPUT_MODE` re-enumerates the device; modes
  without the lighting interface can only be left via the web configurator or
  boot-time button holds.
- `SET_RANGE_RGBW` may be sent to any board; the white component only renders
  on GRBW/RGBW strips (see page 2's colour format).

## Changelog

Everything above describes the protocol as it is now. This section records
what changed between versions, for hosts written against an earlier one.

Versions are the protocol version `PING` reports, not the firmware version.
A host should gate on `>= v1.0` and read the capability pages, never require
an exact version.

### v1.1 - 16 August 2026

Additive. No command ID and no existing payload layout changed: every field a
v1.0 host reads is still at the offset and width it was, so a v1.0 host keeps
working unchanged. Two values it can observe are more accurate than they were,
both listed under **Changed**. Input sampling and per-frame render cost are
unaffected - the new work runs only on capability reads.

**Added**

- **Capability page 5, the light table.** One 12-byte record per light,
  paged four to a report, naming the control that owns each light. It is the
  first page that can express a control owning several lights, which page 2
  structurally cannot. Records carry the owning button ID, GPIO, action,
  player index, case group, grid position and flags.
- **Page 1 appended fields** at `[12..18]`: a feature bitmask, the LED
  framework, the animation namespace, and the render rate in Hz. All sit
  after every v1.0 field, and each reserves 0 for "not reported".
- **Button IDs 18-19 (A3, A4) and 30-41 (E1-E12).** Report-only: page 5 names
  them, `SET_BUTTONS` does not stage them. Colour those lights with
  `SET_RANGE`.
- **A fingerprint on page 5**, matching pages 1 and 2, so one value certifies
  pages 2, 4 and 5 together and guards a paged walk.
- **Per-record flags** written as positive assertions, so a record asserting
  nothing reads as the weaker case.

**Changed**

- `FILL`'s buttons scope now covers every button light, including lights on
  controls page 2 cannot name.
- Page 2 `[6]` reports the extent of the mapped range rather than the sum of
  the ranges above it. On a board with a gap in its chain this is a larger,
  and correct, number: it is what a host needs to size a frame.
- `SET_MODE`'s timeout gained a 10 s ceiling. A host wanting to idle longer
  should send `PING`.
- The map fingerprint now covers the whole pin map, so a profile switch that
  remaps pins changes it even though no stored LED index moved.

**Fixed**

- Staged writes are bounds-checked against the addressable LED space, so a
  stored configuration naming an index past the end of the chain is a
  lighting fault rather than a memory fault.
- A case strip whose configured index falls outside the addressable space now
  reports as having no light, matching every other control. It previously
  clamped only the length, which could publish a zero-width or absurdly wide
  range through page 2 and the light table.
- The takeover is released when the USB host disconnects or suspends, rather
  than waiting for the keepalive to expire.
- This document previously said `SET_BUTTONS` colours every light belonging to
  a control. That is true only on the LED-refactor pipeline; the classic
  pipeline colours the one light its per-control configuration names. No
  firmware behaviour changed - the description was wrong. A host that needs
  every light on any board should read page 5.

### v1.0 - 11 August 2026

Initial public release for review.
