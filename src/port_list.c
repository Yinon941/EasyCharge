/* =========================================================
 * port_list.c
 * Implementation of the charging-port linked list ADT.
 * ========================================================= */

#include "port_list.h"

/* Convert PortType enum to a display string */
static const char *portTypeStr(PortType t) {
    switch (t) {
        case FAST: return "FAST";
        case MID:  return "MID";
        case SLOW: return "SLOW";
        default:   return "UNKNOWN";
    }
}

/* Convert status int to a display string */
static const char *statusStr(int s) {
    switch (s) {
        case 1:  return "Occupied";
        case 2:  return "Free";
        case 3:  return "Out of order";
        default: return "Unknown";
    }
}

/* Allocate a new Port, initialise all fields */
Port *createPort(int num, PortType portType, int status) {
    Port *p = (Port *)malloc(sizeof(Port));
    if (!p) {
        fprintf(stderr, "Error: memory allocation failed in createPort.\n");
        return NULL;
    }
    p->num      = num;
    p->portType = portType;
    p->status   = status;
    p->p2car    = NULL;
    memset(&p->tin, 0, sizeof(Date));
    p->next     = NULL;
    return p;
}

/* Append to end of list */
void addPort(Port **head, Port *newPort) {
    if (*head == NULL) {
        *head = newPort;
        return;
    }
    Port *curr = *head;
    while (curr->next != NULL)
        curr = curr->next;
    curr->next = newPort;
}

/* Remove port with given number and free it */
int removePort(Port **head, int portNum) {
    Port *curr = *head;
    Port *prev = NULL;

    while (curr != NULL) {
        if (curr->num == portNum) {
            if (prev == NULL)
                *head = curr->next;
            else
                prev->next = curr->next;
            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;
}

/* Find first free port matching portType */
Port *findFreePort(Port *head, PortType portType) {
    Port *curr = head;
    while (curr != NULL) {
        if (curr->portType == portType && curr->status == 2)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

/* Find port by number */
Port *findPortByNum(Port *head, int portNum) {
    Port *curr = head;
    while (curr != NULL) {
        if (curr->num == portNum)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

/* Count ports matching the given status.
 * Use status = -1 to count all ports. */
int countPortsByStatus(Port *head, int status) {
    int   count = 0;
    Port *curr  = head;
    while (curr != NULL) {
        if (status == -1 || curr->status == status)
            count++;
        curr = curr->next;
    }
    return count;
}

/* Count working ports (status != 3) of a given type */
int countPortsByType(Port *head, PortType portType) {
    int   count = 0;
    Port *curr  = head;
    while (curr != NULL) {
        if (curr->portType == portType && curr->status != 3)
            count++;
        curr = curr->next;
    }
    return count;
}

/* Free all Port nodes in the list */
void freePorts(Port **head) {
    Port *curr = *head;
    while (curr != NULL) {
        Port *tmp = curr->next;
        free(curr);
        curr = tmp;
    }
    *head = NULL;
}

/* Print the ports list in a readable format */
void printPorts(Port *head) {
    Port *curr = head;
    if (curr == NULL) {
        printf("  (no ports)\n");
        return;
    }
    while (curr != NULL) {
        printf("  Port #%d | Type: %-4s | Status: %s",
               curr->num, portTypeStr(curr->portType), statusStr(curr->status));
        if (curr->status == 1 && curr->p2car != NULL)
            printf(" | Car: %s", curr->p2car->nLicense);
        printf("\n");
        curr = curr->next;
    }
}
