/*
 * iv - embedded editor
 * (C) 2001-2010 Christopher Cole, Cole Design and Development, LLC
 * cole@coledd.com
 *
 * Modified by Raman Gopalan <ramangopalan@gmail.com> Dec, 2012
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "platform_conf.h"

#ifdef BUILD_EDITOR_IV

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include "term.h"
#include "iv.h"
#include "ioctl.h"
#include <setjmp.h>

#ifdef DEBUG
#include <sys/time.h>
FILE *fp_debug;
struct timeval tv_new;
struct timeval tv_old;
#endif

struct alcor_iv_winsize win_size;
int pos_x_line = 0;
int pos_y = 0;
long current_line = 0;
char *line_buffer = NULL;
long line_buffer_end = 0;
long line_buffer_size = 0;
char *filename = NULL;
int file_exists = 0;
int stdin_saved;
char editor_mode = EDITOR_MODE_CMD;
char cursor_mode = 0;
char search_string[MAX_SEARCH] = {0};
char cmd_queue[MAX_CMD_QUEUE] = {CMD_NONE};
int cmd_queue_head = 0;
int cmd_queue_tail = 0;
int env_tabstop = TABSTOP;
extern int errno;

struct ll_struct file_buffer_begin;

static void iv_reset_globals(void);
static void get_winsize(void);
static void redraw_screen(struct ll_struct *file_buffer);
static void sig_winch(int signo);

static jmp_buf ebuf;

#if DEBUG > 1
#define DEBUG_CALL \
  tv_old = tv_new; \
  gettimeofday(&tv_new, 0); \
  fprintf(fp_debug, "%lu:\t", tv_new.tv_usec - tv_old.tv_usec); \
  fprintf(fp_debug, "DEBUG: %s()\n", __FUNCTION__);
#else
#define DEBUG_CALL
#endif

int iv_main(int argc, char **argv)
{
	struct ll_struct *file_buffer = &file_buffer_begin;
	char input;
	char *cmd_line = NULL;
	int cmd_line_index = 0;
	int exit_program = 0;
	int file_flags = 0;

	int f = setjmp(ebuf);
	
	if (f == 0) {
	if (argc > 1) {
		// keep it simple
		if (argv[1][0] == '-') {
			printf("iv version %s\n%s\n%s\n", VERSION, INFO, AUTHOR);
			return 0;
		}
	}
#ifdef DEBUG
	fp_debug = fopen("/mmc/.debug", "wa");
	if (NULL == fp_debug)
		show_error(ERROR_TYPE_FILE, "Cannot open debug output");
#endif
	line_buffer = MALLOC((LINE_BUFFER_CHUNK_SIZE)*sizeof(char));
	if (NULL == line_buffer)
		show_error(ERROR_TYPE_MEMORY, "line buffer");
	line_buffer_size += LINE_BUFFER_CHUNK_SIZE;
	cmd_line = MALLOC((MAX_CMD_LINE_SIZE)*sizeof(char));
	if (NULL == cmd_line)
		show_error(ERROR_TYPE_MEMORY, "cmd line");
	if (NULL == argv[1]) {
		filename = MALLOC(strlen(TEMP_FILE)*sizeof(char) + 1);
		if (NULL == filename)
			show_error(ERROR_TYPE_MEMORY, "temp filename");
		strcpy(filename, TEMP_FILE);
		if (read_file(file_buffer) == 0) {
		        file_flags |= FILE_FLAGS_NEW;
		        // ll_insert(file_buffer, 0, NULL, 0);
		}
	} else {
		filename = MALLOC(strlen(argv[1])*sizeof(char) + 1);
		if (NULL == filename)
			show_error(ERROR_TYPE_MEMORY, "filename");
		strcpy(filename, argv[1]);
		read_file(file_buffer);
	}
	current_line = 0;
	get_winsize();
	if (signal(SIGWINCH, sig_winch) == SIG_ERR)
		show_error(ERROR_TYPE_UNKNOWN, "Signal error");
	redraw_screen(file_buffer);
	printf("%c[%d;%dr", ESC, 0, win_size.ws_row - 1);
	printf("%c[%d;%dH", ESC, win_size.ws_row, 0);
	printf("\"%s\"", filename);
	if (!file_exists)
		printf(" [new file]");
	// else if (file_flags & FILE_FLAGS_READONLY)
	//	printf(" [read only]");
	printf(" %ldL, %ldC%c[H", ll_count(file_buffer), ll_size(file_buffer), ESC);
	fflush(0);
	// update contents of line buffer
	fill_line_buffer(file_buffer, current_line);
	while (!(exit_program |= process_command(file_buffer))) {
		process_output(file_buffer, cmd_line);
		while ((input = term_getch_nt(TERM_INPUT_WAIT))) {
			exit_program |= process_command(file_buffer); // handle pending activity
			if (input == ESC)
				break;
			break;
		}
		cmd_line[cmd_line_index++] = (char)input;
		if (cmd_line_index > MAX_CMD_LINE_SIZE)
			clear_cmd_line(cmd_line, &cmd_line_index);
		if (ESC == input) {
			clear_cmd_line(cmd_line, &cmd_line_index);
			cmd_line[cmd_line_index++] = input;
			while ((cmd_line[cmd_line_index] = term_getch_nt(TERM_INPUT_DONT_WAIT))) {
				cmd_line_index++;
				if (cmd_line_index > MAX_CMD_LINE_SIZE)
					clear_cmd_line(cmd_line, &cmd_line_index);
				break;
			}
		}
		if (editor_mode & EDITOR_MODE_CMD)
			parse_input_cmds(file_buffer, cmd_line, &cmd_line_index);
		else
			do_insert(file_buffer, cmd_line, &cmd_line_index);
	}

	printf("%c[2J%c[H", ESC, ESC);

	ll_free(file_buffer);
	FREE(filename);
	FREE(cmd_line);
	FREE(line_buffer);
#ifdef DEBUG
	fclose(fp_debug);
#endif

	/*
	 * raman: reset globals before exit.
	 */
	iv_reset_globals();

	restore_input_mode();

	} /* longjmp */
	return 0;
}

