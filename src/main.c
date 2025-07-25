#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char* argv[]) {
	printf("Usage: %s -n -f <database file>\n", argv[0]);
	printf("\t -n  -  create new database file\n");
	printf("\t -f  -  (required) path to database file\n");
	printf("\t -d  -  delete an entry from the database <employee name>\n");
	printf("\t -u  -  update an entry in the database <employee name>\n");
	printf("\t\t -N <updated name>\n");
	printf("\t\t -H <updated hours>\n");
	printf("\t\t -A <updated address>\n");

}

int main(int argc, char *argv[]) {
	char* filepath = NULL;
	bool newfile = false;
	bool list = false;
	char* add_string = NULL;
	char* del_query_string = NULL;
	char* update_query_string = NULL;
	char* change_name_string = NULL;
	char* change_addr_string = NULL;
	char* change_hours_string = NULL;
	int c;

	int dbfd = -1;
	struct dbheader_t* dbhdr = NULL;
	struct employee_t* employees = NULL;

	while ((c = getopt(argc, argv, "nf:a:ld:u:N:H:A:")) != -1) {
		switch (c) {
			case 'n':
				newfile = true;
				break;
			case 'f':
				filepath = optarg;
				break;
			case 'l':
				list = true;
				break;
			case 'a':
				add_string = optarg;
				break;
			case 'd':
				del_query_string = optarg;
				break;
			case 'u':
				update_query_string = optarg;
				break;
			case 'N':
				change_name_string = optarg;
				break;
			case 'H':
				change_hours_string = optarg;
				break;
			case 'A':
				change_addr_string = optarg;
				break;
			case '?':
				printf("Unkown option: -%c\n", c);
			default:
				return STATUS_ERROR;
		}
	}

	if (filepath == NULL) {
		printf("Filepath is a required argument.\n");
		print_usage(argv);
		return STATUS_SUCCESS;
	}

	if (newfile) {
		dbfd = create_db_file(filepath);
		if (dbfd == STATUS_ERROR) {
			printf("Unable to create database file.\n");
			return STATUS_ERROR;
		}

		if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
			printf("Failed to create database header.\n");
			return STATUS_ERROR;
		}
	} else {
		dbfd = open_db_file(filepath);
		if (dbfd == STATUS_ERROR) {
			printf("Unable to open database file.\n");
			return STATUS_ERROR;
		}	

		if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
			printf("Database header is invalid.\n");
			return STATUS_ERROR;
		}
	}	

	if (read_employees(dbfd, dbhdr, &employees) != STATUS_SUCCESS) {
		printf("Failed to read employee database.\n");
		return STATUS_ERROR;
	}

	if (add_string) {
		dbhdr->count++;
		employees = realloc(employees, dbhdr->count * sizeof(struct employee_t));

		add_employee(dbhdr, employees, add_string);
	}

	if (del_query_string) {
		if (delete_employee(dbhdr, employees, del_query_string) == STATUS_SUCCESS) {
			printf("%s removed.\n", del_query_string);
		} else {
			printf("%s not found.\n", del_query_string);
		}
		
	}

	if (update_query_string) {
		if (!change_name_string && !change_hours_string && !change_addr_string) {
			printf("(N)ame, (A)ddress, or (H)ours argument required with -u flag.\n");
			printf("Usage: %s -f <database file> -u <employee name> -N <updated employee name> -H <updated hours> -A <updated address>\n", argv[0]);
			printf("-N, -H, -A may be used all together or with only one or two.\n");
			return STATUS_ERROR;
		}
		char* update_strings[] = {change_name_string, change_hours_string, change_addr_string};
		if (update_employee(dbhdr, employees, update_query_string, update_strings) == STATUS_ERROR) {
			printf("%s Not found\n", update_query_string);
		}
	}

	if (list) {
		list_employees(dbhdr, employees);
	}

	output_file(filepath, dbhdr, employees);

	return STATUS_SUCCESS;
}