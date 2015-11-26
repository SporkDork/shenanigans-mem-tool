#ifndef FS_H
#define FS_H

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <sdcard/wiisd_io.h>
#include <ogc/usbstorage.h>
#include <fat.h>

static const char *dev_exts[] = {"sd", "usb"};

int dev_init();
void dev_close(int type);

int exists(char *name);
int is_file(char *name);
char **get_filenames(char *path, int *n_files);
int get_width(char **names, int len);
void delete_str(char *s);
void close_filenames(char **files, int n_files);

#endif
