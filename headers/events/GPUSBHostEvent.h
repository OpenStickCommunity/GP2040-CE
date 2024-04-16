#ifndef _GPUSBHOSTEVENT_H_
#define _GPUSBHOSTEVENT_H_

class GPUSBHostEvent : public GPEvent {
    public:
        GPUSBHostEvent() {}
        GPUSBHostEvent(uint8_t devAddr) {
            this->deviceAddress = devAddr;
        }
        ~GPUSBHostEvent() {}

        uint8_t deviceAddress;
    private:
};

class GPUSBHostMountEvent : public GPUSBHostEvent {
    public:
        GPUSBHostMountEvent() {}
        GPUSBHostMountEvent(uint8_t devAddr) : GPUSBHostEvent(devAddr) {}
        ~GPUSBHostMountEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_USBHOST_MOUNT;
};

class GPUSBHostUnmountEvent : public GPUSBHostEvent {
    public:
        GPUSBHostUnmountEvent() {}
        GPUSBHostUnmountEvent(uint8_t devAddr) : GPUSBHostEvent(devAddr) {}
        ~GPUSBHostUnmountEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_USBHOST_UNMOUNT;
};

#endif