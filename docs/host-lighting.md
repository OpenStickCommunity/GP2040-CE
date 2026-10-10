# Host Lighting

Host Lighting lets software on a connected PC drive a GP2040-CE board's RGB
LEDs in real time over USB, alongside the normal controller function. A host
application can show game state, per-button effects or ambient scenes on the
board's lights. The on-board animations return when the host stops.

## Overview

- The add-on is disabled by default. While it is disabled, every input mode
  presents byte-identical USB descriptors to stock firmware.
- Generic HID, Keyboard, SInput and XInput modes carry it. Console modes never
  do. In XInput mode a console sees a stock controller unless XInput Lighting
  is set to Always On.
- The board's own animations return when the host releases control or
  disconnects. When the host stops or crashes they return within a timeout, 2 s
  by default.
- The board describes its LED layout, controls, profiles and animations from
  its live configuration, user remaps included. Host software needs no
  per-board configuration and no update when a new board is released.
- A host can also switch the input profile, animation, animation speed and
  brightness as the board's hotkeys do, and change the input mode.

Users need only this section, How it works and Supported modes. Host developers
continue with Discovery and Typical host flow, and use the Protocol reference
as needed.

## How it works

The add-on is disabled by default, like all GP2040-CE add-ons. Enable it in the
web configurator, under
`Configuration > Add-Ons Configuration > Host Lighting`. Board makers can ship
it enabled by defining `HOST_LIGHTING_ENABLED 1` in a board config.

When enabled, supported input modes expose one extra vendor-defined HID
interface next to the regular controller interface. Hosts exchange fixed
64-byte reports on it.

- Lighting commands write into a **staged frame** on the board. A `COMMIT`
  publishes it to the LED render loop in one step, so a frame sent in several
  reports never tears.
- The render loop shows the host frame while the host keeps it fresh. The
  board's own animations resume when the host releases control, or when it
  sends no frame or keepalive for a configurable timeout (default 2 s). A
  crashed or disconnected host cannot leave the lights stuck.
- **Whole-frame** takeover replaces all lighting. **Overlay** takeover replaces
  only the pixels the host has staged, and the running animations show on the
  rest.
- `HELLO` and the capability pages report the LED map, colour order, board
  identity and current state from the board's live configuration, user remaps
  included. A host can subscribe to be told when they change.

## Supported modes

| Input mode | Lighting interface |
|---|---|
| Generic HID | Yes |
| Keyboard | Yes |
| SInput | Yes |
| XInput | Yes, see below |
| PS3/PS4/PS5, Switch, Xbox, other console modes | Never |

In XInput mode the `XInput Lighting` setting, in the web configurator's Host
Lighting section, decides when the board presents the lighting interface.

- **Auto** (default): The board boots with its stock identity, the one a
  console sees. A console starts authentication at once and the board stays
  stock. A PC sends no console authentication, so after ~4 seconds the board
  re-enumerates as a composite device, with the lighting interface beside the
  controller interface. The operating system's own Xbox 360 driver binds the
  controller interface, on Windows through MS OS descriptors and on Linux
  through the kernel's xpad vendor match. In this mode the lighting device
  appears a few seconds after plug-in.
- **Always On**: The board presents the composite identity from boot. It works
  only on a PC while this is set.
- **Off**: The board always presents the stock XInput identity. XInput mode has
  no lighting interface.

The composite presents its own USB identity, chosen so that Windows queries the
MS OS descriptors that bind its Xbox 360 driver. A user USB ID override
replaces that identity. Windows caches the result of the query for each
VID:PID. An ID the PC has already seen on a device without those descriptors
can leave the controller interface unbound on that PC.

The lighting interface is HID class and enumerates regardless, so Host Lighting
works under an overridden ID. If the controller side fails after an override in
this mode, choose an ID that PC has not seen before.

## Discovery

Do not match VID:PID. It varies by input mode and user override. Enumerate HID
devices and match the top-level collection:

- Usage page `0xFF47`, usage `0x0002`. The usage is the protocol major version
  (Compatibility and versioning).
- One 64-byte input report and one 64-byte output report, with no report IDs.
  Windows buffers are 65 bytes with a leading `0x00` report-ID byte.

Then send `HELLO`. Require the `GPHL` magic (GP2040-CE Host Lighting) and
protocol major version 2, and check that the reply lists the commands and pages
the host uses. Bind a board persistently by the factory-unique board ID in the
`HELLO` reply, never by device path. A device path changes with the USB port.

On Linux a board's hidraw node is open only to root until a udev rule grants
access. The [gp2040ce-binary-tools][tools] README gives a rule for the lighting
modes' USB IDs.

Boards connected at once are independent lighting devices. Each has its own
interface and is addressed separately.

Each board serves one lighting host at a time. The board does not tell
applications apart. Every application that opens its interface shares:

- one staged frame and one takeover
- one set of `CONFIGURE` settings
- one event subscription
- one reply queue

Every application receives every reply and event. The operating system delivers
them. Windows queues each input report for every open handle (measured on
Windows 11), and Linux's hidraw driver does so by design. `hlp-conformance`
checks it on the system it runs on.

Applications coordinate through `CLAIM`, which records the controlling
application's token and name on page 1. An exclusive claim also has the board
refuse changes from applications that do not claim. The board cannot verify who
sends a request, so claims are cooperative between the applications that claim.

Assume nothing from an earlier USB session. Session state defines the USB
session and what a USB bus reset leaves for the next host.

## Typical host flow

The simplest host, alone on a board, takes three steps.

1. Discover the board and send `HELLO`.
2. `STAGE` colours by action, a control's `GpioAction`, with `COMMIT_AFTER`.
3. Repeat that `STAGE` at least twice per keepalive timeout (2 s by default)
   while the colours should stay.

When it stops, the board's animations return.

The full flow adds the page reads, the claim that coordinates applications
sharing a board, events and a clean shutdown.

1. Discover by usage page and usage. Send `HELLO`. Require the `GPHL` magic and
   major 2, check the capabilities the host uses, and bind by the board ID.
2. Read page 8 with `GET_PAGE`. Read pages 0, 2 and 9-12 as the use case needs
   (table below).
3. Send `CLAIM`. Make it exclusive to keep applications that do not claim from
   changing the board.
   - If the reply shows another holder, show its name, then wait or take over.
     To wait, repeat `CLAIM` 0 with a reply. It changes nothing while another
     application holds the claim, and takes the claim as soon as the board is
     unclaimed.
   - Once holding, send the claim generation the reply shows on every request.
     Set `HOLDER` too while the claim is exclusive.
   - Repeat `CLAIM` with `NO_REPLY` and the same claim flags at least twice per
     keepalive timeout, streaming or not, to keep the claim.
   - Act on a status 4 reply as `CLAIM` sets out. A renewal sent with
     `NO_REPLY` shows nothing. When such a renewal takes a lapsed claim again,
     the next answered request is refused and shows the new generation.
   - Learn of a change of holder within each keepalive timeout. A `CLAIM`
     reply, a status 4 reply, `STATE_CHANGED` and a page 1 read each show it.
4. Send `CONFIGURE` for the takeover mode, timeout and brightness policy, after
   the claim (`CONFIGURE`).
5. `SUBSCRIBE` to `STATE_CHANGED`, then read page 1 once. Or poll page 1.
6. Stream. Stage each frame with `NO_REPLY` and set `COMMIT_AFTER` on its last
   report.
   - While the frame or an unsaved brightness step is shown, set `KEEPALIVE` on
     every repeated `CLAIM`. Otherwise each ends a keepalive timeout after its
     last refresh.
   - A host that stages only the pixels that change watches the takeover number
     (`COMMIT`).
7. When the state token changes, as an event or a page 1 poll shows, read the
   Summary. Read again the cached pages whose change counter moved, passing the
   new token as the expected one. For Lights, read groups 1-3 only. Stage the
   whole frame again when the Controls or Lights counter moved.
8. On shutdown, send `CLAIM` give up. When a host stops without a clean
   shutdown, the keepalive timeout restores the animations and the brightness
   and ends a claim.

