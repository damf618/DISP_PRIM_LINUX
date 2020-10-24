#include <Cuaima_Testing.h>


//If theres a new nodeID detected, we ask for memory and save it together with the
// code received in the corresponding array
/** The RF system is based on an array of RF_Device_t that allows to detect how many
 * actives nodes are in the system. This system allows too identify if there are enough
 * nodes interacting with the system or not 
	
	@param int id, ID of the new RF Device detected.
	@param int Code Code transmitted by this new Device.
	@param RF_List_t* RF_List Array of RF_Device_t to keep track of the memory required
	from the system
	
	@see Header_Validation,Comm_Code
**/
static bool Insert(int id, int Code,RF_List_t* RF_List)
{
	
	RF_Device_t * Mem_Block;
	Mem_Block = (RF_Device_t *)malloc(sizeof(RF_Device_t));
	bool rtn=0;
	
	//If there is space availble, we add the Device to the RF_LIst array
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

void Comm_Nodes(RF_List_t* RF_List)
{
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
		}
	}
	printf("# N Nodes----Get_Node: %d\n",counter);
	RF_List->active_nodes = counter;
}

//Allows to check if a message  is received, is it from a  new RF device or an old one
int Header_Check(int id, RF_List_t* RF_List)
{
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

static void Update_Node_Data(RF_Device_t RF_Device,Nodes_Database_t* Data_RF_List, int i)
{
	Data_RF_List->Counter++;
	Data_RF_List->Nodes_Data[i].Node_ID=RF_Device.Node_ID;
	Data_RF_List->Nodes_Data[i].RF_Code=RF_Device.RF_Code;
	Data_RF_List->Nodes_Data[i].updated=RF_Device.updated;
}

//The link between RF Communication and primario4, this funciton returns the corresponding
//code based on the RF_List generated between calls.
int Comm_Code(RF_List_t* RF_List,Nodes_Database_t* Data_RF_List)
{
	int Final_Code;
	bool Alarm=0;
	bool Fail=0;
	int i;
	int counter=0;
	RF_Device_t * Mem_Block;
	
#if defined (DEBUG)
			printf("Number of Devices: %d\n",RF_List->counter);
#endif	
	
	Data_RF_List->Counter=0;
	
	for(i=0;i<RF_List->counter;i++)
	{	
		Mem_Block=RF_List->RF_Devices[i];
		Update_Node_Data(Mem_Block[0],Data_RF_List,i);
#if defined (DEBUG)
			printf("#%d Device: %d\n",i,Mem_Block[0].Node_ID);
#endif	
		if(Mem_Block[0].updated)
		{
#if defined (DEBUG)
			printf("#%d Device: %d,State: %d\n",i,Mem_Block[0].Node_ID,Mem_Block[0].RF_Code);
#endif	
			counter++;
			Mem_Block[0].updated=0;
			
			if(ALARM_FAIL_CODE==Mem_Block[0].RF_Code)
			{
				Alarm=1;
				Fail=1;
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
	RF_List->active_nodes = counter;

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


//Allows to check if a message  is received, is it from a  new RF device or an old one
bool Header_Validation(int id, int Code, RF_List_t* RF_List)
{
	
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
