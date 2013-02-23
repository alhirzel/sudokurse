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



/* board[row][column] */

/* bitfield:
 * M _ _ _ D3 D2 D1 D0
 *
 * M - mutable, 0 for mutable, 1 for immutable
 * D3 D2 D1 D0 - number
 *
 * entirely zero = blank character
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
#define BLANK (0)



void read_puzzle(char *filename, uint8_t (*board)[9][9]);
void position_cursor(int cursor_row, int cursor_col);
void position_cursor_first_blank(uint8_t (*board)[9][9], int *cursor_row, int *cursor_col);
void draw_board(uint8_t (*board)[9][9], int cursor_row, int cursor_col);
int check_winner(uint8_t (*board)[9][9]);



void read_puzzle(char *filename, uint8_t (*board)[9][9]) {
	FILE *f = fopen(filename, "r");
	int c;

	for (int row = 0; row < 9; row++) {
		for (int col = 0; col < 9; col++) {
			c = fgetc(f);

			/* if '.' then blank, if invalid character then blank... */
			if ((char) c == '.') {
				c = BLANK;
			} else if ((char) c < '1' || (char) c > '9') {
				c = BLANK;
			}

			(*board)[row][col] = (uint8_t) c;

			/* set immutable if not blank */
			if (c != BLANK) {
				(*board)[row][col] |= 0x80;
			}
		}

		/* skip newline characters '\r' and '\n' */
		while (1 == 1) {
			c = fgetc(f);
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
	for (int row = 0; row < 9; row++) {
		for (int col = 0; col < 9; col++) {

			/* if blank... */
			if (BLANK == (*board)[row][col]) {
				position_cursor(row, col);
				*cursor_row = row;
				*cursor_col = col;
				return;
			}
		}
	}
}

#define C_TWINS (1)
void draw_board(uint8_t (*board)[9][9], int cursor_row, int cursor_col) {
	curs_set(0); /* invisible cursor */

	/* draw gridlines */
	for (int i = 0; i < 4*9-3; i++) {
		mvaddch(7,  i+4, '-');
		mvaddch(13, i+4, '-');
	}
	for (int i = 0; i < 2*9-1; i++) {
		mvaddch(i+2, 14, '|');
		mvaddch(i+2, 26, '|');
	}

	/* draw numbers */
	for (int row = 0; row < 9; row++) {
		mvaddch(2*(row+1), 0, ('1' + row) | A_UNDERLINE);
		for (int col = 0; col < 9; col++) {
			mvaddch(0, 4*(col+1), ('1' + col) | A_UNDERLINE);

			uint8_t val = (*board)[row][col];
			chtype newchar;

			/* determine new character */
			if (BLANK == val) {
				newchar = ' ';
			} else {
				newchar = (val & 0xF) + '0';
			}

			/* underline if immutable */
			if (1 == val >> 7) {
				newchar |= A_UNDERLINE;
			}

			/* use alternate color if same as cursor position */
			if ((val & 0xF) == ((*board)[cursor_row][cursor_col] & 0xF)) {
				newchar |= COLOR_PAIR(C_TWINS);
			}

			position_cursor(row, col);
			addch(newchar);
		}
	}
	position_cursor(cursor_row, cursor_col);
	curs_set(2); /* block cursor */
}

#define WIN (((1<<9) - 1) << 1)
#define BV(r, c) (1 << ((*board)[r][c] & 0xF))
int check_winner(uint8_t (*board)[9][9]) {
	uint16_t winner_row, winner_col, winner_group;

	/* build up three integers with bits set as numbers are checked off... */
	for (int i = 0; i < 9; i++) {

		/* search the i'th row and i'th column */
		winner_row = winner_col = 0;
		for (int j = 0; j < 9; j++) {
			winner_row |= BV(i, j);
			winner_col |= BV(j, i);
		}
		if (winner_row != WIN) return 0;
		if (winner_col != WIN) return 0;

		/* search the i'th group */
		winner_group = 0;
		int c = 3 * (i % 3), r = (i - c/3);
		winner_group |= BV(r+0, c+0) | BV(r+0, c+1) | BV(r+0, c+2);
		winner_group |= BV(r+1, c+0) | BV(r+1, c+1) | BV(r+1, c+2);
		winner_group |= BV(r+2, c+0) | BV(r+2, c+1) | BV(r+2, c+2);
		if (winner_group != WIN) return 0;
	}
	return 1;
}

int main(void) {
	int cursor_row = 0, cursor_col = 0;

	initscr();
	noecho();
	curs_set(2); /* block cursor */
	/* colors */
	start_color();
	init_pair(C_TWINS, COLOR_RED, COLOR_BLACK);
	read_puzzle("test.puzzle", &puzzle);
	draw_board(&puzzle, cursor_row, cursor_col);
	position_cursor_first_blank(&puzzle, &cursor_row, &cursor_col);
	refresh();

	while (1 == 1) {

		int c = getch();

		switch ((char) c) {
			
			/* movement commands */
			case 'h':
				cursor_col += 8; cursor_col %= 9;
				draw_board(&puzzle, cursor_row, cursor_col);
				break;
			case 'j':
				cursor_row += 1; cursor_row %= 9;
				draw_board(&puzzle, cursor_row, cursor_col);
				break;
			case 'k':
				cursor_row += 8; cursor_row %= 9;
				draw_board(&puzzle, cursor_row, cursor_col);
				break;
			case 'l':
				cursor_col += 1; cursor_col %= 9;
				draw_board(&puzzle, cursor_row, cursor_col);
				break;

			/* replace character */
			case 'r':
				curs_set(1); /* underline cursor */
				c = getch();
				curs_set(2); /* block cursor */

				/* is this square immutable? if so, alert user */
				if (1 == (puzzle[cursor_row][cursor_col] >> 7)) {
					flash();
					break;
				}

				/* is the new value valid? */
				uint8_t new_value;
				if ((' ' == c) || ('.' == c)) {  /* yes: blank */
					new_value = BLANK;
				} else if (c < '1' || c > '9') { /* no */
					break;
				} else {                         /* yes: a number */
					new_value = (uint8_t) (c - '0');
				}
				puzzle[cursor_row][cursor_col] = new_value;
				draw_board(&puzzle, cursor_row, cursor_col);
				refresh();
				if (check_winner(&puzzle) == 1) {
					endwin();
					puts("You win!");
					exit(EXIT_SUCCESS);
				}
				break;
		}
	}
}

