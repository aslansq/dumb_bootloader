#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../bl.h"

#define BIN_FILE "app.elf.bin"
#define SIGNED_BIN_FILE "app.elf.signed.bin"

extern uint32_t Crc_CalculateCRC32(const uint8_t *s, uint32_t len, uint32_t startVal);

uint64_t get_bin_size() {
	uint64_t file_size;
	FILE *file_ptr;
	file_ptr = fopen(BIN_FILE, "rb");
	if (file_ptr == NULL) {
		perror("Failed to open file\n");
		exit(1);
	}
	fseek(file_ptr, 0, SEEK_END);
	file_size = ftell(file_ptr);
	fclose(file_ptr);

	if(file_size % sizeof(uint32_t)) {
		perror("binary size should be divisible 4\n");
		exit(1);
	}

	return file_size;
}

uint8_t *get_bin(uint64_t bin_size) {
	FILE *file_ptr;
	uint64_t bin_read;

	file_ptr = fopen(BIN_FILE, "rb");

	if (file_ptr == NULL) {
		perror("Failed to open file\n");
		exit(1);
	}

	uint8_t *buf_ptr = (uint8_t *)malloc(bin_size);

	if (buf_ptr == NULL) {
		perror("Failed to allocate memory\n");
		exit(1);
	}

	bin_read = fread(buf_ptr, 1, bin_size, file_ptr);

	if (bin_read != bin_size) {
		perror("Failed to read file\n");
		free(buf_ptr);
		exit(1);
	}
	return buf_ptr;
}

void get_metadata(
	uint8_t *bin_ptr,
	uint64_t bin_size,
	app_metadata_s *app_metadata_ptr
) {
	uint32_t crc32;

	memcpy(
		(void *)app_metadata_ptr,
		(void *)bin_ptr,
		sizeof(app_metadata_s)
	);

	app_metadata_ptr->size = bin_size - APP_METADATA_SIZE;
	app_metadata_ptr->reserved = 0;
	app_metadata_ptr->epoch_time = (uint32_t)time(NULL);
	app_metadata_ptr->reserved1 = 0;

	crc32 = Crc_CalculateCRC32(
		(uint8_t *)app_metadata_ptr,
		sizeof(app_metadata_s) - sizeof(app_metadata_ptr->crc),
		MAGIC_BL_FLAG
	);

	app_metadata_ptr->crc  = crc32;
}

uint32_t get_app_crc(
	uint8_t *bin_ptr,
	uint64_t bin_size,
	app_metadata_s *app_metadata_ptr
) {
	uint32_t crc32;

	crc32 = Crc_CalculateCRC32(
		(uint8_t *)&bin_ptr[APP_METADATA_SIZE],
		app_metadata_ptr->size,
		MAGIC_BL_FLAG
	);

	return crc32;
}

void write_signed_bin(
	app_metadata_s *app_metadata_ptr,
	uint8_t *bin_ptr,
	uint64_t bin_size,
	uint32_t crc
) {
	FILE *file_ptr;
	file_ptr = fopen(SIGNED_BIN_FILE, "wb");
	if (file_ptr == NULL) {
		perror("Failed to open consist file\n");
		exit(1);
	}

	printf("App Metadata Crc: 0x%x\n", app_metadata_ptr->crc);
	printf("App Metadata Size: %d\n", APP_METADATA_SIZE);
	printf("App Size: %ld\n", bin_size - APP_METADATA_SIZE);
	printf("App Crc: 0x%x\n", crc);

	fwrite(bin_ptr, sizeof(uint8_t), bin_size, file_ptr);
	fwrite(&crc, sizeof(uint8_t), sizeof(uint32_t), file_ptr);

	fclose(file_ptr);
}

int main(void) {
	uint64_t bin_size;
	uint8_t *bin_ptr;
	app_metadata_s app_metadata;
	uint32_t app_crc;
	
	bin_size = get_bin_size();
	bin_ptr = get_bin(bin_size);

	get_metadata(
		bin_ptr,
		bin_size,
		&app_metadata
	);

	app_crc = get_app_crc(
		bin_ptr,
		bin_size,
		&app_metadata
	);

	memcpy(
		(void *)bin_ptr,
		(void *)&app_metadata,
		sizeof(app_metadata_s)
	);

	write_signed_bin(
		&app_metadata,
		bin_ptr,
		bin_size,
		app_crc
	);

	free(bin_ptr);

	return 0;
}