/*
 * cmd_add() - appends a new command to the end of the current command queue.
 */
int cmd_add(int cmd)
{
	DEBUG_CALL
	if (CMD_NONE != cmd)
		cmd_queue[cmd_queue_head++] = cmd;
	if (cmd_queue_head >= MAX_CMD_QUEUE) {
		cmd_queue_head = 0;
#ifdef DEBUG
		fprintf(fp_debug, "ERROR: %s()[%d]: cmd_queue grew too large!\n", __FUNCTION__, __LINE__);
#endif
	}
	return 0;
}

/*
 * cmd_get() - returns next command from the queue.
 */
int cmd_get(void)
{
	int cmd = CMD_NONE;

	DEBUG_CALL
	if (cmd_queue_head != cmd_queue_tail)
		cmd = cmd_queue[cmd_queue_tail++];
	if (cmd_queue_tail >= MAX_CMD_QUEUE)
		cmd_queue_tail = 0;
	return cmd;
}

/*
 * clear_cmd_line() - erases current input line, possibly because a command has been recognized.
 */
void clear_cmd_line(char *cmd_line, int *cmd_line_index)
{
	DEBUG_CALL
	bzero(cmd_line, MAX_CMD_LINE_SIZE);
	*cmd_line_index = 0;
}

/*
 * parse_input_cmds() - gets terminal input for command mode
 * TODO: move remaining command execution routines in here out to process_cmd function.
 */
void parse_input_cmds(struct ll_struct *file_buffer, char *cmd_line, int *cmd_line_index)
{
	DEBUG_CALL
	switch (cmd_line[0]) {
	case ESC:
		cmd_add(parse_esc_cmd(cmd_line));
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 0x0c:
		// repaint screen (^L)
		cmd_add(CMD_REDRAW);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 'h':
	case BKSP:
	case 0x44:
		cmd_add(CMD_CURSOR_LEFT);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 'l':
	case 0x43:
		cmd_add(CMD_CURSOR_RIGHT);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 'j':
	case 0x42:
		cmd_add(CMD_CURSOR_DOWN);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 'k':
	case 0x41:
		cmd_add(CMD_CURSOR_UP);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 0x05:
		// scroll up
		if (current_line < ll_count(file_buffer) - 1)
			current_line++;
		else if (pos_y > 0)
			pos_y--;
		cmd_add(CMD_REDRAW);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 0x19:
		// scroll down
		if ((current_line - pos_y) > 0)
			current_line--;
		cmd_add(CMD_REDRAW);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 'r':
		editor_mode |= EDITOR_MODE_REPLACE;
		cmd_add(CMD_DEL_CHAR);
		cmd_add(CMD_INSERT_MODE);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 'x':
		cmd_add(CMD_DEL_CHAR);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 'a':
		cmd_add(CMD_APPEND_AFTER_CURSOR);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 's':
		cmd_add(CMD_DEL_CHAR);
		if (pos_x_line == (ll_get_data_size(file_buffer, current_line) - 1)) {
			cmd_add(CMD_APPEND_AFTER_CURSOR);
			clear_cmd_line(cmd_line, cmd_line_index);
			break;
		}
	case 'i': // insert
	case 'I': // append at end of current line
		if ('I' == cmd_line[0]) {
			pos_x_line = ll_get_data_size(file_buffer, current_line);
			if (pos_x_line < 0)
				pos_x_line = 0;
		}
		cmd_add(CMD_INSERT_MODE);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 'o':
	case 'O':
		// open new line
		if ('o' == cmd_line[0]) {
			current_line++;
			pos_y++;
		}
		line_buffer_end = 0;
		ll_insert(file_buffer, current_line, line_buffer, line_buffer_end);
		pos_x_line = 0;
		cmd_add(CMD_INSERT_MODE);
		cmd_add(CMD_REDRAW);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case 0x0d:
		cmd_add(CMD_CURSOR_DOWN);
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	case ':':
		switch (cmd_line[1]) {
		case 'q':
			switch (cmd_line[2]) {
			case 0x0d:
			case '!':
				cmd_add(CMD_EXIT);
				clear_cmd_line(cmd_line, cmd_line_index);
				break;
			case 0x0:
				break;
			default:
				clear_cmd_line(cmd_line, cmd_line_index);
			}
			break;
		case 'w':
			switch (cmd_line[2]) {
			// for :w /mmc/foofile
			// TODO: Saves the file in /mmc but :wq/:q
			// crashes Alcor.
                        case ' ':
				if (0x0d == cmd_line[*cmd_line_index - 1]) {
			  		cmd_line[*cmd_line_index - 1] = '\0';
			    		if (*cmd_line_index > 2) {
			      			free(filename);
			      			filename = MALLOC(strlen(cmd_line) + 1);
			      			strcpy(filename, cmd_line + 1);
			      			filename = filename + 2;
			    		}
			    		printf("%c[%d;%dH%c[K\"%s\"", ESC, win_size.ws_row, 0,\
					ESC, (write_file(file_buffer) == 0 ? filename: ""));
			    		printf(" [new file]");
			    		printf(" written");
			    		clear_cmd_line(cmd_line, cmd_line_index);
			  	}
			 	break;
			case 'q':
				switch (cmd_line[3])
				{
				case 0x0d:
					cmd_add(CMD_WRITE);
					cmd_add(CMD_EXIT);
					clear_cmd_line(cmd_line, cmd_line_index);
					break;
				case 0x0:
					break;
				default:
					clear_cmd_line(cmd_line, cmd_line_index);
					break;
				}
				break;
			case 0x0d:
				cmd_add(CMD_WRITE);
				clear_cmd_line(cmd_line, cmd_line_index);
				break;
			case 0x0:
				break;
			default:
				clear_cmd_line(cmd_line, cmd_line_index);
			}
			break;
		case 0x0:
			break;
		default:
			clear_cmd_line(cmd_line, cmd_line_index);
			break;
		}
		break;
	case 'Z':
		switch (cmd_line[1]) {
		case 'Z':
			cmd_add(CMD_WRITE);
			cmd_add(CMD_EXIT);
			clear_cmd_line(cmd_line, cmd_line_index);
			break;
		case 0x0:
			break;
		default:
			clear_cmd_line(cmd_line, cmd_line_index);
			break;
		}
		break;
	case 'd':
		switch (cmd_line[1]) {
		case 'd':
			cmd_add(CMD_DELETE_LINE);
			clear_cmd_line(cmd_line, cmd_line_index);
			break;
		case 0x0:
			break;
		default:
			clear_cmd_line(cmd_line, cmd_line_index);
			break;
		}
		break;
	case '/':
		if (0x0d == cmd_line[*cmd_line_index - 1]) {
			cmd_line[*cmd_line_index - 1] = '\0';
			if (*cmd_line_index > 2)
				strcpy(search_string, cmd_line + 1);
			printf("%c[%d;%dH%c[K%s", ESC, win_size.ws_row, 0, ESC, (strlen(search_string) > 0)?((do_search_string(file_buffer, search_string) < 0)?"pattern not found":""):"null search pattern");
			clear_cmd_line(cmd_line, cmd_line_index);
		}
		break;
	case 'n':
		printf("%c[%d;%dH%c[K%s", ESC, win_size.ws_row, 0, ESC, (strlen(search_string) > 0)?((do_search_string(file_buffer, search_string) < 0)?"pattern not found":""):"null search pattern");
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	default:
		clear_cmd_line(cmd_line, cmd_line_index);
		break;
	}
}

