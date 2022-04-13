#include <stdlib.h>

#include "slist.h"


SList slist_new() {
    return NULL;
}

SList slist_push_front(SList list, void *data) {
    SList newItem = malloc(sizeof(SNode));
    newItem->data = data;
    newItem->next = list;

    return newItem;
}