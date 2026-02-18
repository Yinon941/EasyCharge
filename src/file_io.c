/* =========================================================
 * file_io.c
 * Load and save the four EasyCharge data files.
 *
 * File formats (CSV, first line is header):
 *
 * Stations.txt:
 *   ID,StationName,NumOfPorts,CoordX,CoordY
 *
 * Ports.txt:
 *   StationID,PortNumber,PortType,Status,Year,Month,Day,Hour,Min,CarLicense
 *   (CarLicense = -1 when no car is connected)
 *
 * Cars.txt:
 *   License,PortType,TotalPayed,StationID,PortNumber,InQueue
 *   (StationID=0 and PortNumber=0 mean not at any station)
 *
 * LineOfCars.txt:
 *   License,StationID
 * ========================================================= */

#include "file_io.h"
#include "port_list.h"
#include "queue.h"

/* ---- Helpers ---- */

static PortType strToPortType(const char *s) {
    if (strcmp(s, "FAST") == 0) return FAST;
    if (strcmp(s, "MID")  == 0) return MID;
    return SLOW;
}

static const char *portTypeToStr(PortType t) {
    if (t == FAST) return "FAST";
    if (t == MID)  return "MID";
    return "SLOW";
}

/* =========================================================
 * loadFiles
 * ========================================================= */
int loadFiles(StationBST *stations, CarBST *cars) {
    FILE *fp;
    char  line[MAX_LINE_LEN];

    /* --- 1. Stations.txt --- */
    fp = fopen(FILE_STATIONS, "r");
    if (!fp) { fprintf(stderr, "Error: cannot open %s\n", FILE_STATIONS); return 0; }

    fgets(line, sizeof(line), fp); /* skip header */
    while (fgets(line, sizeof(line), fp)) {
        int    id, nPorts;
        char   name[MAX_NAME_LEN];
        double x, y;
        if (sscanf(line, "%d,%63[^,],%d,%lf,%lf", &id, name, &nPorts, &x, &y) == 5)
            insertStation(stations, id, name, nPorts, x, y);
    }
    fclose(fp);

    /* --- 2. Ports.txt (first pass: create Port nodes, skip car link) --- */
    fp = fopen(FILE_PORTS, "r");
    if (!fp) { fprintf(stderr, "Error: cannot open %s\n", FILE_PORTS); return 0; }

    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        int  stID, portNum, status, yr, mo, dy, hr, mn;
        char typeStr[8], carLic[16];

        if (sscanf(line, "%d,%d,%7[^,],%d,%d,%d,%d,%d,%d,%15s",
                   &stID, &portNum, typeStr, &status,
                   &yr, &mo, &dy, &hr, &mn, carLic) == 10) {

            Station *st = searchStationByID(stations, stID);
            if (!st) continue;

            Port *p = createPort(portNum, strToPortType(typeStr), status);
            if (!p) continue;

            p->tin.year  = yr;
            p->tin.month = mo;
            p->tin.day   = dy;
            p->tin.hour  = hr;
            p->tin.min   = mn;

            addPort(&st->portsList, p);
        }
    }
    fclose(fp);

    /* --- 3. Cars.txt --- */
    fp = fopen(FILE_CARS, "r");
    if (!fp) { fprintf(stderr, "Error: cannot open %s\n", FILE_CARS); return 0; }

    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        char   license[LICENSE_LEN];
        char   typeStr[8];
        double paid;
        int    stID, portNum, inqueue;

        if (sscanf(line, "%9[^,],%7[^,],%lf,%d,%d,%d",
                   license, typeStr, &paid, &stID, &portNum, &inqueue) == 6) {

            Car *car = insertCar(cars, license, strToPortType(typeStr), paid);
            if (!car) continue;
            car->inqueue = inqueue;
        }
    }
    fclose(fp);

    /* --- 4. Second pass: link p2car pointers in ports --- */
    fp = fopen(FILE_PORTS, "r");
    if (!fp) { fprintf(stderr, "Error: cannot open %s\n", FILE_PORTS); return 0; }

    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        int  stID, portNum, status, yr, mo, dy, hr, mn;
        char typeStr[8], carLic[16];

        if (sscanf(line, "%d,%d,%7[^,],%d,%d,%d,%d,%d,%d,%15s",
                   &stID, &portNum, typeStr, &status,
                   &yr, &mo, &dy, &hr, &mn, carLic) == 10) {

            if (strcmp(carLic, "-1") == 0) continue;

            Station *st   = searchStationByID(stations, stID);
            if (!st) continue;
            Port    *port = findPortByNum(st->portsList, portNum);
            if (!port) continue;
            Car     *car  = searchCar(cars, carLic);
            if (!car) continue;

            port->p2car = car;
            car->pPort  = port;
        }
    }
    fclose(fp);

    /* --- 5. LineOfCars.txt: build per-station queues --- */
    fp = fopen(FILE_QUEUE, "r");
    if (!fp) { fprintf(stderr, "Error: cannot open %s\n", FILE_QUEUE); return 0; }

    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        char license[LICENSE_LEN];
        int  stID;

        if (sscanf(line, "%9[^,],%d", license, &stID) == 2) {
            Station *st  = searchStationByID(stations, stID);
            Car     *car = searchCar(cars, license);
            if (st && car) {
                enqueue(&st->carQueue, car);
                st->nCars++;
            }
        }
    }
    fclose(fp);

    return 1;
}

