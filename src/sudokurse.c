/* sudokurse.c
 *
 * Copyright 2013 Alex Hirzel <alex@hirzel.us>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include "undo.h"



/* CONSTANTS */

/* These should really be in <ncurses.h>... */
#define CURSOR_INVISIBLE   (0)
#define CURSOR_NORMAL      (1)
#define CURSOR_VERYVISIBLE (2)

/* "blank" value for a given square in the puzzle (see main). */
#define PUZZLE_BLANK (0)

/* Color of squares which are the same value. */
#define COLOR_SAME_NUMBER (1)
#define COLOR_USER_SUPPLIED_VALUE (2)



/* FUNCTION PROTOTYPES */
void read_puzzle(char *filename, uint8_t (*board)[9][9]);
void position_cursor(int cursor_row, int cursor_col);
void position_cursor_first_blank(uint8_t (*board)[9][9], int *cursor_row, int *cursor_col);
void draw_board_box(void);
void draw_board(uint8_t (*board)[9][9], int cursor_row, int cursor_col);
int check_winner(uint8_t (*board)[9][9]);



void read_puzzle(char *filename, uint8_t (*board)[9][9]) {
	FILE *f = fopen(filename, "r");
	int c, row, col;
	uint8_t v;

	for (row = 0; row < 9; row++) {
		for (col = 0; col < 9; col++) {
			c = fgetc(f);

			if (EOF == c) {
				/* TODO implement failure to read file */
			}

			/* if '.' or ' ' then blank, if invalid character then blank... */
			if ((char) c == '.') {
				v = PUZZLE_BLANK;
			} else if ((char) c == ' ') {
				v = PUZZLE_BLANK;
			} else if ((char) c < '1' || (char) c > '9') {
				v = PUZZLE_BLANK;
			} else {
				v = (uint8_t) (c - '0');
			}

			(*board)[row][col] = v;

			/* set immutable if not blank */
			if (v != PUZZLE_BLANK) {
				(*board)[row][col] |= 0x80;
			}
		}

		/* skip newline characters '\r' and '\n' */
		while (1 == 1) {
			c = fgetc(f);

			/* EOF from fgetc is handled at the top of the inner 'for' loop above */

			if (c == '\r') continue;
			if (c == '\n') continue;
			ungetc(c, f);
			break;
		}
	}
	fclose(f);
}



void position_cursor(int cursor_row, int cursor_col) {
	move(2*cursor_row+2, 4*cursor_col+4);
}



void position_cursor_first_blank(uint8_t (*board)[9][9], int *cursor_row, int *cursor_col) {
	int row, col;

	for (row = 0; row < 9; row++) {
		for (col = 0; col < 9; col++) {

			/* if blank... */
			if (PUZZLE_BLANK == (*board)[row][col]) {
				position_cursor(row, col);
				*cursor_row = row;
				*cursor_col = col;
				return;
			}
		}
	}
}



void draw_board_box(void) {
	int i;

	/* draw gridlines */
	for (i = 0; i < 4*9-1; i++) {
		mvaddch(1,  i+3, ACS_HLINE);
		mvaddch(7,  i+3, ACS_HLINE);
		mvaddch(13, i+3, ACS_HLINE);
		mvaddch(19, i+3, ACS_HLINE);
	}
	for (i = 0; i < 2*9+1; i++) {
		mvaddch(i+1, 2, ACS_VLINE);
		mvaddch(i+1, 14, ACS_VLINE);
		mvaddch(i+1, 26, ACS_VLINE);
		mvaddch(i+1, 38, ACS_VLINE);
	}

	/* corners */
	mvaddch(1, 2,  ACS_ULCORNER);
	mvaddch(1, 38, ACS_URCORNER);
	mvaddch(19, 2,  ACS_LLCORNER);
	mvaddch(19, 38, ACS_LRCORNER);

	/* tee's */
	mvaddch(1, 14, ACS_TTEE);
	mvaddch(1, 26, ACS_TTEE);
	mvaddch(19, 14, ACS_BTEE);
	mvaddch(19, 26, ACS_BTEE);
	mvaddch(7,  2, ACS_LTEE);
	mvaddch(13, 2, ACS_LTEE);
	mvaddch(7,  38, ACS_RTEE);
	mvaddch(13, 38, ACS_RTEE);

	/* crossovers */
	mvaddch(7, 14, ACS_PLUS);
	mvaddch(7, 26, ACS_PLUS);
	mvaddch(13, 14, ACS_PLUS);
	mvaddch(13, 26, ACS_PLUS);
}



