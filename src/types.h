#ifndef TYPES_H
#define TYPES_H

/* =========================================================
 * types.h
 * Common type definitions for the EasyCharge system.
 * All structs, enums, typedefs and constants are here.
 * ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* MSVC compatibility: strdup is _strdup in MSVC */
#ifdef _MSC_VER
#define strdup _strdup
#endif

/* ----- Constants & Macros ----- */
#define LICENSE_LEN      10      /* 9 digits + null terminator */
#define MAX_NAME_LEN     64      /* max buffer for station name input */
#define RATE_PER_MIN     1.2     /* NIS per charging minute */
#define MAX_CHARGE_HOURS 10      /* hours before auto-release */
#define TOP_CUSTOMERS    5       /* number of top customers to display */
#define MAX_LINE_LEN     256     /* buffer size for file reading */

/* File names */
#define FILE_STATIONS    "Stations.txt"
#define FILE_PORTS       "Ports.txt"
#define FILE_CARS        "Cars.txt"
#define FILE_QUEUE       "LineOfCars.txt"

/* Error codes */
#define ERR_NOT_FOUND       1
#define ERR_ALREADY_EXISTS  2
#define ERR_INVALID_INPUT   3
#define ERR_IN_USE          4
#define ERR_NO_FREE_PORT    5
#define ERR_EMPTY_QUEUE     6
#define ERR_NO_OUT_OF_ORDER 7
#define ERR_ALLOC           8

/* ----- Port / Charging speed type ----- */
typedef enum {
    FAST = 0,
    MID  = 1,
    SLOW = 2
} PortType;

/* ----- Date (timestamp) ----- */
typedef struct {
    int year;   /* 4-digit year >= 1900 */
    int month;  /* 1-12 */
    int day;
    int hour;   /* 0-23 */
    int min;    /* 0-59 */
} Date;

/* ----- Forward declaration so Car can reference Port ----- */
typedef struct Port Port;

/* ----- Car (customer vehicle) ----- */
typedef struct {
    char    nLicense[LICENSE_LEN]; /* unique 9-digit license plate */
    PortType portType;             /* preferred charging speed */
    double  totalPayed;            /* cumulative payment (NIS) */
    Port   *pPort;                 /* pointer to port while charging, else NULL */
    int     inqueue;               /* 0 = not waiting, 1 = in queue */
} Car;

/* ----- BST node for the cars database ----- */
typedef struct tCar {
    Car        *pCar;
    struct tCar *left;
    struct tCar *right;
} tCar;

/* ----- Queue node (car waiting at a station) ----- */
typedef struct carNode {
    Car           *pCar;
    struct carNode *next;
} carNode;

/* ----- Queue manager ----- */
typedef struct {
    carNode *front;
    carNode *rear;
} qCar;

/* ----- Charging port ----- */
struct Port {
    int      num;       /* port number within the station */
    PortType portType;  /* FAST / MID / SLOW */
    int      status;    /* 1=occupied, 2=free, 3=out of order */
    Car     *p2car;     /* car currently charging, NULL if empty */
    Date     tin;       /* time charging started */
    Port    *next;      /* next port in the linked list */
};

/* ----- Geographic coordinates ----- */
typedef struct {
    double x;
    double y;
} Coord;

/* ----- Charging station (also serves as BST node) ----- */
typedef struct Station {
    int            id;
    char          *name;       /* dynamically allocated */
    int            nPorts;     /* total number of ports */
    Coord          coord;
    Port          *portsList;  /* head of singly-linked ports list */
    int            nCars;      /* number of cars currently in queue */
    qCar           carQueue;   /* queue manager for waiting cars */
    struct Station *left;
    struct Station *right;
} Station;

/* ----- BST manager for stations ----- */
typedef struct {
    Station *root;
} StationBST;

/* ----- BST manager for cars ----- */
typedef struct {
    tCar *root;
} CarBST;

#endif /* TYPES_H */
