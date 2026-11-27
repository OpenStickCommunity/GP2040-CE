#include "addons/keyboard_host.h"
#include "addons/keyboard_host_listener.h"
#include "storagemanager.h"
#include "drivermanager.h"
#include "usbhostmanager.h"
#include "peripheralmanager.h"
#include "class/hid/hid_host.h"

#include <algorithm>

bool KeyboardHostAddon::available() {
  const KeyboardHostOptions& keyboardHostOptions = Storage::getInstance().getAddonOptions().keyboardHostOptions;
	return keyboardHostOptions.enabled && PeripheralManager::getInstance().isUSBEnabled(0);
}

void KeyboardHostAddon::setup() {
  listener = new KeyboardHostListener();
  ((KeyboardHostListener*)listener)->setup();
}

void KeyboardHostAddon::preprocess() {
  ((KeyboardHostListener*)listener)->process();
}

void KeyboardHostAddon::waitForBootInput() {
  const KeyboardHostOptions& options = Storage::getInstance().getAddonOptions().keyboardHostOptions;
  if (!options.bootDelayEnabled) return;

  USBHostManager& host = USBHostManager::getInstance();
  host.start();
  const uint32_t maxWait = std::min<uint32_t>(options.bootDelayMs, 3000);
  for (uint32_t start = getMillis(), unmounted = start, now; (now = getMillis()) - start < maxWait; ) {
    host.process();
    if (!((KeyboardHostListener*)listener)->keyboardMounted()) unmounted = now;
    else if (now - unmounted >= 100) break;
  }
}