A host either claims the board or does not. The simplest host above does not.
The two differ as follows.

| | A host that claims | A host that does not claim | See |
|---|---|---|---|
| Generation in the request flags | The one it was last shown, on every request while it holds the claim. 0 while it does not | 0 | `CLAIM` |
| `HOLDER` flag | On every request while its claim is exclusive | Never | `CLAIM` |
| Keeping a frame or an unsaved brightness step shown | Repeats `CLAIM` with `KEEPALIVE` at least twice per keepalive timeout | Repeats its `STAGE`, or sends `HELLO` with `KEEPALIVE` and `NO_REPLY`, at least twice per timeout | Session state |
| `CONFIGURE` | After its claim, never before | Only while the board is unclaimed. The settings last until the next change of holder | `CONFIGURE` |
| While another application holds the claim | Shows the holder's name, then waits or takes over | Reads page 1 and waits. Sends no `CONFIGURE` and does not drive the LEDs | `CLAIM` |
| Under another application's exclusive claim | Its changes are refused with status 4 until it holds the claim | Its changes are refused with status 4. `HELLO`, `GET_PAGE` and `SUBSCRIBE` without `COMMIT_AFTER` or `KEEPALIVE` still work | `CLAIM` |
| Learning of a change of holder | Within each keepalive timeout | Within each keepalive timeout | Step 3 |
| Shutdown | `CLAIM` give up | `RELEASE` and `SET_BRIGHTNESS` 0xFF, unless another application holds the claim | Step 8 |

Repeat from step 1 after a replug, reboot or input-mode change. Drop the cached
pages and their change counters, which start again at 0 with each boot.

| Use case | Commands and pages |
|---|---|
| Light a control by name | `HELLO`, then `STAGE` by action. No pages |
| Stream a whole frame | 8 |
| Per-light effects with geometry | 8, 11 |
| Hand single lights back to the animation | `UNSTAGE` in overlay takeover. No pages |
| Configure host inputs from the board | 9, 10 |
| Switch the input profile | `SET_PROFILE`; 9 |
| Dim for a session, or set the brightness | `SET_BRIGHTNESS`, unsaved or saved; 1 for the step shown, 8 for the step count |
| Set an animation's speeds | `SET_ANIMATION_SPEED`; 12 for the speeds, 8 for the step count |
| Switch the input mode | `SET_INPUT_MODE`; 2 |
| Full inventory and UI | 0, 2, 8-12 |
| Share a board with other applications | `CLAIM`, the generation; 1 |
| Keep applications that do not claim off the board | `CLAIM` exclusive, `HOLDER` and the generation; 1 |
| Stay current | `SUBSCRIBE`, or poll 1 |

## Protocol reference

The wire protocol on this interface is the Host Lighting Protocol (HLP). The
`GPHL` magic in `HELLO` replies and the `hlp-` prefix of the reference tools
name it. This section describes HLP 2.0. Multi-byte fields are little-endian.

In the Protocol reference and the sections after it, the words must, must not,
should, should not and may have the meanings RFC 2119 gives them. They are
written in lower case. They state what a host is required to do. What the board
does is stated as fact.

### Reports

All transfers are 64-byte reports:

    request  [0] command       [1] sequence  [2] flags   [3..63] payload
    reply    [0] command+0x80  [1] sequence  [2] status  [3..63] payload
    event    [0] 0x80          [1] event     [2] 0       [3..63] payload

The host chooses the sequence byte and the reply echoes it. There is no command
0x00, so `[0]` = 0x80 always marks an event. The board ignores a report whose
`[0]` is 0x00 or has bit 7 set, and sends no reply to it.

Request flags, the same on every command:

| Bit | Name | Effect |
|---|---|---|
| 0 | COMMIT_AFTER | Publish the staged frame after this command succeeds, as `COMMIT` does |
| 1 | NO_REPLY | Send no reply to this command, whatever its status |
| 2 | KEEPALIVE | Refresh the keepalive of a live takeover and of an unsaved brightness step after this command succeeds. It starts neither |
| 3 | HOLDER | Set by the holder of an exclusive claim on every request. Ignored while no exclusive claim is held (`CLAIM`) |
| 4-6 | Generation | The claim generation the sender acts under, 1-7; 0 none (`CLAIM`) |
| 7 | - | Reserved for extending the request flags. A host must send it as zero unless `HELLO` `[18]` lists it. A board that does not list it ignores it |

Status: `0` OK, `1` unsupported command, `2` invalid argument, `3` stale token
(see Pages), `4` claimed (see `CLAIM`).

- A command whose status is not 0 changes nothing, answered or not, and neither
  `COMMIT_AFTER` nor `KEEPALIVE` applies.
- A status 2 reply carries at `[3]` the offset of the invalid request byte, the
  lowest when several are invalid.
- A status 4 reply carries at `[3..24]` the holder, laid out as page 1's
  Controller group.

### Field widths

Each quantity has one width wherever it appears:

| Width | Quantities |
|---|---|
| 3-bit | Claim generation (request flags bits 4-6; the low bits of its Controller byte) |
| 4-bit | Addressing mode, pixel format (`[3]` of `STAGE`, `UNSTAGE` and `FILL`) |
| 8-bit | Pin, key, kind, grid coordinate, enumerated value, string byte; sequence, takeover number, versions, page, profile, brightness, animation speed, player, palette slot, button layout, change counter, reply queue limit; counts, offsets and strides within one report |
| 16-bit | LED index and count, light ordinal and count, animation ordinal, record index and count, control instance and instances, grid size, render rate, time in ms |
| 16-bit, signed | Action |
| 32-bit | State token, host token |
| 64-bit | Board ID |
| 1 bit per member | Bitmaps: request flags (bits 0-3 and 7), claim flags (8), addressing modes (16), pixel formats (16), events (16), pages (32), commands (128), field groups (16), `STAGE` and `UNSTAGE` outcome (32) |
| Per format | Pixel: 24, 32 or 16 bits (Pixel formats) |

Strings are UTF-8, byte for byte as the board stores them. Every string length
is in bytes. A string cut to fit its field can end in a partial character, so a
host must decode leniently.

### Replies

The board queues each reply until the IN endpoint takes it. `HELLO` reply
`[17]` states how many replies the queue holds.

- A host that keeps no more requests unanswered than that limit never loses a
  reply. Applications sharing a board share the limit.
- A request that needs a reply and arrives while the queue is full is
  discarded. It is neither executed nor answered, and its sequence number is
  missing from the replies.
- A request sent with `NO_REPLY` needs no place in the queue and is never
  discarded.
- A USB bus reset and the end of a USB session empty the queue. A reply still
  waiting then is lost.

The board executes requests in the order they arrive and sends their replies in
the same order. Commands can be pipelined. A host must match each reply by its
command byte and sequence number. An event can arrive between two replies, and
a discarded request leaves a gap.

A reply that matches none of a host's requests is another application's, and
the host must ignore it. A reply with a host's command and sequence can still
be another application's. Each reply carries the following to tell them apart.

| Reply | Identified by |
|---|---|
| `GET_PAGE` | Command and sequence; the echoed page, start and field groups (page and start alone in a stale-token reply) |
| `CLAIM` | Command and sequence; the echoed action and token |
| Status 4, to any command | Command and sequence; the holder and generation shown (`CLAIM`) |
| Every other reply | Command and sequence only |

A host may send a request again when its reply does not come, under the same
sequence or a new one. Every command but two has the same effect sent twice as
once.

| Commands | Sent twice |
|---|---|
| Staging, `COMMIT`, `RELEASE`, `CLAIM`, `CONFIGURE`, `SUBSCRIBE`, `HELLO`, `GET_PAGE` | Leave the same state |
| Board features (0x30-0x4F) | Write nothing for a value already in effect |
| `SET_INPUT_MODE`, `REBOOT` | Reboot the board again. A host must confirm these by the re-enumeration instead |

A streaming host whose frame acknowledgement does not come acts in this order.

