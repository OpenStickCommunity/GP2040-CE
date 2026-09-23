# Host Lighting

Host Lighting lets software on a connected PC drive a GP2040-CE board's RGB
LEDs in real time over USB, alongside the normal controller function. Game
state, per-button effects, ambient scenes - anything a host application can
compute, it can show on the board's lights, while the on-board animations
take over automatically the moment the host goes quiet.

## Overview

- The add-on is disabled by default. While it is disabled, every input mode
  presents byte-identical USB descriptors to stock firmware.
- Generic HID, Keyboard, SInput and XInput modes carry it. Console modes
  never do, and in XInput mode a console still sees a stock controller
  unless XInput Lighting is set to Always On.
- The board's own animations return when the host releases control or
  disconnects, and within a timeout (2 s by default) when it stops or
  crashes.
- The board describes itself - LED layout, controls, profiles and
  animations - from its live configuration, user remaps included, so host
  software needs no per-board configuration or updates when new boards are
  released.
- Beyond lighting, a host can switch the input profile, animation,
  animation speed and brightness as the board's hotkeys do, and change the
  input mode.

Users need only this section, How it works and Supported modes. Host
developers continue with Discovery and Typical host flow, and use the
Protocol reference as needed.

## How it works

The add-on is disabled by default, like all GP2040-CE add-ons. Enable it in
the web configurator, under
`Configuration > Add-Ons Configuration > Host Lighting`; board makers can
ship it on by defining `HOST_LIGHTING_ENABLED 1` in a board config.

When enabled, supported input modes expose one extra vendor-defined HID
interface next to the regular controller interface. Hosts exchange fixed
64-byte reports on it:

- Lighting commands write into a **staged frame** on the board; a `COMMIT`
  publishes it atomically to the LED render loop, so multi-report frames never
  tear.
- The render loop shows the host frame while it stays fresh. If the host
  releases control - or simply stops sending frames and keepalives for a
  configurable timeout (default 2 s) - the board's own animations resume
  seamlessly. A crashed or disconnected host can never leave the lights
  stuck.
- **Whole-frame** takeover replaces all lighting; **overlay** takeover only
  replaces pixels the host has explicitly staged, compositing host effects
  over the running animations.
- Everything the host needs to know - LED map, colour order, board identity,
  current state - is served by `HELLO` and the capability pages from the
  board's live configuration, including user remaps, and a host can subscribe
  to be told when it changes.

## Supported modes

| Input mode | Lighting interface |
|---|---|
| Generic HID | Yes |
| Keyboard | Yes |
| SInput | Yes |
| XInput | Yes - see below |
| PS3/PS4/PS5, Switch, Xbox, other console modes | Never |

**XInput** carries a choice, the `XInput Lighting` setting in the web
configurator's Host Lighting section:

