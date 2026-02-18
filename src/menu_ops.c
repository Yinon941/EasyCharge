/* =========================================================
 * menu_ops.c
 * Implementation of all 14 menu operations and helper functions.
 * ========================================================= */

#include "menu_ops.h"
#include "port_list.h"
#include "queue.h"
#include "file_io.h"

/* =========================================================
 * Internal helpers
 * ========================================================= */

/* Return elapsed minutes between two Date structs.
 * Simple approximation: only considers the time fields. */
static double minutesBetween(Date start, Date end) {
    /* Convert both dates to minutes since year 0 */
    long startMins = (long)start.year * 525600L +
                     (long)start.month * 43800L +
                     (long)start.day   * 1440L  +
                     (long)start.hour  * 60L    +
                     (long)start.min;
    long endMins   = (long)end.year * 525600L +
                     (long)end.month * 43800L  +
                     (long)end.day   * 1440L   +
                     (long)end.hour  * 60L     +
                     (long)end.min;
    double diff = (double)(endMins - startMins);
    return (diff < 0) ? 0.0 : diff;
}

/* Get current system time as a Date */
static Date getCurrentTime(void) {
    time_t    now = time(NULL);
    struct tm *t  = localtime(&now);
    Date d;
    d.year  = t->tm_year + 1900;
    d.month = t->tm_mon  + 1;
    d.day   = t->tm_mday;
    d.hour  = t->tm_hour;
    d.min   = t->tm_min;
    return d;
}

/* Safe input: read a non-empty trimmed line into buf (max len).
 * Returns 1 on success, 0 on EOF. */
static int readLine(char *buf, int len) {
    if (!fgets(buf, len, stdin)) return 0;
    /* Remove trailing newline */
    size_t l = strlen(buf);
    if (l > 0 && buf[l - 1] == '\n') buf[l - 1] = '\0';
    return 1;
}

/* Flush remaining characters on stdin after a bad read */
static void flushInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Validate a license string: exactly 9 digits */
static int isValidLicense(const char *lic) {
    if (strlen(lic) != 9) return 0;
    int i;
    for (i = 0; i < 9; i++)
        if (lic[i] < '0' || lic[i] > '9') return 0;
    return 1;
}

/* Euclidean distance between station and user coords */
static double stationDist(Station *st, double x, double y) {
    double dx = st->coord.x - x;
    double dy = st->coord.y - y;
    return sqrt(dx * dx + dy * dy);
}

/* Recursive helper: find the station whose portsList contains tgt */
static Station *findStCarRec(Station *root, Port *tgt) {
    if (!root) return NULL;
    Station *found = findStCarRec(root->left, tgt);
    if (found) return found;
    Port *p = root->portsList;
    while (p) {
        if (p == tgt) return root;
        p = p->next;
    }
    return findStCarRec(root->right, tgt);
}

/* Find the station that owns a given car's port */
static Station *findStationOfCar(StationBST *stations, Car *car) {
    if (!car || !car->pPort) return NULL;
    return findStCarRec(stations->root, car->pPort);
}

/* Recursive helper: find the station whose carQueue contains car */
static Station *findStQueueRec(Station *root, Car *car) {
    if (!root) return NULL;
    Station *found = findStQueueRec(root->left, car);
    if (found) return found;
    carNode *curr = root->carQueue.front;
    while (curr) {
        if (curr->pCar == car) return root;
        curr = curr->next;
    }
    return findStQueueRec(root->right, car);
}

/* Find the station where a car is currently waiting in queue */
static Station *findStationOfQueuedCar(StationBST *stations, Car *car) {
    if (!car) return NULL;
    return findStQueueRec(stations->root, car);
}

/* Advance the queue: connect the next waiting car that matches an available port */
static void advanceQueue(Station *st) {
    carNode *curr = st->carQueue.front;
    while (curr != NULL) {
        Car  *nextCar  = curr->pCar;
        Port *freePort = findFreePort(st->portsList, nextCar->portType);
        if (freePort != NULL) {
            removeFromQueue(&st->carQueue, nextCar->nLicense);
            st->nCars--;
            assignCar2port(nextCar, freePort);
            printf("  Car %s connected from queue to port #%d.\n",
                   nextCar->nLicense, freePort->num);
            /* Restart from front after modification */
            curr = st->carQueue.front;
        } else {
            curr = curr->next;
        }
    }
}