1. It sends that report once more.
2. If that too goes unanswered, it takes the frame as not shown and stages the
   next frame whole.
3. It treats the board as lost only when a `HELLO` then goes unanswered, or the
   operating system reports the device removed. A reply of any status is an
   answer.

Replies wait in each application's input buffer until it reads them, other
applications' replies included. Before sending a request, a host must read the
reports already waiting, keep the events and the replies it still awaits, and
discard the rest. An idle application would otherwise take another's earlier
reply with the same command and sequence for its own. A host should start its
sequence numbers at a random value, so that two applications seldom send the
same one.

A command and its reply typically complete in about 2 ms, one USB frame out and
one back on a 1 ms interrupt endpoint in each direction.

While the board writes its settings flash it answers nothing for about 200 ms.
A request sent meanwhile is not lost. The board takes it when the write ends
and answers in order. The host's call that sends the next request can block
until then. A host must set request timeouts of 250 ms or more, or it takes
such a write for a lost board.

The board writes about a second after a change of animation, animation speed or
saved brightness. It writes at once after a profile switch or another setting a
hotkey changes. This holds whether a host or the board's own hotkeys made the
change.

A streaming host should send each frame's staging reports with `NO_REPLY` and
set `COMMIT_AFTER` on the last one, without `NO_REPLY`. That report stages,
publishes and is the frame's only acknowledgement.

### Session state

The board holds the state below for its hosts. The command descriptions have
the detail.

| State | Starts | Kept by | Ends on | Not ended by |
|---|---|---|---|---|
| Takeover | `COMMIT` or `COMMIT_AFTER` when none is live | A successful staging or lifecycle command (0x10-0x2F), or any successful command with `COMMIT_AFTER` or `KEEPALIVE` | `RELEASE`; the keepalive timeout; a change of holder; USB suspend; the end of the USB session | A USB bus reset while no claim is held |
| Staged frame | `STAGE`, `FILL` | - | `CLEAR`; `RELEASE`; the end of a takeover; a change of holder; USB suspend; the end of the USB session | `COMMIT`; a USB bus reset while no claim is held |
| Unsaved brightness step | `SET_BRIGHTNESS` with save 0 | What keeps a takeover, and another unsaved `SET_BRIGHTNESS` | The keepalive timeout; a change of holder; a saved step; step 0xFF; a brightness hotkey; USB suspend; the end of the USB session | `RELEASE`; a USB bus reset while no claim is held |
| Claim | `CLAIM` 0 on an unclaimed board, or `CLAIM` 1 | The holder's own `CLAIM` 0 or 1, and nothing else | `CLAIM` 2 by the holder; the keepalive timeout; `CLAIM` 1 by another; a USB bus reset; the end of the USB session | `RELEASE`; requests with `HOLDER`; USB suspend |
| `CONFIGURE` settings | `CONFIGURE` | - | A change of holder; the end of the USB session | `RELEASE`; USB suspend; a USB bus reset while no claim is held |
| Subscription | `SUBSCRIBE` | - | The end of the USB session | `RELEASE`; the keepalive timeout; a change of holder; USB suspend; a USB bus reset |

A **USB session** ends when the host deconfigures the board, or when the board
reboots or loses power. A bus-powered board loses power when it is unplugged.
The next session starts unclaimed and unsubscribed, with the default
`CONFIGURE` settings.

A USB bus reset and the enumeration after it do not end the session. A board
that keeps power behind a hub or switch sees no end of session when its cable
is pulled. It sees a USB suspend, then a bus reset at the next plug-in. While
no claim is held, a bus reset drops only the replies still waiting. The
subscription and the `CONFIGURE` settings then pass to the next host.

A change of holder is any of the following:

- a first claim
- a take over
- a give up
- a lapse
- a USB bus reset or end of the USB session that ends a claim

The keepalive timeout keeps running through a USB suspend, so a claim ends
during a suspend that outlasts it. Time the board spends writing its settings
flash counts toward no timeout.

### Commands

| Cmd | Name | Request `[3..]` | Reply `[3..]` |
|---|---|---|---|
| | **Session and discovery (0x01-0x0F)** | | |
| 0x01 | HELLO | - | `[3..6]="GPHL"`, `[7]` major, `[8]` minor, `[9..16]` board ID, `[17..46]` limit, capabilities and session (below) |
| 0x02 | GET_PAGE | `[3]` page, `[4..5]` start record, `[6..9]` expected state token (0 = no check), `[10..11]` field mask (0 = all) | See Pages |
| 0x03 | CLAIM | `[3]` action (0 claim, 1 take over, 2 give up), `[4..7]` host token, `[8..23]` application name, `[24]` claim flags (bit 0 exclusive) | `[3..7]` action and host token as sent; `[8..11]` holder token, `[12..27]` holder name, `[28]` holder's claim flags, `[29]` claim generation |
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

The board ID is the board's 8-byte factory-unique ID. `[18..44]` are capability
bitmaps. Bit n set means the board implements request flag n, command n, page
n, addressing mode n, pixel format n or event n. Request flag bits 4-6 stand
for the claim generation. `[45..46]` are the events this USB session is
subscribed to. A host must check the capabilities it uses (Compatibility and
versioning).

`GET_PAGE` reads a page (Pages).

`CLAIM` records which application controls the board.

| `[3]` | Action | Effect |
|---|---|---|
| 0 | Claim | Takes the claim when the board is unclaimed. Keeps it when the token already holds it |
| 1 | Take over | Always takes the claim |
| 2 | Give up | Clears a claim held under the same token |

In every other case nothing changes, and the reply still has status 0. A
host must pick a random non-zero 32-bit token when it starts. Its name is up
to 16 bytes, NUL-padded. A token of 0 or an unknown action is an invalid
argument.

The reply echoes the action and token sent. It then shows the holder now,
laid out as page 1's Controller group. A holder token of 0 means unclaimed.
A host must take as its own only a reply that echoes what it sent, and must
compare the holder's token with its own. An invalid-argument reply has no
echo.

A claim is a lease. Each claim (0) or take over (1) that leaves the sender
holding it restarts the lease and records the name and claim flags sent. No
other command extends it, whatever its flags. Session state lists what ends
a claim.

Every change of holder, as Session state defines it, does the following.

- changes page 1 and raises `STATE_CHANGED`
- ends the takeover and an unsaved brightness step
- clears the staged frame
- returns the `CONFIGURE` settings to their defaults
- steps the claim generation

The claim generation is a number from 1 to 7. After 7 it steps to 1. The
`CLAIM` reply, a status 4 reply and page 1 show it.

- The holder must send the generation it was shown in bits 4-6 of every
  request's flags.
- An application that holds no claim must send 0. This includes one that
  never claims, one that gave its claim up, and one that was refused and
  found another holder.
- A request other than `CLAIM` whose generation is neither 0 nor the current
  one is refused with status 4. This applies to every command, reads
  included, whether or not a claim is held.
- `CLAIM` ignores the generation.

Claim flag bit 0 makes the claim exclusive. The board ignores the other
bits.

The board does not enforce a shared claim. Every application's commands
still act. While another application holds the claim, a host must not send
`CONFIGURE` or drive the LEDs.

While an exclusive claim is held, a request without both `HOLDER` and the
current generation is refused with status 4 unless it is `CLAIM`, or
`HELLO`, `GET_PAGE` or `SUBSCRIBE` without `COMMIT_AFTER` or `KEEPALIVE`.
The holder of an exclusive claim must set `HOLDER` on every request.

Under an exclusive claim, the `COMMIT_AFTER` and `KEEPALIVE` of a `CLAIM`
sent without `HOLDER` and the current generation act only when it leaves the
sender holding the claim.

Under either rule an unknown command still returns status 1, and a refused
request's arguments are not checked. The board's own hotkeys are not subject
to a claim.

A status 4 reply shows the holder. A host must act on it as follows.

