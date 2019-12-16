#include "header.h"

const char *methods[] = {
	"open",
	"close",
	"read",
	"write",
	"seek",
	"ioctl",
	"ioctlv",
	"async"
};

int method_count() { return sizeof(methods); }
char *get_method(int idx) { return (char*)methods[idx]; }

const char *devices[] = {
	"/dev/aes",
	"/dev/boot2",
	"/dev/di",
	"/dev/es",
	"/dev/flash",
	"/dev/fs",
	"/dev/hmac",
	"/dev/listen",

	"/dev/net/ip/bottom",
	"/dev/net/ip/top",
	"/dev/net/ncd/manage",
	"/dev/net/kd/request",
	"/dev/net/kd/time",
	"/dev/net/ssl",
	"/dev/net/usbeth/top",
	"/dev/net/wd/command",
	"/dev/net/wd/top",

	"/dev/printserver",

	"/dev/stm/eventhook",
	"/dev/stm/immediate",

	"/dev/usb/ehc",
	"/dev/usb/hid",
	"/dev/usb/kbd",
	"/dev/usb/msc",
	"/dev/usb/oh0",
	"/dev/usb/oh1",
	"/dev/usb/shared",
	"/dev/usb/usb",
	"/dev/usb/ven",
	"/dev/usb/wfssrv",

	"/dev/sdio/slot0",
	"/dev/sdio/slot1",

	"/dev/wfsi",
	"/dev/wl0",
};

int ipcdev_count() { return sizeof(devices); }
char *get_ipcdev(int idx) { return (char*)devices[idx]; }

