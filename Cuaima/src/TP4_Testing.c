/*=====[TP4_Testing]==========================================================
 * Copyright 2020 Author Marquez Daniel <damf618@gmail.com>
 * All rights reserved.
 * License: license text or at least name and link
         (example: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 1.0.0
 * Creation Date: 2020/07/25
 */

/*=====[Inclusions of function dependencies]=================================*/

#include "TP4_Testing.h"
#include "primario4.h"
#include "stdio.h"



void Event_Handler(dprimario_t * prim, contact_state_t LocalA,contact_state_t LocalF,
					dprim_state_t Comm, int * Status_Code){
	
	int STATUS[3]={0,0,0};
	
	if(prim->Alarm_Contact!=LocalA)
	{
		STATUS[0]=1;
		prim->Alarm_Contact!=LocalA;
	}
	if(prim->Fail_Contact!=LocalF)
	{
		STATUS[1]=1;
		prim->Fail_Contact!=LocalF;
	}
	if(prim->Comm_State!=Comm)
	{
		STATUS[2]=1;
		prim->Comm_State=Comm;
	}
	*Status_Code=STATUS[0]+STATUS[1]*2+STATUS[2]*4;
}

void FullCheck(dprimario_t * prim,dprim_state_t casea, dprim_state_t casef,dprim_state_t casen,dprim_state_t caseaf){
	contact_state_t LocalA;
	contact_state_t LocalF;
	dprim_state_t Comm;
	int Status_Code=0;
		
	LocalA=ButtonCheck(prim,ALARM);
	LocalF=ButtonCheck(prim,FAIL);
	Comm=CommCheck(prim,caseaf,casea,casef,casen);
	
	Event_Handler(prim,LocalA,LocalF,Comm,&Status_Code);
	
	switch(Status_Code){
		case 1:
			if((LocalA==ACTIVE)||(LocalA==ACTIVE_TRANSITION))
			{
				prim->state=casea;
			}
			else{
				prim->state=casen;
			}
			break;
		case 2:
			if((LocalF==ACTIVE)||(LocalF==ACTIVE_TRANSITION))
			{
				prim->state=casef;
			}
			else{
				prim->state=casen;
			}
			break;
		case 3:
			if(((LocalA==ACTIVE)||(LocalA==ACTIVE_TRANSITION))&&
			((LocalF==ACTIVE)||(LocalF==ACTIVE_TRANSITION)))
			{
				prim->state=caseaf;
			}
			else if((LocalA==ACTIVE)||(LocalA==ACTIVE_TRANSITION))
			{
				prim->state=casea;
			}
			else if((LocalF==ACTIVE)||(LocalF==ACTIVE_TRANSITION))
			{
				prim->state=casef;
			}
			else{
				prim->state=casen;
			}
			break;
		case 4:
			if(Comm==caseaf){
				prim->state=caseaf;
			}
			else if(Comm==casea){
				prim->state=casea;
			}
			else if(Comm==casef){
				prim->state=casef;
			}
			else{
				prim->state=casen;
			}
			break;
		case 5:
			if(Comm==caseaf){
				prim->state=caseaf;
			}
			else if(((LocalA==ACTIVE)||(LocalA==ACTIVE_TRANSITION))&&(Comm==casef))
			{
				prim->state=caseaf;
			}
			else if(((LocalA==ACTIVE)||(LocalA==ACTIVE_TRANSITION))||(Comm==casea))
			{
				prim->state=casea;
			}
			else if(Comm==casef)
			{
				prim->state=casef;
			}
			else{
				prim->state=casen;
			}
			break;
		case 6:
			if(Comm==caseaf){
				prim->state=caseaf;
			}
			else if(((LocalF==ACTIVE)||(LocalF==ACTIVE_TRANSITION))&&(Comm==casea))
			{
				prim->state=caseaf;
			}
			else if(((LocalF==ACTIVE)||(LocalF==ACTIVE_TRANSITION))||(Comm==casef))
			{
				prim->state=casef;
			}
			else if(Comm==casea)
			{
				prim->state=casea;
			}
			else{
				prim->state=casen;
			}
			break;
		case 7:
			if(((LocalA==ACTIVE)||(LocalA==ACTIVE_TRANSITION))&&
			((LocalF==ACTIVE)||(LocalF==ACTIVE_TRANSITION))||(Comm==caseaf))
			{
				prim->state=caseaf;
			}
			else if((LocalA==ACTIVE)||(LocalA==ACTIVE_TRANSITION)&&(Comm==casef))
			{
				prim->state=caseaf;
			}
			else if((LocalF==ACTIVE)||(LocalF==ACTIVE_TRANSITION)&&(Comm==casea))
			{
				prim->state=caseaf;
			}
			else if((LocalA==ACTIVE)||(LocalA==ACTIVE_TRANSITION)||(Comm==casea))
			{
				prim->state=casea;
			}
			else if((LocalF==ACTIVE)||(LocalF==ACTIVE_TRANSITION)||(Comm==casef))
			{
				prim->state=casef;
			}
			else{
				prim->state=casen;
			}
			break;
	}
/*	
	if(Event){
		if(((LocalA==casea)&&(LocalF==casef)) || (Comm==caseaf) || ((LocalA==casea)&&(Comm==casef)) || ((LocalF==casef)&&(Comm==casea)) )
		{
			prim->state=caseaf;
			printf("\r\n ====== Alarm-Fail Status ====== \r\n");
			ResetChange(prim);
		}
		else if((LocalA==casea) || (Comm==casea))
		{
			prim->state=casea;
			printf("\r\n ====== Alarm Status ====== \r\n");
			ResetChange(prim);
		}
		else if((LocalF==casef) || (Comm==casef))
		{
			prim->state=casef;
			printf("\r\n ====== Fail Status ====== \r\n");
			ResetChange(prim);
		}
		else if(((LocalA==casen)&&(LocalF==casen)) || (Comm==casen))
		{
			prim->state=casen;
			printf("\r\n ====== Normal Status ====== \r\n");
			ResetChange(prim);
		}
		else
		{
			printf("\r\n ====== Unexpected Status ====== \r\n");
			prim->state=FAIL;
			ResetChange(prim);
		}
	}
*/ 
	if( (ACTIVE_TRANSITION == LocalA) || (ACTIVE_TRANSITION == LocalA) || (Status_Code>=4) )
	{
		ResetChange(prim);
	}
}