| The reply shows | Meaning | The host |
|---|---|---|
| Another token | It no longer holds the claim | Sends generation 0 from then on. Shows the holder's name. Waits or takes over |
| Token 0 | Its claim lapsed, or a bus reset ended it | Claims again |
| Its own token, another generation | Its claim lapsed and its own renewal took it again | Takes the generation shown. Sends `CONFIGURE` again. Stages the whole frame |
| Its own token and generation, once | The reply answers another application's request | Ignores it |
| Its own token and generation, every time | It holds an exclusive claim without sending `HOLDER` | Sets `HOLDER` |

A host that does not claim sends generation 0, so it receives status 4 only
under another application's exclusive claim. The reply then shows that
application's token. The host does as the first row says: it sends generation
0, shows the holder's name and waits.

`CONFIGURE` sets the takeover mode, the keepalive timeout and whether host
pixels are scaled by the board's brightness. A takeover or brightness value
above 1 is an invalid argument.

Each setting applies at once, to a live takeover too. The timeout also applies
to the claim's lease and to an unsaved brightness step. The settings start as
whole-frame, 2000 ms and brightness applied. Page 1 shows those in force.
Session state lists what returns them to the defaults.

- A host that claims must not send `CONFIGURE` before its claim.
- Settings sent while the board is unclaimed belong to no holder. They last
  until the next change of holder or the end of the USB session, through a
  crash of their sender and a USB bus reset. A host that needs known settings
  must claim.

`SUBSCRIBE` enables events (Events).

Each pixel of the staged frame is staged or unstaged.

| Command | Effect |
|---|---|
| `STAGE`, `UNSTAGE` | Change chosen LEDs (Staging) |
| `FILL` | Stages every LED below the addressable LED limit with one pixel |
| `CLEAR` | Unstages every pixel and sets it to off |

To fill the lights of one kind, send `STAGE` by kind with one entry. An
unstaged pixel shows off in whole-frame takeover and the on-board animation in
overlay. The takeover mode decides nothing else.

`COMMIT` publishes the staged frame in one step and starts a takeover when none
is live. The staged frame is kept while the takeover lasts, so the next frame
can stage only the pixels that change. A host that does so must:

- stage whole frames until page 1 shows its takeover live, then note the
  takeover number (State `+7`)
- stage the whole frame again when page 1 shows flag bit 0 clear or another
  number

Either means that the takeover ended. The host's next frame can already have
started a new one.

`RELEASE` returns the LEDs to the on-board animations at once and clears the
staged frame, even when no takeover is live. A claim survives it.

The following refresh the keepalive of a live takeover and of an unsaved
brightness step when they succeed, answered or not:

- a staging or lifecycle command (0x10-0x2F)
- any command sent with `COMMIT_AFTER` or `KEEPALIVE`

An unsaved `SET_BRIGHTNESS` also restarts its own step's keepalive. No other
command refreshes either, `HELLO` and `CLAIM` included.

When a takeover ends, the board returns to its animations and clears the staged
frame as `CLEAR` does, so every takeover starts from an empty frame. Session
state lists what ends a takeover.

`SET_PROFILE` selects the input profile as GP2040-CE's profile hotkeys do. The
choice is applied live and saved. A profile that does not exist or is not
enabled is an invalid argument. Page 9 lists the profiles.

`SET_ANIMATION` selects the on-board animation, which shows outside a takeover
and under unstaged pixels in overlay. The index is a page 12 ordinal, or 0xFFFF
for off. Off is the state GP2040-CE's animation hotkeys reach after the last
enabled animation. In it no animation lights anything, and only the board's RGB
player and turbo indicators still show. The choice is applied live and saved.
Any other index at or above the page's total, or of an animation not enabled
(page 12 flag bit 0), is an invalid argument.

`SET_ANIMATION_SPEED` sets the idle, pressed and case speeds of one on-board
animation. These are the speeds the web configurator's LED page shows and
GP2040-CE's animation speed hotkeys step. They are applied live and saved. The
index is a page 12 ordinal. A speed of 0xFF leaves that speed as it is. An
index at or above the page's total, or a speed other than 0xFF above the
Summary's speed steps (`+9`), is an invalid argument. The speeds are on page
12, so a change also changes the state token.

`SET_BRIGHTNESS` sets the brightness step the board shows, live.

| `[4]` | Step | Effect |
|---|---|---|
| 1 | Saved | Shown and saved, as GP2040-CE's brightness hotkeys save it. It replaces an unsaved step |
| 0 | Unsaved | Shown only. The saved step stays and nothing is written to flash |

Step 0xFF shows the saved step at once, whatever `[4]`. A brightness hotkey
steps from the saved step and ends an unsaved one. Session state lists what
else ends an unsaved step, and the board then shows the saved step again. A
step other than 0xFF above the Summary's step count (`+8`), or a save above 1,
is an invalid argument.

Page 1 shows the step shown (State `+6`), the saved step (State `+8`) and
whether the two differ (State flag bit 1). Host pixels follow the step shown
when `CONFIGURE` `[6]` is 1.

A board without LED output reports no brightness steps, no speed steps and no
animations (Summary `+8`, `+9`, page 12 empty). On it `SET_BRIGHTNESS` accepts
only 0 and 0xFF, `SET_ANIMATION` only 0xFFFF and `SET_ANIMATION_SPEED` no
index.

`SET_PROFILE`, `SET_ANIMATION`, `SET_ANIMATION_SPEED` and a saved
`SET_BRIGHTNESS` write the board's settings flash.

- A value equal to the current one succeeds and writes nothing. For
  `SET_BRIGHTNESS` the current value is the saved step, and a saved step still
  ends an unsaved one.
- Any other value is saved, a profile at once and the others about a second
  later. The board answers nothing for about 200 ms while it writes (Replies).
- Every write wears the flash. A host should send these commands for a user's
  choice and not as an effect. To dim the animations for a session, it sends an
  unsaved `SET_BRIGHTNESS`, which writes nothing. To dim or animate its own
  pixels, it scales them.

`SET_INPUT_MODE` takes a value of the running firmware's `InputMode` enum
(`proto/enums.proto`). A mode the firmware has no driver for is an invalid
argument. So is the config mode (`INPUT_MODE_CONFIG`), which `REBOOT` target 1
enters. The board saves the mode and reboots into it. It does not write the
current mode again, but still reboots.

A mode without the lighting interface is left only through the web
configurator, a boot-time button hold, the reboot hotkeys or, on a board with a
display, its menu. Page 2 lists the modes with a driver and those with the
interface. Into a mode with page 2 flag bit 1, the interface can take a few
seconds and a second re-enumeration to return (Supported modes).

`REBOOT` targets are the web configurator's reboot modes (its `/api/reboot`
`bootMode`), as the running firmware defines them: 0 into the saved input mode,
1 into the web configurator, 2 into the UF2 bootloader. An unknown target is an
invalid argument. The board sends the reply to `SET_INPUT_MODE` or `REBOOT`
before it reboots. A host must confirm success by the device re-enumerating.

### Pages

`GET_PAGE` reads a capability page.

| Page | Name | What it is for | Read it |
|---|---|---|---|
| | **Board (0-7)** | | |
| 0 | Identity | The board and its firmware: label, board config, platform, version and build | For display and diagnostics. It changes only with the firmware |
| 1 | State | What is happening now: input mode, profile, player, animation, brightness, takeover and its `CONFIGURE` settings, the application holding `CLAIM` and the claim generation | To poll, or as the `STATE_CHANGED` payload. It changes at any time |
| 2 | Input modes | The input modes `SET_INPUT_MODE` accepts: value, name, and whether each presents the lighting interface | Before `SET_INPUT_MODE`. It changes only with the firmware and its settings |
| 3-7 | - | Free for board pages | - |
| | **Configuration (8-15)** | | |
| 8 | Summary | The sizes a host needs: LED extent and limit, render rate, colour format, brightness, grid size, each configuration page's record count, the animation speed steps and the display's button layouts. Also a change counter for each configuration page | First, after `HELLO`, and after each change of the state token |
| 9 | Profiles | The input profiles: number, enabled, label | To show or switch profiles (`SET_PROFILE`) |
| 10 | Controls | Every control in the active profile, lit or not. A control is a GPIO pin or add-on input with an action. Each record gives which instance of its action it is, where it sits, and its lights | To configure host inputs, or to show where each control sits |
| 11 | Lights | Every light: its LEDs, kind, owning pin and action, grid position, palette slot | For per-light effects and geometry |
| 12 | Animations | The on-board animations: enabled, idle, pressed and case effects, and their speeds | To show or pick the idle animation (`SET_ANIMATION`) or set its speeds (`SET_ANIMATION_SPEED`) |
| 13-15 | - | Free for configuration pages | - |
| | **Diagnostics (16-23)** | None in 2.0 | - |
| | **Reserved (24-31)** | For new groups (see Number spaces) | - |

