#include "net_driver.h"

const usbd_class_driver_t net_driver = {
#if CFG_TUSB_DEBUG >= 2
	.name = "NET",
#endif
	.init             = netd_init,
	.reset            = netd_reset,
	.open             = netd_open,
	.control_xfer_cb  = netd_control_xfer_cb,
	.xfer_cb          = netd_xfer_cb,
	.sof              = NULL,
};