- **Auto** (default): the board boots with its stock, console-identical
  identity. A console begins authentication immediately and the board stays
  stock - consoles never see any difference. A PC sends no console
  authentication, so after ~4 seconds the board re-enumerates as a composite
  whose controller interface binds the operating system's own Xbox 360 driver
  (Windows via MS OS descriptors, Linux via the kernel's xpad vendor match),
  with the lighting interface alongside. Hosts should expect the lighting
  device to appear a few seconds after plug-in in this mode.
- **Always On**: the composite identity from boot (PC-only while enabled).
- **Off**: stock XInput identity always; no lighting interface in XInput mode.

The composite presents its own USB identity, chosen so Windows queries the
MS OS descriptors that bind its Xbox 360 driver. A user USB ID override
replaces that identity, and Windows caches the query's verdict per VID:PID:
an ID the PC has already seen on a device without those descriptors can
leave the controller interface unbound there. The lighting interface is
HID-class and enumerates regardless - Host Lighting works in full under an
overridden ID. If the controller side fails after an override in this mode,
choose an ID that PC has not seen before.

## Discovery

Do not match VID:PID - it varies by input mode and user override. Enumerate
HID devices and match the top-level collection:

- **Usage page `0xFF47`, usage `0x0002`** - the usage is the protocol major
  version (see Compatibility and versioning)
- One 64-byte input report and one 64-byte output report, no report IDs
  (Windows buffers are 65 bytes with a leading `0x00` report-ID byte)

Then send `HELLO`, require the `GPHL` magic (GP2040-CE Host Lighting) and
protocol major version 2, and check that the reply lists the commands and
pages the host uses. Bind boards persistently by the factory-unique board ID
in the `HELLO` reply - never by device path, which changes with USB ports.

On Linux a board's hidraw node is open only to root until a udev rule grants
access; the [gp2040ce-binary-tools][tools] README gives one for the lighting
modes' USB IDs.

Multiple boards connected at once appear as fully independent lighting
devices - one interface per board, each addressed separately with no
interaction between them.

Each board serves one lighting host at a time. The board does not tell
applications apart: every application that opens its interface shares one
staged frame, one takeover, one set of `CONFIGURE` settings, one event
subscription and one reply queue, and receives every reply and event.
Applications coordinate through `CLAIM`, which records the controlling
application's token and name on page 1. An exclusive claim also has the
board refuse changes from applications that do not claim. The board cannot
verify who sends a request, so claims stay cooperative between applications
that do.

## Typical host flow

The simplest host, alone on a board, takes three steps: discover the board
and send `HELLO`; `STAGE` colours by action (a button's `GpioAction`) with
`COMMIT_AFTER`; and repeat that `STAGE` at least twice per keepalive timeout
(2 s by default) while the colours should stay. When it stops, the board's
animations return. The full flow below adds the page reads, the claim that
coordinates applications sharing a board, events and a clean shutdown.

1. Discover by usage page and usage; `HELLO`; require the `GPHL` magic and
   major 2, check the capabilities the host uses, and bind by the board ID.
2. `GET_PAGE` 8; then 0, 2 and 9-12 as the use case needs (below).
3. `CLAIM`, exclusive to keep applications that do not claim from changing
   the board. If the reply shows another holder, show its name, then wait or
   take over. Repeat `CLAIM` with `NO_REPLY` and the same claim flags at
   least twice per keepalive timeout, streaming or not, to keep the claim;
   while the claim is exclusive, set `HOLDER` on every request. A host that
   does not claim reads page 1 instead and waits while another application
   holds the claim. Either way, learn of a change of holder within each
   keepalive timeout: a `CLAIM` reply, `STATE_CHANGED` or a page 1 read
   shows it.
4. `CONFIGURE` the takeover, timeout and brightness policy.
5. `SUBSCRIBE` to `STATE_CHANGED`, then read page 1 once, since a change
   before the subscription raises no event; or poll page 1.
6. Stream: stage each frame with `NO_REPLY` and set `COMMIT_AFTER` on its last
   report. While the frame or an unsaved brightness step is shown, set
   `KEEPALIVE` on every repeated `CLAIM`, or each ends a keepalive timeout
   after its last refresh; a host that does not claim sends `HELLO` with
   `KEEPALIVE` and `NO_REPLY` at least twice per timeout instead.
7. When the state token changes - from an event or a page 1 poll - re-read
   the cached pages (Lights: groups 1-3 only), passing the new token as the
   expected one.
8. On shutdown, `CLAIM` give up, which also ends the takeover and an unsaved
   brightness step (`RELEASE` and `SET_BRIGHTNESS` 0xFF without a claim,
   unless another application holds it) - or simply stop; the timeout
   restores the animations and the brightness, and ends the claim.

Repeat from step 1 after a replug, reboot or input-mode change: `CONFIGURE`
settings last until reboot and a subscription until USB unmounts.

| Use case | Commands and pages |
|---|---|
| Light a control by name | None: `HELLO`, then `STAGE` by action |
| Stream a whole frame | 8 |
| Per-light effects with geometry | 8, 11 |
| Hand single lights back to the animation | None: `UNSTAGE` in overlay takeover |
| Configure host inputs from the board | 9, 10 |
| Switch the input profile | `SET_PROFILE`; 9 |
| Dim for a session, or set the brightness | `SET_BRIGHTNESS`, unsaved or saved; 1 for the step shown, 8 for the step count |
| Set an animation's speeds | `SET_ANIMATION_SPEED`; 12 for the speeds, 8 for the step count |
| Switch the input mode | `SET_INPUT_MODE`; 2 |
| Full inventory and UI | 0, 2, 8-12 |
| Share a board with other applications | `CLAIM`; 1 |
| Keep applications that do not claim off the board | `CLAIM` exclusive, `HOLDER`; 1 |
| Stay current | `SUBSCRIBE`, or poll 1 |

## Protocol reference

The wire protocol carried on this interface is the Host Lighting Protocol
(HLP) - the `GPHL` magic in `HELLO` replies and the `hlp-` prefix on the
reference tools refer to it. This section describes HLP 2.0. Multi-byte
fields are little-endian.

### Reports

All transfers are 64-byte reports:

    request  [0] command       [1] sequence  [2] flags   [3..63] payload
    reply    [0] command+0x80  [1] sequence  [2] status  [3..63] payload
    event    [0] 0x80          [1] event     [2] 0       [3..63] payload

The host chooses the sequence byte and the reply echoes it. There is no
command 0x00, so `[0]` = 0x80 always marks an event. A report whose `[0]` is
0x00 or has bit 7 set is not a request: the board ignores it and sends no
reply.

Request flags, the same on every command:

| Bit | Name | Effect |
|---|---|---|
| 0 | COMMIT_AFTER | Publish the staged frame after this command succeeds, as `COMMIT` does |
| 1 | NO_REPLY | Send no reply to this command, whatever its status |
| 2 | KEEPALIVE | Refresh the keepalive of a live takeover and of an unsaved brightness step after this command succeeds; start neither |
| 3 | HOLDER | Sent by the holder of an exclusive claim on every request; ignored while no exclusive claim is held (see `CLAIM`) |
| 4-7 | - | Send as zero; the board ignores them |

Status: `0` OK, `1` unsupported command, `2` invalid argument, `3` stale token
(see Pages), `4` claimed (see `CLAIM`). A command whose status is not OK
changes nothing, answered or not, and neither `COMMIT_AFTER` nor `KEEPALIVE`
applies. A status 2 reply carries at `[3]` the offset of the invalid request
byte; when several are invalid, the lowest. A status 4 reply carries at
`[3..23]` the holder, laid out as page 1's Controller group.

### Field widths

Each quantity has one width wherever it appears:

| Width | Quantities |
|---|---|
| 4-bit | Addressing mode, pixel format (`[3]` of `STAGE`, `UNSTAGE` and `FILL`) |
| 8-bit | Pin, key, kind, grid coordinate, enumerated value, string byte; sequence, takeover number, versions, page, profile, brightness, animation speed, player, palette slot, reply queue limit; counts, offsets and strides within one report |
| 16-bit | LED index and count, light ordinal and count, animation ordinal, record index and count, control instance and instances, grid size, render rate, time in ms |
| 16-bit, signed | Action |
| 32-bit | State token, host token |
| 64-bit | Board ID |
| 1 bit per member | Bitmaps: flags (8), claim flags (8), addressing modes (16), pixel formats (16), events (16), pages (32), commands (128), field groups (16), `STAGE` and `UNSTAGE` outcome (32) |
| Per format | Pixel: 24, 32 or 16 bits (Pixel formats) |

Strings are UTF-8, byte for byte as the board stores them, and every string
length is in bytes. A string cut to fit its field can end in a partial
character, so hosts decode leniently.

### Replies

Replies are guaranteed within a limit. The board queues each reply until the
IN endpoint takes it; `HELLO` reply `[17]` states how many replies the queue
holds. A host that keeps no more requests unanswered than that never loses a
reply; applications sharing a board share the limit. A request that needs a
reply and arrives while the queue is full is discarded, neither executed nor
answered, and its sequence number is missing from the replies. A request
sent with `NO_REPLY` needs no place in the queue, so it is never discarded.

The board executes requests in the order they arrive and sends their replies
in the same order. Commands can be pipelined; match each reply by its command
byte and sequence number, since an event can arrive between two replies and a
discarded request leaves a gap. A reply that matches none of a host's
requests is another application's; the host ignores it. A reply with a
host's command and sequence can still be another application's; a page
reply's echoed page, start and field groups (page and start alone in a
stale-token reply), and a `CLAIM` reply's echoed action and token, tell them
apart.

Replies wait in each application's input buffer until it reads them, other
applications' replies included. Before sending a request, a host reads the
reports already waiting, keeps the events and the replies it still awaits,
and discards the rest: none of them answers the new request, and an idle
application would otherwise take another's earlier reply with the same
command and sequence for its own. A host starts its sequence numbers at a
random value, so two applications seldom send the same one.

A command and its reply typically complete in about **2 ms** - one USB frame
out and one back, as expected for a 1 ms interrupt endpoint in each direction.
While the board writes its settings flash it answers nothing for about
200 ms. It writes about a second after a change of animation, animation
speed or saved brightness, and at once after a profile switch or another
setting a hotkey changes, whether a host or the board's own hotkeys made the
change. Set request timeouts of 250 ms or more, so such a write is not taken
for a lost board.

A streaming host sends each frame's staging reports with `NO_REPLY` and sets
`COMMIT_AFTER` on the last one, without `NO_REPLY`: that report stages,
publishes and is the frame's only acknowledgement.

### Commands

| Cmd | Name | Request `[3..]` | Reply `[3..]` |
|---|---|---|---|
| | **Session and discovery (0x01-0x0F)** | | |
| 0x01 | HELLO | - | `[3..6]="GPHL"`, `[7]` major, `[8]` minor, `[9..16]` board ID, `[17..46]` limit, capabilities and session (below) |
| 0x02 | GET_PAGE | `[3]` page, `[4..5]` start record, `[6..9]` expected state token (0 = no check), `[10..11]` field mask (0 = all) | See Pages |
| 0x03 | CLAIM | `[3]` action (0 claim, 1 take over, 2 give up), `[4..7]` host token, `[8..23]` application name, `[24]` claim flags (bit 0 exclusive) | `[3..7]` action and host token as sent; `[8..11]` holder token, `[12..27]` holder name, `[28]` holder's claim flags |
| 0x04 | CONFIGURE | `[3]` takeover (0 whole-frame, 1 overlay), `[4..5]` keepalive timeout ms (0 = 2000, clamped to 100-10000), `[6]` apply board brightness (0 no, 1 yes) | The settings applied, same layout |
| 0x05 | SUBSCRIBE | `[3..4]` event mask | `[3..4]` events enabled |
| | **Frame staging (0x10-0x1F)** | | |
| 0x10 | STAGE | See Staging | `[3]` applied, `[4]` skipped, `[5..8]` outcome mask |
| 0x11 | UNSTAGE | See Staging | `[3]` applied, `[4]` skipped, `[5..8]` outcome mask |
| 0x12 | FILL | `[3]` bits 4-7 pixel format, bits 0-3 zero; `[4..]` one pixel | - |
| 0x13 | CLEAR | - | - |
| | **Frame lifecycle (0x20-0x2F)** | | |
| 0x20 | COMMIT | - | - |
| 0x21 | RELEASE | - | - |
| | **Board features (0x30-0x4F)** | | |
| 0x30 | SET_PROFILE | `[3]` profile number | - |
| 0x31 | SET_ANIMATION | `[3..4]` index | - |
| 0x32 | SET_ANIMATION_SPEED | `[3..4]` index, `[5]` idle, `[6]` pressed, `[7]` case speed (0xFF unchanged) | - |
| 0x33 | SET_BRIGHTNESS | `[3]` brightness step (0xFF the saved step), `[4]` save (0 no, 1 yes) | - |
| | **Privileged management, magic-guarded (0x70-0x7F)** | | |
| 0x70 | SET_INPUT_MODE | `[3]` InputMode, `[4..7]="MODE"` | - |
| 0x71 | REBOOT | `[3]` target (0 gamepad, 1 web configurator, 2 BOOTSEL), `[4..7]="BOOT"` | - |

`HELLO` reply:

    [3..6]   "GPHL"                    [7]      major    [8] minor
    [9..16]  board ID                  [17]     reply queue limit
    [18]     request flags             [19..34] commands
    [35..38] pages                     [39..40] addressing modes
    [41..42] pixel formats             [43..44] events
    [45..46] events enabled now

The board ID is the board's 8-byte factory-unique ID. `[18..44]` are
capability bitmaps: bit n set means the board implements request flag n,
command n, page n, addressing mode n, pixel format n or event n. `[45..46]`
are the events this USB session is subscribed to. A host checks the
capabilities it uses instead of comparing version numbers.

`GET_PAGE` reads a page; see Pages.

`CLAIM` records which application controls the board. A host picks a random
non-zero 32-bit token when it starts and names itself in up to 16 bytes,
NUL-padded. Claim (0) takes the claim when the board is unclaimed and keeps
it when the token already holds it; take over (1) always takes it; give up
(2) clears a claim held under the same token. Otherwise nothing changes. The
reply is OK either way. It echoes the action and token sent, and a host
takes as its own only a reply that echoes what it sent. It then shows the
holder now, laid out as page 1's Controller group: a host compares the
holder's token with its own; token 0 means unclaimed. A token of 0 or an
unknown action is an invalid argument, and that reply has no echo.

A claim is a lease: each claim (0) or take over (1) that leaves the sender
holding it starts it again and records the name and claim flags sent, and no
other command extends it, requests with `HOLDER` included. It ends on give
up, when its holder sends neither within the keepalive timeout, and on USB
unmount or reboot; `RELEASE` does not end it. Every change of holder - a
first claim, a take over, a give up or a lapse - changes page 1, ends the
takeover and an unsaved brightness step, and clears the staged frame, so
each holder starts from an empty frame and a crashed holder's last frame
does not stay on the LEDs, whatever other applications send.

Claim flag bit 0 makes the claim exclusive; the other bits are ignored.
The board does not enforce a shared claim: commands from every application
still act, so an application that does not hold the claim sends no
`CONFIGURE` and does not drive the LEDs while another holds it. While an
exclusive claim is held, a request without `HOLDER` is refused with status
4 unless it is `CLAIM`, or `HELLO`, `GET_PAGE` or `SUBSCRIBE` without
`COMMIT_AFTER` or `KEEPALIVE`; an unknown command still returns status 1,
and a refused request's arguments are not checked. `CLAIM` is never
refused, so take over still works, but a `CLAIM` sent without `HOLDER`
under an exclusive claim has its `COMMIT_AFTER` and `KEEPALIVE` act only
when it leaves the sender holding the claim. The holder sets `HOLDER` on
every request while its claim is exclusive. The board cannot verify the
flag: an exclusive claim protects the holder from applications that do not
claim, and a previous holder is refused only once it has seen the change
(its next `CLAIM` reply, `STATE_CHANGED` or page 1 read, at the latest
within a keepalive timeout), so a host that takes over stages the whole
frame again once the keepalive timeout in force when it took over has
passed. The board's own hotkeys
are not subject to a claim.

`CONFIGURE` sets the takeover mode, the keepalive timeout and whether host
pixels are scaled by the board's brightness; a takeover or brightness value
above 1 is an invalid argument. Each setting applies at once, to a live
takeover too, and the timeout also to the claim's lease and to an unsaved
brightness step. The settings last until the board reboots and start as
whole-frame, 2000 ms and brightness applied; page 1 shows those in force.
Send `CONFIGURE` after every connect, once no other application holds the
claim, rather than assuming them.

`SUBSCRIBE` enables events; see Events.

Each pixel of the staged frame is staged or unstaged. `STAGE` and `UNSTAGE`
change chosen LEDs; see Staging. `FILL` stages every LED below the
addressable LED limit with one pixel; to fill the lights of one kind,
`STAGE` by kind with one entry. `CLEAR` unstages every pixel and sets it to
off. The takeover mode decides only what an unstaged pixel shows: off in
whole-frame takeover, the on-board animation in overlay.

`COMMIT` publishes the staged frame atomically and starts a takeover when
none is live. The staged frame is kept while the takeover lasts, so the
next frame stages only the pixels that change. A host that does so stages
whole frames until page 1 shows its takeover live, then notes the takeover
number (State `+7`), and stages the whole frame again when page 1 shows
flag bit 0 clear or another number: the takeover ended, and its next frame
may already have started a new one. `RELEASE` returns the LEDs to the
on-board animations at once; a claim survives it.

A staging or lifecycle command (0x10-0x2F), or any command sent with
`COMMIT_AFTER` or `KEEPALIVE`, refreshes the keepalive of a live takeover
and of an unsaved brightness step when it succeeds, whether or not it is
answered. Other commands do not, `HELLO` and `CLAIM` included (an unsaved
`SET_BRIGHTNESS` restarts its own step's keepalive), so an
application that only watches the board or only holds the claim never keeps
another's takeover or unsaved step alive. The takeover ends on
`RELEASE`, when no refreshing command succeeds within the timeout, when the
holder of `CLAIM` changes, and when USB unmounts or suspends. The board then
returns to its animations and clears the staged frame as `CLEAR` does, so
every takeover starts from an empty frame; `RELEASE` clears it even when no
takeover is live.

`SET_PROFILE` selects the input profile as GP2040-CE's profile hotkeys do:
applied live and persisted. A profile that does not exist or is not enabled
is an invalid argument; page 9 lists them.

`SET_ANIMATION` selects the on-board animation, shown outside a takeover and
under unstaged pixels in overlay: the index is an Animations page ordinal,
or 0xFFFF for off, the state GP2040-CE's animation hotkeys reach after the
last enabled animation, in which no animation lights anything and only the
board's RGB player and turbo indicators still show. It is applied live and
persisted. Any other index at or above the page's total, or of an animation
not enabled (page 12 flag bit 0), is an invalid argument.

`SET_ANIMATION_SPEED` sets the idle, pressed and case speeds of one on-board
animation, the speeds the web configurator's LED page shows and GP2040-CE's
animation speed hotkeys step: applied live and persisted. The index is a
page 12 ordinal, and a speed of 0xFF leaves that speed as it is. An index at
or above the page's total, or a speed other than 0xFF above the Summary's
speed steps (`+9`), is an invalid argument. The speeds are on page 12, so a
change also changes the state token.

`SET_BRIGHTNESS` sets the brightness step the board shows, live. With `[4]`
= 1 the step is also saved, as GP2040-CE's brightness hotkeys save it. With
`[4]` = 0 it is unsaved: the saved step stays and nothing is written to
flash. An unsaved step lasts as a takeover does: it ends a keepalive timeout
after the last unsaved `SET_BRIGHTNESS` or other command that refreshes its
keepalive, when the holder of `CLAIM` changes, and on USB unmount, suspend
or reboot, and the board then shows the saved step again; `RELEASE` does not
end it. Step 0xFF shows the saved step at once, whatever `[4]`. A brightness
hotkey steps from the saved step and ends an unsaved one; a saved step
replaces it. A step other than 0xFF above the Summary's step count (`+8`),
or a save above 1, is an invalid argument. Page 1 shows the step shown
(State `+6`) and whether it is unsaved (State flag bit 1). Host pixels
follow the step shown when `CONFIGURE` `[6]` is 1.

A board without LED output reports no brightness steps, no speed steps and
no animations (Summary `+8`, `+9`, page 12 empty), so `SET_BRIGHTNESS`
accepts only 0 and 0xFF, `SET_ANIMATION` only 0xFFFF (off) and
`SET_ANIMATION_SPEED` no index.

For `SET_PROFILE`, `SET_ANIMATION`, `SET_ANIMATION_SPEED` and a saved
`SET_BRIGHTNESS`, a value equal to the current one (for `SET_BRIGHTNESS`,
the saved step) succeeds and writes nothing to flash; a saved step still
ends an unsaved one. Any other value is saved to the board's settings
flash, a profile at once and the others about a second later; the board
answers nothing for about 200 ms while it writes (Replies), and every write
wears the flash. Send these commands for a user's choice, not as an effect:
to dim the animations for a session, send an unsaved `SET_BRIGHTNESS`,
which writes nothing; to dim or animate host pixels, scale them. Time the
board spends writing its settings flash, for these commands or its own
settings, does not count toward the keepalive timeout or a claim's lease.

`SET_INPUT_MODE` values are the running firmware's `InputMode` enum
(`proto/enums.proto`); a mode the firmware has no driver for is an invalid
argument, and so is the config mode (`INPUT_MODE_CONFIG`), which `REBOOT`
target 1 enters. The board saves the mode and reboots into it; the current
mode is not written again, but the board still reboots. A mode without the
lighting interface is left only through the web configurator, a boot-time
button hold, the reboot hotkeys or, on a board with a display, its menu;
page 2 lists the modes with a driver and those with the interface, so a host
can warn before switching. Into a mode with page 2 flag bit 1, the interface
can take a few seconds and a second re-enumeration to return (Supported
modes).

`REBOOT` targets are the web configurator's reboot modes (its `/api/reboot`
`bootMode`), as the running firmware defines them: 0 into the saved input
mode, 1 into the web configurator, 2 into the UF2 bootloader. An unknown
target is an invalid argument. The reply to `SET_INPUT_MODE` or `REBOOT` is
sent before the reboot; a host confirms success by the device re-enumerating.

### Pages

`GET_PAGE` reads a capability page. What each page is for:

| Page | Name | What it is for | Read it |
|---|---|---|---|
| | **Board (0-7)** | | |
| 0 | Identity | Names the board and its firmware: label, board config, platform, version and build | For display and diagnostics; it changes only with the firmware |
| 1 | State | Shows what is happening now: input mode, profile, player, animation, brightness, takeover and its `CONFIGURE` settings, and the application holding `CLAIM` | To poll, or as the `STATE_CHANGED` payload; it changes at any time |
| 2 | Input modes | Lists the input modes `SET_INPUT_MODE` accepts: value, name, and whether each presents the lighting interface | Before `SET_INPUT_MODE`; it changes only with the firmware and its settings |
| 3-7 | - | Free for board pages | - |
| | **Configuration (8-15)** | | |
| 8 | Summary | Sizes a host: LED extent and limit, render rate, colour format, brightness and grid size, each configuration page's record count, and the animation speed steps | First, after `HELLO` |
| 9 | Profiles | Lists the input profiles: number, enabled, label | To show or switch profiles (`SET_PROFILE`) |
| 10 | Controls | Lists every control - a GPIO pin or add-on input with an action - in the active profile, lit or not: which instance of its action it is, where it sits, and its lights | To configure host inputs, or to show where each control sits |
| 11 | Lights | Lists every light: its LEDs, kind, owning pin and action, grid position, palette slot | For per-light effects and geometry |
| 12 | Animations | Lists the on-board animations: enabled, idle, pressed and case effects, and their speeds | To show or pick the idle animation (`SET_ANIMATION`) or set its speeds (`SET_ANIMATION_SPEED`) |
| 13-15 | - | Free for configuration pages | - |
| | **Diagnostics (16-23)** | None in 2.0 | - |
| | **Reserved (24-31)** | For new groups (see Number spaces) | - |

Hosts cache the configuration pages: the state token changes whenever
anything on them changes, including a profile switch and an animation speed.

Every reply has the same header:

    [3]      page, echoed             [4..5]   start record, echoed
    [6..7]   total records            [8]      records in this reply
    [9]      record stride in bytes   [10..11] field groups returned
    [12..59] records                  [60..63] state token

The page, start and field groups sit where the request has them. Pages 0, 1
and 8 are tables of one record. A start at or past the total returns no
records. An unknown page is an invalid argument.

**Field groups.** Each page defines its record as an ordered list of field
groups, one mask bit per group. The reply takes the requested groups in bit
order and returns each one that fits in what remains of the 48-byte record
area; it echoes the groups returned at `[10..11]` and sets the stride to their
total width. It carries as many records as fit in the record area, from the
start record up to the total. Mask 0 requests every group; bits the board
does not define are ignored and never echoed, and a mask naming no group the
page defines returns the total, no records and an empty echo. A group
missing from the echo was not returned and can be requested on its own; only
page 0's strings can fail to fit. Pages 8 and 9 have one group, bit 0.

**State token.** `[60..63]` of every reply carries the current state token.
It changes whenever anything a configuration page (8-15) reports changes,
and for nothing else; board and diagnostics pages are not covered. It is
opaque - compare it for equality only - and never 0. A request naming an
expected token that is no longer current returns status 3 with the page and
start echoed, no records, no field groups and the current token, so a walk
spanning several reads restarts instead of mixing two states.

**Page 0 - Identity**, static for the life of the firmware: the board and
build identification the firmware carries (`headers/version.h.in`, the
board config and the build), one string of up to 48 bytes per group. The
web configurator's `/api/getFirmwareVersion` returns all but the version ID.

| Bit | Group | Example |
|---|---|---|
| 0 | Board label | `Haute42 COSMOX M Ultra` |
| 1 | Board config | `Haute42COSMOXMUltra` |
| 2 | Platform | `rp2040` |
| 3 | Firmware version | `v0.7.12-88-g8c5a8da` |
| 4 | Version ID | `0.7.12` |
| 5 | Build ID | `8c5a8da` |
| 6 | Build type | `Release` |
| 7 | Firmware file name | `GP2040-CE_0.7.12_Haute42COSMOXMUltra` |

Each string is NUL-terminated when shorter than 48 bytes, fills the
group when it is 48, and is cut to 48 when longer; each starts after the
previous one's NUL, or after its 48th byte. Mask 0 returns the groups that
fit, in bit order; the echo shows which, and the rest are read by their own
bits. The board ID is in the `HELLO` reply.

**Page 1 - State**, one record in three field groups:

| Bit | Group | Bytes |
|---|---|---|
| 0 | State | 8 |
| 1 | Controller | 21 |
| 2 | Settings | 4 |

State:

| Off | Field |
|---|---|
| +0 | Flags: bit 0 a host frame is shown; bit 1 the brightness step shown is unsaved; rest zero |
| +1 | Current InputMode |
| +2 | Active input profile (page 9); 1 is the base mapping |
| +3 | Player: the player number the PC's controller driver assigned, as the input mode's driver records it (1-4 in XInput); 0 none |
| +4..5 | Current animation index; 0xFFFF off (`SET_ANIMATION`) |
| +6 | Brightness step shown, 0 to the Summary's step count |
| +7 | Takeover number: increments at every takeover start |

Controller, the application holding `CLAIM`:

| Off | Field |
|---|---|
| +0..3 | Holder token; 0 unclaimed |
| +4..19 | Application name, up to 16 bytes, NUL-padded |
| +20 | Claim flags: bit 0 exclusive; rest zero |

Settings, as `CONFIGURE` last set them:

| Off | Field |
|---|---|
| +0 | Takeover: 0 whole-frame, 1 overlay |
| +1..2 | Keepalive timeout in ms |
| +3 | Board brightness applied to host pixels: 0 no, 1 yes |

State flag bit 0 is set while a takeover is live: from the `COMMIT` that
starts it until `RELEASE`, keepalive expiry, a change of `CLAIM` holder, or
USB unmount or suspend ends it. Bit 1 is set while the step shown differs
from the saved step: from an unsaved `SET_BRIGHTNESS` until the step ends
(`SET_BRIGHTNESS`).

**Page 2 - Input modes**, one record per mode `SET_INPUT_MODE` accepts -
every `InputMode` the firmware has a driver for, the config mode excluded -
in `InputMode` order, fixed while the interface exists:

| Bit | Group | Bytes |
|---|---|---|
| 0 | InputMode, flags | 2 |
| 1 | Name | 22 |

All groups make a 24-byte record, two per reply; bit 0 alone, 24.

| Field | Meaning |
|---|---|
| InputMode | The mode's value, verbatim |
| Flags | Bit 0 lighting: the mode presents this interface, XInput when the `XInput Lighting` setting is Auto or Always On; bit 1 detected: the interface appears only once the board has detected a PC, a few seconds after it enumerates (XInput under Auto); rest zero |
| Name | The firmware's name for the mode, as its display menu shows it; up to 22 bytes, NUL-padded |

A host offering a mode switch lists these modes and warns for one without
flag bit 0.

**Page 8 - Summary**, one 26-byte record:

| Off | Field |
|---|---|
| +0..1 | LED extent: the highest LED index below the limit in use, plus one |
| +2..3 | Addressable LED limit: `STAGE`, `UNSTAGE` and `FILL` write below it |
| +4..5 | Render rate in Hz; 0 not stated |
| +6 | Colour format (the firmware's `LEDFormat`): 0 GRB, 1 RGB, 2 GRBW, 3 RGBW |
| +7 | Brightness maximum, 0-255; at least the step count |
| +8 | Brightness steps: the highest State brightness step |
| +9 | Animation speed steps: the highest speed on page 12 |
| +10..11 | Grid width: the highest light grid X, plus one; 0 when there are no lights |
| +12..13 | Grid height, likewise |
| +14..15 | Profiles: page 9's total |
| +16..17 | Controls: page 10's total |
| +18..19 | Lit controls |
| +20..21 | Unlit controls |
| +22..23 | Lights: page 11's total |
| +24..25 | Animations: page 12's total |

Size a frame buffer from the extent. It is not the sum of the lights' LED
counts: a chain can have gaps. LEDs from the extent up to the limit can be
staged but are never shown. The lit and unlit counts come from the same
walk as the Controls records and carry the same token.

**Grid.** Positions are cells of GP2040-CE's LED layout grid, as the web
configurator's LED Configuration page shows it: (0,0) at the top left, X to
the right, Y down, square cells. They give the lights' arrangement, not
distances. On a board whose configuration gives only per-button LED indexes,
GP2040-CE approximates the grid from the button layout.

**Page 9 - Profiles**, one 18-byte record per input profile, in profile
order. Input profiles are GP2040-CE's button mappings: profile 1 is the base
mapping, and each other profile remaps the pins.

| Off | Field |
|---|---|
| +0 | Profile number, as GP2040-CE numbers them |
| +1 | Flags: bit 0 enabled |
| +2..17 | Label, up to 16 bytes, NUL-padded |

Every defined profile is listed, enabled or not; profile 1 is always
enabled. `SET_PROFILE` selects one and State `+2` shows the active one. The
Controls and Lights pages report the active profile's mapping (add-on
inputs have one mapping for every profile).

**Page 10 - Controls**, one record per input whose action is greater than
zero, lit or not, in key order. An input is a GPIO pin or an add-on input:
a Hall-effect trigger or an I/O-expander pin.

| Bit | Group | Bytes |
|---|---|---|
| 0 | Key, action (signed 16-bit), flags | 4 |
| 1 | Instance, instances (16-bit each) | 4 |
| 2 | Position: first LED (16-bit), grid rectangle | 6 |
| 3 | First light ordinal, light count (16-bit each) | 4 |

All groups make an 18-byte record, two per reply; bits 0-2, three; bits 0
and 1, six; bit 0 alone, twelve.

| Field | Meaning |
|---|---|
| Key | The input, below: an input carries exactly one action |
| Action | The input's action now, verbatim: a GPIO pin's from the active profile, an add-on input's as its add-on resolves it |
| Flags | Bit 0 duplicated: another control has the same action; bit 1 lit: at least one Lights record carries this key as owner pin; rest zero |
| Instance | This control's place among the controls with its action, from 1, in key order |
| Instances | How many controls have this action, lit or not; 1 when not duplicated |
| First LED | The first LED of the control's first light; 0xFFFF when unlit |
| Grid rectangle | Min X, min Y, max X, max Y of the control's lights (Grid, under page 8), 8-bit each; a point for one light. Valid when lit (flag bit 1); zero otherwise |
| First light ordinal | The lowest ordinal among those lights; 0xFFFF when unlit |
| Light count | How many lights carry this key as owner pin |

| Key | Input |
|---|---|
| 0-47 | GPIO pin, by number, below the chip's GPIO count |
| 0x80-0x9F | Hall-effect trigger n = key - 0x80: `HETriggerOptions.triggers[n]` |
| 0xC0-0xCF | PCF8575 pin n = key - 0xC0: `PCF8575Options.pins[n]` |

Other keys are free or reserved (Number spaces); a host treats a key it
does not know as a control it cannot name. A GPIO pin's action comes from
the active profile; an add-on input's comes from its add-on, not the
profile (the state token changes whenever it does), and it is listed while
its add-on runs. The Hall Effect Trigger add-on runs while enabled and
reads `muxChannels` triggers on each of up to four muxes, 32 at most,
with `muxChannels` 1, 4, 8 or 16; only those are listed, and none under
another `muxChannels`. The PCF8575 add-on runs when enabled and its chip
is found on I2C at boot; an expander pin set as an output mirrors a button
and is not an input. Lights follow GPIO pins only, so an add-on control is
never lit and has no position.

A control has a position exactly when it is lit, through its lights; the
rectangle covers all of them, so the Controls page alone says where every
lit control sits. A control's lights need not be consecutive; where the
count exceeds one, the Lights page has each one by owner pin. Instances
above 1 is the same fact as flag bit 0; the numbering holds while the state
token is unchanged.

`NONE`, `RESERVED` and `ASSIGNED_TO_ADDON` are not greater than zero, so I2C,
USB-host and LED-data pins never appear. Modifiers and alternate-direction
actions do, with the action verbatim; a host filters on the actions it uses.
A control absent from this page does not exist, so a host can configure its
inputs from this page alone. The page comes from the pin map and the add-on
settings, so a board with no lights still lists every control, unlit and
without a position.

**Page 11 - Lights**, one record per light: one entry of the board's LED
layout, a run of LEDs with a kind and a grid position. A button or turbo
light is tied to a pin; a case or player light takes a palette slot instead.
A light's identity is its record ordinal, which is also its `STAGE` address.
The page is complete whenever the lighting interface exists; a board with no
LED output has no lights (Summary `+22..23` is 0). LEDs at or past the
addressable LED limit (Summary `+2..3`) are never written and not counted in
the extent, so a host clips a light's LEDs to the limit; an address whose
lights all lie at or past it is skipped (Staging).

| Bit | Group | Bytes |
|---|---|---|
| 0 | First LED, LED count (16-bit each) | 4 |
| 1 | Kind, flags | 2 |
| 2 | Owner pin, owner action (signed 16-bit) | 3 |
| 3 | Owner instance, owner instances (16-bit each) | 4 |
| 4 | Grid X, grid Y | 2 |
| 5 | Palette slot | 1 |

All groups make a 16-byte record, three per reply; positions alone (bit 4),
24 per reply; LED ranges alone (bit 0), 12; owner, instance and position
(bits 2-4), five.

| Field | Meaning |
|---|---|
| First LED | First LED index on the chain. **Not a unique key** - two lights can start at the same index |
| LED count | LEDs in this light |
| Kind | See Actions and kinds |
| Flags | Bit 0 duplicated: the owner pin is a control whose action another control also has; rest zero |
| Owner pin | The GPIO pin a button or turbo light is tied to; 0xFF none |
| Owner action | That pin's action in the active profile, verbatim; 0x8000 none |
| Owner instance, instances | The owning control's instance and instances (Controls group 1); 0 when there is no owner pin or it is not a control |
| Grid X, Y | The light's cell (Grid, under page 8) |
| Palette slot | The light's slot in the animation profile's non-button colour palette (case and player lights); opaque, not unique; 0xFF none |

An owner pin that is not on the Controls page carries no action (`NONE`,
`RESERVED` or `ASSIGNED_TO_ADDON`); the owner action says which. Where a
control owns several lights, several records carry its pin and action, in
ordinal order. The lights are fixed while the interface exists: a profile
switch changes only the owner action, the owner instances and flag bit 0, so
after a token change a host re-reads groups 1-3 and keeps the rest.

**Page 12 - Animations**, one record per on-board animation: the board's
stored animation profiles. The ordinal is the `SET_ANIMATION` and
`SET_ANIMATION_SPEED` index.

| Bit | Group | Bytes |
|---|---|---|
| 0 | Flags, effects | 4 |
| 1 | Speeds | 3 |

All groups make a 7-byte record, six per reply; bit 0 alone, twelve; bit 1
alone, sixteen.

| Field | Meaning |
|---|---|
| Flags | Bit 0 enabled; rest zero |
| Idle, pressed and case effect | `AnimationNonPressedEffects`, `AnimationPressedEffects` and `AnimationNonPressedEffects`, verbatim, 8-bit each |
| Idle, pressed and case speed | The speeds the web configurator's LED page shows, as GP2040-CE stores them, 8-bit each; 0 to the Summary's speed steps (`+9`) |

The total is what the board offers, which varies by board and configuration.

### Actions and kinds

Lights and controls are named by GP2040-CE's own values, reported verbatim:

- **Action**: a control's `GpioAction` (`proto/enums.proto`), signed 16-bit;
  an add-on may define values of its own for its inputs. A button or turbo
  light carries the action of the pin it is tied to.
- **Kind**: a light's `LightType` (`proto/enums.proto`) from the LED
  configuration.

| Action | Control |
|---|---|
| 1-4 | Up, Down, Left, Right |
| 5-8 | B1-B4 |
| 9-12 | L1, R1, L2, R2 |
| 13-14 | S1, S2 |
| 15-16 | A1, A2 |
| 17-18 | L3, R3 |
| 32 | Turbo |
| 41-42 | A3, A4 |
| 43-54 | E1-E12 |

| Kind | Light |
|---|---|
| 0 | Button |
| 1 | Case |
| 2 | Turbo |
| 3-6 | Player LEDs 1-4 |

The tables list the values today; `proto/enums.proto` has the rest. Both
are reported as the running firmware defines them, so an action or kind
GP2040-CE adds later is reported and addressable with no protocol change. A
host treats a kind it does not know as a plain light, and an action it does
not know as one it cannot name.

An action is not unique: a board may wire two buttons to one action (a second
Up, for example), and both lights carry it. Flag bit 0 on the Controls and
Lights records marks such controls and their lights, and the Controls page
numbers them (instance n of instances, in key order). `STAGE` by action
stages every light carrying it; to colour one light of several, stage it by
pin or by ordinal.

A turbo light is tied to a pin as a button light is, normally the turbo
button's, so `STAGE` by action 32 reaches it together with any button light
on a turbo pin; `STAGE` by kind 2 reaches every turbo light, whatever its
pin.

### Staging

`STAGE` request:

    [3]  bits 0-3 addressing, bits 4-7 pixel format
    runs, addressing 0-3:     [4..5] first  [6] count  [7..] count x pixel
    entries, addressing 4-15: [4] n  [5..] n x (address, pixel)

| Addressing | Address | Stages |
|---|---|---|
| 0 LED run | First LED index, 16-bit | `count` consecutive LEDs |
| 1 light run | First light ordinal, 16-bit | `count` consecutive lights |
| 4 light | Light ordinal, 16-bit | That light |
| 5 pin | GPIO pin, 8-bit | Every light whose Lights record carries that owner pin |
| 6 action | Action, signed 16-bit | Every light whose Lights record carries that owner action |
| 7 kind | Kind, 8-bit | Every light of that kind |

A light's pixel goes to every LED of the light. Entries apply in order, so
where lights share an LED the later entry wins. `STAGE` resolves its
addresses to LEDs when it executes: the staged frame holds LED values, and a
later profile switch moves no staged pixel (the token changes, and the host
stages again).

Pixel formats:

| Format | Bytes | Layout |
|---|---|---|
| 0 RGB | 3 | `R, G, B` |
| 1 RGBW | 4 | `R, G, B, W` |
| 2 RGB565 | 2 | 16-bit value: red bits 11-15, green bits 5-10, blue bits 0-4; each widened to 8 bits by repeating its high bits |

Pixels are always sent in the order shown. A pixel format is how a colour
travels, not an LED type: every format works on every chain, and the board
converts each pixel to its chain's colour format (Summary `+6`). The board
applies no gamma correction: pixel values drive the LEDs as its own
animations' values do. When `CONFIGURE` `[6]` is 1 they are scaled by the
board's brightness as its animations are, by step x (maximum / steps,
rounded down) / 255 from State `+6` and Summary `+7` and `+8`; when it is 0
they are sent at full value. `FILL` takes the same pixel formats.

