/* =========================================================
 * station_bst.c
 * Implementation of the stations Binary Search Tree ADT.
 * Key = id (integer).
 * ========================================================= */

#include "station_bst.h"
#include "port_list.h"
#include "queue.h"

/* ---- Internal helpers ---- */

/* Euclidean distance between two points */
static double euclidDist(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}

/* Allocate a new Station node */
static Station *allocStation(int id, const char *name, int nPorts, double x, double y) {
    Station *st = (Station *)malloc(sizeof(Station));
    if (!st) {
        fprintf(stderr, "Error: memory allocation failed for Station.\n");
        return NULL;
    }
    st->id        = id;
    st->name      = strdup(name);
    if (!st->name) {
        free(st);
        return NULL;
    }
    st->nPorts    = nPorts;
    st->coord.x   = x;
    st->coord.y   = y;
    st->portsList = NULL;
    st->nCars     = 0;
    initQueue(&st->carQueue);
    st->left      = NULL;
    st->right     = NULL;
    return st;
}

/* Recursive insert; returns updated subtree root */
static Station *insertRec(Station *root, Station *newSt, Station **result) {
    if (root == NULL) {
        *result = newSt;
        return newSt;
    }
    if (newSt->id < root->id)
        root->left  = insertRec(root->left,  newSt, result);
    else if (newSt->id > root->id)
        root->right = insertRec(root->right, newSt, result);
    else
        *result = NULL; /* duplicate id */
    return root;
}

/* Recursive search by id */
static Station *searchByIDRec(Station *root, int id) {
    if (root == NULL)      return NULL;
    if (id == root->id)    return root;
    if (id < root->id)     return searchByIDRec(root->left,  id);
    return                        searchByIDRec(root->right, id);
}

/* In-order traversal to find station by name */
static Station *searchByNameRec(Station *root, const char *name) {
    if (root == NULL) return NULL;
    Station *found = searchByNameRec(root->left, name);
    if (found) return found;
    if (strcmp(root->name, name) == 0) return root;
    return searchByNameRec(root->right, name);
}

/* In-order traversal to find nearest station */
static void findNearestRec(Station *root, double x, double y,
                           Station **best, double *bestDist) {
    if (root == NULL) return;
    findNearestRec(root->left, x, y, best, bestDist);

    double d = euclidDist(x, y, root->coord.x, root->coord.y);
    if (*best == NULL || d < *bestDist) {
        *best     = root;
        *bestDist = d;
    }

    findNearestRec(root->right, x, y, best, bestDist);
}

/* Find the in-order successor (leftmost in right subtree) */
static Station *minStation(Station *node) {
    while (node->left != NULL)
        node = node->left;
    return node;
}

/* Recursive remove */
static Station *removeRec(Station *root, int id, int *found) {
    if (root == NULL) return NULL;

    if (id < root->id) {
        root->left  = removeRec(root->left,  id, found);
    } else if (id > root->id) {
        root->right = removeRec(root->right, id, found);
    } else {
        *found = 1;
        if (root->left == NULL) {
            Station *tmp = root->right;
            freePorts(&root->portsList);
            freeQueue(&root->carQueue);
            free(root->name);
            free(root);
            return tmp;
        } else if (root->right == NULL) {
            Station *tmp = root->left;
            freePorts(&root->portsList);
            freeQueue(&root->carQueue);
            free(root->name);
            free(root);
            return tmp;
        } else {
            /* Replace with in-order successor data */
            Station *succ = minStation(root->right);
            root->id = succ->id;

            free(root->name);
            root->name = strdup(succ->name);

            root->nPorts  = succ->nPorts;
            root->coord   = succ->coord;
            /* swap port lists and queues */
            freePorts(&root->portsList);
            root->portsList = succ->portsList;
            succ->portsList = NULL;
            freeQueue(&root->carQueue);
            root->carQueue = succ->carQueue;
            initQueue(&succ->carQueue);
            root->nCars = succ->nCars;

            root->right = removeRec(root->right, succ->id, found);
        }
    }
    return root;
}

/* In-order traversal callback helper */
static void traverseRec(Station *root, void (*callback)(Station *)) {
    if (root == NULL) return;
    traverseRec(root->left, callback);
    callback(root);
    traverseRec(root->right, callback);
}

/* Count helper */
static int countRec(Station *root) {
    if (root == NULL) return 0;
    return 1 + countRec(root->left) + countRec(root->right);
}

/* Recursive free */
static void freeRec(Station *root) {
    if (root == NULL) return;
    freeRec(root->left);
    freeRec(root->right);
    freePorts(&root->portsList);
    freeQueue(&root->carQueue);
    free(root->name);
    free(root);
}

/* ---- Public API ---- */

void initStationBST(StationBST *bst) {
    bst->root = NULL;
}

Station *insertStation(StationBST *bst, int id, const char *name,
                       int nPorts, double x, double y) {
    Station *st = allocStation(id, name, nPorts, x, y);
    if (!st) return NULL;

    Station *result = NULL;
    bst->root = insertRec(bst->root, st, &result);

    if (result == NULL) {
        /* duplicate id */
        free(st->name);
        free(st);
    }
    return result;
}

Station *searchStationByID(StationBST *bst, int id) {
    return searchByIDRec(bst->root, id);
}

Station *searchStationByName(StationBST *bst, const char *name) {
    return searchByNameRec(bst->root, name);
}

Station *searchStationByDist(StationBST *bst, double x, double y) {
    Station *best     = NULL;
    double   bestDist = 0.0;
    findNearestRec(bst->root, x, y, &best, &bestDist);
    return best;
}

int removeStation(StationBST *bst, int id) {
    int found = 0;
    bst->root = removeRec(bst->root, id, &found);
    return found;
}

void traverseStations(StationBST *bst, void (*callback)(Station *st)) {
    traverseRec(bst->root, callback);
}

int countStations(StationBST *bst) {
    return countRec(bst->root);
}

void freeStationBST(StationBST *bst) {
    freeRec(bst->root);
    bst->root = NULL;
}
