#ifndef MENU_OPS_H
#define MENU_OPS_H

/* =========================================================
 * menu_ops.h
 * Prototypes for all menu operations and helper functions.
 * ========================================================= */

#include "types.h"
#include "station_bst.h"
#include "car_bst.h"

/* ---- Utility / helper functions ---- */

/* Print a formatted error message for the given error code */
void     displayError(int errCode);

/* Unified station search using a function-pointer dispatch table.
 * mode: 1 = by id, 2 = by name, 3 = by distance from (x,y).
 * x and y are only used when mode == 3.
 * Returns pointer to the found Station, or NULL. */
Station *searchStation(StationBST *stations, int mode, int id,
                       const char *name, double x, double y);

/* Link a car to a port and update all relevant fields.
 * Sets port->p2car, port->status, port->tin, car->pPort, car->inqueue. */
void     assignCar2port(Car *car, Port *port);

/* Print the main menu options */
void     printMenu(void);

/* ---- Menu operation functions ---- */

/* 1 – Locate nearest station */
void locNearSt(StationBST *stations);

/* 2 – Charge a car (assign to port or queue) */
void chargeCar(StationBST *stations, CarBST *cars);

/* 3 – Check car charging status */
void checkCarStatus(CarBST *cars, StationBST *stations);

/* 4 – Stop charging a car */
void stopCharge(CarBST *cars, StationBST *stations);

/* 5 – Display all stations */
void dispAllSt(StationBST *stations);

/* 6 – Display cars at a specific station */
void dispCarsAtSt(StationBST *stations);

/* 7 – Report station statistics */
void reportStStat(StationBST *stations);

/* 8 – Display top 5 customers */
void dispTopCustomers(CarBST *cars);

/* 9 – Add a new port to a station */
void addNewPort(StationBST *stations, CarBST *cars);

/* 10 – Release ports charging over 10 hours */
void releasePorts(StationBST *stations);

/* 11 – Remove an out-of-order port */
void remOutOrderPort(StationBST *stations);

/* 12 – Remove a customer (car) from the BST */
void remCustomer(CarBST *cars);

/* 13 – Close a station */
void closeSt(StationBST *stations);

/* 0 – Exit system (save + free + goodbye) */
void exitSys(StationBST *stations, CarBST *cars);

#endif /* MENU_OPS_H */
