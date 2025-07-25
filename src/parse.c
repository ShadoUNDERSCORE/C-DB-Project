#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

#include "common.h"
#include "parse.h"

void output_file(char* filepath, struct dbheader_t* dbhdr, struct employee_t* employees) {
	int fd = open(filepath, O_RDWR | O_TRUNC);
	int realcount = dbhdr->count;

	dbhdr->magic = htonl(dbhdr->magic);
	dbhdr->filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount));
	dbhdr->version = htons(dbhdr->version);	
	dbhdr->count = htons(dbhdr->count);

	lseek(fd, 0, SEEK_SET);

	write(fd, dbhdr, sizeof(struct dbheader_t));

	int i = 0;
	for (; i < realcount; i++) {
		employees[i].hours = htonl(employees[i].hours);
		write(fd, &employees[i], sizeof(struct employee_t));
	}

	return;
}

int create_db_header(int fd, struct dbheader_t** headerOut) {
	struct dbheader_t* header = calloc(1, sizeof(struct dbheader_t));
	if (header == -1) {
		printf("Calloc failed to create db header.\n");
		return STATUS_ERROR;
	}
	header->version = 0x1;
	header->count = 0;
	header->magic = HEADER_MAGIC;
	header->filesize = sizeof(struct dbheader_t);

	*headerOut = header;
	return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t** headerOut) {
	if (fd < 0) {
		printf("Got a bad File Descriptor from the user.\n");
		return STATUS_ERROR;
	}

	struct dbheader_t* header = calloc(1, sizeof(struct dbheader_t));
	if (header == -1) {
		printf("Calloc failed to create db header.\n");
		return STATUS_ERROR;
	}

	if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
		perror("read");
		free(header);
		return STATUS_ERROR;
	}

	header->version = ntohs(header->version);
	header->count = ntohs(header->count);
	header->magic = ntohl(header->magic);
	header->filesize = ntohl(header->filesize);

	if (header->magic != HEADER_MAGIC) {
		printf("Improper header magic.\n");
		free(header);
		return STATUS_ERROR;
	}

	if (header->version != 1) {
		printf("Improper header version.\n");
		free(header);
		return STATUS_ERROR;
	}

	struct stat dbstat = {0};
	fstat(fd, &dbstat);
	if (header->filesize != dbstat.st_size){
		printf("Corrupted database.\n");
		free(header);
		return STATUS_ERROR;
	}

	*headerOut = header;
}

int read_employees(int fd, struct dbheader_t * dbhdr, struct employee_t **employeesOut) {
	if (fd < 0) {
		printf("Got a bad File Descriptor from the user.\n");
		return STATUS_ERROR;
	}

	int count = dbhdr->count;

	struct employee_t* employees = calloc(count, sizeof(struct employee_t));
	if (employees == -1) {
		printf("Calloc Failed.\n");
		return STATUS_ERROR;
	}

	read(fd, employees, count * sizeof(struct employee_t));

	int i = 0;
	for (; i < count; i++) {
		employees[i].hours = ntohl(employees[i].hours);
	}

	*employeesOut = employees;
	return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t* dbhdr, struct employee_t* employees, char* addstring) {

	char* name = strtok(addstring, ",");
	char* addr = strtok(NULL, ",");
	char* hours = strtok(NULL, ",");

	strncpy(employees[dbhdr->count-1].name, name, sizeof(employees[dbhdr->count-1].name));
	strncpy(employees[dbhdr->count-1].address, addr, sizeof(employees[dbhdr->count-1].address));

	employees[dbhdr->count-1].hours = atoi(hours);

	return STATUS_SUCCESS;
}

void list_employees(struct dbheader_t* dbhdr, struct employee_t* employees) {
	int i = 0;
	for (; i < dbhdr->count; i++) {
		printf("Employee %d\n", i);
		printf("\tName: %s\n", employees[i].name);
		printf("\tAddress: %s\n", employees[i].address);
		printf("\tHours Worked: %d\n", employees[i].hours);
	}
}

int find_employee(struct dbheader_t* dbhdr, struct employee_t* employees, char* query) {
	int i = 0;
	for (; i < dbhdr->count; i++) {
		if (strcmp(employees[i].name, query) == 0) {
			return i;
		}
	}
	return -1;
}

int delete_employee(struct dbheader_t* dbhdr, struct employee_t* employees, char* delQueryString) {
	int id = find_employee(dbhdr, employees, delQueryString);
	if (id == -1) {
		return STATUS_ERROR;
	}

	for (; id < dbhdr->count; id++) {
		employees[id] = employees[id + 1];
	}
	dbhdr->count--;

	return STATUS_SUCCESS;
}

int update_employee(struct dbheader_t* dbhdr, struct employee_t* employees, char* queryString, char* updateStrings[]) {
	int id = find_employee(dbhdr, employees, queryString);
	if (id == -1) {
		return STATUS_ERROR;
	}

	if (updateStrings[0]) {
		// name	
		strcpy(employees[id].name, updateStrings[0]);
	}
	if (updateStrings[1]) {
		// hours
		unsigned int hours_int = atoi(updateStrings[1]);
		employees[id].hours = hours_int;
	}
	if (updateStrings[2]) {
		// address
		strcpy(employees[id].address, updateStrings[2]);
	}
	return STATUS_SUCCESS;
}