**White handling.** RGBW pixels may be sent to any chain; the W byte renders
only on GRBW and RGBW chains. There an RGBW pixel renders as sent, whatever
its W: R, G and B on the colour emitters, W on the white one; send the
subtractive conversion (W = min(R,G,B), RGB reduced by W) and each colour
lands on the emitters that produce it. A pixel with no W - any RGB or
RGB565 pixel - gets achromatic colours (R=G=B) mapped to the white emitter
by the board itself, the same mapping the on-board animations use. Few
RGB565 greys widen to R=G=B; send RGBW to choose the emitters.

`UNSTAGE` takes the same addressing with no pixels, `[3]` bits 4-7 zero:

    runs, addressing 0-3:     [4..5] first  [6] count
    entries, addressing 4-15: [4] n  [5..] n x address

Each LED it reaches becomes as after `CLEAR`: unstaged and off, so in
overlay takeover the on-board animation shows there again.

Capacity per report:

| Addressing | RGB | RGBW | RGB565 | `UNSTAGE` |
|---|---|---|---|---|
| LED run, light run | 19 | 14 | 28 | 32 |
| Light, action | 11 | 9 | 14 | 29 |
| Pin, kind | 14 | 11 | 19 | 32 |

The reply to `STAGE` or `UNSTAGE` sets bit n of the outcome mask when entry n
applied; for a run, bit n is the nth LED or light of the run, and every LED
of a valid LED run applies. An address with no light behind it - an ordinal
past the light count, an action, pin or kind no light carries - or whose
lights all lie at or past the addressable limit is skipped, not an error;
the none values, pin 0xFF and action 0x8000, match no light, nor does a
page 10 add-on key. A skip on an address that used to apply means the
Lights page changed; the state token has changed with it.

