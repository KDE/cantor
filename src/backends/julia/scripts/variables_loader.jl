# Variable loading script.
#
# Install JLD script with `Pkg.add(JLD)` to use it
import JLD
for (var_name, value) in JLD.load("%1")
    s = symbol(var_name)
    @eval (($s) = ($value))
end