/* =========================================================
 * Utility / helper public functions
 * ========================================================= */

void displayError(int errCode) {
    printf("\n[ERROR] ");
    switch (errCode) {
        case ERR_NOT_FOUND:       printf("Record not found.\n");               break;
        case ERR_ALREADY_EXISTS:  printf("Record already exists.\n");          break;
        case ERR_INVALID_INPUT:   printf("Invalid input. Please try again.\n");break;
        case ERR_IN_USE:          printf("Cannot remove: resource is in use.\n");break;
        case ERR_NO_FREE_PORT:    printf("No free port available.\n");         break;
        case ERR_EMPTY_QUEUE:     printf("The queue is empty.\n");             break;
        case ERR_NO_OUT_OF_ORDER: printf("No out-of-order ports found.\n");    break;
        case ERR_ALLOC:           printf("Memory allocation failed.\n");       break;
        default:                  printf("Unknown error (code %d).\n", errCode);
    }
}

/* Function-pointer dispatch table for searchStation */
typedef Station *(*SearchFn)(StationBST *, int, const char *, double, double);

static Station *searchByIDFn(StationBST *st, int id, const char *name, double x, double y) {
    (void)name; (void)x; (void)y;
    return searchStationByID(st, id);
}
static Station *searchByNameFn(StationBST *st, int id, const char *name, double x, double y) {
    (void)id; (void)x; (void)y;
    return searchStationByName(st, name);
}
static Station *searchByDistFn(StationBST *st, int id, const char *name, double x, double y) {
    (void)id; (void)name;
    return searchStationByDist(st, x, y);
}

Station *searchStation(StationBST *stations, int mode, int id,
                       const char *name, double x, double y) {
    SearchFn dispatch[] = { NULL, searchByIDFn, searchByNameFn, searchByDistFn };
    if (mode < 1 || mode > 3) return NULL;
    return dispatch[mode](stations, id, name, x, y);
}

void assignCar2port(Car *car, Port *port) {
    Date now    = getCurrentTime();
    port->p2car = car;
    port->status = 1;       /* occupied */
    port->tin   = now;
    car->pPort  = port;
    car->inqueue = 0;
}

void printMenu(void) {
    printf("\n========== EasyCharge System ==========\n");
    printf(" 1. Locate nearest station\n");
    printf(" 2. Charge car\n");
    printf(" 3. Check car status\n");
    printf(" 4. Stop charge\n");
    printf(" 5. Display all stations\n");
    printf(" 6. Display cars at station\n");
    printf(" 7. Report station statistics\n");
    printf(" 8. Display top customers\n");
    printf(" 9. Add new port\n");
    printf("10. Release charging ports (>10h)\n");
    printf("11. Remove out-of-order port\n");
    printf("12. Remove customer\n");
    printf("13. Close station\n");
    printf(" 0. Exit\n");
    printf("========================================\n");
    printf("Your choice: ");
}

/* =========================================================
 * Menu operation 1 – Locate nearest station
 * ========================================================= */
void locNearSt(StationBST *stations) {
    double x, y;
    printf("Enter your coordinates (X Y): ");
    if (scanf("%lf %lf", &x, &y) != 2) {
        flushInput();
        displayError(ERR_INVALID_INPUT);
        return;
    }
    flushInput();

    Station *st = searchStation(stations, 3, 0, NULL, x, y);
    if (!st) { displayError(ERR_NOT_FOUND); return; }

    double dist = stationDist(st, x, y);
    int totalActive  = countPortsByStatus(st->portsList, -1) -
                       countPortsByStatus(st->portsList, 3);
    int occupied     = countPortsByStatus(st->portsList, 1);

    printf("\n--- Nearest Station ---\n");
    printf("Name    : %s\n",   st->name);
    printf("ID      : %d\n",   st->id);
    printf("Distance: %.4f\n", dist);
    printf("Active ports: %d (FAST: %d | MID: %d | SLOW: %d)\n",
           totalActive,
           countPortsByType(st->portsList, FAST),
           countPortsByType(st->portsList, MID),
           countPortsByType(st->portsList, SLOW));
    printf("Occupied ports : %d\n", occupied);
    printf("Cars in queue  : %d\n", st->nCars);
}

