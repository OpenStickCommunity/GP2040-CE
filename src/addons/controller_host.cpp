#include "addons/controller_host.h"
#include "storagemanager.h"



bool ControllerHostAddon::available()
{
    const ControllerHostOptions& options = Storage::getInstance().getAddonOptions().controllerHostOptions;
    printf("%d\n", options.enabled);
    return options.enabled;
}

void ControllerHostAddon::setup()
{
    printf("setup function.");
}