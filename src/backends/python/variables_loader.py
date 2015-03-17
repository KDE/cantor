import shelve
shelvePythonBackend = shelve.open('%1')
for keyPythonBackend in shelvePythonBackend:
  globals()[keyPythonBackend] = shelvePythonBackend[keyPythonBackend]

shelvePythonBackend.close()
del(shelve)
del(shelvePythonBackend)
del(keyPythonBackend)