/* =========================================================
 * Menu operation 2 – Charge car
 * ========================================================= */
void chargeCar(StationBST *stations, CarBST *cars) {
    char license[LICENSE_LEN];
    char input[MAX_NAME_LEN];

    /* --- Get license number --- */
    printf("Enter car license number (9 digits): ");
    readLine(license, sizeof(license));
    if (!isValidLicense(license)) {
        displayError(ERR_INVALID_INPUT);
        return;
    }

    /* --- Look up or register the car --- */
    Car *car = searchCar(cars, license);
    if (!car) {
        printf("Car not found in database. Registering new car.\n");
        printf("Charging type (FAST / MID / SLOW): ");
        readLine(input, sizeof(input));
        PortType pt;
        if      (strcmp(input, "FAST") == 0) pt = FAST;
        else if (strcmp(input, "MID")  == 0) pt = MID;
        else if (strcmp(input, "SLOW") == 0) pt = SLOW;
        else { displayError(ERR_INVALID_INPUT); return; }

        car = insertCar(cars, license, pt, 0.0);
        if (!car) { displayError(ERR_ALLOC); return; }
    }

    /* A car already charging or waiting cannot be charged again */
    if (car->pPort != NULL || car->inqueue == 1) {
        printf("Car %s is already at a station.\n", license);
        return;
    }

    /* --- Get target station --- */
    printf("Search station by (1) ID  or  (2) Name: ");
    int mode;
    if (scanf("%d", &mode) != 1 || (mode != 1 && mode != 2)) {
        flushInput();
        displayError(ERR_INVALID_INPUT);
        return;
    }
    flushInput();

    Station *st = NULL;
    if (mode == 1) {
        int id;
        printf("Enter station ID: ");
        if (scanf("%d", &id) != 1) { flushInput(); displayError(ERR_INVALID_INPUT); return; }
        flushInput();
        st = searchStation(stations, 1, id, NULL, 0, 0);
    } else {
        printf("Enter station name: ");
        readLine(input, sizeof(input));
        st = searchStation(stations, 2, 0, input, 0, 0);
    }

    if (!st) { displayError(ERR_NOT_FOUND); return; }

    /* --- Find a free port matching car type --- */
    Port *port = findFreePort(st->portsList, car->portType);
    if (port != NULL) {
        assignCar2port(car, port);
        printf("Car %s is now charging at station '%s', port #%d.\n",
               license, st->name, port->num);
    } else {
        /* No free port – add to queue */
        car->inqueue = 1;
        if (!enqueue(&st->carQueue, car)) {
            car->inqueue = 0;
            displayError(ERR_ALLOC);
            return;
        }
        st->nCars++;
        printf("No free %s port at '%s'. Car %s added to queue (position %d).\n",
               (car->portType == FAST) ? "FAST" :
               (car->portType == MID)  ? "MID"  : "SLOW",
               st->name, license,
               posInQueue(&st->carQueue, license, car->portType));
    }
}

/* =========================================================
 * Menu operation 3 – Check car status
 * ========================================================= */