/*
 * do_search_string() - locates a substring henceforth in the file.
 * returns: line number containing the match.
 */
int do_search_string(struct ll_struct *file_buffer, char *string)
{
	long index = current_line;
	long num_lines;
	long length = 0;
	char *line = NULL;
	char *match = NULL;
	int retval = -1;
	
	DEBUG_CALL
	num_lines = ll_count(file_buffer);
	while ((NULL == match) && (index < num_lines)) {
		line = ll_get(file_buffer, index, &length);
		if (index == current_line)
			match = strstr(line + pos_x_line + 1, string);
		else
			match = strstr(line, string);
		index++;
	}
	if (NULL != match) {
		current_line = index - 1;
		pos_x_line = match - line;
		cmd_add(CMD_REDRAW);
		retval = index - 1;
		cursor_mode &= ~CURSOR_MODE_END;
	}
	return retval;
}

/*
 * parse_esc_cmd() - evaluates escape sequence passed in cmd_line in search of commands.
 * returns: command.
 */
int parse_esc_cmd(char *cmd_line)
{
	int cmd = CMD_NONE;

	DEBUG_CALL
	switch (cmd_line[1]) {
	case 0x0:
		// escape key was hit
		printf("%c[%d;%dH%c[K", ESC, win_size.ws_row, 0, ESC);
		break;
	case '[':
		// escape sequence initiated
		switch (cmd_line[2]) {
			case 0x31:
				cmd = CMD_CURSOR_HOME;
				break;
			case 0x33:
				cmd = CMD_DEL_CHAR;
				break;
			case 0x34:
				cmd = CMD_CURSOR_END;
				break;
			case 0x35:
				cmd = CMD_CURSOR_PGUP;
				break;
			case 0x36:
				cmd = CMD_CURSOR_PGDN;
				break;
			case 0x41:
				cmd = CMD_CURSOR_UP;
				break;
			case 0x42:
				cmd = CMD_CURSOR_DOWN;
				break;
			case 0x43:
				cmd = CMD_CURSOR_RIGHT;
				break;
			case 0x44:
				cmd = CMD_CURSOR_LEFT;
				break;
			default:
				break;
		}
		break;
	case 'O':
		// escape sequence initiated
		switch (cmd_line[2]) {
			case 0x46:
				cmd = CMD_CURSOR_END;
				break;
			case 0x48:
				cmd = CMD_CURSOR_HOME;
				break;
			break;
		}
		break;
	}
	return cmd;
}

/*
 * do_insert() - gets terminal input for command mode
 */