Invalid argument, with nothing staged or unstaged: an unknown addressing
mode or pixel format; a count or `n` of 0 or above the capacity; an LED run
reaching past the addressable LED limit (Summary `+2..3`), which is its first
LED when that lies at or past the limit and otherwise its count. Every write
stays within that limit.

### Events

`SUBSCRIBE` `[3..4]`: bit n enables event n; event numbers run 0-15. HLP
2.0 defines event 0, `STATE_CHANGED`. The subscription is shared, so
`SUBSCRIBE` only adds events: none is enabled by default and no command
disables one. The reply gives the events now enabled. A change before the
subscription raises no event, so a host reads page 1 once after subscribing.

A subscription lasts until USB unmounts or the board reboots. `RELEASE`,
keepalive expiry and USB suspend do not end it; a change during suspend is
sent after resume. It is never stored, so a host sends `SUBSCRIBE` at every
connect; `HELLO` `[45..46]` shows the current subscription.

`STATE_CHANGED` (event 0): `[3..63]` are those of a page 1 reply with mask 0 -
every page 1 group and the current state token. The board sends one when any
page 1 field or the token changes, including when it ends a takeover on
keepalive expiry and when the holder of `CLAIM` changes. At most one is
pending at a time, and it carries the state at the moment it is sent. An
event is sent only while no reply is waiting, so it delays a reply by at
most one IN transfer.
Polling page 1 remains fully supported.