/* =========================================================
 * updateFiles helpers – all are static, no global variables
 * ========================================================= */

/* Write all station rows in-order */
static void writeStationRec(Station *root, FILE *fp) {
    if (!root) return;
    writeStationRec(root->left, fp);
    fprintf(fp, "%d,%s,%d,%.2f,%.2f\n",
            root->id, root->name, root->nPorts,
            root->coord.x, root->coord.y);
    writeStationRec(root->right, fp);
}

/* Find the station that owns a given port (used when writing Cars.txt) */
static void findStForPortRec(Station *root, Port *tgt, Station **res) {
    if (!root || *res) return;
    findStForPortRec(root->left, tgt, res);
    if (*res) return;
    Port *p = root->portsList;
    while (p) {
        if (p == tgt) { *res = root; return; }
        p = p->next;
    }
    findStForPortRec(root->right, tgt, res);
}

/* Write all port rows for every station in-order */
static void writePortsRec(Station *root, FILE *fp) {
    if (!root) return;
    writePortsRec(root->left, fp);

    Port *p = root->portsList;
    while (p) {
        const char *lic = (p->p2car) ? p->p2car->nLicense : "-1";
        fprintf(fp, "%d,%d,%s,%d,%d,%d,%d,%d,%d,%s\n",
                root->id, p->num,
                portTypeToStr(p->portType), p->status,
                p->tin.year, p->tin.month, p->tin.day,
                p->tin.hour, p->tin.min, lic);
        p = p->next;
    }

    writePortsRec(root->right, fp);
}

/* Write all car rows in-order through the Cars BST */
static void writeCarsRec(tCar *node, FILE *fp, StationBST *stations) {
    if (!node) return;
    writeCarsRec(node->left, fp, stations);

    Car *car     = node->pCar;
    int  stID    = 0;
    int  portNum = 0;

    if (car->pPort) {
        portNum = car->pPort->num;
        Station *found = NULL;
        findStForPortRec(stations->root, car->pPort, &found);
        if (found) stID = found->id;
    }

    fprintf(fp, "%s,%s,%.2f,%d,%d,%d\n",
            car->nLicense,
            portTypeToStr(car->portType),
            car->totalPayed,
            stID, portNum, car->inqueue);

    writeCarsRec(node->right, fp, stations);
}

/* Write queue entries for every station in-order */
static void writeQueuesRec(Station *root, FILE *fp) {
    if (!root) return;
    writeQueuesRec(root->left, fp);

    carNode *curr = root->carQueue.front;
    while (curr) {
        fprintf(fp, "%s,%d\n", curr->pCar->nLicense, root->id);
        curr = curr->next;
    }

    writeQueuesRec(root->right, fp);
}

void updateFiles(StationBST *stations, CarBST *cars) {
    FILE *fp;

    fp = fopen(FILE_STATIONS, "w");
    if (fp) {
        fprintf(fp, "ID,StationName,NumOfPorts,CoordX,CoordY\n");
        writeStationRec(stations->root, fp);
        fclose(fp);
    } else fprintf(stderr, "Error: cannot write %s\n", FILE_STATIONS);

    fp = fopen(FILE_PORTS, "w");
    if (fp) {
        fprintf(fp, "StationID,PortNumber,PortType,Status,Year,Month,Day,Hour,Min,CarLicense\n");
        writePortsRec(stations->root, fp);
        fclose(fp);
    } else fprintf(stderr, "Error: cannot write %s\n", FILE_PORTS);

    fp = fopen(FILE_CARS, "w");
    if (fp) {
        fprintf(fp, "License,PortType,TotalPayed,StationID,PortNumber,InQueue\n");
        writeCarsRec(cars->root, fp, stations);
        fclose(fp);
    } else fprintf(stderr, "Error: cannot write %s\n", FILE_CARS);

    fp = fopen(FILE_QUEUE, "w");
    if (fp) {
        fprintf(fp, "License,StationID\n");
        writeQueuesRec(stations->root, fp);
        fclose(fp);
    } else fprintf(stderr, "Error: cannot write %s\n", FILE_QUEUE);
}
