
FORMULAFILES	=	formula/aalta_formula.cpp formula/af_utils.cpp formula/olg_formula.cpp formula/olg_item.cpp

PARSERFILES		=	ltlparser/ltl_formula.c ltlparser/ltllexer.c ltlparser/ltlparser.c ltlparser/trans.c 

SOLVER			=	minisat/core/Solver.cc aaltasolver.cpp solver.cpp carsolver.cpp 

CHECKING		=	ltlfchecker.cpp carchecker.cpp evidence.cpp

UTILFILES		=	util/utility.cpp utility.cpp

AIGERFILES		=	aiger/aiger.o

ALLFILES		=	test.cpp $(CHECKING) $(SOLVER) $(FORMULAFILES) $(PARSERFILES) $(UTILFILES) $(AIGERFILES)


CC	    	=   g++ -std=c++11
FLAG    	= -I./ -I./minisat/ -isystem./minisat -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS -fpermissive #-fsanitize=address -fno-omit-frame-pointer
DEBUGFLAG   =	-D DEBUG -g -pg
RELEASEFLAG = -O3

ltlfsyn :	release

ltlparser/ltllexer.c :
	ltlparser/grammar/ltllexer.l
	flex ltlparser/grammar/ltllexer.l

ltlparser/ltlparser.c :
	ltlparser/grammar/ltlparser.y
	bison ltlparser/grammar/ltlparser.y
	
	

.PHONY :    release debug clean

release :   $(ALLFILES)
	    $(CC) $(FLAG) $(RELEASEFLAG) $(ALLFILES) -lm -lz -o aiger-mc

debug :	$(ALLFILES)
	$(CC) $(FLAG) $(DEBUGFLAG) $(ALLFILES) -lm -lz -o aiger-mc

clean :
	rm -f *.o *~ aiger-mc
