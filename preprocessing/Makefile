OTFLAGS = -O3 

CCC=g++ -std=c++17 -g

SRCMAPALGEBRA = ../map/hseg2DFixedSize.cpp ../map/poi2DFixedSize.cpp ../map/seg2DFixedSize.cpp ../map/mbb2DFixedSize.cpp
INCLUDEMAPALGEBRA = -I ../map

all: pps serialrecombine mutexlock conditionalmutex altconditionalmutex tmerge
	 $(info ***** be sure to set lib path to current dir with: export LD_LIBRARY_PATH=.)
	 $(info ***** thrtead affinity env variable: export GOMP_CPU_AFFINITY=0-x, x = num processors)


pps: main.o  libparOverlay.so
	${CCC} ${OPTFLAGS} -o pps -L ./ main.o -l parOverlay  -ltbb

serialrecombine: main.o  libparOverlay-serialrecombine.so
	${CCC} ${OPTFLAGS} -o serialrecombine -L ./ main.o -l parOverlay-serialrecombine -ltbb

mutexlock: main.o  libparOverlay-mutexlock.so
	${CCC} ${OPTFLAGS} -o mutexlock -L ./ main.o -l parOverlay-mutexlock -ltbb

conditionalmutex: main.o  libparOverlay-conditionalmutex.so
	${CCC} ${OPTFLAGS} -o conditionalmutex -L ./ main.o -l parOverlay-conditionalmutex -ltbb

altconditionalmutex: main.o  libparOverlay-altconditionalmutex.so
	${CCC} ${OPTFLAGS} -o altconditionalmutex -L ./ main.o -l parOverlay-altconditionalmutex -ltbb

tmerge: main.o  libparOverlay-tmerge.so
	${CCC} ${OPTFLAGS} -o tmerge -L ./ main.o -l parOverlay-tmerge -ltbb

libparOverlay.so:   parPlaneSweep.o
	${CCC} -shared -Wl,-soname,libparOverlay.so.1   -o libparOverlay.so.1.0.1 parPlaneSweep.o -ltbb
	ln -f -s libparOverlay.so.1.0.1 libparOverlay.so
	ldconfig  -n .

libparOverlay-serialrecombine.so:   parPlaneSweep-serialrecombine.o
	${CCC} -shared -Wl,-soname,libparOverlay-serialrecombine.so.1   -o libparOverlay-serialrecombine.so.1.0.1 parPlaneSweep-serialrecombine.o -ltbb
	ln -f -s libparOverlay-serialrecombine.so.1.0.1 libparOverlay-serialrecombine.so
	ldconfig  -n .

libparOverlay-mutexlock.so:   parPlaneSweep-mutexlock.o
	${CCC} -shared -Wl,-soname,libparOverlay-mutexlock.so.1   -o libparOverlay-mutexlock.so.1.0.1 parPlaneSweep-mutexlock.o -ltbb
	ln -f -s libparOverlay-mutexlock.so.1.0.1 libparOverlay-mutexlock.so
	ldconfig  -n .

libparOverlay-conditionalmutex.so:   parPlaneSweep-conditionalmutex.o
	${CCC} -shared -Wl,-soname,libparOverlay-conditionalmutex.so.1   -o libparOverlay-conditionalmutex.so.1.0.1 parPlaneSweep-conditionalmutex.o -ltbb
	ln -f -s libparOverlay-conditionalmutex.so.1.0.1 libparOverlay-conditionalmutex.so
	ldconfig  -n .

libparOverlay-altconditionalmutex.so:   parPlaneSweep-altconditionalmutex.o
	${CCC} -shared -Wl,-soname,libparOverlay-altconditionalmutex.so.1   -o libparOverlay-altconditionalmutex.so.1.0.1 parPlaneSweep-altconditionalmutex.o -ltbb
	ln -f -s libparOverlay-altconditionalmutex.so.1.0.1 libparOverlay-altconditionalmutex.so
	ldconfig  -n .

libparOverlay-tmerge.so:   parPlaneSweep-tmerge.o
	${CCC} -shared -Wl,-soname,libparOverlay-tmerge.so.1   -o libparOverlay-tmerge.so.1.0.1 parPlaneSweep-tmerge.o -ltbb
	ln -f -s libparOverlay-tmerge.so.1.0.1 libparOverlay-tmerge.so
	ldconfig  -n .

main.o: main.cpp
	${CCC} ${OPTFLAGS} -c main.cpp -ltbb


parPlaneSweep.o: parPlaneSweep.h parPlaneSweep.cpp
	${CCC} ${OPTFLAGS} -fPIC -c parPlaneSweep.cpp -ltbb

parPlaneSweep-serialrecombine.o: parPlaneSweep.h parPlaneSweep-serialrecombine.cpp
	${CCC} ${OPTFLAGS} -fPIC -c parPlaneSweep-serialrecombine.cpp -ltbb

parPlaneSweep-mutexlock.o: parPlaneSweep.h parPlaneSweep-mutexlock.cpp
	${CCC} ${OPTFLAGS} -fPIC -c parPlaneSweep-mutexlock.cpp -ltbb

parPlaneSweep-conditionalmutex.o: parPlaneSweep.h parPlaneSweep-conditionalmutex.cpp
	${CCC} ${OPTFLAGS} -fPIC -c parPlaneSweep-conditionalmutex.cpp -ltbb

parPlaneSweep-altconditionalmutex.o: parPlaneSweep.h parPlaneSweep-altconditionalmutex.cpp
	${CCC} ${OPTFLAGS} -fPIC -c parPlaneSweep-altconditionalmutex.cpp -ltbb

parPlaneSweep-tmerge.o: parPlaneSweep.h parPlaneSweep-tmerge.cpp
	${CCC} ${OPTFLAGS} -fPIC -c parPlaneSweep-tmerge.cpp -ltbb

check-syntax:
	${CCC} -o /dev/null -S ${CHK_SOURCES}

clean:
	rm *.o
	rm *.so*
