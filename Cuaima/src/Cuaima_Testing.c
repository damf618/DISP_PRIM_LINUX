/*=====[TP4_Testing]==========================================================
 * Copyright 2020 Author Marquez Daniel <damf618@gmail.com>
 * All rights reserved.
 * License: license text or at least name and link
         (example: BSD-3-Clause <https://opensource.org/licenses/BSD-3-Clause>)
 *
 * Version: 1.0.0
 * Creation Date: 2020/09/09
 */

/*=====[Inclusions of function dependencies]=================================*/

#include "Cuaima_Testing.h"
#include "stdio.h"

static bool Insert(int id, int Code,RF_List_t* RF_List){
	
	RF_Device_t * Mem_Block;
	Mem_Block = (RF_Device_t *)malloc(sizeof(RF_Device_t));
	bool rtn=0;
	
	if(RF_List->counter<MAX_RF_DEVICES)
	{
		if(NULL!=Mem_Block)
		{
			Mem_Block[0].Node_ID=id;
			Mem_Block[0].RF_Code=Code;
			Mem_Block[0].updated=1;
			RF_List->RF_Devices[RF_List->counter]=Mem_Block;
			RF_List->counter++;
			rtn=1;
		}
	}
	return rtn;
}

int Header_Check(int id, RF_List_t* RF_List){
	int rtn=333;
	int i;
	RF_Device_t * Mem_Block;
	
	for(i=0;i<RF_List->counter;i++)
	{
		Mem_Block=RF_List->RF_Devices[i];
		if(id==Mem_Block[0].Node_ID)
		{
			rtn=i;
		}	
	}
	return rtn;
}

bool Header_Validation(int id, int Code, RF_List_t* RF_List){
	
	bool rtn=1;
	int aux;
	RF_Device_t * Mem_Block;
	
	
	aux = Header_Check(id,RF_List);
	if(333==aux)
	{
		rtn = Insert(id,Code,RF_List);
		if(!rtn)
		{
			rtn=0;
		}
	}else
	{
		Mem_Block=RF_List->RF_Devices[aux];
		Mem_Block[0].RF_Code=Code;
		Mem_Block[0].updated=1;
	}
	return rtn;
}

int Comm_Code(RF_List_t* RF_List){
	int Final_Code;
	bool Alarm=0;
	bool Fail=0;
	int i;
	int counter=0;
	RF_Device_t * Mem_Block;
	
	for(i=0;i<RF_List->counter;i++)
	{
		Mem_Block=RF_List->RF_Devices[i];
		if(Mem_Block[0].updated)
		{
			counter++;
			Mem_Block[0].updated=0;
			if(ALARM_FAIL_CODE==Mem_Block[0].RF_Code)
			{
				Alarm=1;
				Fail=1;
				break;
			}
		
			if(ALARM_CODE==Mem_Block[0].RF_Code)
			{
				Alarm=1;
			}
			
			if(FAIL_CODE==Mem_Block[0].RF_Code)
			{
				Fail=1;
			}
		}
	}	
	if(RF_List->counter!=counter){
		RF_List->comm_incomplete=1;
	}
	if((Alarm)&&(Fail))
	{
		Final_Code = ALARM_FAIL_CODE;
	}else if(Alarm)
	{
		Final_Code = ALARM_CODE;
	}else if(Fail)
	{
		Final_Code = FAIL_CODE;
	}else
	{
		Final_Code = NORMAL_CODE;
	}
	return Final_Code;
}

void Clean_RFDevices(RF_List_t* RF_List){
	int k;
	
	for(k=0;k<RF_List->counter;k++)
	{
		free(RF_List->RF_Devices[k]);
		RF_List->counter=0;
	}
}