void checkCarStatus(CarBST *cars, StationBST *stations) {
    char license[LICENSE_LEN];
    printf("Enter car license number: ");
    readLine(license, sizeof(license));

    Car *car = searchCar(cars, license);
    if (!car) { displayError(ERR_NOT_FOUND); return; }

    if (car->pPort != NULL) {
        /* Car is currently charging */
        Station *st  = findStationOfCar(stations, car);
        Date     now = getCurrentTime();
        double   min = minutesBetween(car->pPort->tin, now);
        printf("Car %s is CHARGING at station '%s', port #%d (%.0f minutes so far).\n",
               license,
               st ? st->name : "unknown",
               car->pPort->num,
               min);
    } else if (car->inqueue == 1) {
        /* Car is waiting in a queue */
        Station *st = findStationOfQueuedCar(stations, car);
        if (st) {
            int pos = posInQueue(&st->carQueue, license, car->portType);
            printf("Car %s is IN QUEUE at station '%s', position %d (for %s port).\n",
                   license, st->name, pos,
                   car->portType == FAST ? "FAST" :
                   car->portType == MID  ? "MID"  : "SLOW");
        } else {
            printf("Car %s is marked as in-queue but station not found.\n", license);
        }
    } else {
        printf("Car %s is NOT currently at any charging station.\n", license);
    }
}

/* =========================================================
 * Menu operation 4 – Stop charge
 * ========================================================= */
void stopCharge(CarBST *cars, StationBST *stations) {
    char license[LICENSE_LEN];
    printf("Enter car license number: ");
    readLine(license, sizeof(license));

    Car *car = searchCar(cars, license);
    if (!car) { displayError(ERR_NOT_FOUND); return; }
    if (car->pPort == NULL) {
        printf("Car %s is not currently charging.\n", license);
        return;
    }

    Date   now      = getCurrentTime();
    double minutes  = minutesBetween(car->pPort->tin, now);
    double cost     = minutes * RATE_PER_MIN;

    car->totalPayed += cost;

    /* Find which station owns the port */
    Station *st = findStationOfCar(stations, car);

    /* Disconnect car from port */
    Port *port  = car->pPort;
    port->p2car = NULL;
    port->status = 2;           /* free */
    memset(&port->tin, 0, sizeof(Date));
    car->pPort  = NULL;
    car->inqueue = 0;

    printf("Car %s disconnected. Duration: %.0f min | Charged: %.2f NIS | Total paid: %.2f NIS\n",
           license, minutes, cost, car->totalPayed);

    /* Advance the queue for this station */
    if (st) {
        advanceQueue(st);
    }
}

/* =========================================================
 * Menu operation 5 – Display all stations
 * ========================================================= */
static void printStationSummary(Station *st) {
    int occupied = countPortsByStatus(st->portsList, 1);
    printf("--------------------------------------------\n");
    printf("Station: %-20s | ID: %d\n", st->name, st->id);
    printf("  Ports breakdown:\n");
    printPorts(st->portsList);
    printf("  Cars charging: %d | Cars in queue: %d\n", occupied, st->nCars);
}

void dispAllSt(StationBST *stations) {
    printf("\n===== All Charging Stations =====\n");
    traverseStations(stations, printStationSummary);
    printf("=================================\n");
}

/* =========================================================
 * Menu operation 6 – Display cars at station
 * ========================================================= */
void dispCarsAtSt(StationBST *stations) {
    char input[MAX_NAME_LEN];
    int  mode;

    printf("Search station by (1) ID  or  (2) Name: ");
    if (scanf("%d", &mode) != 1 || (mode != 1 && mode != 2)) {
        flushInput();
        displayError(ERR_INVALID_INPUT);
        return;
    }
    flushInput();

    Station *st = NULL;
    if (mode == 1) {
        int id;
        printf("Enter station ID: ");
        if (scanf("%d", &id) != 1) { flushInput(); displayError(ERR_INVALID_INPUT); return; }
        flushInput();
        st = searchStation(stations, 1, id, NULL, 0, 0);
    } else {
        printf("Enter station name: ");
        readLine(input, sizeof(input));
        st = searchStation(stations, 2, 0, input, 0, 0);
    }

    if (!st) { displayError(ERR_NOT_FOUND); return; }

    Date now = getCurrentTime();

    printf("\n--- Cars at station '%s' ---\n", st->name);
    printf("[ Charging ]\n");
    int found = 0;
    Port *p = st->portsList;
    while (p != NULL) {
        if (p->status == 1 && p->p2car != NULL) {
            double mins = minutesBetween(p->tin, now);
            printf("  Port #%d | License: %s | Type: %s | Charging: %.0f min\n",
                   p->num, p->p2car->nLicense,
                   p->p2car->portType == FAST ? "FAST" :
                   p->p2car->portType == MID  ? "MID"  : "SLOW",
                   mins);
            found = 1;
        }
        p = p->next;
    }
    if (!found) printf("  (none)\n");

    printf("[ In queue ]\n");
    carNode *curr = st->carQueue.front;
    int pos = 0;
    if (!curr) printf("  (none)\n");
    while (curr != NULL) {
        pos++;
        printf("  #%d | License: %s | Type: %s\n",
               pos, curr->pCar->nLicense,
               curr->pCar->portType == FAST ? "FAST" :
               curr->pCar->portType == MID  ? "MID"  : "SLOW");
        curr = curr->next;
    }
}

