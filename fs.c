#include "fs.h"
#include "ui.h"

int dev_init() {
	if (!__io_wiisd.startup() || !__io_wiisd.isInserted()) {
		if (!__io_usbstorage.startup() || !__io_usbstorage.isInserted()) return 0;
		fatMountSimple("usb", &__io_usbstorage);
		return 2;
	}
	fatMountSimple("sd", &__io_wiisd);
	return 1;
}

void dev_close(int type) {
	if (type < 1 || type > 2) return;
	fatUnmount(type == 1 ? "sd" : "usb");
	if (type == 1) __io_wiisd.shutdown();
	if (type == 2) __io_usbstorage.shutdown();
}

int exists(char *name) {
	struct stat s;
	return stat(name, &s) == 0;
}

int is_file(char *name) {
	struct stat s;
	return (stat(name, &s) == 0) && S_ISREG(s.st_mode);
}

char **get_filenames(char *path, int *n_files) {
	if (!exists(path)) {
		mkdir(path, 0777);
		printf("\x1b[25;10H%s created", path);
		return NULL;
	}
	int count = 0;
	char **files = calloc(200, 4);
	char name[200];
	DIR *dir = opendir(path);
	if (!dir) {
		printf("\x1b[25;10HCould not open \"%s\"", path);
		return NULL;
	}
	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL && count < 200) {
		if (strlen(ent->d_name) > 180) {
			printf("\x1b[25;10HFilename too large, skipping");
			continue;
		}
		sprintf(name, "%s/%s", path, ent->d_name);
		if (!is_file(name)) continue;
		files[count++] = strdup(ent->d_name);
	}
	closedir(dir);
	if (!count) {
		printf("\x1b[26;10HCould not find any files in the given path");
		free(files);
		files = NULL;
	}
	if (n_files) *n_files = count;
	return files;
}

int get_width(char **names, int len) {
	int i;
	int high = 0, l = 0;
	for (i = 0; i < len; i++) {
		if (!names[i]) continue;
		l = strlen(names[i]);
		if (l > high) high = l;
	}
	return high;
}

void delete_str(char *s) {
	memset(s, 0, strlen(s));
	free(s);
	s = NULL;
}

void close_filenames(char **files, int n_files) {
	int i;
	for (i = 0; i < n_files; i++) delete_str(files[i]);
	free(files);
}
