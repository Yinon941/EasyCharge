/* =========================================================
 * queue.c
 * Implementation of the car waiting queue ADT.
 * ========================================================= */

#include "queue.h"

/* Initialize queue pointers to NULL */
void initQueue(qCar *q) {
    q->front = NULL;
    q->rear  = NULL;
}

/* Allocate a new carNode and append it to the rear */
int enqueue(qCar *q, Car *car) {
    carNode *node = (carNode *)malloc(sizeof(carNode));
    if (!node) {
        fprintf(stderr, "Error: memory allocation failed in enqueue.\n");
        return 0;
    }
    node->pCar = car;
    node->next = NULL;

    if (q->rear == NULL) {
        q->front = node;
        q->rear  = node;
    } else {
        q->rear->next = node;
        q->rear       = node;
    }
    return 1;
}

/* Remove the front node and return its Car pointer */
Car *dequeue(qCar *q) {
    if (q->front == NULL)
        return NULL;

    carNode *tmp = q->front;
    Car     *car = tmp->pCar;

    q->front = q->front->next;
    if (q->front == NULL)
        q->rear = NULL;

    free(tmp);
    return car;
}

/* Peek at the front car without removing it */
Car *peekQueue(const qCar *q) {
    return (q->front != NULL) ? q->front->pCar : NULL;
}

/* Return 1 if queue is empty */
int isQueueEmpty(const qCar *q) {
    return (q->front == NULL);
}

/* Return 1-based position of the car (matching portType) in the queue.
 * Only cars whose portType matches are counted for the position index. */
int posInQueue(const qCar *q, const char *license, PortType portType) {
    int       pos  = 0;
    carNode  *curr = q->front;

    while (curr != NULL) {
        if (curr->pCar->portType == portType) {
            pos++;
            if (strcmp(curr->pCar->nLicense, license) == 0)
                return pos;
        }
        curr = curr->next;
    }
    return 0; /* not found */
}

/* Remove a car by license from anywhere in the queue.
 * Adjusts front/rear pointers as needed. */
int removeFromQueue(qCar *q, const char *license) {
    carNode *curr = q->front;
    carNode *prev = NULL;

    while (curr != NULL) {
        if (strcmp(curr->pCar->nLicense, license) == 0) {
            if (prev == NULL)
                q->front = curr->next;   /* removing front */
            else
                prev->next = curr->next;

            if (curr->next == NULL)
                q->rear = prev;          /* removing rear */

            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    return 0; /* car not found in queue */
}

/* Free all carNode allocations (Car objects belong to the BST, not freed here) */
void freeQueue(qCar *q) {
    carNode *curr = q->front;
    while (curr != NULL) {
        carNode *tmp = curr->next;
        free(curr);
        curr = tmp;
    }
    q->front = NULL;
    q->rear  = NULL;
}

/* Count cars in the queue that match the given portType */
int countQueueByType(const qCar *q, PortType portType) {
    int      count = 0;
    carNode *curr  = q->front;
    while (curr != NULL) {
        if (curr->pCar->portType == portType)
            count++;
        curr = curr->next;
    }
    return count;
}
