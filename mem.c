#include "gfx.h"
#include "ui.h"
#include "mem.h"
#include "cache.h"

int get_addr_range(u32 addr, int var_sz) {
	int i;
	for (i = 0; i < N_RANGES; i++)
		if (addr >= mem_ranges_start[i] && addr <= mem_ranges_end[i] - var_sz) return i+1;
	return 0;
}

int get_invalid_addr_range(u32 addr) {
	u32 start = 0;
	u32 end = mem_ranges_start[0];
	if (addr >= 0 && addr < end) return 1;
	if (addr == 0xFFFFFFFF) return N_RANGES + 1;
	int i;
	for (i = 0; i < N_RANGES; i++) {
		start = mem_ranges_end[i];
		end = i+1 >= N_RANGES ? 0xFFFFFFFF : mem_ranges_start[i+1];
		if (addr >= start && addr < end) return i+2;
	}
	return 0;
}

/*
void test_mem2() {
	u32 *block = (u32*)0x90000000;
	u32 addr = 0x90000000;
	int i, tally = 0, high = 0;
	for (i = 0; i < 0xD00000; i++) {
		if (block[i]) {
			if (tally > high) {
				high = tally;
				addr = 0x90000000 + (i * 4);
			}
			tally = 0;
		}
		else tally += 4;
	}
	printf("\x1b[23;20HLargest block in MEM2: %08X %X", addr, high);
}
*/

void sync_exec_range(u32 addr) {
	int range = get_addr_range(addr, 4);
	if (range < 1 || range > 2) return;
	u32 sz = mem_ranges_end[range-1] - addr;
	sync_before_exec((void*)addr, sz);
}

void refresh_view(u8 *set, u32 top) {
	int range = get_addr_range(top, 4);
	if (!range) return;
	u32 addr = top, value = 0;
	if (top > mem_ranges_end[range-1] - 96) addr = mem_ranges_end[range-1] - 96;
	if (range == 1 || range == 2) sync_before_read((void*)addr, 96);
	int i;
	for (i = 0; i < 9; i++) {
		addr = *((vu32*)(top + i*8));
		value = *((vu32*)(top + i*8 + 4));
		set_numbers(set + i*16, addr, value);
	}
}

void poke_view(u32 addr, int cursor, u8 value) {
	int range = get_addr_range(addr, 4);
	if (!range) return;

	if (range == 1 || range == 2) sync_before_read((void*)addr, 4);

	u32 v = *((vu32*)(addr & ~3));
	int shift = 28 - (cursor & 7) * 4;

	v &= ~(0xf << shift);
	v |= (value & 0xf) << shift;

	*((vu32*)(addr & ~3)) = v;

	if (range == 1 || range == 2) sync_after_write((void*)addr, 4);
}

void file_memory_transfer(u32 addr, u32 size, FILE *f, int mode) {

	if ((u64)addr + (u64)size > 0x100000000ULL) size = (int)(0x100000000ULL - (u64)addr);
	if (size < 0) return;

	int range, inv_range;
	u32 offset = 0, transfer = 0;
	
	while (offset < size) {
		range = get_addr_range(addr + offset, 1);
		if (!range) {
			inv_range = get_invalid_addr_range(addr + offset);
			if (!inv_range || inv_range > N_RANGES) break;

			offset = mem_ranges_start[inv_range-1] - addr;
			if (offset >= size) break;

			if (mode == 0) { // read from file
				fseek(f, offset, SEEK_SET);
			}
			else { // write to file
				int i;
				for (i = 0; i < offset; i++) fputc(0, f);
			}
		}
		else {
			transfer = size - offset;
			int r = get_addr_range(addr + size, 1);
			if (!r || r > range) transfer = mem_ranges_end[range-1] - (addr + offset);

			if (mode == 0) { // read from file
				fread((u8*)(addr + offset), 1, transfer, f);

				if (range == 1 || range == 2) {
					sync_after_write((void*)(addr + offset), transfer);
				}
			}
			else { // write to file
				if (range == 1 || range == 2) {
					sync_before_read((void*)(addr + offset), transfer);
				}

				fwrite((u8*)(addr + offset), 1, transfer, f);
			}

			if (range+1 > N_RANGES) break;
			offset = mem_ranges_start[range+1];
		}
	}
}

void extract(u32 addr, u32 size, char *name) {

	printf("\x1b[21;37HExtract");
	draw_box(COLOR_BLACK, COLOR_TEAL, 0, 296, 336, 56, 16);

	FILE *f = fopen(name, "wb");
	if (!f) {
		printf("\x1b[23;40HError: could not create file");
		return;
	}

	file_memory_transfer(addr, size, f, TO_FILE);

	fclose(f);
}

void patch(u32 addr, u32 size, char *name) {

	FILE *f = fopen(name, "rb");
	if (!f) return;

	fseek(f, 0, SEEK_END);
	int sz = ftell(f);
	rewind(f);
	if (sz <= 0) return;
	if (sz > size && size > 0) sz = size;
	
	file_memory_transfer(addr, sz, f, TO_MEM);

	fclose(f);

	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
}
