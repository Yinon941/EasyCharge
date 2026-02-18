/* =========================================================
 * main.c
 * Entry point for the EasyCharge charging station management system.
 *
 * Flow:
 *   1. Load data from the four CSV files into BSTs / linked lists.
 *   2. Show menu in a loop until the user chooses to exit.
 *   3. Dispatch each choice to the appropriate handler function.
 *   4. Save updated data to files and free all memory on exit.
 * ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "types.h"
#include "station_bst.h"
#include "car_bst.h"
#include "queue.h"
#include "port_list.h"
#include "file_io.h"
#include "menu_ops.h"

int main(void) {
    StationBST stations;
    CarBST     cars;

    initStationBST(&stations);
    initCarBST(&cars);

    /* Load all data before showing the menu */
    printf("Loading EasyCharge data...\n");
    if (!loadFiles(&stations, &cars)) {
        fprintf(stderr, "Fatal: could not load required data files. Exiting.\n");
        return 1;
    }
    printf("Data loaded successfully.\n");

    int choice = -1;

    while (choice != 0) {
        printMenu();

        if (scanf("%d", &choice) != 1) {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid choice. Please enter a number.\n");
            choice = -1;
            continue;
        }
        /* Consume the trailing newline */
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        switch (choice) {
            case 1:  locNearSt(&stations);                   break;
            case 2:  chargeCar(&stations, &cars);            break;
            case 3:  checkCarStatus(&cars, &stations);       break;
            case 4:  stopCharge(&cars, &stations);           break;
            case 5:  dispAllSt(&stations);                   break;
            case 6:  dispCarsAtSt(&stations);                break;
            case 7:  reportStStat(&stations);                break;
            case 8:  dispTopCustomers(&cars);                break;
            case 9:  addNewPort(&stations, &cars);           break;
            case 10: releasePorts(&stations);                break;
            case 11: remOutOrderPort(&stations);             break;
            case 12: remCustomer(&cars);                     break;
            case 13: closeSt(&stations);                     break;
            case 0:  exitSys(&stations, &cars);              break;
            default:
                printf("Invalid choice. Please select 0-13.\n");
                break;
        }

        /* Save after every modifying operation */
        if (choice >= 1 && choice <= 13)
            updateFiles(&stations, &cars);
    }

    return 0;
}