A host should cache the configuration pages. The state token changes whenever
anything on them changes, a profile switch and an animation speed included. The
Summary's change counters say which pages changed.

Every reply has the same header:

    [3]      page, echoed             [4..5]   start record, echoed
    [6..7]   total records            [8]      records in this reply
    [9]      record stride in bytes   [10..11] field groups returned
    [12..59] records                  [60..63] state token

The page, start and field groups sit where the request has them. Pages 0, 1 and
8 are tables of one record. A start at or past the total returns no records. An
unknown page is an invalid argument.

Each page defines its record as an ordered list of **field groups**, one mask
bit per group. The reply takes the requested groups in bit order and returns
each one that fits in what remains of the 48-byte record area. It echoes the
groups returned at `[10..11]` and sets the stride to their total width. It
carries as many records as fit in the record area, from the start record up to
the total.

- Mask 0 requests every group.
- Bits the board does not define are ignored and never echoed.
- A mask naming no group the page defines returns the total, no records and an
  empty echo.
- A group missing from the echo was not returned and can be requested on its
  own. Only page 0's strings can fail to fit.
- Page 9 has one group, bit 0.

`[60..63]` of every reply carries the current **state token**.

- It changes whenever anything a configuration page (8-15) reports changes, and
  for nothing else. Board and diagnostics pages are not covered.
- It is opaque and never 0. A host must compare it for equality only.
- It is computed from what those pages report, the Summary's change counters
  apart, so the same configuration gives the same token.

A request naming an expected token that is no longer current returns status 3
with the page and start echoed, no records, no field groups and the current
token. A host must then restart a walk that spans several reads, so that it
does not mix two states.

**Page 0 - Identity** holds the board and build identification the firmware
carries (`headers/version.h.in`, the board config and the build), one string of
up to 48 bytes per group. It is static for the life of the firmware. The web
configurator's `/api/getFirmwareVersion` returns all but the version ID.

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

A string shorter than 48 bytes is NUL-terminated. A string of 48 bytes fills
the group, and a longer one is cut to 48. Each string starts after the previous
one's NUL, or after its 48th byte. Mask 0 returns the groups that fit, in bit
order. The echo shows which, and the rest are read by their own bits. The board
ID is in the `HELLO` reply.

**Page 1 - State**, one record in three field groups:

| Bit | Group | Bytes |
|---|---|---|
| 0 | State | 9 |
| 1 | Controller | 22 |
| 2 | Settings | 4 |

State:

| Off | Field |
|---|---|
| +0 | Flags: bit 0 a host frame is shown; bit 1 the brightness step shown is unsaved; rest zero |
| +1 | Current InputMode |
| +2 | Active input profile (page 9). 1 is the base mapping |
| +3 | Player number the host assigned, 0 none (below) |
| +4..5 | Current animation index; 0xFFFF off (`SET_ANIMATION`) |
| +6 | Brightness step shown, 0 to the Summary's step count |
| +7 | Takeover number, incremented at every takeover start |
| +8 | Saved brightness step. The board returns to it when an unsaved step ends. Equal to `+6` otherwise |

The player number is the one the input mode's driver records.

| Input mode | Player |
|---|---|
| XInput | 1-4 |
| SInput | The player byte the host sent |
| Generic HID, Keyboard | Always 0. The driver records none |

Controller, the application holding `CLAIM`:

| Off | Field |
|---|---|
| +0..3 | Holder token; 0 unclaimed |
| +4..19 | Application name, up to 16 bytes, NUL-padded |
| +20 | Claim flags: bit 0 exclusive; rest zero |
| +21 | Claim generation, 1-7 (`CLAIM`); rest zero |

Settings, as `CONFIGURE` last set them, or the defaults:

| Off | Field |
|---|---|
| +0 | Takeover: 0 whole-frame, 1 overlay |
| +1..2 | Keepalive timeout in ms |
| +3 | Board brightness applied to host pixels: 0 no, 1 yes |

State flag bit 0 is set while a takeover is live, from the `COMMIT` that starts
it until it ends (Session state). Bit 1 is set while the step shown differs
from the saved step, from an unsaved `SET_BRIGHTNESS` until the step ends.

**Page 2 - Input modes**, one record per mode `SET_INPUT_MODE` accepts, in
`InputMode` order. These are every `InputMode` the firmware has a driver for,
the config mode excluded. The records are fixed while the interface exists.

| Bit | Group | Bytes |
|---|---|---|
| 0 | InputMode, flags | 2 |
| 1 | Name | 22 |

All groups make a 24-byte record, two per reply. Bit 0 alone gives 24 per
reply.

| Field | Meaning |
|---|---|
| InputMode | The mode's value, verbatim |
| Flags | Bit 0 lighting; bit 1 detected; rest zero (below) |
| Name | The firmware's name for the mode, as its display menu shows it; up to 22 bytes, NUL-padded |

Flag bit 0, lighting, is set for a mode that presents this interface. XInput
presents it when the `XInput Lighting` setting is Auto or Always On. Flag bit
1, detected, is set for a mode whose interface appears only once the board has
detected a PC, a few seconds after it enumerates. XInput under Auto is such a
mode.

A host that offers a mode switch should list these modes and warn for one
without flag bit 0.

**Page 8 - Summary**, one record in two field groups:

| Bit | Group | Bytes |
|---|---|---|
| 0 | Summary | 28 |
| 1 | Changes | 8 |

Summary:

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
| +26 | Button layout, left: `DisplayOptions.buttonLayout` (`ButtonLayout`), verbatim |
| +27 | Button layout, right: `DisplayOptions.buttonLayoutRight` (`ButtonLayoutRight`), verbatim |

Changes:

| Off | Field |
|---|---|
| +0..7 | Change counters of pages 8 to 15, 8-bit each (below) |

A host should size its frame buffer from the extent. The extent is not the sum
of the lights' LED counts, since a chain can have gaps. LEDs from the extent up
to the limit can be staged but are never shown. The lit and unlit counts come
from the same walk as the Controls records and carry the same token.

The button layouts are the ones the board's display draws, set by the board's
configuration or its user. `proto/enums.proto` lists them. They hint at the
board's shape, such as a stick, an all-button layout, a dance pad or a drum. A
host can use them to draw or describe the board and should not depend on them.
A board without a display still carries its configuration's values.

A change counter steps, 255 back to 0, when the board finds that anything its
page reports has changed. Page 8's counter covers its Summary group.

- A counter steps only when the state token changes, and once however much of
  its page changed since the token before.
- A page the board does not have keeps counter 0.
- A read steps no counter.
- The counters start at 0 with each boot. The same configuration still gives
  the same token, so a cached page and its counter are good for one connection
  only.

After the state token changes, a host reads again the cached pages whose
counter differs from the one it read with its copy. It needs no read while the
token is the one it holds.

Positions are cells of GP2040-CE's LED layout **grid**, as the web
configurator's LED Configuration page shows it. Cell (0,0) is at the top left,
X runs right and Y runs down, and the cells are square. Positions give the
lights' arrangement, not distances. On a board whose configuration gives only
per-button LED indexes, GP2040-CE approximates the grid from the button layout.
Nothing reports which boards those are, since the user can edit either grid
afterwards.

**Page 9 - Profiles**, one 18-byte record per input profile, in profile order.
Input profiles are GP2040-CE's button mappings. Profile 1 is the base mapping,
and each other profile remaps the pins.