bool primControl(dprimario_t * pPrimario){

	if(NULL==pPrimario)
		return 0;

	switch( pPrimario->state ) {

		case NORMAL:
			printf("\n== Estado Principal ==\n  ** NORMAL **");
			FullCheck (pPrimario,PREALARM,PREFAIL,NORMAL,PRE_ALARM_FAIL);
			break;
		case ALARM:
			printf("\n== Estado Principal ==\n  ** ALARMA **");
			FullCheck (pPrimario,ALARM,PREFAIL,PRENORMAL,PRE_ALARM_FAIL);
			break;
		case FAIL:
			printf("\n== Estado Principal ==\n  ** FALLA **");
			FullCheck (pPrimario,PREALARM,FAIL,PRENORMAL,PRE_ALARM_FAIL);
			break;
		case ALARM_FAIL:
			printf("\n== Estado Principal ==\n  ** ALARMA-FALLA **");
			FullCheck (pPrimario,ALARM,FAIL,PRENORMAL,ALARM_FAIL);
			break;
		case PRENORMAL:
			printf("\n-- Estado Secundario --\n  ** PRE-NORMAL **");
			FullCheck (pPrimario,PREALARM,PREFAIL,NORMAL,PRE_ALARM_FAIL);
			break;
		case PREALARM:
			printf("\n-- Estado Secundario --\n  ** PRE-ALARMA **");
			FullCheck (pPrimario,ALARM,PREFAIL,PRENORMAL,ALARM);
			break;
		case PREFAIL:
			printf("\n-- Estado Secundario --\n  ** PRE-FALLA **");
			FullCheck (pPrimario,PREALARM,FAIL,PRENORMAL,PRE_ALARM_FAIL);
			break;
		case PRE_ALARM_FAIL:
			printf("\n-- Estado Secundario --\n  ** PRE-ALARMA/FALLA **");
			FullCheck (pPrimario,ALARM,FAIL,PRENORMAL,ALARM_FAIL);
			break;
		default:
			pPrimario->state=FAIL;
		}
	return 1;
}
