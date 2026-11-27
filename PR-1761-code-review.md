# Code review: OpenStickCommunity/GP2040-CE#1761 — Add Bluetooth LE HID gamepad support

| | |
|---|---|
| PR | https://github.com/OpenStickCommunity/GP2040-CE/pull/1761 (draft) |
| Head | `c066e9ed` on `draft/bluetooth-le-hid` |
| Base | upstream `main` at `0ae800d6` |
| Size | 43 files, +2423 / −40, 6 commits |
| Reviewed | 2026-11-26, against Pico SDK 2.3.1 (the version the project requires) |

This file is untracked and is not part of the PR.

## Verdict

F1–F15's Bluetooth-related findings have been addressed in the local working tree. Firmware builds and focused web tests pass. The unrelated font-data and PR-scope hygiene notes are explicitly deferred below. No changes have been tested on hardware.

Nothing here was tested on hardware. Findings are based on reading the PR, reading the Pico SDK 2.3.1 and BTstack sources it links against, and inspecting locally built firmware. Each finding says how it was established.

| ID | Severity | Finding |
|---|---|---|
| [F1](#f1) | Critical | BTstack's flash storage overlaps the GP2040-CE config region on 2 MB boards (Pico W) |
| [F2](#f2) | High | BTstack allocates with `malloc` from interrupt context and can deadlock the main loop |
| [F3](#f3) | High | Selecting Bluetooth on firmware built without it faults at USB enumeration, and the choice is saved |
| [F4](#f4) | Medium | Bond keys are exported in plain form by `/api/getConfig` |
| [F5](#f5) | Medium | HID characteristics need no encryption, and Bluetooth mode is always open to pairing |
| [F6](#f6) | Medium | Analog axes are never sent, so left-stick and right-stick D-pad modes send no directions |
| [F7](#f7) | Medium | The Bluetooth identity reuses the USB vendor/product ID with a different button layout |
| [F8](#f8) | Medium | Bond data is changed from interrupt context while other code saves the config |
| [F9](#f9) | Medium | The new config message uses most of the remaining worst-case config size budget |
| [F10](#f10) | Medium | The Pimoroni board config targets the wrong SDK board |
| [F11](#f11) | Medium | The radio is started in every Web Config session on every CYW43 board |
| [F12](#f12) | Low | No report is sent after a reconnect until an input changes |
| [F13](#f13) | Low | Debug logging, diagnostic LED codes and an unused idle state are left in |
| [F14](#f14) | Low | Smaller firmware issues |
| [F15](#f15) | Low | Web UI and PR hygiene |

## Follow-up: F1–F3 (2026-11-27)

The following fixes are present in the working tree; they have not yet been committed to the PR branch.

| ID | Status | Change |
|---|---|---|
| [F1](#f1) | Fixed in working tree | BLE builds place BTstack's 8 KB flash bank at offset `0x1F6000`, immediately below FlashPROM's fixed config region. Compile-time checks reject overlap/out-of-flash layouts, and BLE initialization checks that the linked image ends before either reserved region. |
| [F2](#f2) | Fixed in working tree | Removed `HAVE_MALLOC`; BTstack uses its static object pools and a 512-byte static ATT database. |
| [F3](#f3) | Fixed in working tree | Unsupported boot modes fall back to XInput. Non-BLE firmware rejects BLE mode selections in both settings and boot-mode APIs, and Boot Mode Mapping hides BLE when unsupported. Backup imports surface rejected settings. |

Validation: the non-Bluetooth Pico, Pico W Release, and Pimoroni Pico Lipo 2XL W Release firmware builds passed with Pico SDK 2.3.1; web tests/build, focused ESLint, Prettier, and `git diff --check` passed. Broader ESLint reports 20 existing errors in `BootModeMapping.tsx` and `WebApi.js`, none on changed lines. The latest Pico W ELF ends at offset `0x1B9908`, and Pimoroni at `0x1B0C30`; both are below the BTstack bank at `0x1F6000`. `tsc --noEmit` reports 133 diagnostics across the web project; two are on unchanged Boot Mode Mapping lines. No hardware validation has been performed.

## Follow-up: remaining Bluetooth findings

The changes below are in the working tree and still require the user's hardware validation. No pairing, reconnect, HID security, axis, or battery behavior is claimed as hardware-tested.

| ID | Status | Change |
|---|---|---|
| [F1](#f1) | Fixed in working tree | The reserved 8 KB bank remains at `0x1F6000`; factory reset now erases both BTstack flash banks. |
| [F4](#f4), [F8](#f8), [F9](#f9) | Fixed in working tree | Removed bond keys from `Config` and switched back to BTstack's native TLV LE-device database. Bond updates no longer mutate or save the application config; bond material is not exported and the protobuf size budget is restored. Forgetting bonds uses BTstack's delete API so per-peer CCC records are cleared. |
| [F5](#f5) | Fixed in working tree | HID report access requires BTstack's 16-byte encryption level. Pairing is opt-in from Web Config; normal BLE mode advertises non-discoverably for bonded reconnects, rejects unresolved peers when pairing is off, and declines Just Works requests. |
| [F6](#f6) | Fixed in working tree | BLE reports now include scaled left/right stick axes and both analog triggers in a 9-byte report body. |
| [F7](#f7) | Fixed in working tree | BLE now uses product ID `0x82C1`, distinct from the USB DInput identity. |
| [F10](#f10) | Fixed in working tree | The Pimoroni target uses the SDK's `pimoroni_pico_lipo2xl_w` board/platform definition and its 16 MB/RP2350B pin configuration. Battery-divider calibration still needs the user's hardware check. |
| [F11](#f11) | Fixed in working tree | Normal boot initializes BLE only for saved BLE input mode. Web Config starts the radio only after the Bluetooth status endpoint is requested. |
| [F12](#f12) | Fixed in working tree | Reconnect reset clears the last-sent report cache so the current state is sent again. |
| [F13](#f13), [F14](#f14) | Fixed in working tree | Removed periodic/IRQ logging, diagnostic LED patterns, and the ineffective idle throttle; removed duplicate BTstack run-loop init and redundant ADC init; validate disconnect handles; corrected pairing decline logic; refreshed bond recency on bonded reconnect; request low-latency connection parameters; and enable bounded CYW43 HCI flow control using the Pico SDK reference buffer settings. |
| [F15](#f15) | Addressed; remaining locale translations deferred | Added mock Bluetooth endpoints, preserved support state during failed polls, formatted the component, retained Bluetooth strings in English, Spanish, and Japanese, and wired `npm test` into the package and Node CI workflow. |

### Deferred as unrelated to Bluetooth

- **F14 unused font data:** defer dropping unused font rows; it does not affect BLE.
- **F15 PR scope hygiene:** defer splitting unrelated web TypeScript changes into another PR; it is not a Bluetooth behavior change.

These are the only findings deferred as unrelated to Bluetooth. The board-specific battery-voltage divider assumptions in F10 are not independently validated; real-hardware testing remains outstanding.

### Deferred pending localization review

Bluetooth strings are intentionally retained only in English, Spanish, and Japanese so they can be reviewed before translations are proposed for German, French, Korean, Brazilian Portuguese, Turkish, and Simplified Chinese. Those locales continue to fall back to English.

The detailed findings below record the original PR state. Their recommendations and observations are historical; use the follow-up status above for the current working tree.

---

## Critical

<a id="f1"></a>
### F1. BTstack's flash storage overlaps the config region on 2 MB boards

**Where:** [CMakeLists.txt](CMakeLists.txt) (BLE block), [src/le_device_db_proto.cpp](src/le_device_db_proto.cpp), [src/gp2040.cpp](src/gp2040.cpp)

The PR replaces BTstack's bond database with a protobuf-backed one because the stock one "mapped to the same flash region as FlashPROM". That only removes one of the users of BTstack's flash storage. The storage itself is still set up and still written.

How it was established:

- `cyw43_arch_init()` calls `btstack_cyw43_init()`, which calls `setup_tlv()` unconditionally (`pico_cyw43_driver/btstack_cyw43.c`, lines 28–65). That initialises BTstack's flash bank before the stubbed `le_device_db_tlv_configure()` is reached.
- If the bank has no valid header, initialisation erases bank 0 and writes one (`btstack_tlv_flash_bank.c`, lines 591–594).
- The bank defaults to the last 8 KB of flash on RP2040 (`pico/btstack_flash_bank.h`, line 28). The PR does not override `PICO_FLASH_BANK_STORAGE_OFFSET`.
- GP2040-CE's config lives at a fixed `0x101F8000`–`0x10200000`, with its footer in the last 12 bytes and the data directly below it ([lib/FlashPROM/src/FlashPROM.h](lib/FlashPROM/src/FlashPROM.h), [src/config_utils.cpp](src/config_utils.cpp) `loadConfigInner`).
- A local Release build of the `PicoW` config contains the literal `0x101fe000` in the flash-bank code, so the bank is `0x101FE000`–`0x10200000`: the top 8 KB of the config region.
- The storage is used at runtime by code the PR does not replace: the security manager stores its root keys there (`sm.c`, lines 3795 and 3814) and the ATT server stores notification subscriptions for bonded hosts there (`att_server.c`, lines 1120 and 1153).

What happens on a Pico W:

1. The user flashes this firmware and opens Web Config. Bluetooth starts three seconds after boot in every Web Config session, whether or not the user wants Bluetooth.
2. BTstack finds no header at `0x101FE000` and erases 4 KB there. If the stored config is larger than about 4 KB, part of it is now gone. The copy in RAM is still intact.
3. If the user reboots without saving, the CRC check fails and the config falls back to defaults.
4. If the user saves, FlashPROM rewrites the whole 32 KB from its cache and wipes BTstack's data. The next start repeats step 2.
5. If BTstack ever moves to bank 1, it erases the 4 KB that hold the config footer.

Not measured: the size of a real stored config, which decides whether step 2 destroys data on the first boot. Bank 1 always contains the footer regardless.

Boards with larger flash are not affected by the overlap. On Pico 2 W the bank is at `0x103FD000`, and on a 16 MB board it is at `0x10FFD000`. They still have a split design: bonds live in the config, while root keys and notification subscriptions live in BTstack's bank. "Forget saved devices" and a factory reset clear only the first.

There are also now two independent flash writers with different locking: FlashPROM writes from a timer alarm using `multicore_lockout_start_blocking()`, and BTstack writes through `flash_safe_execute()`.

**Recommendation:**

- Set `PICO_FLASH_BANK_STORAGE_OFFSET` for all boards to a reserved range outside the config region, for example the 8 KB directly below it (`0x1F6000`). The PicoW build ends at `0x101BAAE0`, which leaves about 245 KB of room.
- Add a `static_assert` that the bank and the config region do not overlap, and one that the firmware image ends below both.
- Decide whether the custom bond database is still needed once the bank is relocated. Using BTstack's own database in a safe location would remove F4, F8 and F9, at the cost of clearing the bank explicitly on factory reset.
- Combine with F11 so the radio is not touched unless the user asks for Bluetooth.

**Follow-up:** The BLE target reserves the 8 KB at offset `0x1F6000`. `BLEHIDManager.cpp` has compile-time checks for bank/config non-overlap and config bounds, plus a runtime guard before radio initialization that checks the linked image ends before both regions. Factory reset erases both BTstack banks; forgetting devices deletes each bond through BTstack so its associated CCC records are removed.

---

## High

<a id="f2"></a>
### F2. BTstack allocates with `malloc` from interrupt context

**Where:** [headers/btstack_config.h](headers/btstack_config.h) (`HAVE_MALLOC`), [CMakeLists.txt](CMakeLists.txt) (`pico_cyw43_arch_none`)

`pico_cyw43_arch_none` runs BTstack in a low-priority interrupt on core 0 (it links `pico_async_context_threadsafe_background`). With `HAVE_MALLOC` defined, BTstack takes its connection and security-manager objects from `malloc` inside that interrupt (`btstack_memory.c`, line 168).

The SDK's `malloc` wrapper takes a non-recursive mutex (`pico_malloc/malloc.c`, `MALLOC_ENTER`). If the interrupt fires while core 0's main loop is inside `malloc`, the interrupt waits forever for a mutex held by the code it interrupted. The firmware hangs.

Core 0 allocates in normal operation: input events are heap-allocated on button changes, and Web Config handlers build JSON documents and strings. BTstack allocates mainly on connect and pair, so the window is small, but the result is a hard hang with no recovery. Both symbols are present in the built PicoW image (`btstack_memory_hci_connection_get`, `malloc_mutex`).

**Recommendation:** Remove `HAVE_MALLOC` and rely on the static pools. Most `MAX_NR_*` counts are already defined; the build will name any that are missing.

**Follow-up (2026-11-27):** `HAVE_MALLOC` was removed. The Pico W build required `MAX_ATT_DB_SIZE`, now set to 512 bytes, and then compiled and linked successfully using static BTstack pools. Runtime connection/pairing behavior remains untested on hardware.

<a id="f3"></a>
### F3. Selecting Bluetooth on firmware without it faults at USB enumeration

**Where:** [www/src/Pages/BootModeMapping.tsx](www/src/Pages/BootModeMapping.tsx), [src/webconfig.cpp](src/webconfig.cpp) (`setGamepadOptions`), [src/drivermanager.cpp](src/drivermanager.cpp), [src/usbdriver.cpp](src/usbdriver.cpp)

The Settings page disables Save for Bluetooth when the firmware reports it as unsupported. Other paths have no such check:

- Boot Mode Mapping lists "Wireless → Bluetooth (BLE)" on every board.
- The firmware accepts any `inputMode` and `inputModeB1`…`R2` value without validation.
- Restoring a backup taken from a Bluetooth board carries the mode over.

On a non-Bluetooth build, `DriverManager::setup(INPUT_MODE_BLE)` falls into `default: return` and leaves the driver null. TinyUSB is still started, and its descriptor callbacks dereference the driver without a null check (for example `tud_descriptor_device_cb` in [src/usbdriver.cpp](src/usbdriver.cpp)). The device faults as soon as a host enumerates it.

A boot-mode selection is saved with `Storage::save(true)`, so the fault repeats on every normal boot until the user forces Web Config or BOOTSEL. The PR added null checks for the driver in `GP2040::run()` but not on this path.

The null dereference existed before for any unknown mode value. This PR is what makes one reachable from the UI.

**Recommendation:**

- In `GP2040::setup()`, fall back to a default USB mode when the selected mode has no driver and is not a supported wireless mode.
- Reject unsupported modes in the Web Config setters.
- Hide the Bluetooth option in Boot Mode Mapping when `supported` is false.

**Follow-up (2026-11-27):** `DriverManager::setup()` now reports unsupported modes, and `GP2040::setup()` falls back to XInput before USB starts. Both `setGamepadOptions` and `setBootModeOptions` reject BLE selections on non-BLE builds before mutating or saving configuration. Boot Mode Mapping queries BLE support and filters the BLE option; backup imports also display a failure if settings are rejected. Non-Bluetooth firmware and web builds pass. The invalid-mode recovery path has not been exercised on hardware.

---

## Medium

<a id="f4"></a>
### F4. Bond keys are exported by `/api/getConfig`

**Where:** [proto/config.proto](proto/config.proto) (`Config.bleConfig`)

The generated field list marks `bleConfig` as exportable, so `ConfigUtils::toJSON` includes each bond's long-term key and identity-resolving key, base64-encoded. `/api/setConfig` accepts them back. The comparable PS4 key material is protected with `[(nanopb).disallow_export = true]`.

Config dumps get shared for support, and a leaked long-term key lets someone impersonate the controller to that host or decrypt the link.

**Recommendation:** Add `[(nanopb).disallow_export = true]` to the `bleConfig` field.

<a id="f5"></a>
### F5. HID characteristics need no encryption, and Bluetooth mode is always open to pairing

**Where:** [src/ble_hid.gatt](src/ble_hid.gatt), [src/BLEHIDManager.cpp](src/BLEHIDManager.cpp) (`init`, `_smPacketHandler`)

- The Report characteristic is `READ | NOTIFY` with no security flag. BTstack's own `hids.gatt` marks it `ENCRYPTION_KEY_SIZE_16`, and the HID-over-GATT profile requires an encrypted link. Any device in range can connect and subscribe to inputs without pairing.
- The comment in the GATT file says the encryption flag "blocks service discovery before pairing". Permissions apply to reading and writing values, not to discovery. A rejected read is what normally prompts the host to pair.
- Outside Web Config, `_pairingMode` is always true at boot. The controller advertises as generally discoverable and auto-confirms Just Works pairing from anyone. The pairing toggle in the UI applies only while in Web Config.
- Only one connection is accepted, so an unpaired device that connects first keeps the real host out.

**Recommendation:** Restore the encryption requirement on the HID characteristics and retest pairing on the target hosts. Consider advertising to bonded hosts only when bonds exist, with open pairing as an explicit action. If the open behaviour is intended, say so in the UI text.

<a id="f6"></a>
### F6. Analog axes are never sent

**Where:** [src/OutputManager.cpp](src/OutputManager.cpp)

The report descriptor declares X, Y, Z and Rz, but the four bytes are always `0x80`. The comment notes "no analog input yet". Two consequences are less obvious:

- With D-pad mode set to left stick or right stick, `Gamepad::process()` moves the directions into `lx`/`ly` or `rx`/`ry` and removes them from `state.dpad` ([src/gamepad.cpp](src/gamepad.cpp), line 321 onward). Over Bluetooth those directions are not sent at all.
- Analog add-on input and analog triggers are dropped.

**Recommendation:** Fill the axes from `state.lx`, `ly`, `rx`, `ry`, scaled to 8 bits, before this leaves draft.

<a id="f7"></a>
### F7. The Bluetooth identity reuses the USB vendor/product ID with a different layout

**Where:** [headers/ble_identity.h](headers/ble_identity.h)

The PnP ID is `10C4:82C0`, the same as USB DInput mode, "so BLE presents the same gamepad identity known to SDL / Steam mapping databases". The layouts differ: USB DInput sends 32 buttons plus a hat switch, while Bluetooth sends 18 buttons in a different order with the D-pad as buttons 13–16 and no hat.

The community SDL database has a Windows entry for this ID (`03000000c4100000c082000000000000`, named "KADE") that expects a hat for the D-pad and `a:b1,b:b2,x:b0,y:b3`. If SDL on Windows gives the Bluetooth device the same GUID, face buttons will be swapped and the D-pad will not work in SDL and Steam titles. Whether the GUID matches needs a test on a Windows host.

**Recommendation:** Either match the USB DInput report layout, including the hat, or use a distinct product ID for Bluetooth.

<a id="f8"></a>
### F8. Bond data is changed from interrupt context while other code saves the config

**Where:** [src/le_device_db_proto.cpp](src/le_device_db_proto.cpp)

- The `le_device_db_*` callbacks write directly into the live `Config` from BTstack's interrupt. `_saveBonds()` and `clearBonds()` hold the BTstack lock while saving, but every other `Storage::save()` caller does not (Web Config setters, hotkeys, boot-mode changes). An interrupt during one of those encodes can write a half-updated bond.
- `_saveBonds()` saves the whole live config, so it also persists any unsaved in-memory change, such as a hotkey-toggled D-pad mode.
- `Storage::save()` returns true before anything reaches flash, because FlashPROM defers the write by 50 ms on an alarm. `_bondSaveFailed`, the retry path and the "could not save pairings" warning in the UI can therefore almost never trigger.

**Recommendation:** Keep bond changes in a private staging copy and copy them into `Config` under the lock just before saving, or take the BTstack lock inside the save path when Bluetooth is built in. Remove the save-failed flag or connect it to a real failure signal.

<a id="f9"></a>
### F9. The new config message uses most of the remaining size budget

**Where:** [proto/config.proto](proto/config.proto)

The generated header reports `Config_size` = 32557 and `BLEConfig_size` = 358. The limit enforced by the `static_assert` in [src/config_utils.cpp](src/config_utils.cpp) is 32768 minus a 12-byte footer. That leaves 199 bytes, down from roughly 560 before this PR. The next feature that adds config fields is likely to fail the assert.

**Recommendation:** Raise this with upstream maintainers. Relocating bonds to BTstack's own storage (see F1) would give the space back.

<a id="f10"></a>
### F10. The Pimoroni board config targets the wrong SDK board

**Where:** [configs/PimoroniPicoLipo2XLW/PimoroniPicoLipo2XLW.cmake](configs/PimoroniPicoLipo2XLW/PimoroniPicoLipo2XLW.cmake), [configs/PimoroniPicoLipo2XLW/BoardConfig.h](configs/PimoroniPicoLipo2XLW/BoardConfig.h)

- The config sets `PICO_BOARD pico2_w`, which describes an RP2350A with 30 GPIOs and 4 MB flash. SDK 2.3.1 ships `pimoroni_pico_lipo2xl_w.h` (RP2350B, 16 MB, same CYW43 pins, battery sense on GPIO 43).
- `set(PICO_NUM_GPIOS 48)` has no effect. The name does not appear anywhere in the SDK or in this repository. The local build confirms the `pico2_w` header is used, so GPIOs 30–47 cannot be mapped.
- The battery ADC on GPIO 43 works only because the SDK's range checks are compiled out by default.
- The comments disagree with each other: `BoardConfig.h` describes GPIO 29 as the battery divider output, and [src/BLEHIDManager.cpp](src/BLEHIDManager.cpp) says the read uses "GPIO29 (ADC3)", while `BatteryConfig.h` says GPIO 43.
- The note that declaring 16 MB "causes the RP2350 boot/partition path to erase the FlashPROM EEPROM region" is probably a misdiagnosis for one of the two cases it lists. With 16 MB declared, BTstack's bank sits at `0x10FFD000`–`0x10FFF000`, inside the `0x10FF8000` placement that was tried. The `0x101F8000` case is not explained by this.

**Recommendation:** Switch to `pimoroni_pico_lipo2xl_w` after fixing F1, retest the 16 MB behaviour, and correct the comments. The divider ratio and voltage range in `BatteryConfig.h` were not checked against the board schematic.

<a id="f11"></a>
### F11. The radio is started in every Web Config session

**Where:** [src/gp2040.cpp](src/gp2040.cpp) (`wirelessOnly || configMode`), [src/BLEHIDManager.cpp](src/BLEHIDManager.cpp) (`process`, `_doInit`)

On every CYW43 board, including the existing `PicoW` and `WaveshareRP2350BPlusW` configs, entering Web Config starts the radio three seconds after boot. `cyw43_arch_init()` and the Bluetooth power-on run synchronously in the main loop, so USB and the web server are not serviced during the firmware download. If it fails, the blocking attempt repeats every five seconds.

This changes behaviour for users who never use Bluetooth, and it is what makes F1 reachable without opting in. The stall duration was not measured.

**Recommendation:** Start the radio in Web Config only when the Bluetooth settings card requests it, or when the saved input mode is Bluetooth.

---

## Low

<a id="f12"></a>
### F12. No report is sent after a reconnect until an input changes

**Where:** [src/BLEHIDManager.cpp](src/BLEHIDManager.cpp) (`_handleDisconnect`, `sendReport`)

`_lastSentReport` is not cleared on disconnect. After a reconnect, a state equal to the last one sent is treated as a duplicate, so the host gets nothing until something changes. Some hosts show axes at zero rather than centred until the first report.

The comment that `_lastSentReport` is "only accessed here (IRQ context)" is also wrong. `sendReport` reads it from the main loop, under the lock.

**Recommendation:** Clear `_lastSentReport` in `_resetReportQueue()`.

<a id="f13"></a>
### F13. Debug logging, diagnostic LED codes and an unused idle state

**Where:** [src/BLEHIDManager.cpp](src/BLEHIDManager.cpp)

- `printf` status output runs every five seconds, and send logging every two seconds, including from the interrupt handler.
- The disconnect-reason blink code keeps its state in function-level statics. If a host connects mid-sequence, the next disconnect reuses the stale count and the LED can be left on.
- The `IDLE` power state changes no connection parameters. Its only effect is a 50 ms throttle on reports that are already filtered for duplicates.
- A second `uint32_t now` shadows the first inside `process()`.

**Recommendation:** Remove these or put them behind a debug define before upstream review.

<a id="f14"></a>
### F14. Smaller firmware issues

- **Redundant run-loop init.** `_doInit()` calls `btstack_run_loop_init()` a second time; `btstack_cyw43_init()` already did. It is harmless now and would trip an assert if BTstack assertions are enabled.
- **`cyw43_arch_poll()` does nothing here.** The threadsafe-background context has no poll function, so the header comment that `process()` "drives cyw43_arch_poll" is misleading.
- **`btstack_config.h` contents.** `MAX_NR_HIDS_CONNECTIONS` and `ENABLE_HID_DEVICE` are not referenced by BTstack. `ENABLE_HCI_CONTROLLER_TO_HOST_FLOW_CONTROL` and `MAX_NR_CONTROLLER_ACL_BUFFERS` are supported by the SDK but not set; compare against the pico-examples reference config.
- **Unreachable branch.** In `SM_EVENT_JUST_WORKS_REQUEST`, the decline branch requires `!_connected`, which cannot be true during pairing.
- **Disconnect handle not checked.** `_handleDisconnect()` resets state for any disconnect event without comparing the handle.
- **ADC reset.** `adc_init()` runs again in `_doInit()` three seconds after `GP2040::setup()` already called it, which resets the ADC block. This affects only boards with `BATTERY_ADC_GPIO`.
- **Eviction order.** The bond sequence number changes only on pairing, so eviction is least-recently-paired, not least-recently-used.
- **Stale comments.** `sendReport` is documented as sending a "3-byte" report (it is 7), and "Static TLV context removed" is left in `BLEHIDManager.cpp`.
- **Unused font data.** Only `GP_Font_Standard` is used at runtime ([src/addons/display.cpp](src/addons/display.cpp)). The placeholder rows added to `GP_Font_Basic.h` and `GP_Font_Big.h` can be dropped.
- **CMake path.** The TLV exclusion uses `${PICO_SDK_PATH}/lib/btstack`; `${PICO_BTSTACK_PATH}` is the variable the SDK uses for the same path.
- **Connection interval.** There is no preferred-connection-parameters characteristic or update request, so latency is whatever the host picks.

<a id="f15"></a>
### F15. Web UI and PR hygiene

- **Tests are not wired in.** [www/scripts/tests/bluetooth-api.test.js](www/scripts/tests/bluetooth-api.test.js) passes when run by hand, but `package.json` has no `test` script and the Node workflow runs only `npm ci` and `npm run build`.
- **No mock endpoints.** [www/server/app.js](www/server/app.js) has no handlers for `/api/getBLEHIDStatus` or `/api/setBLEHIDControls`, so the card shows "Status unavailable" under `npm run dev`.
- **Save button flicker.** A failed or timed-out status poll sets `bleSupported` to false, which disables Save until the next successful poll.
- **Formatting.** [www/src/Components/BluetoothSettings.tsx](www/src/Components/BluetoothSettings.tsx) does not pass `prettier --check`.
- **Scope.** The commit "Fix straightforward web TypeScript errors" touches six unrelated files. The PR description says the branch contains only Bluetooth commits. Consider splitting it out.
- **Translations.** Only `en` has the new strings. The other eight locales fall back to English.

---

## Checked and found correct

- **Bond persistence mechanism.** The original PR stored bonds in protobuf; the working tree now uses BTstack's flash-backed TLV database. Persistence across an actual power cycle still requires hardware validation.
- **Empty bond slots.** `le_device_db_info()` leaves its outputs untouched for invalid entries, which is what `sm.c` relies on (`BD_ADDR_TYPE_UNKNOWN` sentinel, lines 1488 and 1562).
- **Report format.** The updated descriptor and `OutputManager` agree on an 8-bit button field and six 8-bit axes (two sticks and two triggers), for a 9-byte report body; the report ID is carried by the Report Reference descriptor.
- **Null driver in Bluetooth builds.** `GP2040::run()`, `GP2040Aux` and the add-ons that read the joystick mid value all check for null. TinyUSB is not started in wireless-only mode, so its callbacks are not reached.
- **Enum value.** `INPUT_MODE_BLE = 18` is unused upstream (17 and 19 are taken), and `www/src_gen/enums.ts` matches.
- **Flash layout on larger boards.** The 4 MB and 16 MB bank locations do not overlap the config region.
- **Web code quality.** Focused ESLint on Bluetooth UI/API files and the production web build pass. Broader ESLint reports 20 existing errors in `BootModeMapping.tsx` and `WebApi.js`, none on changed lines. `tsc --noEmit` reports 133 diagnostics across the web project; none are in the Bluetooth settings component or service, and the two Boot Mode Mapping diagnostics are on unchanged lines.

## Verification performed

The checks in this first table describe the original PR state, not the current working tree:

| Original PR check | Result |
|---|---|
| `PicoW` Release firmware build, SDK 2.3.1, `SKIP_WEBBUILD=TRUE` | Builds and links |
| `PimoroniPicoLipo2XLW` Release firmware build | Builds and links; no warnings in the new sources |
| PicoW ELF inspection (`nm`, disassembly, map) | Bank at `0x101FE000`; image ends at `0x101BAAE0`; only the protobuf bond database is linked |
| `node --test scripts/tests/bluetooth-api.test.js` | 8 of 8 pass |
| ESLint on changed web files | No errors on PR-added lines; the errors reported are on lines the PR did not change |
| `tsc --noEmit` | 131 errors in total at original review time |
| PR CI at review time | Node build passed; all firmware board builds still queued |

### Follow-up checks (2026-11-27)

| Check | Result |
|---|---|
| Non-Bluetooth firmware build, SDK 2.3.1, Python 3.13 | Passed |
| PicoW Release firmware build, SDK 2.3.1, Python 3.13 | Passed after adding `MAX_ATT_DB_SIZE 512` |
| PicoW ELF flash boundary | `__flash_binary_end` at `0x101B9908` (offset `0x1B9908`), below BTstack offset `0x1F6000` and config offset `0x1F8000` |
| Pimoroni Pico Lipo 2XL W ELF flash boundary | `__flash_binary_end` at `0x101B0C30` (offset `0x1B0C30`), below BTstack offset `0x1F6000` and 16 MB config offset `0xFF8000` |
| Web configurator `npm run build` | Passed; existing Japanese duplicate-key and large-chunk warnings remain |
| Prettier on modified web files | Passed |
| `tsc --noEmit` | 133 diagnostics total; the two in `BootModeMapping.tsx` (lines 81 and 157) are on unchanged lines |
| Focused ESLint on Bluetooth UI, API service, mock server, and tests | Passed |
| Expanded ESLint including legacy `BootModeMapping.tsx` and `WebApi.js` | 20 existing errors; none are on changed lines |
| `git diff --check` | Passed |
| Pico W Release, Pimoroni Pico Lipo 2XL W Release, and non-Bluetooth Pico firmware builds | Passed with Pico SDK 2.3.1 |

Not verified:

- Any behaviour on hardware, including pairing, reconnect, host compatibility and the F1 sequence.
- Bond persistence across power cycles, battery-divider calibration, host compatibility, and radio start-up impact.

## Hardware validation and deferred follow-up

- User to test pairing, bonded reconnect, report axes/triggers, forget-device/factory-reset behavior, and battery sensing on real hardware.
- Address the deferred unused font rows and PR-scope hygiene separately if desired.
