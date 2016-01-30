#include "command_protocol.h"
#include "hid_keyboard.h"
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#define __USE_BSD
#define __USE_MISC
#include <termios.h>
typedef int file;
#else
#pragma comment(lib,"setupapi.lib")
#pragma comment(lib,"hid.lib")
#include <windows.h>
typedef HANDLE file;
#endif
#include "client_proto.h"
int valid_map[MAX_ID + 1];
uint16_t id_size[MAX_ID + 1];
uint8_t id_data[MAX_ID + 1][BLK_USER_SIZE];
uint8_t id_mask[MAX_ID + 1][BLK_MASK_SIZE];

int interactive_initialize();
int interactive_login();

void get_random(size_t n, uint8_t *buf)
{
#ifndef _WIN32
	int rnd_fd = open("/dev/random", O_RDONLY);
	read(rnd_fd, buf, n);
	close(rnd_fd);
#endif
}

void get_string(uint8_t *str, int len)
{
	fflush(stdout);
	fgets(str, len, stdin);
	int slen = strlen(str);
	if (str[slen - 1] == '\n')
		str[slen - 1] = 0;
}

int get_number()
{
	char str[128];
	fflush(stdout);
	get_string(str, sizeof(str));
	return atoi(str);
}

int change_master_password()
{
	uint8_t old_passwd[16];
	uint8_t new_passwd[16];
	printf("Enter old password:");
	get_string(old_passwd, sizeof(old_passwd));
	printf("Enter new password:");
	get_string(new_passwd, sizeof(new_passwd));
	int rc = pmdev_change_master_password(old_passwd, new_passwd);
	if (rc != 0) {
		printf("Change password failed\n");
	}
}

int logged_in()
{
	printf("Logged in:\n");
	printf("\n");
	printf("1) List accounts\n");
	printf("2) Type account\n");
	printf("3) Print account\n");
	printf("4) New account\n");
	printf("5) Delete account\n");
	printf("6) logout\n\n");
	printf("Selection:");
	fflush(stdout);
	int c = fgetc(stdin);
	while (fgetc(stdin) != '\n');
	char field[128];
	int i;
	int count;
	switch (c) {
	case '1':
		printf("\nAccount list:\n");
		for (i = MIN_ID; i <= MAX_ID; i++) {
			if (!valid_map[i])
				continue;
			printf("%d)", i);
			int j = 0;
			int k = 0;
			while (id_data[i][k]) {
				int len = id_data[i][k]; k++;
				int private = len & 0x80;
				len &= ~0x80;
				memcpy(field,
					&(id_data[i][k]),
					len);
				field[len] = 0;
				if (private && 0) {
					printf("<private>:", field);
				} else {
					printf("%s:", field);
				}
				k += len;
			}
			printf("\n");
		}
		printf("\n");
		break;
	case '2': {
		printf("Account id:");
		fflush(stdout);
		int id = get_number();

		printf("Fetching data...(press button)...");
		fflush(stdout);
		pmdev_open_id(id);
		int rc = pmdev_read_id(id, id_data[id], id_mask[id]);
		valid_map[id] = (rc >= 0);
		printf("DONE\n");

		int k = 0;
		int i = 0;

		uint8_t keys[128];
		int n_keys = 0;

		while (id_data[id][k] && k < id_size[id]) {
			int len = id_data[id][k]; k++;
			int private = len & 0x80;
			len &= ~0x80;
			memcpy(field,
				&(id_data[id][k]),
				len);
			field[len] = 0;
			if (i == 1 || i == 2) {
				int l;
				for (l = 0; l < len; l++) {
					keys[n_keys++] = id_data[id][k + l];
				}
				uint8_t term;
				if (i == 1) {
					term = '\t';
				} else {
					term = '\n';
				}
				keys[n_keys++] = term;
			}
			k += len;
			i++;
		}
		rc = pmdev_type(keys, n_keys);
		if (rc < 0)
			return -1;
	} break;
	case '3': {
		printf("Account id:");
		fflush(stdout);
		int id = get_number();

		printf("Fetching data...(press button)...");
		fflush(stdout);
		pmdev_open_id(id);
		int rc = pmdev_read_id(id, id_data[id], id_mask[id]);
		valid_map[id] = (rc == 0);
		printf("DONE\n");

		int j = 0;
		int k = 0;
		while (id_data[id][k] &&  k < id_size[id]) {
			int len = id_data[id][k]; k++;
			int private = len & 0x80;
			len &= ~0x80;
			memcpy(field,
				&(id_data[id][k]),
				len);
			field[len] = 0;
			printf("%s:", field);
			k += len;
		}
		printf("\n");
	} break;
	case '4': {
		char account_name[128];
		char user_name[128];
		char password[128];
		fflush(stdin);
		printf("Account name:");
		get_string(account_name, 128);

		printf("User name:");
		get_string(user_name, 128);

		printf("Password:");
		get_string(password, 128);

		int id;
		for(id = MIN_ID; id <= MAX_ID && valid_map[id]; id++);

		printf("Writing to account %d...(press button)...", id);
		fflush(stdout);
		uint8_t msg[BLK_SIZE + 3];
		int k = 0;

		id_data[id][k] = strlen(account_name); k++;
		memcpy(id_data[id] + k, account_name, strlen(account_name));
			k += strlen(account_name);

		id_data[id][k] = strlen(user_name); k++;
		memcpy(id_data[id] + k, user_name, strlen(user_name));
			k += strlen(user_name);

		id_data[id][k] = strlen(password) | 0x80; k++;
		memcpy(id_data[id] + k, password, strlen(password));
		int pwd_begin = k;
		k += strlen(password);
		int pwd_end = k;

		memset(id_mask[id], 0, BLK_MASK_SIZE);
		for (int i = pwd_begin; i < pwd_end; i++) {
			int byte = i / 8;
			int bit = 1<<(i % 8);
			id_mask[id][byte] = (id_mask[id][byte] & ~(bit)) | bit;
		}

		int rem = SUB_BLK_SIZE - (k % SUB_BLK_SIZE);
		if (rem != SUB_BLK_SIZE) {
			uint8_t n = (uint8_t)(rem - 1);
			id_data[id][k] = ((uint8_t)(rem - 1)) | 0x80; k++;
			if (n)
				get_random(n, id_data[id] + k); k += n;
		}
		id_size[id] = k;
		valid_map[id] = 1;

		pmdev_open_id(id);
		pmdev_write_id(id, id_size[id], id_data[id], id_mask[id]);

		printf("DONE\n");
		} break;
	case '5': {
		printf("Account id:");
		fflush(stdout);
		int id = get_number();

		printf("Deleting account %d...(press button)...", id);
		fflush(stdout);
		pmdev_delete_id(id);
		memset(&id_data[id][0], 0, BLK_USER_SIZE);
		valid_map[id] = 0;
		printf("DONE\n");
		} break;
	case '6':
		pmdev_logout();
		break;
	}
	return 0;
}

