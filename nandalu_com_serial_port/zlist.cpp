/*  =========================================================================
    zlist - generic type-free list container

    -------------------------------------------------------------------------
    Copyright (c) 1991-2012 iMatix Corporation <www.imatix.com>
    Copyright other contributors as noted in the AUTHORS file.

    This file is part of CZMQ, the high-level C binding for 0MQ:
    http://czmq.zeromq.org.

    This is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or (at
    your option) any later version.

    This software is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this program. If not, see
    <http://www.gnu.org/licenses/>.
    =========================================================================
*/

/*
@header
    Provides a generic container implementing a fast singly-linked list. You
    can use this to construct multi-dimensional lists, and other structures
    together with other generic containers like zhash.
@discuss
@end
*/

#include <stdlib.h>
#include <assert.h>
#include "zlist.h"

//  List node, used internally only

typedef struct _node_t {
    struct _node_t
        *next;
    void
        *item;
} node_t;

//  Actual list object

struct _zlist {
    node_t
        *head, *tail;
    node_t
        *cursor;
    size_t
        size;
};


//  --------------------------------------------------------------------------
//  List constructor

zlist_t *
zlist_new (void)
{
    zlist_t *self = (zlist_t *) malloc (sizeof (zlist_t));
	self->head = NULL;
	self->tail = NULL;
	self->cursor =NULL;
	self->size = 0;
    return self;
}


//  --------------------------------------------------------------------------
//  List destructor

void
zlist_destroy (zlist_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        zlist_t *self = *self_p;
        node_t *node, *next;
        for (node = (*self_p)->head; node != NULL; node = next) {
            next = node->next;
            free (node);
        }
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Return the item at the head of list. If the list is empty, returns NULL.
//  Leaves cursor pointing at the head item, or NULL if the list is empty.

void *
zlist_first (zlist_t *self)
{
    assert (self);
    self->cursor = self->head;
    if (self->cursor)
        return self->cursor->item;
    else
        return NULL;
}

//  --------------------------------------------------------------------------
//  Return the item at the tail of list. If the list is empty, returns NULL.
//  Leaves cursor pointing at the tail item, or NULL if the list is empty.

void *
zlist_last (zlist_t *self)
{
    assert (self);
    self->cursor = self->tail;
    if (self->cursor)
        return self->cursor->item;
    else
        return NULL;
}


//  --------------------------------------------------------------------------
//  Return the item at the head of list. If the list is empty, returns NULL.
//  Leaves cursor as-is.

void *
zlist_head (zlist_t *self)
{
    assert (self);
    return self->head
      ? self->head->item
      : NULL;
}


//  --------------------------------------------------------------------------
//  Return the item at the tail of list. If the list is empty, returns NULL.
//  Leaves cursor as-is.

void *
zlist_tail (zlist_t *self)
{
    assert (self);
    return self->tail
      ? self->tail->item
      : NULL;
}

//  --------------------------------------------------------------------------
//  Return the next item. If the list is empty, returns NULL. To move to
//  the start of the list call zlist_first(). Advances the cursor.

void *
zlist_next (zlist_t *self)
{
    assert (self);
    if (self->cursor)
        self->cursor = self->cursor->next;
    else
        self->cursor = self->head;
    if (self->cursor)
        return self->cursor->item;
    else
        return NULL;
}


//  --------------------------------------------------------------------------
//  Add item to the end of the list

int
zlist_append (zlist_t *self, void *item)
{
    node_t *node;
    node = (node_t *) malloc (sizeof (node_t));
    if (!node)
        return -1;

    node->item = item;
    if (self->tail)
        self->tail->next = node;
    else
        self->head = node;
    self->tail = node;
    node->next = NULL;
    self->size++;
    self->cursor = NULL;

    return 0;
}


//  --------------------------------------------------------------------------
//  Insert item at the beginning of the list

int
zlist_push (zlist_t *self, void *item)
{
    node_t *node;
    node = (node_t *) malloc (sizeof (node_t));
    if (!node)
        return -1;

    node->item = item;
    node->next = self->head;
    self->head = node;
    if (self->tail == NULL)
        self->tail = node;
    self->size++;
    self->cursor = NULL;
    return 0;
}


//  --------------------------------------------------------------------------
//  Remove item from the beginning of the list, returns NULL if none

void *
zlist_pop (zlist_t *self)
{
    node_t *node = self->head;
    void *item = NULL;
    if (node) {
        item = node->item;
        self->head = node->next;
        if (self->tail == node)
            self->tail = NULL;
        free (node);
        self->size--;
    }
    self->cursor = NULL;
    return item;
}


//  --------------------------------------------------------------------------
//  Remove the item from the list, if present. Safe to call on items that
//  are not in the list.

void
zlist_remove (zlist_t *self, void *item)
{
    node_t *node, *prev = NULL;

    //  First off, we need to find the list node
    for (node = self->head; node != NULL; node = node->next) {
        if (node->item == item)
            break;
        prev = node;
    }
    if (node) {
        if (prev)
            prev->next = node->next;
        else
            self->head = node->next;

        if (node->next == NULL)
            self->tail = prev;

        free (node);
        self->size--;
        self->cursor = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Make copy of itself

zlist_t *
zlist_copy (zlist_t *self)
{
    if (!self)
        return NULL;

    zlist_t *copy = zlist_new ();
    if (copy) {
        node_t *node;
        for (node = self->head; node; node = node->next) {
            if (!zlist_append (copy, node->item)) {
                zlist_destroy(&copy);
                break;
            }
        }
    }
    return copy;
}


//  --------------------------------------------------------------------------
//  Return the number of items in the list

size_t
zlist_size (zlist_t *self)
{
    return self->size;
}


//  --------------------------------------------------------------------------
//  Sort list
//  Uses a comb sort, which is simple and reasonably fast
//  Algorithm based on Wikipedia C pseudo-code

void
zlist_sort (zlist_t *self, zlist_compare_fn *compare)
{
    size_t gap = self->size;
    bool swapped = false;
    while (gap > 1 || swapped) {
        if (gap > 1)
            gap = (size_t) ((double) gap / 1.247330950103979);
        
        node_t *base = self->head;
        node_t *test = self->head;
        int jump = gap;
        while (jump--)
            test = test->next;
        
        swapped = false;
        while (base && test) {
            if ((*compare) (base->item, test->item)) {
                //  It's trivial to swap items in a generic container
                void *item = base->item;
                base->item = test->item;
                test->item = item;
                swapped = true;
            }
            base = base->next;
            test = test->next;
        }
    }
}