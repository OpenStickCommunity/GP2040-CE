# PS4 Mode

By default the PS4 mode in GP2040-CE can be used with an 8-minute timeout on:

* PlayStation 4 with all PS4 games
* PlayStation 5 when playing PlayStation 4 games

Using PS4 mode on a PC does not result in any timeout behavior, but note that XInput is the suggested mode on PC for best compatibility.

## 8-Minute Timeout

PlayStation 4 and PlayStation 5 use an authentication mechanism to ensure only authorized controllers are used on the console. If a device doesn't implement this authentication mechanism the controller is subject to an 8-minute timeout.

The 8-minute timeout works like this:

1. Plug your controller into the PS4/5
2. Press the PS button to initialize the controller
3. Play for roughly 8 minutes
4. Console rejects the controller due to failed authentication
5. User unplugs their non-functional controller
6. Go back to step 1 and repeat

GP2040-CE allows you to upload the files required to authenticate your device via the [PS4 Mode addon](web-configurator?id=ps4-mode), which effectively removes this timeout issue. If you're using an OLED display, the input mode will change from `PS4` to `PS4:AS` to indicate your device has successfully authenticated with the PS4/PS5 console.

!> **The GP2040-CE project will not provide files or information related to authenticating your device beyond what the firmware has implemented. Please do not ask via any of our communication channels as this may result in a blacklist/ban.**

## Credits

PS4 mode was built using the efforts of another open source firmware project named [Passing Link](https://github.com/passinglink/passinglink). The team behind Passing Link are in no way affiliated with the GP2040-CE project, however the work and information they've made available was greatly appreciated so we could bring this highly-requested feature to our users.