int logged_out()
{
	printf("Logged out:\n");
	printf("\n");
	printf("1) Login\n");
	printf("2) Initialize\n");
	printf("3) Change master password\n");
	printf("4) Quit\n\n");
	printf("Selection:");
	fflush(stdout);
	int c = fgetc(stdin);
	while (fgetc(stdin) != '\n');
	int rc;
	switch (c) {
	case '1':
		rc = interactive_login();
		break;
	case '2':
		rc = interactive_initialize();
		break;
	case '3':
		rc = change_master_password();
	        break;
	case '4':
		rc = 1;
		break;
	default:
		rc = 1;
		break;
	}
	return rc;
}

int interactive_login()
{
	uint8_t passwd[16];
	printf("Enter password:");
	get_string(passwd, sizeof(passwd));
	int passwd_len = strlen(passwd);
	printf("\n");
	printf("Logging in...");

	int rc = pmdev_login(passwd);
	if (rc == 0) {
		printf("DONE\n");
	} else {
		printf("FAIL\n");
		return 0;
	}

	fflush(stdout);
	printf("Getting public data\n");
	int i;
	int percent = 0;
	for (i = MIN_ID; i <= MAX_ID; i++) {
		int rc = pmdev_read_id(i, id_data[i], id_mask[i]);
		valid_map[i] = (rc >= 0);
		if (((i*100)/MAX_ID) >= percent) {
			printf("%d%% ", percent);
			fflush(stdout);
			percent = ((i*100)/MAX_ID) + 1;
		}
	}
	printf("\n");
	return 0;
}

int initialize_percent = 0;
int initialize_progress_max = 0;

int interactive_initialize()
{
	int count;
	int rc;
	uint8_t passwd[16];
	printf("Enter password:");

	uint8_t rand_data[AES_BLK_SIZE * 3];
	get_random(sizeof(rand_data), rand_data);

	get_string(passwd, sizeof(passwd));
	int passwd_len = strlen(passwd);
	printf("\n");

	initialize_percent = 0;
	initialize_progress_max = 0;

	printf("Initializing...(press button)...");
	fflush(stdout);

	rc = pmdev_initialize_device(passwd, rand_data);
	if (rc < 0)
		printf("FAIL\n");
	else
		printf("DONE\n");
	return 0;
}

int main(int argc, char **argv)
{
	pmdev_initialize_api();

	int state;

	if (argc < 2)
	{
		printf("Arg: Connection name required...\n");
		return -1;
	}

	printf("Starting up...\n");

	int rc = pmdev_open_connection(argv[1]);
	if (rc) {
		fprintf(stderr, "Failed to open connection to device\n");
		exit(-1);
	}

#ifdef USE_RAW_HID
	rc = pmdev_disconnect();
	rc = pmdev_connect();
#endif
	rc = pmdev_startup(&state);
	if (rc < 0) {
		fprintf(stderr, "Startup failed\n");
		exit(-1);
	}
	printf("started up\n");
	while (1) {
		int rc;
		switch(state) {
		case UNINITIALIZED:
			printf("Device not initialized. Initializing now.\n\n");
			rc = interactive_initialize();
			break;
		case INITIALIZING: {
			int progress = (initialize_progress_max * initialize_percent) / 100;
			int x,y;
			int rc = pmdev_get_progress(progress, INITIALIZING, &x, &y);
			if (rc < 0)
				exit(-1);
			initialize_progress_max = y - 1;
			printf("%d%% ", initialize_percent);
			if (x >= y) printf("\n");
			fflush(stdout);
			initialize_percent += 5;
		} break;
		case LOGGED_OUT:
			rc = logged_out();
			break;
		case LOGGED_IN:
			rc = logged_in();
			break;
		default:
			rc = 1;
			break;
		}
		if (rc)
			break;
		pmdev_get_device_state(&state);
	}
}