void do_insert(struct ll_struct *file_buffer, char *cmd_line, int *cmd_line_index)
{
	int cmd;
	
	DEBUG_CALL
	switch(cmd_line[0]) {
		case ESC:
			cmd_add(CMD_COMMAND_MODE);
			cmd = parse_esc_cmd(cmd_line);
			if (CMD_NONE == cmd) {
				// escape key was hit
				cmd_add(CMD_CURSOR_LEFT);
			} else {
				// cursor movement
				cmd_add(cmd);
				cmd_add(CMD_INSERT_MODE);
			}
			clear_cmd_line(cmd_line, cmd_line_index);
			break;
		case BKSP:
			cmd_add(CMD_COMMAND_MODE);
			if (pos_x_line > 0) {
				cmd_add(CMD_CURSOR_LEFT);
				cmd_add(CMD_DEL_CHAR);
				cmd_add(CMD_APPEND_AFTER_CURSOR);
			}
			clear_cmd_line(cmd_line, cmd_line_index);
			break;
		case '\n':
		case 0x0d:
			ll_delete(file_buffer, current_line);
			ll_insert(file_buffer, current_line, line_buffer, pos_x_line);
			ll_insert(file_buffer, current_line + 1, line_buffer + pos_x_line, line_buffer_end - pos_x_line);
			fill_line_buffer(file_buffer, current_line + 1);
			printf("%c[%d;%dH%c[2K", ESC, win_size.ws_row, 0, ESC);
			pos_x_line = 0;
			current_line++;
			pos_y++;
			cmd_add(CMD_REDRAW);
			clear_cmd_line(cmd_line, cmd_line_index);
			break;
		default:
			printf("%c", cmd_line[0]);
			insert_into_line_buffer(pos_x_line, cmd_line[0]);
			pos_x_line++;
			display_line(file_buffer, -1);
			clear_cmd_line(cmd_line, cmd_line_index);
			break;
	}
	if (editor_mode & EDITOR_MODE_REPLACE) {
		cmd_add(CMD_COMMAND_MODE);
		clear_cmd_line(cmd_line, cmd_line_index);
	}
}

/*
 * process_command() - perform any actions that may be waiting in cmd.
 */
int process_command(struct ll_struct *file_buffer)
{
	int retval = 0;
	int cmd;
	int pos_x_temp;
	
	DEBUG_CALL
	while(CMD_NONE != (cmd = cmd_get())) {
		switch(cmd) {
			case CMD_WRITE:
				write_file(file_buffer);
				printf("%c[%d;%dH%c[2K", ESC, win_size.ws_row, 0, ESC);
				printf("\"%s\" %ldL, %ldC written", filename, ll_count(file_buffer), ll_size(file_buffer));
				break;
			case CMD_REDRAW:
				redraw_screen(file_buffer);
				break;
			case CMD_CURSOR_LEFT:
				do_cursor_left(file_buffer);
				break;
			case CMD_CURSOR_RIGHT:
				do_cursor_right(file_buffer);
				break;
			case CMD_CURSOR_DOWN:
				do_cursor_down(file_buffer);
				break;
			case CMD_CURSOR_UP:
				do_cursor_up(file_buffer);
				break;
			case CMD_CURSOR_HOME:
				cursor_mode &= ~CURSOR_MODE_END;
				pos_x_line = 0;
				break;
			case CMD_CURSOR_END:
				cursor_mode |= CURSOR_MODE_END;
				break;
			case CMD_CURSOR_PGUP:
				if (current_line > 0) {
					pos_x_temp = get_print_pos(line_buffer, pos_x_line);
					current_line -= (win_size.ws_row - 1);
					if (current_line < 0)
						current_line = 0;
					pos_y -= (win_size.ws_row - 1);
					if (pos_y < 0)
						pos_y = 0;
					cmd_add(CMD_REDRAW);
					fill_line_buffer(file_buffer, current_line);
					pos_x_line = set_print_pos(line_buffer, pos_x_temp);
				}
				break;
			case CMD_CURSOR_PGDN:
				pos_x_temp = get_print_pos(line_buffer, pos_x_line);
				current_line += (win_size.ws_row - 1);
				if (current_line >= ll_count(file_buffer)) {
					current_line = (ll_count(file_buffer) - 1);
					pos_y = 0;
				}
				// TODO: only refresh if needed
				cmd_add(CMD_REDRAW);
				fill_line_buffer(file_buffer, current_line);
				pos_x_line = set_print_pos(line_buffer, pos_x_temp);
				break;
			case CMD_DELETE_LINE:
				ll_delete(file_buffer, current_line);
				if (current_line > ll_count(file_buffer) - 1)
					cmd_add(CMD_CURSOR_UP);
				cmd_add(CMD_REDRAW);
				break;
			case CMD_APPEND_AFTER_CURSOR:
				if (ll_get_data_size(file_buffer, current_line) >  pos_x_line)
					pos_x_line++;
				cmd_add(CMD_INSERT_MODE);
				break;
			case CMD_INSERT_MODE:
				editor_mode &= ~EDITOR_MODE_CMD;
				fill_line_buffer(file_buffer, current_line);
				printf("%c[%d;%dH%c[K-- %s --", ESC, win_size.ws_row, 0, ESC, (editor_mode & EDITOR_MODE_REPLACE) ? "REPLACE" : "INSERT");
				break;
			case CMD_COMMAND_MODE:
				editor_mode |= EDITOR_MODE_CMD;
				editor_mode &= ~EDITOR_MODE_REPLACE;
				printf("%c[%d;%dH%c[K", ESC, win_size.ws_row, 0, ESC);
				// copy the line buffer back into the file buffer
				ll_delete(file_buffer, current_line);
				ll_insert(file_buffer, current_line, line_buffer, line_buffer_end);
				break;
			case CMD_DEL_CHAR:
				fill_line_buffer(file_buffer, current_line);
				if (ll_get_data_size(file_buffer, current_line) > 0) {
					delete_from_line_buffer(pos_x_line);
					ll_delete(file_buffer, current_line);
					ll_insert(file_buffer, current_line, line_buffer, line_buffer_end);
					display_line(file_buffer, current_line);
					if (pos_x_line > (ll_get_data_size(file_buffer, current_line) - 1)) {
						pos_x_line = ll_get_data_size(file_buffer, current_line) - 1;
						if (pos_x_line < 0)
							pos_x_line = 0;
					}
				}
				break;
			case CMD_EXIT:
				retval = 1;
				break;
			default:
#ifdef DEBUG
				show_error(ERROR_TYPE_UNKNOWN, "Unknown command");
#endif
				break;
		}
		if (pos_y > (win_size.ws_row - 2)) {
			pos_y = win_size.ws_row - 2;
			printf("%cD", ESC);
			display_line(file_buffer, current_line);
		}
		else if (pos_y < 0) {
			pos_y = 0;
			printf("%cM", ESC);
			display_line(file_buffer, current_line);
		}
	}
	return retval;
}

