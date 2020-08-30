from firebase import firebase
import time as t

firebase = firebase.FirebaseApplication('https://maestria1-24022020.firebaseio.com/', None)

#data =  { 'Extra': 'FALSE',
#          'RFDSecundario': 'FALSE',
#          }
#result = firebase.post('/SISTEMA_PRUEBA_RPI/COMRF',data)
#print(result)

#data =  { 'Alarma': 'TRUE',
#          'Extra': 'TRUE',
#		  'Falla': 'TRUE',
#          }
#result = firebase.post('/SISTEMA_PRUEBA_RPI/Contactos',data)
#print(result)

#data =  { 'Alarma': 'FALSE',
#          'Falla': 'FALSE',
#          }
#result = firebase.post('/SISTEMA_PRUEBA_RPI/PROPIOS',data)
#print(result)


#myfile = open("STATES_LOG.txt", "rt") # open lorem.txt for reading text
#contents = myfile.read()         	  # read the entire file to string
#myfile.close()                   	  # close the file
#print(contents)                  	  # print string contents
#contents

#Asumimos que la estructura ya fue declarada dentro del sistema. 

def LastNlines(fname, N, data): 
	del data[:]
	with open(fname) as file: 
		for line in (file.readlines() [-N:]):
			data.append(line)
	return True
			

fname= "STATES_LOG.txt"
N = 3
Refresh_data=[]

while(True):
	reading=False
	try: 
		reading=LastNlines(fname, N, Refresh_data) 
	except: 
		print("Error Abriendo archivo")
	
	if(reading==True):
		try:
			data =  { 
					'RFDSecundario': Refresh_data[2],
					}
			result = firebase.patch('/SISTEMA DE DETECCION INCENDIO DAMF/COM_RF',data)
			print(result)

			data =  { 
					'Reciente': Refresh_data[0],
					}
			result = firebase.patch('/SISTEMA DE DETECCION INCENDIO DAMF/Conexion_Reciente',data)
			print(result)

			data =  { 'Estado': Refresh_data[1],
					}
			result = firebase.patch('/SISTEMA DE DETECCION INCENDIO DAMF/Estado_Local',data)
			print(result)
			t.sleep(0.5)
		except:
			print("No new Data Available")
			t.sleep(1)
	else:
		t.sleep(0.1)
