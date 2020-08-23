#include <iostream>
#include <sstream>
#include <string>		
#include <fstream>						// CREACION DE DOCUMENTACION 
using std::ofstream;
using std::endl;

static std::fstream textfile;
void Text_Init(char *name  ){
//textfile.open("REGISTRO_SISTEMA_ALARMA_INCENDIO.csv", std::ofstream::out | std::ofstream::trunc);
	textfile.open("REGISTRO_SISTEMA_ALARMA_INCENDIO.csv", std::ofstream::out | std::ofstream::trunc);
	textfile <<"DATA LOG ALARM NOTIFICATION SYSTEM"<< endl;
}

void Text_Write(char *text  ){
	//textfile <<(char *)text<< endl;
	textfile <<text<< endl;
}