/*
 * get_print_pos() - returns the x location the cursor should be at for any given index into the line_buffer.
 */
int get_print_pos(char *line_buffer, int pos_x_line)
{
	int x = 0;
	int index;

	DEBUG_CALL
	if ((line_buffer_end > 0) && ('\t' == line_buffer[0]))
		x = env_tabstop - 1;
	for (index = 0; index < pos_x_line; index++) {
		// TODO: combine some code with set_print_pos()
		if ('\t' == line_buffer[index+1]) {
			x += env_tabstop + 1;
			x /= env_tabstop;
			x *= env_tabstop;
			x--;
		}
		else if (iscntrl(line_buffer[index+1]))
			x+=2;
		else
			x++;
	}
	if ((x>0) && (line_buffer[x] == 0))
		x--;
	return x;
}

/*
 * set_print_pos() - returns the index into the line_buffer that will yield the given pos_x.
 */
int set_print_pos(char *line_buffer, int pos_x)
{
	int x = 0;
	int line_index = 0;
	
	DEBUG_CALL
	while ((line_index < line_buffer_size) && (x <= pos_x)) {
		if ('\t' == line_buffer[line_index]) {
			x += env_tabstop + 1;
			x /= env_tabstop;
			x *= env_tabstop;
		}
		else if (iscntrl(line_buffer[line_index]))
			x += 2;
		else
			x++;
		line_index++;
	}
	line_index--;
	return line_index;
}

/*
 * process_output() - takes command of managing what is displayed and plotting the cursor.
 */
void process_output(struct ll_struct *file_buffer, char *cmd_line)
{
	int pos_x;
	int max_pos_x;
	int max_pos_x_line;

	DEBUG_CALL
	max_pos_x = get_print_pos(line_buffer, line_buffer_end);
	if (pos_x_line > max_pos_x)
		pos_x_line = max_pos_x - 1;
	if (pos_x_line < 0)
		pos_x_line = 0;
	if (editor_mode & EDITOR_MODE_CMD)
		max_pos_x--;
	max_pos_x_line = line_buffer_end;
	if (editor_mode & EDITOR_MODE_CMD)
		max_pos_x_line--;
	if (max_pos_x_line < 0)
		max_pos_x_line = 0;
	pos_x = get_print_pos(line_buffer, pos_x_line);
	// BUG HERE
	while (pos_x > win_size.ws_col) {
		pos_x -= win_size.ws_col;
		pos_y++;
	}
	if (editor_mode & EDITOR_MODE_CMD) {
		if (cursor_mode & CURSOR_MODE_END) {
			pos_x_line = max_pos_x_line;
			pos_x = max_pos_x;
		}
		if ((':' == cmd_line[0]) || ('/' == cmd_line[0]))
			printf("%c[%d;%dH%c[K%s", ESC, win_size.ws_row, 0, ESC, cmd_line);
		else
			printf("%c[%d;%dH", ESC, pos_y+1, pos_x+1);
	} else {
		// adjust the cursor position
		if (pos_x_line == 0)
			pos_x = 0;
		// are we in insert mode, sitting on a tabstop?
		else if (('\t' == line_buffer[pos_x_line]) | (iscntrl(line_buffer[pos_x_line])))
			pos_x = get_print_pos(line_buffer, pos_x_line - 1) + 1;
		printf("%c[%d;%dH", ESC, pos_y+1, pos_x+1);
	}
	fflush(0);
}

void do_cursor_right(struct ll_struct *file_buffer)
{
	DEBUG_CALL
	if (pos_x_line < ll_get_data_size(file_buffer, current_line) - 1)
		pos_x_line++;
}

void do_cursor_left(struct ll_struct *file_buffer)
{
	DEBUG_CALL
	cursor_mode &= ~CURSOR_MODE_END;
	pos_x_line--;
}

void do_cursor_down(struct ll_struct *file_buffer)
{
	int pos_x;

	DEBUG_CALL
	pos_x = get_print_pos(line_buffer, pos_x_line);
	if (current_line < ll_count(file_buffer) - 1) {
		pos_y += (1 + (int)((ll_get_data_size(file_buffer, current_line) - 1) / (win_size.ws_col)));
		current_line++;
		fill_line_buffer(file_buffer, current_line);
	}
	pos_x_line = set_print_pos(line_buffer, pos_x);
}

