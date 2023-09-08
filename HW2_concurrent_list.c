#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "concurrent_list.h"

struct node
{
    int value;
    node* next;
    pthread_mutex_t lock;
};

struct list
{
    node* head;
    pthread_mutex_t lock;
};

void print_node(node* node)
{
    if (node)
    {
        printf("%d ", node->value);
    }
}

list* create_list()
{
    list* newList = (list*)malloc(sizeof(list));
    if (newList == NULL)
        exit;
    newList->head = NULL;
    pthread_mutex_init(&newList->lock, NULL);
    return newList; // REPLACE WITH YOUR OWN CODE
}

void delete_list(list* list)
{
    if (list == NULL) exit;
    node* toDelete = list->head;
    node* next = NULL;
    while (toDelete)
    {
        next = toDelete->next;
        pthread_mutex_destroy(&toDelete->lock);
        free(toDelete);
        toDelete = next;
    }
    pthread_mutex_destroy(&list->lock);
    free(list);
    return;
}

void insert_value(list* list, int value)
{
    if (list == NULL) exit;

    node* current = list->head;
    node* prev = NULL;

    node* newnode = (node*)malloc(sizeof(list));
    if (newnode == NULL)    exit;
    pthread_mutex_init(&newnode->lock, NULL);
    newnode->next = NULL;
    newnode->value = value;

    if (current != NULL)//if head == NULL
        pthread_mutex_lock(&current->lock);
    else        //here we want to insert the head
    {
        pthread_mutex_lock(&list->lock);
        list->head = newnode;
        pthread_mutex_unlock(&list->lock);
        return;
    }

    while (current != NULL && current->value < value)
    {
        prev = current;
        current = current->next;
        if (current != NULL)
            pthread_mutex_lock(&current->lock);
        if (current != NULL && current->value < value)
            pthread_mutex_unlock(&prev->lock);
    }
    newnode->next = current;
    if (prev != NULL)
    {
        prev->next = newnode;
        pthread_mutex_unlock(&prev->lock);
    }
    if (current != NULL)
    {
        if (prev == NULL)
        {
            pthread_mutex_lock(&list->lock);
            list->head = newnode;
            pthread_mutex_unlock(&list->lock);
        }

        pthread_mutex_unlock(&current->lock);
    }
    return;
}

void remove_value(list* list, int value)
{
    if (list == NULL) exit;

    node* current = list->head;
    node* prev = NULL;

    if (current == NULL)return;
    pthread_mutex_lock(&current->lock);//head

    if (current->value == value)//if we want to remove the head
    {
        pthread_mutex_lock(&list->lock);
        if (current->next)
            pthread_mutex_lock(&current->next->lock);
        list->head = current->next;
        if (list->head)
            pthread_mutex_unlock(&list->head->lock);
        pthread_mutex_unlock(&list->lock);
        //here we want to delete
        pthread_mutex_unlock(&current->lock);
        pthread_mutex_destroy(&current->lock);
        free(current);
        return;
    }
    while (current != NULL)
    {
        prev = current;
        current = current->next;
        if (current != NULL)
            pthread_mutex_lock(&current->lock);
        if (current != NULL && current->value != value)
            pthread_mutex_unlock(&prev->lock);
        if (current != NULL && current->value == value)
        {
            if (current->next)
                pthread_mutex_lock(&current->next->lock);
            prev->next = current->next;
            if (current->next)
                pthread_mutex_unlock(&current->next->lock);
            pthread_mutex_unlock(&prev->lock);
            //delete the current
            pthread_mutex_unlock(&current->lock);
            pthread_mutex_destroy(&current->lock);
            free(current);
            return;
        }
    }
    pthread_mutex_unlock(&current->lock);
    pthread_mutex_unlock(&prev->lock);
    return;
}

void print_list(list* list)
{
    if ( !list || !list->head)
    {
        printf("\n");
        return;
    }
    node* current = list->head;
    node* prev = NULL;
    pthread_mutex_lock(&current->lock);

    while (current)
    {
        print_node(current);
        prev = current;
        current = current->next;
        if (current)
            pthread_mutex_lock(&current->lock);
        pthread_mutex_unlock(&prev->lock);//already printed
    }

    printf("\n"); // DO NOT DELETE
}

void count_list(list* list, int (*predicate)(int))
{
    int count = 0; // DO NOT DELETE
    node* current = list->head;
    node* prev = NULL;

    while (current)
    {
        count += predicate(current->value);
        prev = current;
        current = current->next;
        if (current)
            pthread_mutex_lock(&current->lock);
        pthread_mutex_unlock(&prev->lock);//already printed
    }

    printf("%d items were counted\n", count); // DO NOT DELETE
}