/* =========================================================
 * Menu operation 7 – Report station statistics
 * ========================================================= */
void reportStStat(StationBST *stations) {
    char input[MAX_NAME_LEN];
    int  mode;

    printf("Search station by (1) ID  or  (2) Name: ");
    if (scanf("%d", &mode) != 1 || (mode != 1 && mode != 2)) {
        flushInput();
        displayError(ERR_INVALID_INPUT);
        return;
    }
    flushInput();

    Station *st = NULL;
    if (mode == 1) {
        int id;
        printf("Enter station ID: ");
        if (scanf("%d", &id) != 1) { flushInput(); displayError(ERR_INVALID_INPUT); return; }
        flushInput();
        st = searchStation(stations, 1, id, NULL, 0, 0);
    } else {
        printf("Enter station name: ");
        readLine(input, sizeof(input));
        st = searchStation(stations, 2, 0, input, 0, 0);
    }

    if (!st) { displayError(ERR_NOT_FOUND); return; }

    int total      = countPortsByStatus(st->portsList, -1);
    int occupied   = countPortsByStatus(st->portsList, 1);
    int outOfOrder = countPortsByStatus(st->portsList, 3);
    int queueLen   = st->nCars;

    double utilRate  = (total > 0) ? (100.0 * occupied   / total) : 0.0;
    double faultRate = (total > 0) ? (100.0 * outOfOrder / total) : 0.0;

    printf("\n--- Statistics: %s (ID %d) ---\n", st->name, st->id);
    printf("Total ports      : %d\n", total);
    printf("Occupied         : %d  (%.1f%%)\n", occupied,   utilRate);
    printf("Out of order     : %d  (%.1f%%)\n", outOfOrder, faultRate);
    printf("Cars in queue    : %d\n", queueLen);

    /* Load level */
    printf("Load level       : ");
    if (queueLen == 0 || (occupied > 0 && (double)occupied / queueLen > 1.0)) {
        printf("No load (queue empty or ratio > 1)\n");
    } else {
        double ratio = (queueLen > 0) ? (double)occupied / (double)queueLen : 1.0;
        if      (ratio < 0.2) printf("Very heavy load (ratio = %.2f)\n", ratio);
        else if (ratio < 1.0) printf("Heavy load (ratio = %.2f)\n",      ratio);
        else                  printf("Balanced (ratio = %.2f)\n",         ratio);
    }
}

/* =========================================================
 * Menu operation 8 – Display top customers
 * ========================================================= */
void dispTopCustomers(CarBST *cars) {
    Car *top[TOP_CUSTOMERS];
    getTopCustomers(cars, top);

    printf("\n===== Top %d Customers =====\n", TOP_CUSTOMERS);
    int i;
    for (i = 0; i < TOP_CUSTOMERS; i++) {
        if (top[i] != NULL)
            printf("  %d. License: %-10s | Total paid: %.2f NIS\n",
                   i + 1, top[i]->nLicense, top[i]->totalPayed);
    }
}

/* =========================================================
 * Menu operation 9 – Add new port
 * ========================================================= */
