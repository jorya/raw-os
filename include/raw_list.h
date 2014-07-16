/*
     raw os - Copyright (C)  Lingjun Chen(jorya_txj).

    This file is part of raw os.

    raw os is free software; you can redistribute it it under the terms of the 
    GNU General Public License as published by the Free Software Foundation; 
    either version 3 of the License, or  (at your option) any later version.

    raw os is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
    without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. if not, write email to jorya.txj@gmail.com
                                      ---

    A special exception to the LGPL can be applied should you wish to distribute
    a combined work that includes raw os, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/


/* 	2012-4  Created by jorya_txj
  *	xxxxxx   please added here
  */


#ifndef RAW_LIST_H
#define RAW_LIST_H
/*
 * Doubly-link list
 */
typedef struct LIST {
	struct LIST	*next;
	struct LIST	*previous;
} LIST;


#define raw_list_entry(node, type, member)    ((type *)((RAW_U8 *)(node) - (RAW_U32)(&((type *)0)->member)))


/*
 * List initialization
 */
RAW_INLINE void list_init(LIST *list_head)
{
	list_head->next = list_head;
	list_head->previous = list_head;
}

/*
 * return TRUE if the list is empty
 */
RAW_INLINE RAW_BOOLEAN is_list_empty(LIST *list)
{

	return (list->next == list);	
	
}

/*
 * add element to list
 * add element before head.
 */
RAW_INLINE void list_insert(LIST *head, LIST *element)
{
	
	element->previous = head->previous;
	element->next = head;
	
	head->previous->next = element;	
	head->previous = element;
}


RAW_INLINE void list_delete(LIST *element)
{

	element->previous->next = element->next;
	element->next->previous = element->previous;
	
}


#endif

