import sys

class CatchOutPythonBackend:
  def __init__(self):
    self.value = ''
  def write(self, txt):
    self.value += txt

outputPythonBackend = CatchOutPythonBackend()
errorPythonBackend  = CatchOutPythonBackend()
sys.stdout = outputPythonBackend
sys.stderr = errorPythonBackend