void addNewPort(StationBST *stations, CarBST *cars) {
    char input[MAX_NAME_LEN];
    int  mode;
    (void)cars; /* car linkage happens via advanceQueue */

    printf("Search station by (1) ID  or  (2) Name: ");
    if (scanf("%d", &mode) != 1 || (mode != 1 && mode != 2)) {
        flushInput();
        displayError(ERR_INVALID_INPUT);
        return;
    }
    flushInput();

    Station *st = NULL;
    if (mode == 1) {
        int id;
        printf("Enter station ID: ");
        if (scanf("%d", &id) != 1) { flushInput(); displayError(ERR_INVALID_INPUT); return; }
        flushInput();
        st = searchStation(stations, 1, id, NULL, 0, 0);
    } else {
        printf("Enter station name: ");
        readLine(input, sizeof(input));
        st = searchStation(stations, 2, 0, input, 0, 0);
    }

    if (!st) { displayError(ERR_NOT_FOUND); return; }

    printf("Charging type for new port (FAST / MID / SLOW): ");
    readLine(input, sizeof(input));
    PortType pt;
    if      (strcmp(input, "FAST") == 0) pt = FAST;
    else if (strcmp(input, "MID")  == 0) pt = MID;
    else if (strcmp(input, "SLOW") == 0) pt = SLOW;
    else { displayError(ERR_INVALID_INPUT); return; }

    /* Assign next available port number */
    int maxNum = 0;
    Port *p = st->portsList;
    while (p != NULL) {
        if (p->num > maxNum) maxNum = p->num;
        p = p->next;
    }
    int newNum = maxNum + 1;

    Port *newPort = createPort(newNum, pt, 2); /* status = free */
    if (!newPort) { displayError(ERR_ALLOC); return; }

    addPort(&st->portsList, newPort);
    st->nPorts++;
    printf("Port #%d (%s) added to station '%s'.\n",
           newNum,
           pt == FAST ? "FAST" : pt == MID ? "MID" : "SLOW",
           st->name);

    /* Advance queue for newly added port */
    advanceQueue(st);
}

/* =========================================================
 * Menu operation 10 – Release ports charging > 10 hours
 * ========================================================= */
void releasePorts(StationBST *stations) {
    int id;
    printf("Enter station ID: ");
    if (scanf("%d", &id) != 1) {
        flushInput();
        displayError(ERR_INVALID_INPUT);
        return;
    }
    flushInput();

    Station *st = searchStation(stations, 1, id, NULL, 0, 0);
    if (!st) { displayError(ERR_NOT_FOUND); return; }

    Date  now    = getCurrentTime();
    int   freed  = 0;
    Port *p      = st->portsList;

    while (p != NULL) {
        if (p->status == 1 && p->p2car != NULL) {
            double hours = minutesBetween(p->tin, now) / 60.0;
            if (hours > MAX_CHARGE_HOURS) {
                Car   *car     = p->p2car;
                double minutes = minutesBetween(p->tin, now);
                double cost    = minutes * RATE_PER_MIN;

                car->totalPayed += cost;
                printf("  Released car %s (%.1f h, %.2f NIS charged).\n",
                       car->nLicense, hours, cost);

                p->p2car  = NULL;
                p->status = 2;
                memset(&p->tin, 0, sizeof(Date));
                car->pPort  = NULL;
                car->inqueue = 0;
                freed++;
            }
        }
        p = p->next;
    }

    if (freed == 0)
        printf("No cars exceeded %d hours at station '%s'.\n", MAX_CHARGE_HOURS, st->name);
    else {
        printf("%d car(s) released.\n", freed);
        advanceQueue(st);
    }
}

/* =========================================================
 * Menu operation 11 – Remove out-of-order port
 * ========================================================= */
