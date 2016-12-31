# Makefile for SWMM5

objs = swmm5.o climate.o controls.o culvert.o datetime.o dwflow.o dynwave.o error.o \
       exfil.o findroot.o flowrout.o forcmain.o gage.o gwater.o hash.o hotstart.o iface.o \
       infil.o inflow.o input.o inputrpt.o keywords.o kinwave.o landuse.o lid.o \
       lidproc.o link.o massbal.o mathexpr.o mempool.o node.o odesolve.o output.o \
       project.o qualrout.o rain.o rdii.o report.o roadway.o routing.o runoff.o shape.o snow.o \
       stats.o statsrpt.o subcatch.o surfqual.o table.o toposort.o toolkitAPI.o transect.o treatmnt.o xsect.o


swmm5 : $(objs)
	gcc-4.9 -fopenmp -shared -o swmm5.so -lm -lpthread $(objs)

swmm5.o       : consts.h macros.h enums.h error.h datetime.h objects.h funcs.h text.h globals.h swmm5.h
climate.o     : headers.h
controls.o    : headers.h
culvert.o     : headers.h
datetime.o    : datetime.h
dwflow.o      : headers.h
dynwave.o     : headers.h
error.o       : error.h
exfil.o       : headers.h infil.h exfil.h
findroot.o    : findroot.h
flowrout.o    : headers.h
forcmain.o    : headers.h
gage.o        : headers.h
gwater.o      : headers.h odesolve.h
hash.o        : hash.h
hotstart      : headers.h
iface.o       : headers.h
infil.o       : headers.h infil.h
inflow.o      : headers.h
input.o       : headers.h lid.h
inputrpt.o    : headers.h lid.h
keywords.o    : text.h
kinwave.o     : headers.h findroot.h
landuse.o     : headers.h
lid.o         : headers.h infil.h lid.h
lidproc.o     : headers.h lid.h
link.o        : headers.h
massbal.o     : headers.h
mathexpr.o    : mathexpr.h
mempool.o     : mempool.h
node.o        : headers.h findroot.h
odesolve.o    : odesolve.h
output.o      : headers.h
project.o     : headers.h hash.h lid.h mempool.h
qualrout.o    : headers.h
rain.o        : headers.h
rdii.o        : headers.h
report.o      : headers.h
roadway.o     : headers.h
routing.o     : headers.h
runoff.o      : headers.h odesolve.h
shape.o       : headers.h
snow.o        : headers.h
stats.o       : headers.h
statsrpt.o    : headers.h lid.h
subcatch.o    : headers.h lid.h odesolve.h
surfqual.o    : headers.h lid.h
table.o       : headers.h
toolkitAPI.o  : headers.h swmm5.h toolkitAPI.h hash.h
toposort.o    : headers.h
transect.o    : headers.h
treatmnt.o    : headers.h
xsect.o       : headers.h findroot.h