void draw_board(uint8_t (*board)[9][9], int cursor_row, int cursor_col) {
	int row, col;

	curs_set(CURSOR_INVISIBLE);

	draw_board_box();

	/* draw numbers */
	for (row = 0; row < 9; row++) {
		mvaddch(2*(row+1), 0, (chtype) ('1' + row));
		for (col = 0; col < 9; col++) {
			mvaddch(0, 4*(col+1), (chtype) ('1' + col));

			uint8_t val_under_cursor = (*board)[row][col];
			chtype newchar;

			/* determine new character */
			if (PUZZLE_BLANK == val_under_cursor) {

				/* blank */
				newchar = '.';

			} else {
				newchar = (val_under_cursor & 0xF) + '0';
			}

			/* underline if immutable */
			if (1 == val_under_cursor >> 7) {
				newchar |= A_BOLD;
			}

			/* use alternate color if same as cursor position */
			if ((val_under_cursor & 0xF) == ((*board)[cursor_row][cursor_col] & 0xF)) {
				newchar |= COLOR_PAIR(COLOR_SAME_NUMBER);
			}

			position_cursor(row, col);
			addch(newchar);
		}
	}
	position_cursor(cursor_row, cursor_col);
	curs_set(CURSOR_VERYVISIBLE);
}



#define WIN (((1<<9) - 1) << 1)
#define BV(r, c) (1 << ((*board)[r][c] & 0xF))

/* returns 1 if game is won, 0 otherwise */
int check_winner(uint8_t (*board)[9][9]) {
	uint16_t winner_row, winner_col, winner_group;
	int r, c, i, j;

	/* build up three integers with bits set as numbers are checked off... */
	for (i = 0; i < 9; i++) {

		/* search the i'th row and i'th column */
		winner_row = winner_col = 0;
		for (j = 0; j < 9; j++) {
			winner_row |= BV(i, j);
			winner_col |= BV(j, i);
		}
		if (winner_row != WIN) return 0;
		if (winner_col != WIN) return 0;

		/* search the i'th group */
		winner_group = 0;
		c = 3 * (i % 3), r = (i - c/3);
		winner_group |= BV(r+0, c+0) | BV(r+0, c+1) | BV(r+0, c+2);
		winner_group |= BV(r+1, c+0) | BV(r+1, c+1) | BV(r+1, c+2);
		winner_group |= BV(r+2, c+0) | BV(r+2, c+1) | BV(r+2, c+2);
		if (winner_group != WIN) return 0;
	}
	return 1;
}



