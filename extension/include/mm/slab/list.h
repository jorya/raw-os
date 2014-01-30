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



/* 	
 *	2012-9  Slab Memory Managment
 *          by Hu Zhigang <huzhigang.rawos@gmail.com>
 */


#ifndef KERN_LIST_H_
#define KERN_LIST_H_


/** Doubly linked list head and link type. */
typedef struct link {
	struct link *prev;	/**< Pointer to the previous item in the list. */
	struct link *next;	/**< Pointer to the next item in the list. */
} link_t;

/** Initialize doubly-linked circular list link
 *
 * Initialize doubly-linked list link.
 *
 * @param link Pointer to link_t structure to be initialized.
 */
RAW_INLINE void link_initialize(link_t *link)
{
	link->prev = NULL;
	link->next = NULL;
}

/** Initialize doubly-linked circular list
 *
 * Initialize doubly-linked circular list.
 *
 * @param head Pointer to link_t structure representing head of the list.
 */
RAW_INLINE void list_initialize(link_t *head)
{
	head->prev = head;
	head->next = head;
}

/** Add item to the beginning of doubly-linked circular list
 *
 * Add item to the beginning of doubly-linked circular list.
 *
 * @param link Pointer to link_t structure to be added.
 * @param head Pointer to link_t structure representing head of the list.
 */
RAW_INLINE void list_prepend(link_t *link, link_t *head)
{
	link->next = head->next;
	link->prev = head;
	head->next->prev = link;
	head->next = link;
}

/** Add item to the end of doubly-linked circular list
 *
 * Add item to the end of doubly-linked circular list.
 *
 * @param link Pointer to link_t structure to be added.
 * @param head Pointer to link_t structure representing head of the list.
 */
RAW_INLINE void list_append(link_t *link, link_t *head)
{
	link->prev = head->prev;
	link->next = head;
	head->prev->next = link;
	head->prev = link;
}

/** Remove item from doubly-linked circular list
 *
 * Remove item from doubly-linked circular list.
 *
 * @param link 	Pointer to link_t structure to be removed from the list it is
 * 		contained in.
 */
RAW_INLINE void list_remove(link_t *link)
{
	link->next->prev = link->prev;
	link->prev->next = link->next;
	link_initialize(link);
}

/** Query emptiness of doubly-linked circular list
 *
 * Query emptiness of doubly-linked circular list.
 *
 * @param head Pointer to link_t structure representing head of the list.
 */
RAW_INLINE bool list_empty(link_t *head)
{
	return head->next == head ? true : false;
}


/** Split or concatenate headless doubly-linked circular list
 *
 * Split or concatenate headless doubly-linked circular list.
 *
 * Note that the algorithm works both directions:
 * concatenates splitted lists and splits concatenated lists.
 *
 * @param part1	Pointer to link_t structure leading the first (half of the
 *		headless) list.
 * @param part2	Pointer to link_t structure leading the second (half of the
 *		headless) list. 
 */
RAW_INLINE void headless_list_split_or_concat(link_t *part1, link_t *part2)
{
	link_t *hlp;

	part1->prev->next = part2;
	part2->prev->next = part1;	
	hlp = part1->prev;
	part1->prev = part2->prev;
	part2->prev = hlp;
}


/** Split headless doubly-linked circular list
 *
 * Split headless doubly-linked circular list.
 *
 * @param part1	Pointer to link_t structure leading the first half of the
 *		headless list.
 * @param part2	Pointer to link_t structure leading the second half of the
 *		headless list. 
 */
RAW_INLINE void headless_list_split(link_t *part1, link_t *part2)
{
	headless_list_split_or_concat(part1, part2);
}

/** Concatenate two headless doubly-linked circular lists
 *
 * Concatenate two headless doubly-linked circular lists.
 *
 * @param part1 Pointer to link_t structure leading the first headless list.
 * @param part2 Pointer to link_t structure leading the second headless list.
 */
RAW_INLINE void headless_list_concat(link_t *part1, link_t *part2)
{
	headless_list_split_or_concat(part1, part2);
}

#define list_get_instance(link, type, member) \
	((type *)(((RAW_U8 *)(link)) - ((RAW_U8 *)&(((type *)NULL)->member))))

extern bool list_member(const link_t *link, const link_t *head);
extern void list_concat(link_t *head1, link_t *head2);

#endif

/** @}
 */
