def LastNlines(fname, N, data): 
	del data[:]
	with open(fname) as file: 
		for line in (file.readlines() [-N:]):
			data.append(line)
	return True


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
		t.sleep(0.5)
