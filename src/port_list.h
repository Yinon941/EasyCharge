#ifndef PORT_LIST_H
#define PORT_LIST_H

/* =========================================================
 * port_list.h
 * ADT interface for the singly-linked list of charging ports.
 * Each Station holds one portsList (head pointer to Port).
 * ========================================================= */

#include "types.h"

/* Allocate and initialise a new Port node.
 * Returns NULL on allocation failure. */
Port *createPort(int num, PortType portType, int status);

/* Append a port to the end of the list rooted at *head.
 * Updates *head when the list was previously empty. */
void  addPort(Port **head, Port *newPort);

/* Remove the port with the given number from the list.
 * Frees the Port node. Returns 1 on success, 0 if not found. */
int   removePort(Port **head, int portNum);

/* Find the first FREE port (status == 2) that matches portType.
 * Returns a pointer to it, or NULL if none found. */
Port *findFreePort(Port *head, PortType portType);

/* Find a port by its number. Returns pointer or NULL. */
Port *findPortByNum(Port *head, int portNum);

/* Count ports by status: pass 1 for occupied, 2 for free, 3 for out-of-order.
 * Pass -1 to count all ports regardless of status. */
int   countPortsByStatus(Port *head, int status);

/* Count ports by type (FAST / MID / SLOW).
 * Only counts ports that are NOT out-of-order (status != 3). */
int   countPortsByType(Port *head, PortType portType);

/* Free the entire ports linked list.
 * Does NOT free the Car objects pointed to by p2car. */
void  freePorts(Port **head);

/* Print all ports in the list */
void  printPorts(Port *head);

#endif /* PORT_LIST_H */
