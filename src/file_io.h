#ifndef FILE_IO_H
#define FILE_IO_H

/* =========================================================
 * file_io.h
 * Interface for loading and saving the four data files.
 * ========================================================= */

#include "types.h"
#include "station_bst.h"
#include "car_bst.h"

/* Load all four files into the data structures.
 * Must be called once at startup, before showing the menu.
 * Returns 1 on success, 0 if any file could not be opened. */
int loadFiles(StationBST *stations, CarBST *cars);

/* Write current in-memory state back to all four files.
 * Called after every data-modifying operation and at exit. */
void updateFiles(StationBST *stations, CarBST *cars);

#endif /* FILE_IO_H */
