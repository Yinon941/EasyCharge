#ifndef QUEUE_H
#define QUEUE_H

/* =========================================================
 * queue.h
 * ADT interface for the car waiting queue (qCar).
 * Each station holds one qCar that manages carNode elements.
 * ========================================================= */

#include "types.h"

/* Initialize a queue to empty state */
void  initQueue(qCar *q);

/* Add a car to the rear of the queue */
int   enqueue(qCar *q, Car *car);

/* Remove and return the car at the front of the queue.
 * Returns NULL if the queue is empty. */
Car  *dequeue(qCar *q);

/* Return the front car without removing it (NULL if empty) */
Car  *peekQueue(const qCar *q);

/* Return 1 if the queue is empty, 0 otherwise */
int   isQueueEmpty(const qCar *q);

/* Return the position (1-based) of a car with the given license
 * in the portion of the queue that matches portType.
 * Returns 0 if the car is not found. */
int   posInQueue(const qCar *q, const char *license, PortType portType);

/* Remove a specific car (by license) from anywhere in the queue.
 * Returns 1 on success, 0 if not found. */
int   removeFromQueue(qCar *q, const char *license);

/* Free all carNode allocations in the queue (does NOT free the Car objects) */
void  freeQueue(qCar *q);

/* Count how many cars in the queue match the given portType */
int   countQueueByType(const qCar *q, PortType portType);

#endif /* QUEUE_H */