| Off | Field |
|---|---|
| +0 | Profile number, as GP2040-CE numbers them |
| +1 | Flags: bit 0 enabled |
| +2..17 | Label, up to 16 bytes, NUL-padded |

Every defined profile is listed, enabled or not. Profile 1 is always enabled.
`SET_PROFILE` selects one and State `+2` shows the active one. The Controls and
Lights pages report the active profile's mapping. Add-on inputs have one
mapping for every profile.

**Page 10 - Controls**, one record per input whose action is greater than zero,
lit or not, in key order. An input is a GPIO pin or an add-on input. An add-on
input is a Hall-effect trigger or an I/O-expander pin.

| Bit | Group | Bytes |
|---|---|---|
| 0 | Key, action (signed 16-bit), flags | 4 |
| 1 | Instance, instances (16-bit each) | 4 |
| 2 | Position: first LED (16-bit), grid rectangle | 6 |
| 3 | First light ordinal, light count (16-bit each) | 4 |

All groups make an 18-byte record, two per reply. Bits 0-2 give three per
reply, bits 0 and 1 six, and bit 0 alone twelve.

| Field | Meaning |
|---|---|
| Key | The input (table below). An input carries one action |
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

Other keys are free or reserved (Number spaces). A host must treat a key it
does not know as a control it cannot name.

- A GPIO pin's action comes from the active profile.
- An add-on input's action comes from its add-on and not from the profile. The
  state token changes whenever it does. The input is listed while its add-on
  runs.
- The Hall Effect Trigger add-on runs while enabled. It reads `muxChannels`
  triggers on each of up to four muxes, 32 at most, with `muxChannels` 1, 4, 8
  or 16. Only those triggers are listed, and none under another `muxChannels`.
- The PCF8575 add-on runs when it is enabled and its chip is found on I2C at
  boot. An expander pin set as an output mirrors a button and is not an input.
- Lights follow GPIO pins only, so an add-on control is never lit and has no
  position.

A control has a position when it is lit, through its lights, and only then. The
rectangle covers all of them, so the Controls page alone says where every lit
control sits. A control's lights need not be consecutive. Where the count
exceeds one, the Lights page has each one by owner pin. Instances above 1 is
the same fact as flag bit 0. The numbering holds while the state token is
unchanged.

`NONE`, `RESERVED` and `ASSIGNED_TO_ADDON` are not greater than zero, so I2C,
USB-host and LED-data pins never appear. Modifiers and alternate-direction
actions appear, with the action verbatim. A host filters on the actions it
uses. A control absent from this page does not exist, so a host can configure
its inputs from this page alone. The page comes from the pin map and the add-on
settings, so a board with no lights still lists every control, unlit and
without a position.

**Page 11 - Lights**, one record per light. A light is one entry of the board's
LED layout, a run of LEDs with a kind and a grid position. A button or turbo
light is tied to a pin. A case or player light takes a palette slot instead. A
light's identity is its record ordinal, which is also its `STAGE` address.

The page is complete whenever the lighting interface exists. A board with no
LED output has no lights (Summary `+22..23` is 0). LEDs at or past the
addressable LED limit (Summary `+2..3`) are never written and not counted in
the extent, so a host must clip a light's LEDs to the limit. An address whose
lights all lie at or past it is skipped (Staging).

| Bit | Group | Bytes |
|---|---|---|
| 0 | First LED, LED count (16-bit each) | 4 |
| 1 | Kind, flags | 2 |
| 2 | Owner pin, owner action (signed 16-bit) | 3 |
| 3 | Owner instance, owner instances (16-bit each) | 4 |
| 4 | Grid X, grid Y | 2 |
| 5 | Palette slot | 1 |

All groups make a 16-byte record, three per reply. Positions alone (bit 4) give
24 per reply, LED ranges alone (bit 0) 12, and owner, instance and position
(bits 2-4) five.

| Field | Meaning |
|---|---|
| First LED | First LED index on the chain. It is not a key, since two lights can start at the same index |
| LED count | LEDs in this light |
| Kind | See Actions and kinds |
| Flags | Bit 0 duplicated: the owner pin is a control whose action another control also has; rest zero |
| Owner pin | The GPIO pin a button or turbo light is tied to; 0xFF none |
| Owner action | That pin's action in the active profile, verbatim; 0x8000 none |
| Owner instance, instances | The owning control's instance and instances (Controls group 1); 0 when there is no owner pin or it is not a control |
| Grid X, Y | The light's cell (Grid, under page 8) |
| Palette slot | The light's slot in the animation profile's non-button colour palette (case and player lights); opaque, not unique; 0xFF none |

An owner pin that is not on the Controls page carries no action (`NONE`,
`RESERVED` or `ASSIGNED_TO_ADDON`). The owner action says which. Where a
control owns several lights, several records carry its pin and action, in
ordinal order.

The lights are fixed while the interface exists. A profile switch changes only
the owner action, the owner instances and flag bit 0. After a token change a
host reads groups 1-3 again and keeps the rest.

**Page 12 - Animations**, one record per on-board animation, the board's stored
animation profiles. The ordinal is the `SET_ANIMATION` and
`SET_ANIMATION_SPEED` index.

| Bit | Group | Bytes |
|---|---|---|
| 0 | Flags, effects | 4 |
| 1 | Speeds | 3 |

All groups make a 7-byte record, six per reply. Bit 0 alone gives twelve per
reply, and bit 1 alone sixteen.

| Field | Meaning |
|---|---|
| Flags | Bit 0 enabled; rest zero |
| Idle, pressed and case effect | `AnimationNonPressedEffects`, `AnimationPressedEffects` and `AnimationNonPressedEffects`, verbatim, 8-bit each |
| Idle, pressed and case speed | The speeds the web configurator's LED page shows, as GP2040-CE stores them, 8-bit each; 0 to the Summary's speed steps (`+9`) |

The total is what the board offers, which varies by board and configuration.
Actions and kinds lists the effects.

### Actions and kinds

Lights and controls are named by GP2040-CE's own values, reported verbatim.

- An **action** is a control's `GpioAction` (`proto/enums.proto`), signed
  16-bit. An add-on can define values of its own for its inputs. A button or
  turbo light carries the action of the pin it is tied to.
- A **kind** is a light's `LightType` (`proto/enums.proto`) from the LED
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

Page 12's effects are GP2040-CE's own values too (`proto/enums.proto`). The
idle and case effects are `AnimationNonPressedEffects` values. The pressed
effect is an `AnimationPressedEffects` value.

| Idle or case effect | Animation |
|---|---|
| 0 | Static colour |
| 1-2 | Rainbow: synced, rotating |
| 3-4 | Chase: by index, sequential |
| 5-6 | Chase: circle clockwise, anticlockwise |
| 7-10 | Chase: left to right, right to left, top to bottom, bottom to top |
| 11-15 | Chase, back and forth: by index, sequential, circle, horizontal, vertical |
| 16 | Chase: random |
| 17-18 | Jiggle: one colour, two colours |
| 19 | Rain |

| Pressed effect | Animation |
|---|---|
| 0 | Static colour |
| 1 | Random |
| 2-3 | Jiggle: one colour, two colours |
| 4-5 | Burst, small burst |

The tables list the values today, and `proto/enums.proto` has the rest. All are
reported as the running firmware defines them. An action, kind or effect that
GP2040-CE adds later is reported with no protocol change, and such an action or
kind is addressable. A host must treat a kind it does not know as a plain
light, and an action or effect it does not know as one it cannot name.

An action is not unique. A board can wire two buttons to one action, a second
Up for example, and both lights carry it. Flag bit 0 on the Controls and Lights
records marks such controls and their lights, and the Controls page numbers
them. `STAGE` by action stages every light carrying it. To colour one light of
several, stage it by pin or by ordinal.

A turbo light is tied to a pin as a button light is, normally the turbo
button's. `STAGE` by action 32 reaches it together with any button light on a
turbo pin. `STAGE` by kind 2 reaches every turbo light, whatever its pin.

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
where lights share an LED the later entry wins. `STAGE` resolves its addresses
to LEDs when it executes. The staged frame holds LED values, so a later profile
switch moves no staged pixel. The token changes then, and the host stages
again.

