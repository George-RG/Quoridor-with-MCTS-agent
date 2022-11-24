#include<stdio.h>
#include <stdlib.h>

#include "List.h"

typedef struct _node
{
    Pointer value;
    struct _node* Link;
}ListNode;

typedef struct _list
{
    int count;
    ListNode* first;
    CompareFunc compare;
    DestroyFunc destroy;
}List;

ListPtr CreateList(CompareFunc compare,DestroyFunc destroy)
{
    ListPtr ls = malloc(sizeof(List));
    ls->count=0;
    ls->first=NULL;
    ls->compare=compare;
    ls->destroy = destroy;
    return ls;
}

int ListEmpty(ListPtr ls)
{
    return(ls->count == 0);
}

int ListSize(ListPtr ls)
{
    return ls->count;
}

void ListInsert(Pointer Item, ListPtr ls)
{
    ls->count++;

    ListNode* temp = ls->first;
    ls->first = malloc(sizeof(ListNode));
    ls->first->value=Item;
    ls->first->Link=temp;
}

Pointer ListRemove_nth(ListPtr ls, int N)
{
    Pointer temp;
    ListNode* cursor=ls->first;
    ListNode* temp_free;
    int i;

    if(N<=1)
    {
        temp = ls->first->value;
        temp_free = ls->first;
        ls->first=ls->first->Link;
        free(temp_free);
        ls->count--;
        return (temp);
    }

    for(i=0;i<N-2;i++)
    {
        cursor = cursor->Link;
        if(cursor->Link==NULL)
            break;
    }

    temp_free = cursor->Link;
    temp = cursor->Link->value;
    cursor->Link = cursor->Link->Link;
    free(temp_free);

    ls->count--;

    return temp;
}

int ListFind(ListPtr ls, Pointer value)
{
    int i;
    ListNode* cursor=ls->first;

    for(i=0;i<ls->count;i++)
    {   
        if(!ls->compare(cursor->value, value))
            return i+1;
    
        cursor=cursor->Link;
    }
    return -1;
}


void freeList(ListPtr ls)
{
    ListNode* temp;
    while(ls->first != NULL)
    {
        temp = ls->first;
        ls->first = ls->first->Link;
        if(ls->destroy!=NULL)
            ls->destroy(temp->value);
        free(temp);
    }
    free(ls);
}

Pointer ListGetNth(ListPtr ls, int N)
{
    ListNode* temp = ls->first;
    if(ls->first == NULL)
        return NULL;

    for(int i=1; i<N; i++)
    {
        if(temp == NULL)
            return NULL;

        temp = temp->Link;
    }
    return temp->value;
}

void ListToArray(ListPtr ls, Pointer* array)
{
    ListNode* temp = ls->first;
    for(int i=0; i<ls->count; i++)
    {
        array[i] = temp->value;
        temp = temp->Link;
    }
}
