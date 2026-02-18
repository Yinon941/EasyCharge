#ifndef CAR_BST_H
#define CAR_BST_H

/* =========================================================
 * car_bst.h
 * ADT interface for the Binary Search Tree of cars (customers).
 * The BST is ordered by nLicense (lexicographic order).
 * ========================================================= */

#include "types.h"

/* Initialise a CarBST manager (sets root to NULL) */
void  initCarBST(CarBST *bst);

/* Allocate a new Car and insert it into the BST.
 * Returns a pointer to the new Car on success, NULL on failure.
 * If a car with the same license already exists, returns the existing Car. */
Car  *insertCar(CarBST *bst, const char *license, PortType portType, double totalPayed);

/* Search for a car by license number.
 * Returns pointer to the Car, or NULL if not found. */
Car  *searchCar(CarBST *bst, const char *license);

/* Remove a car from the BST and free the tCar node and Car object.
 * Returns 1 on success, 0 if not found. */
int   removeCar(CarBST *bst, const char *license);

/* Fill the top[] array with pointers to the TOP_CUSTOMERS cars
 * that have the highest totalPayed. Remaining slots are NULL.
 * Uses in-order traversal with an insertion-sort approach. */
void  getTopCustomers(CarBST *bst, Car *top[TOP_CUSTOMERS]);

/* In-order traversal: print all cars */
void  printCarBST(CarBST *bst);

/* Free the entire BST (nodes and Car objects).
 * Should only be called at system shutdown. */
void  freeCarBST(CarBST *bst);

#endif /* CAR_BST_H */