### Keys and joins

| Entity | Key | Appears on | Join |
|---|---|---|---|
| Board | Factory-unique ID | `HELLO` | Bind persistently by it |
| Input mode | `InputMode` | Page 2 records, State `+1`, `SET_INPUT_MODE` | - |
| Profile | Profile number | Page 9 records, State `+2`, `SET_PROFILE` | - |
| Control | Page 10 key: GPIO pin or add-on input | Page 10 records, page 11 owner pin, `STAGE` by pin | Page 11 owner pin = page 10 key |
| Light | Page 11 ordinal | `STAGE` light and light run, page 10 first light ordinal | - |
| LED | Chain index | Page 11 first LED and count, page 8 extent, `STAGE` LED run | A light covers its first LED and the `count - 1` after it |
| Animation | Page 12 ordinal | `SET_ANIMATION`, `SET_ANIMATION_SPEED`, page 1 animation index | - |
| Controlling application | Host token | `CLAIM`, page 1 Controller | - |

An action and a kind are attributes, not keys: `STAGE` by either reaches
every light carrying it, and `UNSTAGE` takes the same addresses. Player,
turbo and case lights are lights, keyed by ordinal. Player and case lights
have no pin; a turbo light is tied to a pin as a button light is, and joins
its control by that pin. An add-on control has no lights. No join uses a
sentinel: 0xFF, 0xFFFF and 0x8000 mean none and never equal a key.

