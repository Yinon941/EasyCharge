#ifndef STATION_BST_H
#define STATION_BST_H

/* =========================================================
 * station_bst.h
 * ADT interface for the Binary Search Tree of stations.
 * The BST is ordered by station id (integer).
 * ========================================================= */

#include "types.h"

/* Initialise the manager (root = NULL) */
void     initStationBST(StationBST *bst);

/* Allocate and insert a new Station into the BST.
 * name is duplicated dynamically.
 * Returns pointer to the new Station, or NULL on failure / duplicate id. */
Station *insertStation(StationBST *bst, int id, const char *name,
                       int nPorts, double x, double y);

/* Search by id – returns pointer to Station or NULL */
Station *searchStationByID(StationBST *bst, int id);

/* Search by name (case-sensitive) – linear in-order scan.
 * Returns pointer to Station or NULL. */
Station *searchStationByName(StationBST *bst, const char *name);

/* Search for the station with minimum Euclidean distance from (x, y).
 * Returns pointer to the nearest Station, or NULL if BST is empty. */
Station *searchStationByDist(StationBST *bst, double x, double y);

/* Remove a station from the BST.
 * Frees the Station node, its name, all ports, and its car queue.
 * Returns 1 on success, 0 if not found. */
int      removeStation(StationBST *bst, int id);

/* In-order traversal: call callback for each station */
void     traverseStations(StationBST *bst, void (*callback)(Station *st));

/* Count total stations in the BST */
int      countStations(StationBST *bst);

/* Free the entire BST (stations, ports, queues, names) */
void     freeStationBST(StationBST *bst);

#endif /* STATION_BST_H */
