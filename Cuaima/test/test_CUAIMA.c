/*
 **===== Casos de Prueba ---¡COMPLETADAS!---=====
 * -- Recibir codigos y comparar con la lista. 1-> Existe, 0-> No Existe 
 * -- En caso de no existir dentro agregarlo a la lista  
 * -- Extraer los codigos y comparar para determinar cual es el estado correcto 
 * -- Trabajar con Memoria Dinamica para anadir RFDevices limite 20
 * -- Si se alcanza el limite de 20 retornar error 0 en caso contrario 1
 * -- Validar que todos los elementos contenidos fueron Actualizados o no
 *      1-->Incomplete 0--> OK!
 * -- Poder Refrescar el codigo de un id existente y notificar el refresco
 *
 **===== Casos de Prueba ---¡PENDIENTES!---=====
 *    
 * Como gestionar la recepcion? cuando dictar el estado? While un tiempo esperar... si 
 * se mantiene incomplete..... Dictamoso un estado y modificamos el estado aINCOMPLETE 
 * 
 * FSM
 * 
*/

#include "unity.h"
#include "Cuaima_Testing.h"

#define N_INICIAL 3

typedef struct Casos_Prueba_s {
	int ID;
	int Code;
	bool result;
} Casos_Prueba_t;

static const Casos_Prueba_t Casos_Pruebas[]=
{
	{
		.ID = 1,
		.Code = NORMAL_CODE,
	},
	{
		.ID = 2,
		.Code = NORMAL_CODE,
	},
	{
		.ID = 5,
		.Code = NORMAL_CODE,
	},	
};


RF_List_t RF_List;

static int caso_actual = 0;
int n = sizeof(Casos_Pruebas) / sizeof(struct Casos_Prueba_s);

void setUp(void)
{
	RF_Device_t * Mem_Block;
	RF_List.counter=0;
	RF_List.comm_incomplete=0;
	
	for(caso_actual = 0; caso_actual <n; caso_actual++)
	{
		Mem_Block = (RF_Device_t *)malloc(sizeof(RF_Device_t));
		if(NULL!=Mem_Block)
		{
			Mem_Block[0].Node_ID=Casos_Pruebas[caso_actual].ID;
			Mem_Block[0].RF_Code=Casos_Pruebas[caso_actual].Code;
			RF_List.RF_Devices[RF_List.counter]=Mem_Block;
			RF_List.counter++;
		}
	}
}

void tearDown(void)
{
	int k;
	RF_Device_t * Mem_Block;
	
	for(k=0;k<RF_List.counter;k++)
	{
		Mem_Block=RF_List.RF_Devices[k];
#if defined(DEBUG)
		printf("%d- Liberando Memoria del Nodo #%d \n",k,Mem_Block[0].Node_ID);
#endif
		free(RF_List.RF_Devices[k]);
	}
}

//! @test Prueba de error ante Estado no definido.
void test_Recepcion_de_Codigo(void){
	
	int rtn=0;
	int  header_from=1;
		
	rtn = Header_Check(header_from,&RF_List);
	TEST_ASSERT_EQUAL(0,rtn);	// --> Prueba de Transicion Correcta de Estado
	
	header_from=10;
		
	rtn = Header_Check(header_from,&RF_List);
	TEST_ASSERT_EQUAL(333,rtn);	// --> Prueba de Transicion Correcta de Estado
	
	header_from=5;
		
	rtn = Header_Check(header_from,&RF_List);
	TEST_ASSERT_EQUAL(2,rtn);	// --> Prueba de Transicion Correcta de Estado
}

//! @test Prueba de error ante Estado no definido.
void test_No_Existe_y_lo_agrego_a_la_lista(void){
	
	bool aux;
	int rtn=0;
	int  header_from=10;
	int Code=NORMAL_CODE;
	
	Header_Validation(header_from,Code,&RF_List);
	
	rtn = Header_Check(header_from,&RF_List);
	TEST_ASSERT_EQUAL(3,rtn);	// --> Prueba de Transicion Correcta de Estado
	TEST_ASSERT_EQUAL(n+1,RF_List.counter);	// --> Prueba de Transicion Correcta de Estado
}

