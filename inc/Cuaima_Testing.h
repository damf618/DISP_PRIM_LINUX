/*=====[TP4_Testing]==========================================================
 * Copyright 2020 Author Marquez Daniel <damf618@gmail.com>
 * All rights reserved.
 * License: license text or at least name and link
         (example: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 1.0.0
 * Creation Date: 2020/07/25
 */

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef CUAIMA_TESTING_H_
#define CUAIMA_TESTING_H_

/*=====[Inclusions of public function dependencies]==========================*/

#include"stdint.h"
#include"stdbool.h"
#include <stdio.h>
#include <stdlib.h>
#include <primario4.h>

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/
#define MAX_RF_DEVICES 100
#define RF_DEVICE_DATA 2
/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

typedef struct RF_Device_s
{
	int Node_ID;
	int RF_Code;
	bool updated;
} RF_Device_t;

typedef struct{
	int Counter;
	RF_Device_t Nodes_Data[MAX_RF_DEVICES];
} Nodes_Database_t;

typedef struct RF_List_s
{
	RF_Device_t* RF_Devices[MAX_RF_DEVICES];
	int counter;
	int n_nodes;
	int active_nodes;
} RF_List_t;


/*=====[Prototypes (declarations) of public functions]=======================*/

int Header_Check(int id, RF_List_t * RF_Devices);

bool Header_Validation(int id, int Code, RF_List_t* RF_List);

int Comm_Code(RF_List_t* RF_List,Nodes_Database_t* Data_RF_List);

void Clean_RFDevices(RF_List_t* RF_List);
/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* CUAIMA_TESTING_H_ */