void do_cursor_up(struct ll_struct *file_buffer)
{
	int pos_x;

	DEBUG_CALL
	if (current_line > 0) {
		pos_x = get_print_pos(line_buffer, pos_x_line);
		current_line--;
		pos_y -= (1 + (int)((ll_get_data_size(file_buffer, current_line) - 1) / (win_size.ws_col)));
		fill_line_buffer(file_buffer, current_line);
		pos_x_line = set_print_pos(line_buffer, pos_x);
	}
}

/*
 * read_file() - reads file to be edited into memory
 * returns: 0 upon success
 */
int read_file(struct ll_struct *file_buffer)
{
	FILE *fp;
	int c;
	long line_buffer_index = 0;
	long current_line = 0;

	DEBUG_CALL
	fp = fopen(filename,"rb");
	if (NULL == fp) {
		// A new file is to be created.
		// TODO: check to make sure the user has write perms in this dir
		// for the newly created file.
		return 0;
	}

	file_exists = 1;
	while ((c = fgetc(fp)) != EOF) {
		if ('\n' == c) {
			ll_insert(file_buffer, current_line, line_buffer, line_buffer_index);
			current_line++;
			line_buffer_index = 0;
		} else {
			line_buffer[line_buffer_index] = c;
			line_buffer_index++;
			if (line_buffer_index >= line_buffer_size)
				grow_line_buffer();
		}
	}
	fclose(fp);
	return FILE_READ_SUCCESS;
}

/*
 * write_file() - writes file buffer out to a file
 * returns: 0 upon success.
 */
int write_file(struct ll_struct *file_buffer)
{
	FILE *fp = NULL;
	char *data = NULL;
	long line_number = 0;
	long length = 0;
	long index_data;
	
	DEBUG_CALL
	fp = fopen(filename,"wb");
	if (NULL == fp)
		show_error(ERROR_TYPE_FILE, "Cannot write");
	while ((data = ll_get(file_buffer, line_number++, &length)) != NULL) {
		for (index_data = 0; index_data < length; index_data++)
			fputc(data[index_data], fp);
		fputc('\n', fp); // TODO: this may not be correct at EOF
				 // FIX: would be to include \n OR \r\n at every EOL in file
				 //      and be newline type agnostic, like vim
// Line terminator descriptor:
// 00 no nl
// 01 \n
// 10 \r
// 11 \r\n

	}
	fclose(fp);
	return 0;
}

/*
 * display_line() - Print desired line from file buffer at current location on screen.  If line
 * specified is -1, then the working line buffer is shown on current line on screen.
 * returns: 0 upon success.
 */
int display_line(struct ll_struct *file_buffer, long line)
{
	long index_data;
	char *data;
	long length = 0;
	int index;
	
	DEBUG_CALL
	if (line < 0) {
		length = line_buffer_end;
		data = line_buffer;
	}
	else
		data = ll_get(file_buffer, line, &length);
	for (index = (length / win_size.ws_col); index >= 0; index--)
		printf("%c[%d;%dH%c[K", ESC, pos_y + 1 + index, 0, ESC);
	if (NULL != data)
		for (index_data = 0; index_data < length; index_data++)
			my_putchar(data[index_data]);
	return 0;
}

void my_putchar(char c)
{
	DEBUG_CALL
	if (isprint(c))
		putchar(c);
	else
		switch(c) {
			case '\t':
				putchar(c);
				break;
			default:
				printf("^%c", c + '@');
				break;
		}
}

/*
 * ll_insert() - Copies length data from src into the specified index of the linked list.
 * returns: 0 upon success.  If index is beyond end of list, the new entry is appended
 * to the end.
 */
int ll_insert(struct ll_struct *ll, long index, char *src, long length)
{
	struct ll_struct *ptr;
	struct ll_struct *ptr_new;
	
	DEBUG_CALL
	ptr = ll;
	// advance to the next pointer in the list
	if (NULL != (*ptr).next)
		while((index-- > 0)&&(NULL != ((*ptr).next)))
			ptr = (*ptr).next;
	// check to see if we are at the end of the list, we may need to insert ourselves.
	ptr_new = MALLOC(sizeof(struct ll_struct));
	if(NULL == ptr_new)
		show_error(ERROR_TYPE_MEMORY, "file buffer handle");
	// set previous pointer
	(*ptr_new).prev = ptr;
	// set next pointer
	(*ptr_new).next = (*ptr).next;
	// update the next structure with our address
	if (NULL != (*ptr).next)
		(*(*ptr).next).prev = ptr_new;
	// update the previous structure with our address
	(*ptr).next = ptr_new;
	(*ptr_new).data = MALLOC(sizeof(char)*length);
	if(NULL == (*ptr_new).data)
		show_error(ERROR_TYPE_MEMORY, "file buffer data");
	strncpy((*ptr_new).data, src, length);
	(*ptr_new).length = length;
	return 0;
}

/*
 * insert_into_line_buffer() - inserts a character into the working line buffer.
 * returns: 0 upon success.
 * TODO: check malloc'ed bounds & realloc if necessary
 */
