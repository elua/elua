/*
 * iv.h
 */

#ifndef IV_H
#define IV_H
 
#define	INFO			"A tiny editor for embedded systems."
#define	AUTHOR			"By Christopher Cole (cole@coledd.com)"
#define VERSION			"0.39 (modified)"
#define	ESC			0x1b

/*
 * raman: see term_translate() in
 * common.c
 */
#define	BKSP			0x08

#define	LINE_BUFFER_CHUNK_SIZE	1024
#define	MAX_CMD_LINE_SIZE	128
#define MAX_CMD_QUEUE		32
#define MAX_SEARCH		128
#define	TEMP_FILE		"/mmc/untitled"
#define	DEFAULT_NUM_COLUMNS	80
#define	DEFAULT_NUM_ROWS	25
#define TABSTOP			8

/*
 * raman: use longjmp in
 * show_error();
 */
#define SHOW_ERROR_MEMORY	-1
#define SHOW_ERROR_FILE		-2
#define SHOW_ERROR_UNKNOWN	-3

#define RESTORE_INPUT_RET_VALUE -5

#define	EDITOR_MODE_CMD		(1<<0)
#define	EDITOR_MODE_REPLACE	(1<<1)

#define	CURSOR_MODE_END		(1<<0)

#define FILE_FLAGS_NEW		(1<<0)
#define FILE_FLAGS_READONLY	(1<<1)
#define FILE_READ_SUCCESS       (1<<2)

enum {
	CMD_NONE,
        CMD_APPEND_AFTER_CURSOR,
        CMD_COMMAND_MODE,
        CMD_CURSOR_DOWN,
        CMD_CURSOR_END,
        CMD_CURSOR_HOME,
        CMD_CURSOR_LEFT,
        CMD_CURSOR_PGDN,
        CMD_CURSOR_PGUP,
        CMD_CURSOR_RIGHT,
        CMD_CURSOR_UP,
        CMD_DELETE_LINE,
        CMD_DEL_CHAR,
        CMD_DUMMY,
        CMD_EXIT,
        CMD_INSERT_MODE,
        CMD_REDRAW,
        CMD_WRITE,
        CMD_WRITE_QUIT
};

enum {
	ERROR_TYPE_UNKNOWN,
	ERROR_TYPE_FILE,
	ERROR_TYPE_MEMORY
};

/*
 * The file buffer is a series of dynamically allocated lines:
 * Double Linked List line element structure:
 */
struct ll_struct {
	struct ll_struct *next;
	struct ll_struct *prev;
	char *data;
	long length;
	unsigned long flags; // modified, tagged for yank (?)
};

struct alcor_iv_winsize {
  unsigned short int ws_row;
  unsigned short int ws_col;
};

void clear_cmd_line(char *cmd_line, int *cmd_line_index);
int cmd_add(int cmd);
int cmd_get(void);
int delete_from_line_buffer(long pos);
int display_line(struct ll_struct *file_buffer, long line);
void do_cursor_down(struct ll_struct *file_buffer);
void do_cursor_left(struct ll_struct *file_buffer);
void do_cursor_right(struct ll_struct *file_buffer);
void do_cursor_up(struct ll_struct *file_buffer);
int do_search_string(struct ll_struct *file_buffer, char *string);
int get_print_pos(char *line_buffer, int pos_x_line);
void grow_line_buffer(void);
void do_insert(struct ll_struct *file_buffer, char *cmd_line, int *cmd_line_index);
int fill_line_buffer(struct ll_struct *file_buffer, long index);
int insert_into_line_buffer(long pos, int input);
long ll_count(struct ll_struct *ll);
int ll_delete(struct ll_struct *ll, long index);
int ll_free(struct ll_struct *ll);
char *ll_get(struct ll_struct *ll, long index, long *length);
long ll_get_data_size(struct ll_struct *ll, long index);
long ll_get_data_print_size(struct ll_struct *ll, long index);
int ll_insert(struct ll_struct *ll, long index, char *src, long length);
long ll_size(struct ll_struct *ll);
void my_putchar(char c);
int parse_esc_cmd(char *cmd_line);
void parse_input_cmds(struct ll_struct *file_buffer, char *cmd_line, int *cmd_line_index);
int process_command(struct ll_struct *file_buffer);
void process_output(struct ll_struct *file_buffer, char *cmd_line);
int read_file(struct ll_struct *file_buffer);
void restore_input_mode(void);
int set_input_mode(void);
int set_print_pos(char *line_buffer, int pos_x);
void show_error(char error_type, char *error_message);
int write_file(struct ll_struct *file_buffer);
int iv_main(int argc, char **argv);

#ifdef DEBUG
int ll_dump(struct ll_struct *ll);
void *my_malloc(size_t size);
void my_free(void *ptr);
#define MALLOC my_malloc
#define FREE my_free
#else
#define MALLOC malloc
#define FREE free
#endif

#endif // IV_H