Pixel formats:

| Format | Bytes | Layout |
|---|---|---|
| 0 RGB | 3 | `R, G, B` |
| 1 RGBW | 4 | `R, G, B, W` |
| 2 RGB565 | 2 | 16-bit value: red bits 11-15, green bits 5-10, blue bits 0-4; each widened to 8 bits by repeating its high bits |

Pixels are always sent in the order shown. A pixel format is how a colour
travels and is not an LED type. Every format works on every chain, and the
board converts each pixel to its chain's colour format (Summary `+6`). The
board applies no gamma correction. Pixel values drive the LEDs as its own
animations' values do. `FILL` takes the same pixel formats.

When `CONFIGURE` `[6]` is 1, pixels are scaled by the board's brightness as its
animations are. The scale is step x (maximum / steps, rounded down) / 255, from
State `+6` and Summary `+7` and `+8`. When it is 0 they are sent at full value.

RGBW pixels may be sent to any chain. The W byte renders only on GRBW and RGBW
chains.

- On those chains an RGBW pixel renders as sent, whatever its W, with R, G and
  B on the colour emitters and W on the white one. With the subtractive
  conversion (W = min(R,G,B), RGB reduced by W), each colour lands on the
  emitters that produce it.
- A pixel with no W is any RGB or RGB565 pixel. The board maps its achromatic
  colours (R=G=B) to the white emitter, as it does for the on-board animations.
  Few RGB565 greys widen to R=G=B, so a host sends RGBW to choose the emitters.

`UNSTAGE` takes the same addressing with no pixels, `[3]` bits 4-7 zero:

    runs, addressing 0-3:     [4..5] first  [6] count
    entries, addressing 4-15: [4] n  [5..] n x address

Each LED it reaches becomes as after `CLEAR`, unstaged and off. In overlay
takeover the on-board animation shows there again.

Capacity per report:

| Addressing | RGB | RGBW | RGB565 | `UNSTAGE` |
|---|---|---|---|---|
| LED run, light run | 19 | 14 | 28 | 32 |
| Light, action | 11 | 9 | 14 | 29 |
| Pin, kind | 14 | 11 | 19 | 32 |

The reply to `STAGE` or `UNSTAGE` sets bit n of the outcome mask when entry n
applied. For a run, bit n is the nth LED or light of the run, and every LED of
a valid LED run applies.

An address is skipped, which is not an error, in these cases:

- no light is behind it, as with an ordinal past the light count or an action,
  pin or kind no light carries
- its lights all lie at or past the addressable limit

The none values, pin 0xFF and action 0x8000, match no light. Nor does a page 10
add-on key. A skip on an address that used to apply means the Lights page
changed. The state token has changed with it.

The following are invalid arguments, and nothing is staged or unstaged:

- an unknown addressing mode or pixel format
- a count or `n` of 0 or above the capacity
- an LED run reaching past the addressable LED limit (Summary `+2..3`)

For such a run the invalid byte is its first LED when that lies at or past the
limit, and otherwise its count. Every write stays within that limit.

### Events

`SUBSCRIBE` `[3..4]` is an event mask. Bit n enables event n, and event numbers
run 0-15. HLP 2.0 defines event 0, `STATE_CHANGED`.

- The subscription is shared, so `SUBSCRIBE` only adds events. None is enabled
  by default and no command disables one.
- The reply gives the events now enabled.
- A change before the subscription raises no event, so a host must read page 1
  once after subscribing.
- A subscription lasts until the USB session ends (Session state). It is never
  stored, so a host must send `SUBSCRIBE` at every connect. `HELLO` `[45..46]`
  shows the current subscription.
- A change during a USB suspend is sent after resume.

`STATE_CHANGED` (event 0) carries at `[3..63]` what a page 1 reply with mask 0
carries, which is every page 1 group and the current state token.

- The board sends one when any page 1 field or the token changes.
- At most one is pending at a time, and it carries the state at the moment it
  is sent.
- An event is sent only while no reply is waiting, so it delays a reply by at
  most one IN transfer.

Polling page 1 remains supported.

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

An action and a kind are attributes and not keys. `STAGE` by either reaches
every light carrying it, and `UNSTAGE` takes the same addresses. Player, turbo
and case lights are lights, keyed by ordinal. Player and case lights have no
pin. A turbo light is tied to a pin as a button light is, and joins its control
by that pin. An add-on control has no lights. No join uses a sentinel. 0xFF,
0xFFFF and 0x8000 mean none and never equal a key.

### Number spaces

Every numbered space is grouped by function, with spare numbers in each group.
An addition takes the next free number in its group. A new group takes a
reserved range.

| Space | Groups | Free today |
|---|---|---|
| Request flags, bits 0-7 | 0-3 flags, 4-6 the claim generation | 7 |
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

The bitmaps in `HELLO` cover these ranges.

### Worked examples

Captured from a Haute42 COSMOX (16 LEDs) in XInput mode. Bytes are in hex.
Each report is 64 bytes, and every byte not shown is zero.

`HELLO`:

    request  01 41 00                 HELLO, sequence 0x41, no flags
    reply    81 41 00                 its reply: sequence 0x41, status 0
             47 50 48 4c 02 00        [3..8]   "GPHL", major 2, minor 0
             xx xx xx xx xx xx xx xx  [9..16]  board ID
             10 7f                    [17..18] queue limit 16; flags 0-3 and
                                               the claim generation
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
    reply    82 42 00                 its reply: sequence 0x42, status 0
             08 00 00 01 00           [3..7]   page 8, start 0, total 1
             01 24 03 00              [8..11]  1 record, stride 36, groups 0-1
             10 00 64 00 28 00        [12..17] LED extent 16, limit 100, 40 Hz
             00 63 0a 0a              [18..21] GRB; brightness maximum 99,
                                               10 steps; 10 speed steps
             11 00 09 00              [22..25] grid 17 x 9
             01 00 15 00 10 00 05 00  [26..33] 1 profile; 21 controls, 16 lit
                                               and 5 unlit
             10 00 04 00              [34..37] 16 lights, 4 animations
             1b 1f                    [38..39] button layouts 27 and 31, the
                                               board's own
             00 00 00 00 00 00 00 00  [40..47] change counters, pages 8-15
             xx xx xx xx              [60..63] state token

`STAGE` of LEDs 0-3 as red, green, blue and white, published by
`COMMIT_AFTER`:

    request  10 43 01                 STAGE, sequence 0x43, COMMIT_AFTER
             00                       [3]      addressing 0 (LED run), RGB
             00 00 04                 [4..6]   first LED 0, count 4
             ff 00 00 00 ff 00        [7..18]  four pixels
             00 00 ff ff ff ff
    reply    90 43 00                 its reply: sequence 0x43, status 0
             04 00                    [3..4]   4 applied, 0 skipped
             0f 00 00 00              [5..8]   outcome: LEDs 0-3 of the run

The reply is the frame's acknowledgement. The four LEDs are staged, the
frame is published, and a takeover starts when none is live.

`STAGE` by action, of B1 red and E1 blue, on a board with no E1 light:

    request  10 44 00                 STAGE, sequence 0x44, no flags
             06 02                    [3..4]   addressing 6 (action), RGB; 2
                                               entries
             05 00 ff 00 00           [5..9]   action 5 (B1), red
             2b 00 00 00 ff           [10..14] action 43 (E1), blue
    reply    90 44 00                 its reply: sequence 0x44, status 0
             01 01                    [3..4]   1 applied, 1 skipped
             01 00 00 00              [5..8]   outcome: entry 0 applied

