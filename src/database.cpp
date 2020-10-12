#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <cstdio>
#include <unistd.h> 

#include <File_Interface.h>
#include "database.h"  

int Init_database(){

sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char* sql;

   /* Open database */
   rc = sqlite3_open(database, &db);
   
   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(1);
   } else {
      fprintf(stdout, "Opened database successfully\n");
   }

/*
   // Create SQL statement 
   sql = "CREATE TABLE EVENTS (" \
   " ID INTEGER PRIMARY KEY AUTOINCREMENT," \
   " Date TEXT," \
   " State TEXT,"\
   " RF_State TEXT,"\
   " Nodes TEXT );";
*/
	sql = "DELETE FROM EVENTS;"; 
   //sprintf(sql,"DELETE FROM EVENTS;");
   
   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
   
   if( rc != SQLITE_OK )
   {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      return 1;
   }
   else {
      fprintf(stdout, "Table created successfully\n");
   }
   
   //sprintf(sql,"DELETE FROM sqlite_sequence WHERE NAME='EVENTS';");
   sql = "DELETE FROM sqlite_sequence WHERE NAME='EVENTS';";
   
   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
   
   if( rc != SQLITE_OK )
   {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      return 1;
   }
   else {
      fprintf(stdout, "Table ID Reset successfully\n");
   }
   
   sqlite3_close(db);
   return 0;

}

int Init_RF_database(){

sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char* sql;

   /* Open database */
   rc = sqlite3_open(RFdatabase, &db);
   
   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(1);
   } else {
      fprintf(stdout, "Opened database successfully\n");
   }

/*
   // Create SQL statement 
   sql = "CREATE TABLE EVENTS (" \
   " ID INTEGER PRIMARY KEY AUTOINCREMENT," \
   " Date TEXT," \
   " State TEXT,"\
   " RF_State TEXT,"\
   " Nodes TEXT );";
*/
	sql = "DELETE FROM RF_NODES;"; 
   //sprintf(sql,"DELETE FROM EVENTS;");
   
   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
   
   if( rc != SQLITE_OK )
   {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      return 1;
   }
   else {
      fprintf(stdout, "Table created successfully\n");
   }
   
   //sprintf(sql,"DELETE FROM sqlite_sequence WHERE NAME='EVENTS';");
   sql = "DELETE FROM sqlite_sequence WHERE NAME='RF_NODES';";
   
   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
   
   if( rc != SQLITE_OK )
   {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      return 1;
   }
   else {
      fprintf(stdout, "Table ID Reset successfully\n");
   }
   
   sqlite3_close(db);
   return 0;

}

int database_update(u_int16_t* Line_Counter,char* STATE, char* RF, char* NODES){
  
   sqlite3 *db;
   int rc;
	
   char TEXT[50];

   rc = sqlite3_open(database, &db);

   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   } else {
      fprintf(stderr, "Opened database successfully\n");
   }

	if(Line_Counter[0]>=N_RECORD_EVENTS)
	{
		Init_database();
	}
   timestamp(TEXT);
   
   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db, "INSERT INTO EVENTS (Date,State,RF_State,Nodes) "  \
   "VALUES (?,?,?,?);", -1, &stmt, NULL);
   
   if(sqlite3_bind_text(stmt, 1, TEXT,-1, SQLITE_TRANSIENT)!=SQLITE_OK)
   {
      printf("\nCould not bind text1.\n");
      sqlite3_finalize(stmt);
      return 1;
   }
   
   if(sqlite3_bind_text(stmt, 2, STATE,-1, SQLITE_TRANSIENT)!=SQLITE_OK)
   {
      printf("\nCould not bind text2.\n");
      sqlite3_finalize(stmt);
      return 1;
   }
   if(sqlite3_bind_text(stmt, 3, RF,-1,SQLITE_TRANSIENT)!=SQLITE_OK)
   {
      printf("\nCould not bind text3.\n");
      sqlite3_finalize(stmt);
      return 1;
   }
   if(sqlite3_bind_text(stmt, 4, NODES,-1,SQLITE_TRANSIENT)!=SQLITE_OK)
   {
      printf("\nCould not bind text4.\n");
      sqlite3_finalize(stmt);
      return 1;
   } 
   
   if(sqlite3_step(stmt)!= SQLITE_DONE) {
      printf("\nCould not step (execute) stmt.\n");
      sqlite3_finalize(stmt);
      return 1;
   }
   sqlite3_finalize(stmt);

   sqlite3_close(db);
   return (0);
}

int RF_database_update(int ID,int CODE,int STATUS){
   sqlite3 *db;
   int rc;

   rc = sqlite3_open(RFdatabase, &db);

   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   } else {
      fprintf(stderr, "Opened database successfully\n");
   }
   
   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db, "INSERT INTO RF_NODES (ID,CODE,STATUS) "  \
   "VALUES (?,?,?);", -1, &stmt, NULL);
   
   if(sqlite3_bind_int(stmt, 1, ID)!=SQLITE_OK)
   {
      printf("\nCould not bind int.\n");
      sqlite3_finalize(stmt);
      return 1;
   }
   if(sqlite3_bind_int(stmt, 2, CODE)!=SQLITE_OK)
   {
      printf("\nCould not bind int.\n");
      sqlite3_finalize(stmt);
      return 1;
   }
   if(sqlite3_bind_int(stmt, 3, STATUS)!=SQLITE_OK)
   {
      printf("\nCould not bind int.\n");
      sqlite3_finalize(stmt);
      return 1;
   }
   
   if(sqlite3_step(stmt)!= SQLITE_DONE) {
      printf("\nCould not step (execute) stmt.\n");
      sqlite3_finalize(stmt);
      return 1;
   }
   sqlite3_finalize(stmt);

   sqlite3_close(db);
   return (0);
}
