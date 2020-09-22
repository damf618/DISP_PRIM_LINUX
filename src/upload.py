from firebase import firebase
import threading
import subprocess
import time as t
import signal

firebase = firebase.FirebaseApplication('https://maestria1-24022020.firebaseio.com/', None)
fname= "STATES_LOG.txt"
N = 4
Refresh_data=[]

class myThread (threading.Thread):
   def __init__(self, threadID, name):
      threading.Thread.__init__(self)
      self.threadID = threadID
      self.name = name
      self.shutdown_flag = threading.Event()

   def run(self):

      print ("Starting " + self.name)
      subprocess.call("./DISP_PRIM")
      print('Thread #{}, {} stopped'.format(self.threadID,self.name))
      self.shutdown_flag.set()
      

      

class ServiceExit(Exception):
    """
    Custom exception which is used to trigger the clean exit
    of all running threads and the main program.
    """
    pass


def service_shutdown():
    print('Closing signal Received')
    raise ServiceExit()


def LastNlines(fname, N, data): 
	del data[:]
	with open(fname) as file: 
		for line in (file.readlines() [-N:]):
			data.append(line)
	return True
	
	
def main():
	print('Starting main program')
	
	thread1 = myThread(1, "Comunicacion RF")
	thread1.start()
	while not thread1.shutdown_flag.is_set():
		try:
			reading=LastNlines(fname, N, Refresh_data)
			data =  { 'RFDSecundario': Refresh_data[2],}
			result = firebase.patch('/SISTEMA DE DETECCION INCENDIO DAMF/COM_RF',data)
		    
			data =  {'Reciente': Refresh_data[0],}
			result = firebase.patch('/SISTEMA DE DETECCION INCENDIO DAMF/Conexion_Reciente',data)
			    
			data =  { 'Estado': Refresh_data[1],}
			result = firebase.patch('/SISTEMA DE DETECCION INCENDIO DAMF/Estado_Local',data)
			    
			data =  { 'RFDSecundario': Refresh_data[3],}
                        result = firebase.patch('/SISTEMA DE DETECCION INCENDIO DAMF/ACTIVE_NODES',data)
            
			t.sleep(0.5) 
		except:
			t.sleep(1.25)
	print('Exiting main program')
	thread1.join()
 
 
if __name__ == '__main__':
    main()
	
	
