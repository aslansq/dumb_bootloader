#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	struct timespec start_ts;
	int serial_port;
	const char *serial_port_name;
	const char *bin_file_name;
} main_handle_s;

main_handle_s main_handle = {
	.serial_port_name = "/dev/ttyUSB0",
	.bin_file_name = "app.elf.signed.bin"
};

uint32_t time_get_ms(main_handle_s *handle_ptr)
{
	struct timespec ts;
	uint64_t ns = 0;
	uint32_t ms = 0;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	if(ts.tv_nsec > handle_ptr->start_ts.tv_nsec) {
		ns = 1000000000 + ts.tv_nsec - handle_ptr->start_ts.tv_nsec;
		ts.tv_sec -= 1;
	}

	ms = ((ts.tv_sec - handle_ptr->start_ts.tv_sec) * 1000) + (ns / 1000000);
	return ms;
}

void serial_port_attr_init(main_handle_s *handle_ptr) {
	// Create new termios struct, we call it 'tty' for convention
	struct termios tty;

	// Read in existing settings, and handle any error
	if(tcgetattr(handle_ptr->serial_port, &tty) != 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
		exit(1);
	}

	tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
	tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
	tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
	tty.c_cflag |= CS8; // 8 bits per byte (most common)
	tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO; // Disable echo
	tty.c_lflag &= ~ECHOE; // Disable erasure
	tty.c_lflag &= ~ECHONL; // Disable new-line echo
	tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
	// tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
	// tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

	tty.c_cc[VTIME] = 0; // this should make it nonblock
	tty.c_cc[VMIN] = 0;

	// Set in/out baud rate to be 9600
	cfsetispeed(&tty, B9600);
	cfsetospeed(&tty, B9600);

	// Save tty settings, also checking for error
	if (tcsetattr(handle_ptr->serial_port, TCSANOW, &tty) != 0) {
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
		exit(1);
	}
}

// read until buffer full or timeout occured
ssize_t serial_port_read(
	main_handle_s *handle_ptr,
	uint8_t *buf_ptr,
	size_t buf_size,
	uint32_t ms_timeout
) {
	uint32_t start_time = time_get_ms(handle_ptr);
	uint32_t curr_time;
	bool is_to = false;
	ssize_t idx = 0;
	ssize_t num_of_read = 0;
	uint8_t u8;

	while(
		is_to == false &&
		idx < buf_size
	) {
		num_of_read = read(
			handle_ptr->serial_port,
			&u8,
			sizeof(uint8_t)
		);

		if(num_of_read == sizeof(uint8_t)) {
			buf_ptr[idx] = u8;
			idx++;
		}

		curr_time = time_get_ms(handle_ptr);
		is_to = (curr_time - start_time) > ms_timeout ? true : false;
	}

	return idx;
}

ssize_t serial_port_write(
	main_handle_s *handle_ptr,
	const uint8_t *buf_ptr,
	size_t buf_size
) {
	return write(
		handle_ptr->serial_port,
		buf_ptr,
		buf_size
	);
}

uint64_t get_bin_size(main_handle_s *handle_ptr) {
	uint64_t file_size;
	FILE *file_ptr;
	file_ptr = fopen(handle_ptr->bin_file_name, "rb");
	if (file_ptr == NULL) {
		perror("Failed to open file");
		exit(1);
	}
	fseek(file_ptr, 0, SEEK_END);
	file_size = ftell(file_ptr);
	fclose(file_ptr);
	return file_size;
}

uint8_t *get_bin(main_handle_s *handle_ptr, uint64_t bin_size) {
	FILE *file_ptr;
	uint64_t bin_read;

	file_ptr = fopen(handle_ptr->bin_file_name, "rb");

	if (file_ptr == NULL) {
		perror("Failed to open file");
		exit(1);
	}

	uint8_t *buf_ptr = (uint8_t *)malloc(bin_size);

	if (buf_ptr == NULL) {
		perror("Failed to allocate memory");
		exit(1);
	}

	bin_read = fread(buf_ptr, 1, bin_size, file_ptr);

	if (bin_read != bin_size) {
		perror("Failed to read file");
		free(buf_ptr);
		exit(1);
	}
	return buf_ptr;
}

void main_init(main_handle_s *handle_ptr) {
	clock_gettime(CLOCK_MONOTONIC, &handle_ptr->start_ts);

	handle_ptr->serial_port = open(handle_ptr->serial_port_name, O_RDWR);
	if (handle_ptr->serial_port < 0) {
		perror("Error opening serial port");
	}

	handle_ptr->serial_port = open(handle_ptr->serial_port_name, O_RDWR);

	serial_port_attr_init(handle_ptr);

}

void main_deinit(main_handle_s *handle_ptr) {
	close(handle_ptr->serial_port);
}

int main(void) {
	main_init(&main_handle);
	uint32_t i;
	uint64_t bin_size;
	uint8_t *bin_ptr;
	uint8_t rx;
	ssize_t num_of_read;

	bin_size = get_bin_size(&main_handle);
	bin_ptr = get_bin(&main_handle, bin_size);

	if(bin_ptr == NULL) {
		printf("binary read fail\n");
		exit(1);
	}

	if(bin_size % sizeof(uint32_t)) {
		printf("binary size should be divisable by 4");
		exit(1);
	}

	for(i = 0; i < bin_size; ++i) {
		serial_port_write(&main_handle, &bin_ptr[i], sizeof(uint8_t));
		num_of_read = serial_port_read(
			&main_handle,
			&rx,
			sizeof(uint8_t),
			2000
		);

		if(num_of_read != sizeof(uint8_t)) {
			printf("comm err\n");
			break;
		} else {
			printf("%02x\n", rx);
		}
	}

	printf("\n");

	free(bin_ptr);
	main_deinit(&main_handle);
	return 0;
}