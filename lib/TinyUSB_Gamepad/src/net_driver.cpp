#include "net_driver.h"

const usbd_class_driver_t net_driver = {
#if CFG_TUSB_DEBUG >= 2
	.name = "NET",
#endif
	.init             = netd_init,
	.reset            = netd_reset,
	.open             = netd_open,
	.control_request  = netd_control_request,
	.control_complete = netd_control_complete,
	.xfer_cb          = netd_xfer_cb,
	.sof              = NULL,
};
