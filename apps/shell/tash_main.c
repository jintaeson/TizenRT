/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/// @file   tash_main.c
/// @brief  Main functions of TinyAra SHell (TASH)

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#if !defined(CONFIG_DISABLE_POLL)
#include <sys/select.h>
#endif
#include <tinyara/ascii.h>
#include "tash_internal.h"

enum tash_input_state_e {
	IN_VOID,
	IN_WORD,
	IN_QUOTE
};

/* Following defines are fixed to avoid many configuration variables for TASH */
#define TASH_TOKEN_MAX        (32)
#ifdef CONFIG_TASH
#define TASH_TRY_MAXCNT       (5)
#if !defined(CONFIG_DISABLE_POLL)
#define SELECT_TIMEOUT_SECS   (6)
#define SELECT_TIMEOUT_USECS  (0)
#endif
#define TASH_TASK_STACKSIZE   (4096)
#define TASH_TASK_PRIORITY    (125)

#define TASH_PROMPT           "TASH>>"
#endif							/* CONFIG_TASH */

static int tash_running = FALSE;

static void tash_clear_line(int fd, int len)
{
	if (write(fd, (const void *)"\r", sizeof("\r")) <= 0) {
		shdbg("TASH: echo failed (errno = %d)\n", get_errno());
	}

	for (int i = 0; i < len; ++i) {
		if (write(fd, (const void *)" ", sizeof(" ")) <= 0) {
			shdbg("TASH: echo failed (errno = %d)\n", get_errno());
		}
	}
}

static void tash_print_cmd(int fd, char *cmd, int pos)
{
	if (write(fd, (const void *)"\r", sizeof("\r")) <= 0) {
		shdbg("TASH: echo failed (errno = %d)\n", get_errno());
	}

	if (write(fd, (const void *)TASH_PROMPT, sizeof(TASH_PROMPT)) <= 0) {
		shdbg("TASH: echo failed (errno = %d)\n", get_errno());
	}

	if (write(fd, (const void *)cmd, pos) <= 0) {
		shdbg("TASH: echo failed (errno = %d)\n", get_errno());
	}
}

static void tash_remove_char(char *char_pos)
{
	int remaining;

	/* Get a length of string remained */

	remaining = strlen(char_pos + 1);

	/* Add one for NULL */

	remaining++;

	/* remove character from buffer */

	memmove(char_pos, char_pos + 1, remaining);
}

#ifdef CONFIG_TASH
/** @brief Read the input command
 *  @ingroup tash
 */
static char *tash_read_input_line(int fd)
{
	int bufsize = TASH_LINEBUFLEN;
	int pos = 0;
	int nbytes = 0;
	int char_idx = 0;
	bool is_tab_pressed = false;
#if !defined(CONFIG_DISABLE_POLL)
	fd_set tfd;
	struct timeval stimeout;
	stimeout.tv_sec = SELECT_TIMEOUT_SECS;
	stimeout.tv_usec = SELECT_TIMEOUT_USECS;
#endif
	char *buffer = tash_alloc(sizeof(char) * bufsize);

	if (!buffer) {
		shdbg("TASH: input buffer alloc failed\n");
		return NULL;
	}

	memset(buffer, 0x0, bufsize);

	do {
#if !defined(CONFIG_DISABLE_POLL)
		FD_ZERO(&tfd);
		FD_SET(fd, &tfd);

		if ((select(fd + 1, &tfd, NULL, NULL, &stimeout)) && FD_ISSET(fd, &tfd)) {
#endif
			/* read characters */
			nbytes = read(fd, &buffer[pos], (bufsize - pos));
			if (nbytes < 0) {
				shdbg("TASH: can not read uart\n");
				return buffer;
			}

			for (char_idx = 0; char_idx < nbytes; char_idx++) {
				/* treat backspace and delete */
				if ((buffer[pos] == ASCII_BS) || (buffer[pos] == ASCII_DEL)) {
					int valid_char_pos = pos + 1;
					if (pos > 0) {
						pos--;
						/* Update screen */
						if (write(fd, "\b \b", 3) <= 0) {
							shdbg("TASH: echo failed (errno = %d)\n", get_errno());
						}
					}

					if ((buffer[valid_char_pos] != 0x0) && (valid_char_pos < TASH_LINEBUFLEN)) {
						memmove(&buffer[pos], &buffer[valid_char_pos], (bufsize - valid_char_pos));
					}
					is_tab_pressed = false;
				} else if (buffer[pos] == ASCII_TAB) {
					if (pos > 0 && tash_do_autocomplete(buffer, &pos, is_tab_pressed) == true) {
						tash_print_cmd(fd, buffer, pos);
					}
					is_tab_pressed = true;
				}
#if TASH_MAX_STORE   > 0
				else if (buffer[pos] == ASCII_ESC && buffer[pos + 1] == ASCII_LBRACKET) {
					/* ASCII_ESC + ASCII_LBRACKET + ASCII_A is up
					 * ASCII_ESC + ASCII_LBRACKET + ASCII_B is down
					 */

					int len = pos;
					char_idx += 2;

					buffer[pos] = 0;
					if (tash_search_cmd(buffer, &pos, buffer[pos + 2]) == true) {
						tash_clear_line(fd, sizeof(TASH_PROMPT) + len);

						tash_print_cmd(fd, buffer, pos);
					}
					is_tab_pressed = false;
				}
#endif
				else {
					if (buffer[pos] == ASCII_CR) {
						buffer[pos] = ASCII_LF;
					}

					/* echo */
					if (write(fd, &buffer[pos], 1) <= 0) {
						shdbg("TASH: echo failed (errno = %d)\n", get_errno());
					}

					pos++;
					if (pos >= TASH_LINEBUFLEN) {
						printf("\nTASH: length of input character is too long, maximum length is %d\n", TASH_LINEBUFLEN);
						buffer[0] = ASCII_NUL;
						return buffer;
					}
					is_tab_pressed = false;
				}
			}
#if !defined(CONFIG_DISABLE_POLL)
		}
#endif
	} while (buffer[pos - 1] != ASCII_LF);

	buffer[pos - 1] = ASCII_NUL;
	return buffer;
}

