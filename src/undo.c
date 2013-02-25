/* undo.c
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

#include <stdlib.h>
#include "undo.h"



void undo_push(struct undo_move_record **list, struct undo_move_record *new_item) {
	new_item->next = *list;
	*list = new_item;
}



/* returns NULL if no undo operation is left on the stack */
struct undo_move_record *undo_pop(struct undo_move_record **list) {
	struct undo_move_record *to_be_returned;

	to_be_returned = *list;
	if (NULL != to_be_returned) {
		*list = (*list)->next;
	}

	return to_be_returned;
}



void undo_free_entire_list(struct undo_move_record **list) {
	struct undo_move_record *to_be_freed;

	while ((to_be_freed = undo_pop(list))) {
		free(to_be_freed);
	}

	*list = (struct undo_move_record *) NULL;
}

