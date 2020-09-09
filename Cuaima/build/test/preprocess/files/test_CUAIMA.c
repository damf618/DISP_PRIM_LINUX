#include "build/temp/_test_CUAIMA.c"
#include "src/Cuaima_Testing.h"
#include "/var/lib/gems/2.5.0/gems/ceedling-0.30.0/vendor/unity/src/unity.h"






typedef struct Casos_Prueba_s {

 int ID;

 int Code;

 

_Bool 

     result;

} Casos_Prueba_t;



static const Casos_Prueba_t Casos_Pruebas[]=

{

 {

  .ID = 1,

  .Code = 321,

 },

 {

  .ID = 2,

  .Code = 321,

 },

 {

  .ID = 5,

  .Code = 321,

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

  if(

    ((void *)0)

        !=Mem_Block)

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







  free(RF_List.RF_Devices[k]);

 }

}





void test_Recepcion_de_Codigo(void){



 int rtn=0;

 int header_from=1;



 rtn = Header_Check(header_from,&RF_List);

 UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((rtn)), (

((void *)0)

), (UNITY_UINT)(91), UNITY_DISPLAY_STYLE_INT);



 header_from=10;



 rtn = Header_Check(header_from,&RF_List);

 UnityAssertEqualNumber((UNITY_INT)((333)), (UNITY_INT)((rtn)), (

((void *)0)

), (UNITY_UINT)(96), UNITY_DISPLAY_STYLE_INT);



 header_from=5;



 rtn = Header_Check(header_from,&RF_List);

 UnityAssertEqualNumber((UNITY_INT)((2)), (UNITY_INT)((rtn)), (

((void *)0)

), (UNITY_UINT)(101), UNITY_DISPLAY_STYLE_INT);

}





void test_No_Existe_y_lo_agrego_a_la_lista(void){



 

_Bool 

     aux;

 int rtn=0;

 int header_from=10;

 int Code=321;



 Header_Validation(header_from,Code,&RF_List);



 rtn = Header_Check(header_from,&RF_List);

 UnityAssertEqualNumber((UNITY_INT)((3)), (UNITY_INT)((rtn)), (

((void *)0)

), (UNITY_UINT)(115), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((n+1)), (UNITY_INT)((RF_List.counter)), (

((void *)0)

), (UNITY_UINT)(116), UNITY_DISPLAY_STYLE_INT);

}





void test_Update_de_Codigos(void){



 

_Bool 

     aux;

 int rtn=0;

 int header_from=10;

 int Code=654;

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

  UnityAssertEqualNumber((UNITY_INT)((654)), (UNITY_INT)((Mem_Block[0].RF_Code)), ((Text_ID)), (UNITY_UINT)(141), UNITY_DISPLAY_STYLE_INT);

  UnityAssertEqualNumber((UNITY_INT)((1)), (UNITY_INT)((Mem_Block[0].updated)), ((Text_ID)), (UNITY_UINT)(142), UNITY_DISPLAY_STYLE_INT);

 }



}





void test_Evaluacion_de_Diferentes_Codigos(void){



 dprim_state_t result;

 int header_from=10;

 int Code=321;

 RF_Device_t * Mem_Block;

 int k;



 result=Comm_Code(&RF_List);

 UnityAssertEqualNumber((UNITY_INT)((321)), (UNITY_INT)((result)), (

((void *)0)

), (UNITY_UINT)(157), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((RF_List.comm_incomplete)), (

((void *)0)

), (UNITY_UINT)(158), UNITY_DISPLAY_STYLE_INT);















 header_from=11;

 Code=654;



 Header_Validation(header_from,Code,&RF_List);



 result=Comm_Code(&RF_List);

 UnityAssertEqualNumber((UNITY_INT)((654)), (UNITY_INT)((result)), (

((void *)0)

), (UNITY_UINT)(172), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((1)), (UNITY_INT)((RF_List.comm_incomplete)), (

((void *)0)

), (UNITY_UINT)(173), UNITY_DISPLAY_STYLE_INT);



 RF_List.comm_incomplete=0;

 for(k=0;k<RF_List.counter;k++)

 {

  Mem_Block=RF_List.RF_Devices[k];

  Header_Validation(Mem_Block[0].Node_ID,Mem_Block[0].RF_Code,&RF_List);

 }



 header_from=12;

 Code=987;



 Header_Validation(header_from,Code,&RF_List);



 result=Comm_Code(&RF_List);

 UnityAssertEqualNumber((UNITY_INT)((258)), (UNITY_INT)((result)), (

((void *)0)

), (UNITY_UINT)(188), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((RF_List.comm_incomplete)), (

((void *)0)

), (UNITY_UINT)(189), UNITY_DISPLAY_STYLE_INT);



 for(k=0;k<RF_List.counter;k++)

 {

  Mem_Block=RF_List.RF_Devices[k];

  Header_Validation(Mem_Block[0].Node_ID,Mem_Block[0].RF_Code,&RF_List);

 }



 header_from=13;

 Code=258;



 Header_Validation(header_from,Code,&RF_List);



 result=Comm_Code(&RF_List);

 UnityAssertEqualNumber((UNITY_INT)((258)), (UNITY_INT)((result)), (

((void *)0)

), (UNITY_UINT)(203), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((RF_List.comm_incomplete)), (

((void *)0)

), (UNITY_UINT)(204), UNITY_DISPLAY_STYLE_INT);

 printf("Nro de DISPOSITIVOS: %d\n",RF_List.counter);

}





void test_Alcanzo_el_limite_MAX_de_Devices(void){



 

_Bool 

     aux;

 int i;

 int Code=321;



 for (i=0;i<20*2;i++)

 {

  aux=Header_Validation(i,Code,&RF_List);

  if(!aux)

  {

   printf("Error Nro Max Alcanzado\n");

   break;

  }

 }

 UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((aux)), (

((void *)0)

), (UNITY_UINT)(224), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((20)), (UNITY_INT)((RF_List.counter)), (

((void *)0)

), (UNITY_UINT)(225), UNITY_DISPLAY_STYLE_INT);

}
