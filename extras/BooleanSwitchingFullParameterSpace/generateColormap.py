import matplotlib.pyplot as plt
import matplotlib.colors as colors
import matplotlib.cm as cmx
import numpy as np

import sys

# define some random data that emulates your indeded code:
N = int(sys.argv[1])
# np.random.seed(101)
# curves = [np.random.random(20) for i in range(N)]
values = range(N)

paired = cm = plt.get_cmap('Paired')
cNorm  = colors.Normalize(vmin=0, vmax=values[-1])
scalarMap = cmx.ScalarMappable(norm=cNorm, cmap=paired)
print scalarMap.get_clim()

mycolors=[]

print "{",
for idx in values:
	newcolor=colors.rgb2hex(scalarMap.to_rgba(values[idx]))
	mycolors.append(newcolor)
	print "\""+newcolor+"\",",

print "};"
