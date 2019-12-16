#include "header.h"

struct _device_t {
	const char *type;
	const DISC_INTERFACE *interface;
}
devs[] = {{"sd", &__io_wiisd}, {"usb", &__io_usbstorage}};
const int n_devs = 2;

struct _device_t *device = NULL;

int dev_init() {
	if (device)
		dev_close();

	int i;
	for (i = 0; i < n_devs; i++) {
		if (!devs[i].interface->startup() || !devs[i].interface->isInserted())
			continue;

		device = &devs[i];
		fatMountSimple(device->type, device->interface);
		break;
	}

	return device != NULL;
}

const char *dev_type() {
	if (!device)
		return NULL;

	return device->type;
}

void dev_close() {
	if (!device)
		return;

	device->interface->shutdown();
	device = NULL;
}

int entry_type(char *name) {
	struct stat s;
	if (stat(name, &s) != 0) // if the entry doesn't exist
		return TYPE_NONE;

	if (S_ISREG(s.st_mode)) // if the entry is a file
		return TYPE_FILE;
	else if (S_ISDIR(s.st_mode)) // if the entry is a folder
		return TYPE_DIR;

	return TYPE_OTHER;
}

ui_entry *get_filenames(char *path, int *total) {
	if (entry_type(path) == TYPE_NONE) {
		mkdir(path, 0777);
		printf("\x1b[25;10H%s created", path);
		return NULL;
	}

	int count = 0;
	ui_entry *names = NULL;

	DIR *dir = opendir(path);
	if (!dir) {
		printf("\x1b[25;10HCould not open \"%s\"", path);
		return NULL;
	}

	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		names = realloc(names, (count + 1) * sizeof(char*));
		names[count].str = strdup(ent->d_name);

		int type = entry_type(ent->d_name);
		names[count].info = type;
		names[count].more = type == 2; // if type == 2, the current entry is a folder

		count++;
	}
	closedir(dir);

	if (!count) {
		free(names);
		names = NULL;
	}

	if (total) *total = count;
	return names;
}