void remOutOrderPort(StationBST *stations) {
    int id;
    printf("Enter station ID: ");
    if (scanf("%d", &id) != 1) {
        flushInput();
        displayError(ERR_INVALID_INPUT);
        return;
    }
    flushInput();

    Station *st = searchStation(stations, 1, id, NULL, 0, 0);
    if (!st) { displayError(ERR_NOT_FOUND); return; }

    /* List out-of-order ports */
    printf("Out-of-order ports at '%s':\n", st->name);
    Port *p   = st->portsList;
    int   any = 0;
    while (p != NULL) {
        if (p->status == 3) {
            printf("  Port #%d (%s)\n", p->num,
                   p->portType == FAST ? "FAST" :
                   p->portType == MID  ? "MID"  : "SLOW");
            any = 1;
        }
        p = p->next;
    }

    if (!any) { displayError(ERR_NO_OUT_OF_ORDER); return; }

    int portNum;
    printf("Enter port number to remove: ");
    if (scanf("%d", &portNum) != 1) {
        flushInput();
        displayError(ERR_INVALID_INPUT);
        return;
    }
    flushInput();

    /* Verify selected port is indeed out-of-order */
    Port *target = findPortByNum(st->portsList, portNum);
    if (!target) { displayError(ERR_NOT_FOUND); return; }
    if (target->status != 3) {
        printf("Port #%d is not out-of-order.\n", portNum);
        return;
    }

    if (removePort(&st->portsList, portNum)) {
        st->nPorts--;
        printf("Port #%d removed from station '%s'.\n", portNum, st->name);
    } else {
        displayError(ERR_NOT_FOUND);
    }
}

/* =========================================================
 * Menu operation 12 – Remove customer
 * ========================================================= */
void remCustomer(CarBST *cars) {
    char license[LICENSE_LEN];
    printf("Enter car license number: ");
    readLine(license, sizeof(license));

    Car *car = searchCar(cars, license);
    if (!car) { displayError(ERR_NOT_FOUND); return; }

    if (car->pPort != NULL || car->inqueue == 1) {
        displayError(ERR_IN_USE);
        printf("Cannot remove car %s: it is currently %s.\n",
               license, car->pPort ? "charging" : "in queue");
        return;
    }

    if (removeCar(cars, license))
        printf("Car %s removed successfully.\n", license);
    else
        displayError(ERR_NOT_FOUND);
}

/* =========================================================
 * Menu operation 13 – Close station
 * ========================================================= */
void closeSt(StationBST *stations) {
    char input[MAX_NAME_LEN];
    int  mode;

    printf("Search station by (1) ID  or  (2) Name: ");
    if (scanf("%d", &mode) != 1 || (mode != 1 && mode != 2)) {
        flushInput();
        displayError(ERR_INVALID_INPUT);
        return;
    }
    flushInput();

    Station *st = NULL;
    int      targetID = 0;

    if (mode == 1) {
        printf("Enter station ID: ");
        if (scanf("%d", &targetID) != 1) {
            flushInput();
            displayError(ERR_INVALID_INPUT);
            return;
        }
        flushInput();
        st = searchStation(stations, 1, targetID, NULL, 0, 0);
    } else {
        printf("Enter station name: ");
        readLine(input, sizeof(input));
        st = searchStation(stations, 2, 0, input, 0, 0);
        if (st) targetID = st->id;
    }

    if (!st) { displayError(ERR_NOT_FOUND); return; }

    /* Disconnect all cars currently charging */
    Port *p = st->portsList;
    while (p != NULL) {
        if (p->p2car != NULL) {
            p->p2car->pPort  = NULL;
            p->p2car->inqueue = 0;
            p->p2car = NULL;
        }
        p = p->next;
    }

    /* Dequeue all waiting cars */
    while (!isQueueEmpty(&st->carQueue)) {
        Car *car = dequeue(&st->carQueue);
        if (car) { car->inqueue = 0; }
    }
    st->nCars = 0;

    char stName[MAX_NAME_LEN];
    strncpy(stName, st->name, MAX_NAME_LEN - 1);
    stName[MAX_NAME_LEN - 1] = '\0';

    if (removeStation(stations, targetID))
        printf("Station '%s' (ID %d) has been closed and removed.\n", stName, targetID);
    else
        displayError(ERR_NOT_FOUND);
}

/* =========================================================
 * Menu operation 0 – Exit system
 * ========================================================= */
void exitSys(StationBST *stations, CarBST *cars) {
    printf("\nSaving data...\n");
    updateFiles(stations, cars);

    printf("Releasing memory...\n");
    freeStationBST(stations);
    freeCarBST(cars);

    printf("EasyCharge system closed successfully. Goodbye!\n");
}