### Number spaces

Every numbered space is grouped by function, with spare numbers in each
group. An addition takes the next free number in its group; a new group takes
a reserved range.

| Space | Groups | Free today |
|---|---|---|
| Request flags, bits 0-7 | - | 4-7 |
| Status, 0-255 | - | 5-255 |
| Claim flags, bits 0-7 | - | 1-7 |
| Commands | `0x01-0x0F` session and discovery, `0x10-0x1F` frame staging, `0x20-0x2F` frame lifecycle, `0x30-0x4F` board features, `0x50-0x6F` reserved for new groups, `0x70-0x7F` privileged management; `0x00` is never a command | `0x06-0x0F`, `0x14-0x1F`, `0x22-0x2F`, `0x34-0x4F`, `0x72-0x7F` |
| `HELLO` reply | `[3..16]` identity, `[17]` limits, `[18..44]` capabilities, `[45..46]` session | `[47..63]` |
| Pages, 0-31 | 0-7 board, 8-15 configuration, 16-23 diagnostics, 24-31 reserved for new groups | 3-7, 13-15, 16-23 |
| Field groups, bits 0-15 of each page | In the order of the page's record | After each page's last group |
| Addressing (`STAGE`, `UNSTAGE`), 0-15 | 0-3 runs, 4-15 entries | 2-3, 8-15 |
| Page 10 keys, 0-254 | `0x00-0x7F` GPIO pins, `0x80-0xBF` Hall-effect triggers, `0xC0-0xDF` I/O-expander pins, `0xE0-0xFE` reserved for new groups; `0xFF` is never a key | `0x30-0x7F`, `0xA0-0xBF`, `0xD0-0xDF` |
| Pixel formats, 0-15 | - | 3-15 |
| Events, 0-15 | - | 1-15 |