static int tash_open_console(void)
{
	int cnt = 0;
	int ret;

	do {
		ret = open("/dev/console", O_RDWR);
		if (ret > 0) {
			break;
		}
#ifndef CONFIG_DISABLE_SIGNALS
		usleep(20);
#endif
		if (cnt == TASH_TRY_MAXCNT) {
			shdbg("TASH: can not open uart, tried (%d) times\n", cnt);
			ret = ERROR;
		}
	} while (cnt++ < TASH_TRY_MAXCNT);

	return ret;
}

/** @brief TASH loop
 *  @ingroup tash
 */
static int tash_main(int argc, char *argv[])
{
	char *line_buff;
	int fd;
	int nbytes;
	int ret = OK;

	ret = tash_init();
	if (ret == ERROR) {
		exit(EXIT_FAILURE);
	}

	fd = tash_open_console();
	if (fd < 0) {
		exit(EXIT_FAILURE);
	}

	tash_running = TRUE;

	do {
		nbytes = write(fd, (const void *)TASH_PROMPT, sizeof(TASH_PROMPT));
		if (nbytes <= 0) {
			shdbg("TASH: prompt is not displayed (errno = %d)\n", get_errno());
#ifndef CONFIG_DISABLE_SIGNALS
			usleep(20);
#endif
			continue;
		}
		line_buff = tash_read_input_line(fd);
		shvdbg("TASH: input string (%s)\n", line_buff);

		tash_execute_cmdline(line_buff);

		tash_free(line_buff);
	} while (tash_running);

	(void)close(fd);
	return 0;					/* TBD: For now, it always returns success */
}

int tash_start(void)
{
	int pid;
	int errcode;

	pid = task_create("tash", TASH_TASK_PRIORITY, TASH_TASK_STACKSIZE, tash_main, (FAR char *const *)NULL);
	if (pid < 0) {
		errcode = errno;
		DEBUGASSERT(errcode > 0);
		return -errcode;
	}

	return pid;
}

void tash_stop(void)
{
	tash_running = FALSE;
}
#endif							/* CONFIG_TASH */

int tash_execute_cmdline(char *buff)
{
	int argc;
	char *argv[TASH_TOKEN_MAX];
	enum tash_input_state_e state;
	bool is_nextcmd = false;
	int ret = OK;

#if TASH_MAX_STORE > 0
	if (buff[0] == '!') {
		int cmd_number = atoi(buff + 1);
		tash_get_cmd_from_history(cmd_number, buff);
	}

	tash_store_cmd(buff);
#endif

	do {
		for (argc = 0, argv[argc] = NULL, is_nextcmd = false, state = IN_VOID; *buff && argc < TASH_TOKEN_MAX - 1 && is_nextcmd == false; buff++) {
			switch (state) {

			case IN_VOID:
				switch (*buff) {

				case ASCII_SPACE:
					/* ignore, do nothing */

					break;

				case ASCII_HASH:
					/* following string is a comment, let's quit parsing */

					is_nextcmd = false;
					*buff = ASCII_NUL;
					*(buff + 1) = ASCII_NUL;
					break;

				case ASCII_QUOTE:
					/* Argument is started with opening double quatation mark */

					state = IN_QUOTE;
					argv[argc++] = buff + 1;
					break;

				case ASCII_LF:
				case ASCII_SEMICOLON:
					/* Command is finished, excute it */

					is_nextcmd = true;
					*buff = ASCII_NUL;
					break;

				default:
					state = IN_WORD;
					argv[argc++] = buff;
					break;
				}
				break;

			case IN_QUOTE:
				if (*buff == '"') {
					if (*(buff - 1) == ASCII_BACKSLASH) {
						/* # character, need to remove a backslash */

						tash_remove_char(buff - 1);
					} else {
						/* closing double quatation mark, need to remove quatation mark */

						tash_remove_char(buff);

						state = IN_WORD;
					}
					buff--;
				}
				break;

			case IN_WORD:
				switch (*buff) {
				case ASCII_SPACE:
					/* end of argument */

					state = IN_VOID;
					*buff = '\0';
					break;

				case ASCII_QUOTE:
					if (*(buff - 1) == ASCII_BACKSLASH) {
						/* # character, need to remove a backslash */

						tash_remove_char(buff - 1);
					} else {
						/* opening double quatation mark, need to remove quatation mark */

						tash_remove_char(buff);
						state = IN_QUOTE;
					}
					buff--;
					break;

				case ASCII_LF:
				case ASCII_SEMICOLON:
					/* Command is finished, excute it */

					is_nextcmd = true;
					*buff = ASCII_NUL;
					break;

				default:
					/* do nothing */

					break;
				}
				break;
			}
		}

		/* unclosed quotation */

		if (state == IN_QUOTE) {
			shdbg("TASH: unclosed double quotation mark\n");
			argc = 0;
		}

		/* make a null at end of argv */

		argv[argc] = NULL;

		/* excute a command if it is valid */

		if (argc > 0) {
			ret = tash_execute_cmd(argv, argc);
		}
	} while (is_nextcmd == true);

	return ret;
}
