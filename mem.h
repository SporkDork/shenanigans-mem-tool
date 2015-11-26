#ifndef MEM_H
#define MEM_H

#include <gccore.h>
#include <stdio.h>

#define N_RANGES 6
#define TO_MEM 0
#define TO_FILE 1

static const u32 mem_ranges_start[] = {
	0x80000000, 0x90000000, 0xC0000000, 0xCC000000, 0xCD000000, 0xD0000000
};

static const u32 mem_ranges_end[] = {
	0x84000000, 0xA0000000, 0xC4000000, 0xCC007000, 0xCE000000, 0xE0000000
};

int get_addr_range(u32 addr, int var_sz);
int get_invalid_addr_range(u32 addr);

void sync_exec_range(u32 addr);

void refresh_view(u8 *set, u32 top);
void poke_view(u32 addr, int cursor, u8 value);

void file_memory_transfer(u32 addr, u32 size, FILE *f, int mode);
void extract(u32 addr, u32 size, char *name);
void patch(u32 addr, u32 size, char *name);

#endif