The bitmaps in `HELLO` cover these ranges exactly.

### Worked examples

Captured from a Haute42 COSMOX (16 LEDs) in XInput mode. Bytes are in hex;
each report is 64 bytes, and every byte not shown is zero.

`HELLO`:

    request  01 41 00                 HELLO, sequence 0x41, no flags
    reply    81 41 00                 its reply: sequence 0x41, status OK
             47 50 48 4c 02 00        [3..8]   "GPHL", major 2, minor 0
             xx xx xx xx xx xx xx xx  [9..16]  board ID
             10 0f                    [17..18] queue limit 16; flags 0-3
             3e 00 0f 00 03 00 0f 00  [19..34] commands 0x01-0x05, 0x10-0x13,
             00 00 00 00 00 00 03 00           0x20-0x21, 0x30-0x33, 0x70-0x71
             07 1f 00 00              [35..38] pages 0-2 and 8-12
             f3 00 07 00 01 00        [39..44] addressing 0, 1 and 4-7; pixel
                                               formats 0-2; event 0
             00 00                    [45..46] no events enabled

`GET_PAGE` 8, the Summary:

    request  02 42 00                 GET_PAGE, sequence 0x42, no flags
             08 00 00                 [3..5]   page 8, start record 0
             00 00 00 00 00 00        [6..11]  no token check; mask 0, all
    reply    82 42 00                 its reply: sequence 0x42, status OK
             08 00 00 01 00           [3..7]   page 8, start 0, total 1
             01 1a 01 00              [8..11]  1 record, stride 26, group 0
             10 00 64 00 28 00        [12..17] LED extent 16, limit 100, 40 Hz
             00 64 0a 0a              [18..21] GRB; brightness maximum 100,
                                               10 steps; 10 speed steps
             0d 00 08 00              [22..25] grid 13 x 8
             01 00 15 00 10 00 05 00  [26..33] 1 profile; 21 controls, 16 lit
                                               and 5 unlit
             10 00 04 00              [34..37] 16 lights, 4 animations
             bc fd 9f af              [60..63] state token

