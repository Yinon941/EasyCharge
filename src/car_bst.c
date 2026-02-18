/* =========================================================
 * car_bst.c
 * Implementation of the cars Binary Search Tree ADT.
 * Key = nLicense (string, lexicographic comparison).
 * ========================================================= */

#include "car_bst.h"

/* ---- Internal helpers ---- */

/* Allocate a new tCar node wrapping a Car */
static tCar *newNode(Car *car) {
    tCar *node = (tCar *)malloc(sizeof(tCar));
    if (!node) {
        fprintf(stderr, "Error: memory allocation failed in car BST.\n");
        return NULL;
    }
    node->pCar  = car;
    node->left  = NULL;
    node->right = NULL;
    return node;
}

/* Recursive insert; returns updated subtree root */
static tCar *insertRec(tCar *root, Car *car, Car **result) {
    if (root == NULL) {
        tCar *node = newNode(car);
        *result = (node != NULL) ? car : NULL;
        return node;
    }
    int cmp = strcmp(car->nLicense, root->pCar->nLicense);
    if (cmp < 0)
        root->left  = insertRec(root->left,  car, result);
    else if (cmp > 0)
        root->right = insertRec(root->right, car, result);
    else
        *result = root->pCar; /* already exists, return existing */
    return root;
}

/* Recursive search */
static Car *searchRec(tCar *root, const char *license) {
    if (root == NULL)
        return NULL;
    int cmp = strcmp(license, root->pCar->nLicense);
    if (cmp == 0)  return root->pCar;
    if (cmp < 0)   return searchRec(root->left,  license);
    return             searchRec(root->right, license);
}

/* Find the in-order successor (leftmost node in right subtree) */
static tCar *minNode(tCar *node) {
    while (node->left != NULL)
        node = node->left;
    return node;
}

/* Recursive delete; frees the tCar node and the Car object */
static tCar *removeRec(tCar *root, const char *license, int *found) {
    if (root == NULL) return NULL;

    int cmp = strcmp(license, root->pCar->nLicense);
    if (cmp < 0) {
        root->left  = removeRec(root->left,  license, found);
    } else if (cmp > 0) {
        root->right = removeRec(root->right, license, found);
    } else {
        *found = 1;
        if (root->left == NULL) {
            tCar *tmp = root->right;
            free(root->pCar);
            free(root);
            return tmp;
        } else if (root->right == NULL) {
            tCar *tmp = root->left;
            free(root->pCar);
            free(root);
            return tmp;
        } else {
            /* Two children: swap Car pointers instead of copying data.
             * This preserves all cross-references (port->p2car, queue nodes)
             * that point to the original Car objects. */
            tCar *succ      = minNode(root->right);
            Car  *carDelete = root->pCar;   /* car to remove (pPort==NULL, inqueue==0) */
            root->pCar      = succ->pCar;   /* root now holds successor's Car */
            succ->pCar      = carDelete;    /* successor node holds the car to delete */
            /* Delete the successor node (now holding carDelete, safe to free) */
            root->right = removeRec(root->right, root->pCar->nLicense, found);
        }
    }
    return root;
}

/* In-order traversal collecting cars for top-customers */
static void collectTop(tCar *root, Car *top[TOP_CUSTOMERS]) {
    if (root == NULL) return;
    collectTop(root->left, top);

    /* Insert pCar into top[] keeping it sorted descending by totalPayed */
    Car *candidate = root->pCar;
    int  i;
    for (i = 0; i < TOP_CUSTOMERS; i++) {
        if (top[i] == NULL || candidate->totalPayed > top[i]->totalPayed) {
            /* Shift down */
            int j;
            for (j = TOP_CUSTOMERS - 1; j > i; j--)
                top[j] = top[j - 1];
            top[i] = candidate;
            break;
        }
    }

    collectTop(root->right, top);
}

/* In-order print helper */
static void printRec(tCar *root) {
    if (root == NULL) return;
    printRec(root->left);
    printf("  License: %-10s | Type: %s | Paid: %.2f NIS | %s\n",
           root->pCar->nLicense,
           root->pCar->portType == FAST ? "FAST" :
           root->pCar->portType == MID  ? "MID"  : "SLOW",
           root->pCar->totalPayed,
           root->pCar->inqueue  ? "In queue" :
           root->pCar->pPort   != NULL ? "Charging" : "Not at station");
    printRec(root->right);
}

/* Recursive free */
static void freeRec(tCar *root) {
    if (root == NULL) return;
    freeRec(root->left);
    freeRec(root->right);
    free(root->pCar);
    free(root);
}

/* ---- Public API ---- */

void initCarBST(CarBST *bst) {
    bst->root = NULL;
}

Car *insertCar(CarBST *bst, const char *license, PortType portType, double totalPayed) {
    Car *car = (Car *)malloc(sizeof(Car));
    if (!car) {
        fprintf(stderr, "Error: memory allocation failed for Car.\n");
        return NULL;
    }
    strncpy(car->nLicense, license, LICENSE_LEN - 1);
    car->nLicense[LICENSE_LEN - 1] = '\0';
    car->portType   = portType;
    car->totalPayed = totalPayed;
    car->pPort      = NULL;
    car->inqueue    = 0;

    Car *result = NULL;
    bst->root = insertRec(bst->root, car, &result);

    /* If the license already existed, discard the newly allocated Car */
    if (result != car)
        free(car);

    return result;
}

Car *searchCar(CarBST *bst, const char *license) {
    return searchRec(bst->root, license);
}

int removeCar(CarBST *bst, const char *license) {
    int found = 0;
    bst->root = removeRec(bst->root, license, &found);
    return found;
}

void getTopCustomers(CarBST *bst, Car *top[TOP_CUSTOMERS]) {
    int i;
    for (i = 0; i < TOP_CUSTOMERS; i++)
        top[i] = NULL;
    collectTop(bst->root, top);
}

void printCarBST(CarBST *bst) {
    printRec(bst->root);
}

void freeCarBST(CarBST *bst) {
    freeRec(bst->root);
    bst->root = NULL;
}