int main(void) {
	int cursor_row = 0, cursor_col = 0;

	/* Puzzle data structure
	 *
	 * 9x9 array of integers indexed first by row then column (i.e.
	 * puzzle[row][col]. Each uint8_t is a bitfield:
	 *
	 *   7 6 5 4 3 2 1 0
	 *   M _ _ _ a b c d
	 *
	 * M - mutable (0 for mutable, 1 for immutable)
	 * abcd - the number in this position
	 *
	 * NOTE: "blank" is represented by all fields equal to zero.
	 */
	uint8_t puzzle[9][9] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
	};

	/* Holds pointer to first 'undo_move_record' */
	struct undo_move_record *undo_stack = NULL;
	struct undo_move_record *temp_move_ptr;

	initscr();
	noecho();
	curs_set(CURSOR_VERYVISIBLE);

	/* Set up colors */
	start_color();
	init_pair(COLOR_SAME_NUMBER, COLOR_RED, COLOR_BLACK);
	init_pair(COLOR_USER_SUPPLIED_VALUE, COLOR_GREEN, COLOR_BLACK);

	/* start of game logic - read puzzle and put cursor on first blank square
	 * (starting the cursor on an immutable square is suboptimal) */
	read_puzzle("test.puzzle", &puzzle);
	position_cursor_first_blank(&puzzle, &cursor_row, &cursor_col);
	draw_board(&puzzle, cursor_row, cursor_col);
	refresh();

	/* infinite loop reading keyboard input */
	while (1 == 1) {

		int c = getch();

		if (ERR == c) {
			endwin();
			fputs("Error reading key.", stderr);
			undo_free_entire_list(&undo_stack);
			exit(EXIT_FAILURE);
		}

		switch ((char) c) {
#define INCR_MOD_9(x) (x = (x + 1) % 9)
#define DECR_MOD_9(x) (x = (x + 8) % 9)

			/* movement commands */
			case 'h':
				DECR_MOD_9(cursor_col);
				draw_board(&puzzle, cursor_row, cursor_col);
				break;
			case 'j':
				INCR_MOD_9(cursor_row);
				draw_board(&puzzle, cursor_row, cursor_col);
				break;
			case 'k':
				DECR_MOD_9(cursor_row);
				draw_board(&puzzle, cursor_row, cursor_col);
				break;
			case 'l':
				INCR_MOD_9(cursor_col);
				draw_board(&puzzle, cursor_row, cursor_col);
				break;

			/* diagonal movement commands borrowed from NetHack. */
			/* TOOD these conflict with 'undo' functionality
			case 'u':
				DECR_MOD_9(cursor_row);
				INCR_MOD_9(cursor_col);
				draw_board(&puzzle, cursor_row, cursor_col);
				break;
			case 'y':
				DECR_MOD_9(cursor_row);
				DECR_MOD_9(cursor_col);
				draw_board(&puzzle, cursor_row, cursor_col);
				break;
			case 'b':
				INCR_MOD_9(cursor_row);
				DECR_MOD_9(cursor_col);
				draw_board(&puzzle, cursor_row, cursor_col);
				break;
			case 'n':
				INCR_MOD_9(cursor_row);
				INCR_MOD_9(cursor_col);
				draw_board(&puzzle, cursor_row, cursor_col);
				break;
			*/

			/* replace character */
			case 'r':
				curs_set(CURSOR_NORMAL);
				c = getch();
				curs_set(CURSOR_VERYVISIBLE);

				/* is this square immutable? if so, alert user */
				if (1 == (puzzle[cursor_row][cursor_col] >> 7)) {
					flash();
					break;
				}

				/* have now established that it is safe to change this square */

				/* is the new value valid? */
				uint8_t new_value;
				if ((' ' == c) || ('.' == c)) {  /* yes: blank */
					new_value = PUZZLE_BLANK;
				} else if (c < '1' || c > '9') { /* no */
					break;
				} else {                         /* yes: a number */
					new_value = (uint8_t) (c - '0');
				}

				/* save this change on the 'undo' stack */
				temp_move_ptr = malloc(sizeof(struct undo_move_record));
				temp_move_ptr->row = cursor_row;
				temp_move_ptr->col = cursor_col;
				temp_move_ptr->old_value = puzzle[temp_move_ptr->row][temp_move_ptr->col];
				temp_move_ptr->new_value = new_value;
				undo_push(&undo_stack, temp_move_ptr);

				/* make the change */
				puzzle[cursor_row][cursor_col] = new_value;
				draw_board(&puzzle, cursor_row, cursor_col);
				refresh();

				/* check for a win */
				if (1 == check_winner(&puzzle)) {
					endwin();
					puts("You win!");
					undo_free_entire_list(&undo_stack);
					exit(EXIT_SUCCESS);
				}
				break;

			/* undo */
			case 'u': // conflicts with diagonal movement (up/right)
			case 'U':
				temp_move_ptr = undo_pop(&undo_stack);

				if (NULL != temp_move_ptr) {

					/* undo last move */
					cursor_row = temp_move_ptr->row;
					cursor_col = temp_move_ptr->col;
					puzzle[cursor_row][cursor_col] = temp_move_ptr->old_value;
					free(temp_move_ptr);
					draw_board(&puzzle, cursor_row, cursor_col);

				} else {

					/* no more undo history to utilize */
					flash();

				}
				break;

		}
	}
}

