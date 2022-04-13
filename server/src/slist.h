#ifndef __SLIST_H__

#define __SLIST_H__

#include <stddef.h>

typedef struct _SNode {
    void *data;
    struct _SNode* next;
} SNode;
typedef SNode* SList;

/*
 * Returns a new list (NULL).
 */
SList slist_new();

/*
 * Adds an element in front of the list.
 */
SList slist_push_front(SList list, void *data);

#endif // __SLIST_H_