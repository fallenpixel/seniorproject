 
OPTFLAGS = -O3

CCC=g++ -std=c++17

SRCMAPALGEBRA = ../map/hseg2DFixedSize.cpp ../map/poi2DFixedSize.cpp ../map/seg2DFixedSize.cpp ../map/mbb2DFixedSize.cpp
INCLUDEMAPALGEBRA = -I ../map

all: pps pps-tbb pps-c17
	 $(info ***** be sure to set lib path to current dir with: export LD_LIBRARY_PATH=.)
	 $(info ***** thrtead affinity env variable: export GOMP_CPU_AFFINITY=0-x, x = num processors)


pps: main.o  libparOverlay.so
	${CCC} ${OPTFLAGS} -o pps -fopenmp -L ./ main.o -l parOverlay

libparOverlay.so:   parPlaneSweep.o
	${CCC} -fopenmp -shared -Wl,-soname,libparOverlay.so.1   -o libparOverlay.so.1.0.1 parPlaneSweep.o 
	ln -f -s libparOverlay.so.1.0.1 libparOverlay.so
	ldconfig  -n .

main.o: main.cpp
	${CCC} ${OPTFLAGS} -c main.cpp 

parPlaneSweep.o: parPlaneSweep.h parPlaneSweep.cpp
	${CCC} ${OPTFLAGS} -fPIC  -c parPlaneSweep.cpp
	
pps-tbb: main.o  libparOverlay-tbb.so
	${CCC} ${OPTFLAGS} -o pps-tbb  -L ./ main.o -l parOverlay-tbb -l tbb

libparOverlay-tbb.so:   parPlaneSweep-tbb.o
	${CCC} -shared -Wl,-soname,libparOverlay-tbb.so.1   -o libparOverlay-tbb.so.1.0.1 parPlaneSweep-tbb.o -l tbb
	ln -f -s libparOverlay-tbb.so.1.0.1 libparOverlay-tbb.so
	ldconfig  -n .

parPlaneSweep-tbb.o: parPlaneSweep.h parPlaneSweep-tbb.cpp
	${CCC} ${OPTFLAGS} -fPIC -c parPlaneSweep-tbb.cpp

pps-c17: main.o  libparOverlay-c17.so
	${CCC} ${OPTFLAGS} -o pps-c17  -L ./ main.o -l parOverlay-c17 -pthread

libparOverlay-c17.so:   parPlaneSweep-c17.o
	${CCC} -shared -Wl,-soname,libparOverlay-c17.so.1   -o libparOverlay-c17.so.1.0.1 parPlaneSweep-c17.o -pthread -ltbb
	ln -f -s libparOverlay-c17.so.1.0.1 libparOverlay-c17.so
	ldconfig  -n .

parPlaneSweep-c17.o: parPlaneSweep.h parPlaneSweep-c17.cpp
	${CCC} ${OPTFLAGS} -fPIC -c parPlaneSweep-c17.cpp


check-syntax:
	${CCC} -o /dev/null -S ${CHK_SOURCES}

clean:
	rm *.o
	rm *.so*
