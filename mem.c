#include "header.h"

#define BANK_MEM1_CCH 0
#define BANK_MEM2_CCH 1
#define BANK_MEM1_UNC 2
#define BANK_REGS 3
#define BANK_MEM2_UNC 4

#define N_BANKS 5

#define TO_MEM 0
#define TO_FILE 1

struct range {
	int start;
	int end;
} bank[] = {
	{0x80000000, 0x84000000},
	{0x90000000, 0xA0000000},
	{0xC0000000, 0xC4000000},
	{0xCC000000, 0xCE000000},
	{0xD0000000, 0xE0000000}
};

int bank_of(u32 addr, int var_sz) {
	int i;
	for (i = 0; i < N_BANKS; i++) {
		if (addr >= bank[i].start && addr <= bank[i].end - var_sz)
			return i;
	}

	return -1;
}

int next_bank(u32 addr) {
	u32 begin = 0;
	int i;
	for (i = 0; i < N_BANKS; i++) {
		if (addr >= begin && addr < bank[i].start)
			return i;

		begin = bank[i].start;
	}

	return -1;
}

void file_memory_transfer(u32 addr, s32 size, FILE *f, int mode) {
	// ensure we can access all bytes of the file
	if ((u64)addr + (u64)size > 0x100000000ULL)
		size = (s32)(0x100000000ULL - (u64)addr);

	if (size < 0)
		return;

	u32 left = size;
	while (left > 0) {
		int range = bank_of(addr, 1);
		if (range < 0) {
			int next = next_bank(addr);
			if (next < 0)
				break;

			addr = bank[next].start;
			continue;
		}

		u32 max = bank[range].end - addr;
		u32 transfer = left < max ? left : max;

		// read from file, write to memory
		if (mode == 0) {
			fread((u8*)addr, 1, transfer, f);

			if (range == BANK_MEM1_CCH || range == BANK_MEM2_CCH)
				DCFlushRange((void*)addr, transfer);
		}
		// read from memory, write to file
		else {
			if (range == BANK_MEM1_CCH || range == BANK_MEM2_CCH)
				DCInvalidateRange((void*)addr, transfer);

			fwrite((u8*)addr, 1, transfer, f);
		}

		addr += transfer;
		left -= transfer;
	}
}

void extract(u32 addr, s32 size, char *name) {
	FILE *f = fopen(name, "wb");
	if (!f) {
		printf("\x1b[23;40HError: could not create file");
		return;
	}

	file_memory_transfer(addr, size, f, TO_FILE);
	fclose(f);
}

void patch(u32 addr, s32 size, char *name) {
	FILE *f = fopen(name, "rb");
	if (!f) return;

	fseek(f, 0, SEEK_END);
	int sz = ftell(f);
	rewind(f);
	if (sz <= 0) return;
	if (sz > size && size > 0) sz = size;
	
	file_memory_transfer(addr, sz, f, TO_MEM);
	fclose(f);
}

u32 peek(u32 addr) {
	int bank = bank_of(addr, sizeof(u32));
	if (bank == BANK_MEM1_CCH || bank == BANK_MEM2_CCH)
		DCInvalidateRange((void*)addr, sizeof(u32));

	return *((vu32*)addr);
}

void poke(u32 addr, u32 value) {
	*((vu32*)addr) = value;

	int bank = bank_of(addr, sizeof(u32));
	if (bank == BANK_MEM1_CCH || bank == BANK_MEM2_CCH)
		DCFlushRange((void*)addr, sizeof(u32));
}

void execute(u32 addr, u32 value) {
	struct range *mem1 = &bank[BANK_MEM1_CCH];
	struct range *mem2 = &bank[BANK_MEM2_CCH];

	// Prepare MEM1
	u32 len1 = (u32)(mem1->end - mem1->start);
	DCFlushRange((void*)mem1->start, len1); // flush MEM1 d-cache (commits memory edits)
	ICInvalidateRange((void*)mem1->start, len1); // invalidate MEM1 i-cache (prepares execution) (brutal)

	// Prepare MEM2
	u32 len2 = (u32)(mem2->end - mem2->start);
	DCFlushRange((void*)mem2->start, len2);
	ICInvalidateRange((void*)mem2->start, len2);

	void (*entry)(u32,u32);
	entry = (void(*)(u32,u32))addr;
	entry(addr, value);
}
