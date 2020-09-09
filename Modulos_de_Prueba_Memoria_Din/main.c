/*==================[inclusiones]============================================*/
#include <stdio.h>
#include <stdlib.h> /* declarations for malloc(), free() etc. */

/*==================[definiciones y macros]==================================*/
#define MAX_NODE_ID 3
/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
typedef struct RF_Device_s 
{
	int Node_ID;
	int RF_Code;
} RF_Device_t;


int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
    //RF_Device_t RFdevice;
    int i;
    int n = MAX_NODE_ID;
    RF_Device_t * Mem_Block;
    int mem_flag;
	RF_Device_t * Mem_Block_Array[n];
	int Mem_Block_Counter=0;
		
    //Mem_Block = (RF_Device_t *)malloc(n * sizeof(RF_Device_t));
	
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( 1 )
    {
		/*
		for(i=0;i<n;i++)
		{
			printf("ID del nodo: \n");
			scanf("%d", &RFdevice.Node_ID);
			printf("RF Code del nodo #%d: \n",RFdevice.Node_ID);
			scanf("%d", &RFdevice.RF_Code);
			
			Mem_Block[i]=RFdevice;
		}
		for(i=0;i<n;i++){
			printf("%d- El codigo del Nodo #%d es : %d \n",i,Mem_Block[i].Node_ID,
													Mem_Block[i].RF_Code);
		}
		*/
		printf("Necesitamos mas memoria?");
		scanf("%d",&mem_flag);
		if (1==mem_flag)
		{
			if(n>Mem_Block_Counter)
			{
				Mem_Block = (RF_Device_t *)malloc(sizeof(RF_Device_t));
				if(NULL!=Mem_Block)
				{
					printf("ID del nodo: \n");
					scanf("%d", &Mem_Block[0].Node_ID);
					printf("RF Code del nodo #%d: \n",Mem_Block[0].Node_ID);
					scanf("%d", &Mem_Block[0].RF_Code);
					Mem_Block_Array[Mem_Block_Counter]=Mem_Block;
					Mem_Block_Counter++;
				}
				else{
					printf("Imposible en este momento \n");
				}
			}
			else
			{
				printf("Max Number of Nodes was reached");
				break;
			}	
		}else 
		{
			
			for(i=0;i<Mem_Block_Counter;i++)
			{
				Mem_Block=Mem_Block_Array[i];
				printf("%d- El codigo del Nodo #%d es : %d \n",i,Mem_Block[0].Node_ID,
													Mem_Block[0].RF_Code);
			}
			break;
		}
    }
    for(i=0;i<Mem_Block_Counter;i++)
    {
		Mem_Block=Mem_Block_Array[i];
		printf("%d- Liberando Memoria del Nodo #%d \n",i,Mem_Block[0].Node_ID);
		free(Mem_Block_Array[i]);
	}
    
    // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
    // directamenteno sobre un microcontroladore y no es llamado por ningun
    // Sistema Operativo, como en el caso de un programa para PC.
    return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

/*==================[fin del archivo]========================================*/
