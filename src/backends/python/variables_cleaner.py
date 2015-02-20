for keyPythonBackend in dir():
  if (not 'PythonBackend' in keyPythonBackend)\
      and (not '__' in keyPythonBackend):
    del(globals()[keyPythonBackend])

del(keyPythonBackend)