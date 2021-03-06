/*=============================================================================
 * Copyright (c) 2020, DANIEL MARQUEZ <DAMF618@GMAIL.COM>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2020/04/12
 * Version: 1
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __DATABASE_H__
#define __DATABASE_H__

#define database "db_test.db"
#define RFdatabase "RF_db_test.db"


/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

int Init_database(void);

int database_update(u_int16_t* Line_Counter,char* STATE, char* RF, char* NODES);

int RF_database_update(int ID,int CODE,int STATUS);

int Init_RF_database(void);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __DATABASE_H__ */

