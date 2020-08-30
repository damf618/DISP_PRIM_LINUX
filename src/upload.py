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

with open ("STATES_LOG.txt", 'rt') as myfile:  # Open lorem.txt for reading text
    contents = myfile.read()              # Read the entire file to a string
print(contents)                           # Print the string

with open ("STATES_LOG.txt", 'rt') as myfile:  # Open lorem.txt for reading
    for myline in myfile:              # For each line, read to a string,
        print(myline)                  # and print the string.

mylines =[]
with open ("STATES_LOG.txt", 'rt') as myfile:  # Open lorem.txt for reading
    for myline in myfile:              # For each line, read to a string,
		mylines.append(myline)

print("PRUEBA 1")    
print(myline)

print("PRUEBA 2")
print(myline[0]) 
print(myline[1])
print(myline[2])
print(myline[3])

print("PRUEBA 3")
print(mylines[0])
print(mylines[1])

data =  { 'Extra': 'TRUE',
          'RFDSecundario': 'TRUE',
          }
result = firebase.patch('/SISTEMA_PRUEBA_RPI/COMRF',data)
print(result)


data =  { 'Alarma': 'TRUE',
          'Extra': 'FALSE',
		  'Falla': 'FALSE',
          }
result = firebase.patch('/SISTEMA_PRUEBA_RPI/Contactos',data)
print(result)


data =  { 'Alarma': 'TRUE',
          'Falla': 'TRUE',
          }
result = firebase.patch('/SISTEMA_PRUEBA_RPI/PROPIOS',data)
print(result)