//! @test Prueba de error ante Estado no definido.
void test_Update_de_Codigos(void){
	
	bool aux;
	int rtn=0;
	int  header_from=10;
	int Code=FAIL_CODE;
	char Text_ID[30];
	RF_Device_t * Mem_Block;
	
	for(caso_actual = 0; caso_actual <n; caso_actual++)
	{
		Mem_Block=RF_List.RF_Devices[caso_actual];
		aux = Header_Validation(Casos_Pruebas[caso_actual].ID,Code,&RF_List);
		
		if(!aux)
		{
			printf("Error Nro Max Alcanzado\n");
			break;
		}
				
		sprintf(Text_ID,"Caso de Prueba Nro: %d",caso_actual);
		TEST_ASSERT_EQUAL_MESSAGE(FAIL_CODE,Mem_Block[0].RF_Code,Text_ID);
		TEST_ASSERT_EQUAL_MESSAGE(1,Mem_Block[0].updated,Text_ID);
	}
	
}

//! @test Prueba de error ante Estado no definido.
void test_Evaluacion_de_Diferentes_Codigos(void){
	
	dprim_state_t result;
	int  header_from=10;
	int Code=NORMAL_CODE;
	RF_Device_t * Mem_Block;
	int k;
	
	result=Comm_Code(&RF_List);
	TEST_ASSERT_EQUAL(NORMAL_CODE,result);	// --> Prueba de Transicion Correcta de Estado
	TEST_ASSERT_EQUAL(0,RF_List.comm_incomplete);	// --> Prueba de Transicion Correcta de Estado
	
	header_from=11;
	Code=FAIL_CODE;
	
	Header_Validation(header_from,Code,&RF_List);
	
	result=Comm_Code(&RF_List);
	TEST_ASSERT_EQUAL(FAIL_CODE,result);	// --> Prueba de Transicion Correcta de Estado
	TEST_ASSERT_EQUAL(1,RF_List.comm_incomplete);
	
	RF_List.comm_incomplete=0;
	for(k=0;k<RF_List.counter;k++)
	{
		Mem_Block=RF_List.RF_Devices[k];
		Header_Validation(Mem_Block[0].Node_ID,Mem_Block[0].RF_Code,&RF_List);
	}
	
	header_from=12;
	Code=ALARM_CODE;
	
	Header_Validation(header_from,Code,&RF_List);
	
	result=Comm_Code(&RF_List);
	TEST_ASSERT_EQUAL(ALARM_FAIL_CODE,result);	// --> Prueba de Transicion Correcta de Estado
	TEST_ASSERT_EQUAL(0,RF_List.comm_incomplete);
	
	for(k=0;k<RF_List.counter;k++)
	{
		Mem_Block=RF_List.RF_Devices[k];
		Header_Validation(Mem_Block[0].Node_ID,Mem_Block[0].RF_Code,&RF_List);
	}
	
	header_from=13;
	Code=ALARM_FAIL_CODE;
	
	Header_Validation(header_from,Code,&RF_List);
	
	result=Comm_Code(&RF_List);
	TEST_ASSERT_EQUAL(ALARM_FAIL_CODE,result);	// --> Prueba de Transicion Correcta de Estado
	TEST_ASSERT_EQUAL(0,RF_List.comm_incomplete);
	printf("Nro de DISPOSITIVOS: %d\n",RF_List.counter);
}

//! @test Prueba de error ante Estado no definido.
void test_Alcanzo_el_limite_MAX_de_Devices(void){
	
	bool aux;
	int  i;
	int Code=NORMAL_CODE;
	
	for (i=0;i<MAX_RF_DEVICES*2;i++)
	{
		aux=Header_Validation(i,Code,&RF_List);
		if(!aux)
		{
			printf("Error Nro Max Alcanzado\n");
			break;
		}
	}
	TEST_ASSERT_EQUAL(0,aux);	// --> Prueba de Transicion Correcta de Estado
	TEST_ASSERT_EQUAL(MAX_RF_DEVICES,RF_List.counter);	// --> Prueba de Transicion Correcta de Estado
}