`CLAIM` by an application named Beta while Alpha holds an exclusive claim:

    request  03 45 00                 CLAIM, sequence 0x45, no flags
             00 b2 00 00 00           [3..7]   action 0 (claim), token 0xB2
             42 65 74 61              [8..23]  name "Beta"
             00                       [24]     claim flags: shared
    reply    83 45 00                 its reply: sequence 0x45, status 0
             00 b2 00 00 00           [3..7]   action and token as sent
             a1 00 00 00              [8..11]  the holder's token, 0xA1, not
                                               Beta's. Beta does not hold
             41 6c 70 68 61           [12..27] the holder's name, "Alpha"
             01 02                    [28..29] exclusive; generation 2

Beta then takes over (action 1) and the generation becomes 3. Alpha has not
yet seen the change, and its next frame is refused:

    request  10 46 29                 STAGE, sequence 0x46; COMMIT_AFTER,
                                      HOLDER, generation 2
             00 00 00 01 ff 00 00     [3..9]   LED run: LED 0, red
    reply    90 46 04                 its reply: sequence 0x46, status 4
             b2 00 00 00              [3..6]   the holder's token, 0xB2
             42 65 74 61              [7..22]  the holder's name, "Beta"
             00 03                    [23..24] shared; generation 3

`STATE_CHANGED`, sent when that take over happened:

    event    80 00 00                 event 0, STATE_CHANGED
             01 00 00 01 00           [3..7]   page 1, start 0, total 1
             01 23 07 00              [8..11]  1 record, stride 35, groups 0-2
             00 00 01 02              [12..15] no frame shown; XInput; profile
                                               1; player 2
             00 00 0a 01 0a           [16..20] animation 0; brightness step 10;
                                               takeover number 1; saved step
                                               10
             b2 00 00 00              [21..24] the holder's token, 0xB2
             42 65 74 61              [25..40] the holder's name, "Beta"
             00 03                    [41..42] shared; generation 3
             00 d0 07 01              [43..46] whole-frame; 2000 ms; brightness
                                               applied
             xx xx xx xx              [60..63] state token

`GET_PAGE` 11 with a field mask, for the kind and flags of every light:

    request  02 47 00                 GET_PAGE, sequence 0x47, no flags
             0b 00 00                 [3..5]   page 11, start record 0
             00 00 00 00 02 00        [6..11]  no token check; mask: group 1
    reply    82 47 00                 its reply: sequence 0x47, status 0
             0b 00 00 10 00           [3..7]   page 11, start 0, total 16
             10 02 02 00              [8..11]  16 records, stride 2, group 1
             00 00 00 00 00 00 00 01  [12..43] kind 0 (button) and flags of
             00 00 00 00 00 00 00 00           each light. 3, 12, 13 and 15
             00 00 00 00 00 00 00 00           carry flag bit 0, duplicated
             00 01 00 01 00 00 00 01
             xx xx xx xx              [60..63] state token

`GET_PAGE` 9 naming an expected token that is no longer current:

    request  02 48 00                 GET_PAGE, sequence 0x48, no flags
             09 00 00                 [3..5]   page 9, start record 0
             44 33 22 11 00 00        [6..11]  expected token; mask 0
    reply    82 48 03                 its reply: sequence 0x48, status 3
             09 00 00                 [3..5]   page 9, start 0, as sent
             00 00 00 00 00 00        [6..11]  no total, records or groups
             xx xx xx xx              [60..63] the current state token

## Compatibility and versioning

- The top-level usage is the protocol major version. A 2.x board presents usage
  `0x0002` on usage page `0xFF47`, and a later major presents its own number,
  so a host finds only boards of a major it speaks. `HELLO` `[7]` confirms it.
  A host can tell the user that a board runs another major by matching the
  usage page alone. Pre-release firmware (protocol 1.x) used usage `0x004C`
  there.
- A host must check `HELLO`'s capability bitmaps for what it uses and gate on
  nothing else. The minor version counts additions, for display and support.
  The major version changes only if the report framing or the meaning of a
  `HELLO` byte changes.
- A command ID, page number, field group and HLP-defined value keeps its
  meaning and layout for good. Additions are new commands, pages, field groups,
  request flags, values and trailing fields. A change of meaning or layout
  takes a new ID or page number.
- A field group's width never changes once it ships. Page 1's groups together
  never exceed the 48-byte record area, since `STATE_CHANGED` carries all of
  them.
- Values carried from GP2040-CE are those of the running firmware: actions,
  kinds, `InputMode`, colour format, effect IDs, button layouts and reboot
  targets.
- A host must send unused and reserved bytes and bits as zero. The board
  ignores unknown flag bits, mask bits, bits and bytes a layout marks zero, and
  trailing payload bytes. It zeroes every unused reply byte and bit.
- Additions take free numbers as Number spaces sets out. Bit 7 of `[0]` stays
  the reply flag and `0x80` the event marker.
- An unknown command (0x01-0x7F) returns status `1` and an unknown page status
  `2`. `HELLO`'s bitmaps say in advance which exist, so no host needs to probe.
- The addressable LED limit (Summary `+2..3`, 100 today) follows the render
  pipeline, so raising it needs no protocol change.

## Performance

The LED render loop's rate is the ceiling for visible updates, and the board
renders at 40 Hz today. A host should read the rate from the Summary page
(`+4..5`) and not assume one. A host that streams faster than the board renders
causes no error, and nothing reports it. The extra frames are never shown.

### Input sampling cost

Measured on a Haute42 COSMOX M Ultra (RP2040, 46 LEDs, three reports per frame)
in Generic mode. The input loop was instrumented to report its average pass
time, the input sampling interval, over 100,000 passes. Every build carries the
same instrumentation.

Absolute pass times move by several microseconds between builds as code shifts
in the RP2040's flash cache. Builds of this firmware with the add-on off
measured 26.1 to 32.3 us, and stock upstream 28.8 us. The cost of Host Lighting
is therefore measured against the same build with the add-on off.

| Host Lighting | Input sampling interval | Added |
|---|---|---|
| Add-on off | 32.27 us | - |
| On, no host | 32.70 us | 0.43 us |
| On, a host subscribed to events | 32.88 us | 0.61 us |
| Streaming at 60 fps | 34.08 us | 1.81 us |
| Streaming at 100 fps | 34.41 us | 2.14 us |

At 100 fps the board still samples its inputs about 29 times per 1 ms USB poll.
With the add-on off, the lighting code costs a flag test per pass. The page
tables and the state token are built on the USB core, never on the render path.

### Protocol throughput

Measured on the M Ultra and a 16-LED Haute42 COSMOX in Generic, Keyboard,
SInput and XInput, streaming whole frames as `STAGE` LED runs with `NO_REPLY`
and `COMMIT_AFTER` on the last report. With one frame in flight, the next frame
is sent when the previous one is acknowledged.

| Board | Reports per frame | Round trip, median | 60 fps, frames acknowledged | One frame in flight |
|---|---|---|---|---|
| 46 LEDs | 3 | 2.00 ms | 180 of 180 | 250 frames/s |
| 16 LEDs | 1 | 2.00 ms | 180 of 180 | 500 frames/s |

The 1 ms polling interval of each endpoint sets these figures. A report goes
out at one poll and its reply comes back at the next, so a three-report frame
and its acknowledgement take 4 ms and a one-report frame 2 ms.

## Host implementations

Any application that can read and write HID reports can implement the protocol.
Examples:

- [gp2040ce-binary-tools][tools] has small Python reference clients for
  discovery, capability decoding, LED control, settings, events, board
  management, conformance checks and performance measurements.
- hlp-spice2x shows the cabinet lighting of games running under spice2x on a
  GP2040-CE controller's buttons.
- MESH (Modern Emulator State Hub) is a cross-platform desktop app that drives
  controller lighting from emulator game state. It discovers boards
  automatically, seeds per-button LED maps from the capability pages, and
  leaves the board's own animations running when idle.

[tools]: https://github.com/OpenStickCommunity/gp2040ce-binary-tools

## Changelog

Versions are the Host Lighting Protocol version `HELLO` reports, not the
GP2040-CE firmware version. A minor version adds capabilities (Compatibility
and versioning).

### v2.0 (unreleased)

First release in GP2040-CE. Version 1.x was an unmerged pre-release protocol
on usage `0x004C`. 2.0 does not support it.
