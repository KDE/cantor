import shelve
shelvePythonBackend = shelve.open('%1', 'n')
for keyPythonBackend in dir():
  if (not 'PythonBackend' in keyPythonBackend)\
      and (not '__' in keyPythonBackend)\
      and (not '<module ' in str(globals()[keyPythonBackend])):
    shelvePythonBackend[keyPythonBackend] = globals()[keyPythonBackend]
shelvePythonBackend.close()
del(shelve)
del(shelvePythonBackend)
del(keyPythonBackend)
