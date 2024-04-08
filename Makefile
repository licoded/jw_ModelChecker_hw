

CC	    =   g++ -std=c++11
FLAG    = 
DEBUGFLAG   =	-D DEBUG -g -pg
RELEASEFLAG = -O3

	

.PHONY :    release debug clean

release :   $(ALLFILES)