int insert_into_line_buffer(long pos, int input)
{
	DEBUG_CALL
	memmove(line_buffer + pos + 1, line_buffer + pos, line_buffer_end - pos);
	line_buffer[pos] = input;
	line_buffer_end++;
	if (line_buffer_end >= line_buffer_size)
		grow_line_buffer();
	return 0;
}

/*
 * delete_from_line_buffer() - deletes specified character from the working line buffer.
 * returns: 0 upon success.
 */
int delete_from_line_buffer(long pos)
{
	DEBUG_CALL
	memmove(line_buffer + pos, line_buffer + pos + 1, line_buffer_end - pos);
	line_buffer_end--;
	return 0;
}

/*
 * sig_winch() - called whenever the window size is changed
 */
static void sig_winch(int signo)
{
	DEBUG_CALL
	get_winsize();
	cmd_add(CMD_REDRAW);
}

/*
 * raman: get_winsize() - retrieves the current screen
 * dimensions from the eLua base system.
 */
static void get_winsize(void)
{
	win_size.ws_row = term_get_lines();
	win_size.ws_col = term_get_cols();
}

/*
 * redraw_screen() - updates the display with the contents of the file buffer.
 */
static void redraw_screen(struct ll_struct *file_buffer)
{
	int y = 0;
	long file_line_number;
	char *data = NULL;
	long length = 0;
	long index_data;

	DEBUG_CALL
	printf("%c[2J%c[H", ESC, ESC);
	file_line_number = current_line - pos_y;
	while ((y < (win_size.ws_row - 1)) && ((data = ll_get(file_buffer, file_line_number, &length)) != NULL)) {
		// TODO: write a common function for this.
		if (y > 0)
			printf("\n");
		for (index_data = 0; index_data < length; index_data++)
			my_putchar(data[index_data]);
		y += (1 + (int)((ll_get_data_size(file_buffer, file_line_number) - 1) / (win_size.ws_col)));
		file_line_number++;
	}
	while (y++ < (win_size.ws_row - 1))
		printf("\n~");
	if (!(editor_mode & EDITOR_MODE_CMD))
		printf("%c[%d;%dH%c[K-- INSERT --", ESC, win_size.ws_row, 0, ESC);
	fflush(0);
}

/*
 * show_error() - displays a passed error message, then exits with an error.
 * raman: modified to use longjmp();
 */
void show_error(char error_type, char *error_message)
{
	DEBUG_CALL
	int err;
	term_clrscr();
	switch (error_type) {
		case ERROR_TYPE_MEMORY:
			printf("\nMemory");
			err = SHOW_ERROR_MEMORY;
			break;
		case ERROR_TYPE_FILE:
			printf("\nFile");
			err = SHOW_ERROR_FILE;
			break;
		default:
			printf("\nUnkown");
			err = SHOW_ERROR_UNKNOWN;
			break;
	}
	printf(" error: %s\n", error_message);

	iv_reset_globals();
	longjmp(ebuf, err);
}

/*
 * restore_input_mode() - sets the terminal back to its original behavior before exiting.
 * raman: a longjmp will take us there.
 */
void restore_input_mode(void)
{
        DEBUG_CALL
	term_clrscr();
	longjmp(ebuf, RESTORE_INPUT_RET_VALUE);
}

/*
 * iv_reset_globals() - restore default values for global variables.
 */
static void iv_reset_globals(void)
{
	int i;

        pos_x_line = 0;
        pos_y = 0;
        current_line = 0;
        line_buffer = NULL;
        line_buffer_end = 0;
        line_buffer_size = 0;
        filename = NULL;
        editor_mode = EDITOR_MODE_CMD;
        cursor_mode = 0;
        cmd_queue_head = 0;
        cmd_queue_tail = 0;
        env_tabstop = TABSTOP;
        file_exists = 0;

        /* clean file_buffer_begin */
        file_buffer_begin.next = NULL;
        file_buffer_begin.prev = NULL;
        file_buffer_begin.data = NULL;
        file_buffer_begin.length = 0;
        file_buffer_begin.flags = 0;

	for (i = 0; i < MAX_SEARCH; i++)
		search_string[i] = 0;

	for (i = 0; i < MAX_CMD_QUEUE; i++)
		cmd_queue[i] = CMD_NONE;
}

/*
 * ll_free() - Walk backwards through list and free() each hunk of data
 */
int ll_free(struct ll_struct *ll)
{
	struct ll_struct *ptr;
	
	DEBUG_CALL
	ptr = ll;
	if (NULL != ptr) {
		while(NULL != ((*ptr).next))
			ptr = (*ptr).next;
		while(NULL != (*ptr).data) {
			FREE((*ptr).data);
			ptr = (*ptr).prev;
			FREE((*ptr).next);
		}
	}
	return 0;
}

/*
 * ll_get() - returns: a pointer to the data at index.  Also returns length.
 */
char *ll_get(struct ll_struct *ll, long index, long *length)
{
	struct ll_struct *ptr;
	char *data = NULL;

	DEBUG_CALL
	*length = 0;
	ptr = ll;
	if (NULL != (*ptr).next) {
		ptr = (*ptr).next;
		while ((NULL != ptr) && (index > 0)) {
			if (NULL == (*ptr).next)
				break;
			ptr=(*ptr).next;
			index--;
		}
		if (0 == index) {
			data = (*ptr).data;
			*length = (*ptr).length;
		}
	}
	return data;
}

