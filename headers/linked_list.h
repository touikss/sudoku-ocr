#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "../headers/struct.h"

typedef struct Node_T
{
    void *value;
    struct Node_T *next;
    struct Node_T *prev;
} Node;

typedef struct
{
    Node *head;
    Node *tail;
    size_t length;
} MyList;

void *Line_tovptr(Line line);
void *Square_tovptr(Square square);


Node *initializeNode(void *value);
void initializeMyList(MyList *list, size_t nb, void *value);


Node *get_node(MyList *list, size_t index);
void *get_value(MyList *list, size_t index);


void append(MyList *list, void *value);
void prepend(MyList *list, void *value);
void insert(MyList *list, size_t index, void *value);


void *prepop(MyList *list);
void *pop(MyList *list);
void *removeAt(MyList *list, size_t index);


void free_node(Node *node);
void free_list(MyList *list);


void printNode(Node node);

void printMyList(MyList *list);

#endif