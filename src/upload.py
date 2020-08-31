from firebase import firebase
import threading
import subprocess
import time as t

firebase = firebase.FirebaseApplication('https://maestria1-24022020.firebaseio.com/', None)
fname= "STATES_LOG.txt"
N = 3
Refresh_data=[]

class myThread (threading.Thread):
   def __init__(self, threadID, name):
      threading.Thread.__init__(self)
      self.threadID = threadID
      self.name = name
   def run(self):
      print "Starting " + self.name
      subprocess.call("./DISP_PRIM")

def LastNlines(fname, N, data): 
	del data[:]
	with open(fname) as file: 
		for line in (file.readlines() [-N:]):
			data.append(line)
	return True
	

thread1 = myThread(1, "Dispositivo Secundario")
thread1.start()
			
while(True):
	reading=False
	try: 
		reading=LastNlines(fname, N, Refresh_data) 
	except: 
		#print("Error Abriendo archivo")
		reading=False
	
	if(reading==True):
		try:
			data =  { 
					'RFDSecundario': Refresh_data[2],
					}
			result = firebase.patch('/SISTEMA DE DETECCION INCENDIO DAMF/COM_RF',data)
			#print(result)

			data =  { 
					'Reciente': Refresh_data[0],
					}
			result = firebase.patch('/SISTEMA DE DETECCION INCENDIO DAMF/Conexion_Reciente',data)
			#print(result)

			data =  { 'Estado': Refresh_data[1],
					}
			result = firebase.patch('/SISTEMA DE DETECCION INCENDIO DAMF/Estado_Local',data)
			#print(result)
			t.sleep(0.5)
		except:
			#print("No new Data Available")
			t.sleep(1)
	else:
		t.sleep(0.1)