`STAGE` of LEDs 0-3 as red, green, blue and white, published by
`COMMIT_AFTER`:

    request  10 43 01                 STAGE, sequence 0x43, COMMIT_AFTER
             00                       [3]      addressing 0 (LED run), RGB
             00 00 04                 [4..6]   first LED 0, count 4
             ff 00 00 00 ff 00        [7..18]  four pixels
             00 00 ff ff ff ff
    reply    90 43 00                 its reply: sequence 0x43, status OK
             04 00                    [3..4]   4 applied, 0 skipped
             0f 00 00 00              [5..8]   outcome: LEDs 0-3 of the run

The reply is the frame's acknowledgement: the four LEDs are staged, the
frame is published, and a takeover starts when none is live.

## Compatibility and versioning

- The top-level usage is the protocol major version: a 2.x board presents
  usage `0x0002` on usage page `0xFF47`, and a later major presents its own
  number, so a host finds only boards of a major it speaks. `HELLO` `[7]`
  confirms it.
- Capabilities, not version numbers: a host checks `HELLO`'s capability
  bitmaps for what it uses and gates on nothing else. The minor version
  counts additions, for display and support. The major version changes only
  if the report framing or the meaning of a `HELLO` byte changes.
- Nothing changes meaning: a command ID, page number, field group and
  HLP-defined value keeps its meaning and layout for good. Additions are new
  commands, pages, field groups, request flags, values and trailing fields;
  a change of meaning or layout takes a new ID or page number. A field
  group's width never changes once it ships, and page 1's groups together
  never exceed the 48-byte record area, since `STATE_CHANGED` carries all of
  them. Values carried from GP2040-CE - actions, kinds, `InputMode`, colour
  format, effect IDs, reboot targets - are those of the running firmware.
- Hosts send unused and reserved bytes and bits as zero. Boards ignore
  unknown flag bits, mask bits, bits and bytes a layout marks zero, and
  trailing payload bytes, and zero every unused reply byte and bit.
- Additions take free numbers as Number spaces sets out; bit 7 of `[0]`
  stays the reply flag and `0x80` the event marker.
- Unknown commands (0x01-0x7F) return status `1`, unknown pages status
  `2`; `HELLO`'s bitmaps say in advance which exist, so no host needs to
  probe.
- The addressable LED limit (Summary `+2..3`, 100 today) follows the render
  pipeline, so raising it needs no protocol change.

## Performance

The LED render loop's rate is the effective ceiling for visible updates - the
board renders at 40 Hz today. **Read the actual rate from the Summary page
(`+4..5`) rather than assuming one.** A host streaming faster than the board
renders is not an error and nothing reports it; the extra frames are simply
never shown.

### Input sampling cost

Measured on a Haute42 COSMOX M Ultra (RP2040, 46 LEDs, three reports per
frame) in Generic mode, with the input loop instrumented to report its
average pass time, the input sampling interval, over 100,000 passes. Every
build carries the same instrumentation.

Absolute pass times move by several microseconds between builds as code
shifts in the RP2040's flash cache: builds of this firmware with the add-on
off measured 26.1 to 32.3 us, stock upstream 28.8 us. The cost of Host
Lighting is therefore measured against the same build with the add-on off:

| Host Lighting | Input sampling interval | Added |
|---|---|---|
| Add-on off | 32.27 us | - |
| On, no host | 32.70 us | 0.43 us |
| On, a host subscribed to events | 32.88 us | 0.61 us |
| Streaming at 60 fps | 34.08 us | 1.81 us |
| Streaming at 100 fps | 34.41 us | 2.14 us |

At 100 fps the board still samples its inputs about 29 times per 1 ms USB
poll. With the add-on off, the lighting code costs a flag test per pass.

Building the page tables and computing the state token happen on the USB
core, never on the render path.

### Protocol throughput

Measured on the M Ultra and a 16-LED Haute42 COSMOX in Generic, Keyboard,
SInput and XInput, streaming whole frames as `STAGE` LED runs with
`NO_REPLY` and `COMMIT_AFTER` on the last report. With one frame in flight,
the next frame is sent when the previous one is acknowledged.

| Board | Reports per frame | Round trip, median | 60 fps, frames acknowledged | One frame in flight |
|---|---|---|---|---|
| 46 LEDs | 3 | 2.00 ms | 180 of 180 | 250 frames/s |
| 16 LEDs | 1 | 2.00 ms | 180 of 180 | 500 frames/s |

These figures are set by the 1 ms polling interval of each endpoint: a
report goes out at one poll and its reply comes back at the next, so a
three-report frame and its acknowledgement take 4 ms and a one-report frame
2 ms.

## Host implementations

Any application that can read and write HID reports can implement the
protocol. Examples:

- **[gp2040ce-binary-tools][tools]**: small Python reference clients
  covering discovery, capability decoding, LED control, settings, events,
  board management, conformance checks and performance measurements.
- **hlp-spice2x**: shows the cabinet lighting of games running
  under spice2x on a GP2040-CE controller's buttons.
- **MESH** (Modern Emulator State Hub): a cross-platform desktop app
  that drives controller lighting from emulator game state. It discovers
  boards automatically, seeds per-button LED maps from the capability pages,
  and leaves the board's own animations running when idle.

[tools]: https://github.com/OpenStickCommunity/gp2040ce-binary-tools

## Changelog

Versions are the Host Lighting Protocol version `HELLO` reports, not the
GP2040-CE firmware version. A minor version adds capabilities; see
Compatibility and versioning.

### v2.0 - unreleased

First release in GP2040-CE. Version 1.x was an unmerged pre-release
protocol on usage `0x4C`; 2.0 does not support it.
