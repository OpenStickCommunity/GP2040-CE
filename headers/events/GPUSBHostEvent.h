#ifndef _GPUSBHOSTEVENT_H_
#define _GPUSBHOSTEVENT_H_

class GPUSBHostEvent : public GPEvent {
    public:
        GPUSBHostEvent() {}
        GPUSBHostEvent(uint8_t devAddr, uint16_t vid, uint16_t pid) {
            this->deviceAddress = devAddr;
            this->vendorID = vid;
            this->productID = pid;
        }
        virtual ~GPUSBHostEvent() {}

        uint8_t deviceAddress = 0;
        uint16_t vendorID = 0;
        uint16_t productID = 0;
    private:
};

class GPUSBHostMountEvent : public GPUSBHostEvent {
    public:
        GPUSBHostMountEvent() {}
        GPUSBHostMountEvent(uint8_t devAddr, uint16_t vid, uint16_t pid) : GPUSBHostEvent(devAddr, vid, pid) {}
        virtual ~GPUSBHostMountEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_USBHOST_MOUNT;
};

class GPUSBHostUnmountEvent : public GPUSBHostEvent {
    public:
        GPUSBHostUnmountEvent() {}
        GPUSBHostUnmountEvent(uint8_t devAddr, uint16_t vid, uint16_t pid) : GPUSBHostEvent(devAddr, vid, pid) {}
        virtual ~GPUSBHostUnmountEvent() {}

        GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_USBHOST_UNMOUNT;
};

#endif