long ll_size(struct ll_struct *ll)
{
	struct ll_struct *ptr;
	long size = 0;

	DEBUG_CALL
	ptr = ll;
	if (NULL != ptr) {
		while (NULL != ((*ptr).next)) {
			ptr = (*ptr).next;
			size += (*ptr).length + 1; // TODO: possibly incorrect assumption (keep a noeol flag)
		}
	}
	return size;
}

/*
 * ll_get_data_size() - returns: size of data at specified line.
 */
long ll_get_data_size(struct ll_struct *ll, long index)
{
	// TODO: cache this somehow; either per-line or full table.
	struct ll_struct *ptr = NULL;
	
	DEBUG_CALL
	ptr = ll;
	if (NULL != ptr) {
		ptr = (*ptr).next;
		while ((NULL != ptr) && (index > 0)) {
			if (NULL == (*ptr).next)
				break;
			ptr = (*ptr).next;
			index--;
		}
		if ((0 == index) && (NULL != ptr))
			return (*ptr).length;
	}
	return 0;
}

/*
 * ll_get_data_print_size() - returns: size of printed data at specified index.
 * This routine basically returns size with tabs expanded.
 */
long ll_get_data_print_size(struct ll_struct *ll, long index)
{
	struct ll_struct *ptr = NULL;
	long length;
	long data_index;
	
	DEBUG_CALL
	ptr = ll;
	if (NULL != ptr) {
		ptr = (*ptr).next;
		while ((NULL != ptr) && (index > 0)) {
			if (NULL == (*ptr).next)
				break;
			ptr = (*ptr).next;
			index--;
		}
		if ((0 == index) && (NULL != ptr)) {
			length = data_index = 0;
			while(data_index < (*ptr).length) {
				if('\t' == ((*ptr).data[data_index]))
					length += env_tabstop - (length % env_tabstop);
				else if(iscntrl((*ptr).data[data_index]))
					length += 2;
				else
					length++;
				data_index++;
			}
			return length;
		}
	}
	return 0;
}

long ll_count(struct ll_struct *ll)
{
	struct ll_struct *ptr;
	long count = 0;
	
	DEBUG_CALL
	ptr = ll;
	if (NULL != ptr) {
		while(NULL != ((*ptr).next)) {
			ptr = (*ptr).next;
			count++;
		}
	}
	return count;
}

int ll_delete(struct ll_struct *ll, long index)
{
	struct ll_struct *ptr = NULL;
	struct ll_struct *ptr_next = NULL;
	
	DEBUG_CALL
	ptr = ll;
	if (NULL != ptr) {
		if (NULL != (*ptr).next) {
			ptr = (*ptr).next;
			while ((NULL != ptr) && (index > 0)) {
				if (NULL == (*ptr).next)
					break;
				ptr = (*ptr).next;
				index--;
			}
		}
		if (0 == index) {
			ptr_next = (*ptr).next;
			if (ptr != ll) {
				if (NULL != (*ptr).next)
					(*(*ptr).next).prev = (*ptr).prev;
				FREE((*ptr).data);
				ptr = (*ptr).prev;
				// We don't need to free next structure if it doesn't exist
				if (NULL != ptr_next)
					FREE((*ptr).next);
				(*ptr).next = ptr_next;
			}
			return 0;
		}
	}
	return -1;
}

void grow_line_buffer(void)
{
	DEBUG_CALL
	do {
		line_buffer = realloc(line_buffer, sizeof(char)*(line_buffer_size + LINE_BUFFER_CHUNK_SIZE));
		if (NULL == line_buffer)
			show_error(ERROR_TYPE_MEMORY, "grow line buffer");
		line_buffer_size += LINE_BUFFER_CHUNK_SIZE;
	} while (line_buffer_size < line_buffer_end);
}

int fill_line_buffer(struct ll_struct *file_buffer, long index)
{
	char *ll_data_ptr;

	DEBUG_CALL
	ll_data_ptr = ll_get(file_buffer, index, &line_buffer_end);
	if (NULL != ll_data_ptr)
		strncpy(line_buffer, ll_data_ptr, line_buffer_end);
	return 0;
}

#if DEBUG > 0
int ll_dump(struct ll_struct *ll)
{
	struct ll_struct *ptr;
	
	DEBUG_CALL
	ptr = ll;
	while(NULL != ((*ptr).next)) {
		ptr = (*ptr).next;
		printf("*prev=0x%08lx, ", (unsigned long)(*ptr).prev);
		printf("*curr=0x%08lx, ", (unsigned long)ptr);
		printf("*next=0x%08lx, ", (unsigned long)(*ptr).next);
		printf("*data=0x%08lx, ", (unsigned long)(*ptr).data);
		printf("data='%s'\n", (*ptr).data);
	}
	return 0;
}

/*
 * my_malloc() - a debugging wrapper for memory allocation analysis.
 */
void *my_malloc(size_t size)
{
	void *ptr;

	DEBUG_CALL
	ptr = malloc(size);
#if DEBUG > 2
	fprintf(fp_debug, "M: 0x%08x\n", (unsigned int)ptr);
#endif
	return ptr;
}

/*
 * my_free() - a debugging wrapper for memory freeing analysis.
 */
void my_free(void *ptr)
{
	DEBUG_CALL
#if DEBUG > 2
	fprintf(fp_debug, "F: 0x%08x\n", (unsigned int)ptr);
#endif
	if (NULL == ptr)
		show_error(ERROR_TYPE_MEMORY, "Tried to free NULL pointer");
	free(ptr);
}
#endif // DEBUG

#endif // BUILD_EDITOR_IV
