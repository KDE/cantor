# Variable loading script
import JLD
for (var_name, value) in JLD.load("%1")
    s = symbol(var_name)
    @eval (($s) = ($value))
end
