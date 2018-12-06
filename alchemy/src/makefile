INLINE_LIMIT=-finline-limit=100000
#INLINE_WARN=-Winline
STATIC=-static

######### Profile
#CLFLAGS=-pg -g $(STATIC) -DNDEBUG $(INLINE_WARN) $(INLINE_LIMIT) 

######### Profile
#CLFLAGS=-pg -g -DNDEBUG $(INLINE_WARN) $(INLINE_LIMIT) 

######### Profile & optimize
#CLFLAGS=-pg -g -O3 $(STATIC) -DNDEBUG $(INLINE_WARN) $(INLINE_LIMIT) 

######## Optimize
#CLFLAGS=-O3 $(STATIC) $(INLINE_WARN) $(INLINE_LIMIT)

######## Optimize and turn off asserts
CLFLAGS=-O3 -DNDEBUG $(STATIC) $(INLINE_WARN) $(INLINE_LIMIT)

########## Debug
#CLFLAGS=-g

BASEDIR=..

GPP=g++
FLEX=flex
BISON=bison

BIN=$(BASEDIR)/bin
OBJDIR=$(BASEDIR)/bin/obj
MF=makefile

SRCDIR=$(BASEDIR)/src
UTIL=$(SRCDIR)/util
PARSER=$(SRCDIR)/parser
LOGIC=$(SRCDIR)/logic
INFER=$(SRCDIR)/infer
LEARNWTS=$(SRCDIR)/learnwts
LEARNSTRUCT=$(SRCDIR)/learnstruct

REPLACEFOLCPP=$(PARSER)/replacefolcpp.pl

LIBCINC= -I. -I$(UTIL) -I$(LOGIC) -I$(PARSER) -I$(LEARNWTS) -I$(LEARNSTRUCT) \
	-I$(INFER)
LIBLINC= -L. -L$(UTIL) -L$(LOGIC) -L$(PARSER) -L$(LEARNWTS) -L$(LEARNSTRUCT) \
	-L$(INFER)
LIBLINK= -ldl 

CFLAGS=$(CLFLAGS) -Wno-deprecated -Wall $(LIBCINC)
LFLAGS=$(CLFLAGS) -Wno-deprecated -Wall $(LIBLINC)


#############################################################################
#add non-executables .cpp files to OBJS 
#if .cpp file reside in a dir other than the current one, add .o rule below
OBJS=$(OBJDIR)/arguments.o $(OBJDIR)/powerset.o \
	$(OBJDIR)/fol.o $(OBJDIR)/listobj.o \
	$(OBJDIR)/predicatetemplate.o $(OBJDIR)/functiontemplate.o \
	$(OBJDIR)/function.o $(OBJDIR)/predicate.o $(OBJDIR)/term.o \
	$(OBJDIR)/domain.o $(OBJDIR)/clause.o $(OBJDIR)/clausesampler.o \
	$(OBJDIR)/groundclause.o \
	$(OBJDIR)/feature.o \
	$(OBJDIR)/featureArray.o $(OBJDIR)/markovBlanket.o 

#add .cpp files (including executables') to CPPFILES.
CPPFILES=$(UTIL)/arguments.cpp $(UTIL)/powerset.cpp \
	$(PARSER)/fol.cpp $(PARSER)/listobj.cpp \
	$(LOGIC)/predicatetemplate.cpp $(LOGIC)/functiontemplate.cpp \
	$(LOGIC)/function.cpp $(LOGIC)/predicate.cpp $(LOGIC)/term.cpp \
	$(LOGIC)/domain.cpp  $(LOGIC)/clause.cpp $(LOGIC)/clausesampler.cpp\
	$(INFER)/groundclause.cpp \
	$(LEARNWTS)/learnwts.cpp $(LEARNWTS)/transferwts.cpp \
	$(LEARNWTS)/atominfer.cpp \
	$(LEARNWTS)/fb.cpp \
	$(LEARNWTS)/d_at.cpp \
	$(LEARNWTS)/printClauses.cpp \
	$(LEARNWTS)/fastinfer.cpp \
	$(INFER)/infer.cpp \
	$(LEARNSTRUCT)/learnstruct.cpp \
	$(LEARNSTRUCT)/busl-learnstruct.cpp \
	$(LEARNSTRUCT)/feature.cpp \
	$(LEARNSTRUCT)/featureArray.cpp $(LEARNSTRUCT)/markovBlanket.cpp 



##############################################################################
#EXECUTABLES: add executables rules here

all: learnwts infer learnstruct busl transferwts fastinfer

learnwts: $(OBJDIR)/learnwts.oo
	cp $(OBJDIR)/learnwts.oo $(BIN)/learnwts

transferwts: $(OBJDIR)/transferwts.oo
	cp $(OBJDIR)/transferwts.oo $(BIN)/transferwts

fastinfer: $(OBJDIR)/fastinfer.oo
	cp $(OBJDIR)/fastinfer.oo $(BIN)/fastinfer

fb: $(OBJDIR)/fb.oo
	cp $(OBJDIR)/fb.oo $(BIN)/fb

print: $(OBJDIR)/printClauses.oo
	cp $(OBJDIR)/printClauses.oo $(BIN)/print

infer: $(OBJDIR)/infer.oo
	cp $(OBJDIR)/infer.oo $(BIN)/infer

learnstruct: $(OBJDIR)/learnstruct.oo
	cp $(OBJDIR)/learnstruct.oo $(BIN)/learnstruct

busl: $(OBJDIR)/busl-learnstruct.oo
	cp $(OBJDIR)/busl-learnstruct.oo $(BIN)/busl


$(OBJDIR)/learnwts.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/learnwts.o -c $(LEARNWTS)/learnwts.cpp

$(OBJDIR)/transferwts.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/transferwts.o -c $(LEARNWTS)/transferwts.cpp

$(OBJDIR)/fastinfer.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/fastinfer.o -c $(LEARNWTS)/fastinfer.cpp

$(OBJDIR)/atominfer.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/atominfer.o -c $(LEARNWTS)/atominfer.cpp

$(OBJDIR)/d_at.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/d_at.o -c $(LEARNWTS)/d_at.cpp

$(OBJDIR)/fb.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/fb.o -c $(LEARNWTS)/fb.cpp

$(OBJDIR)/printClauses.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/printClauses.o -c $(LEARNWTS)/printClauses.cpp


$(OBJDIR)/infer.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/infer.o -c $(INFER)/infer.cpp

$(OBJDIR)/learnstruct.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/learnstruct.o -c $(LEARNSTRUCT)/learnstruct.cpp

$(OBJDIR)/busl-learnstruct.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/busl-learnstruct.o -c $(LEARNSTRUCT)/busl-learnstruct.cpp


%.oo: %.o $(OBJS) $(LIBS)
	echo $<
	$(GPP) $(LFLAGS) -o $@ $(OBJS) $< $(LIBLINK)


##############################################################################
#add add non-executables .o rules here

$(OBJDIR)/fol.o: $(PARSER)/fol.y $(PARSER)/follex.y
	$(FLEX) -o$(PARSER)/follex.cpp $(PARSER)/follex.y 
	$(BISON) $(PARSER)/fol.y;
	mv -f fol.tab.c  $(PARSER)/fol.cpp
	$(REPLACEFOLCPP) $(PARSER)/fol.cpp
	$(GPP) $(CFLAGS) -o $(OBJDIR)/fol.o -c $(PARSER)/fol.cpp


#.o rule
#$(OBJDIR)/file.o:
#	$(GPP) $(CFLAGS) -o $(OBJDIR)/file.o -c dir/file.cpp


$(OBJDIR)/arguments.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/arguments.o -c $(UTIL)/arguments.cpp

$(OBJDIR)/powerset.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/powerset.o -c $(UTIL)/powerset.cpp

$(OBJDIR)/listobj.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/listobj.o -c $(PARSER)/listobj.cpp 

$(OBJDIR)/predicatetemplate.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/predicatetemplate.o -c $(LOGIC)/predicatetemplate.cpp 

$(OBJDIR)/functiontemplate.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/functiontemplate.o -c $(LOGIC)/functiontemplate.cpp 

$(OBJDIR)/function.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/function.o -c $(LOGIC)/function.cpp 

$(OBJDIR)/predicate.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/predicate.o -c $(LOGIC)/predicate.cpp 

$(OBJDIR)/term.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/term.o -c $(LOGIC)/term.cpp

$(OBJDIR)/domain.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/domain.o -c $(LOGIC)/domain.cpp  

$(OBJDIR)/clause.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/clause.o -c $(LOGIC)/clause.cpp  

$(OBJDIR)/clausesampler.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/clausesampler.o -c $(LOGIC)/clausesampler.cpp  

$(OBJDIR)/groundclause.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/groundclause.o -c $(INFER)/groundclause.cpp

$(OBJDIR)/feature.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/feature.o -c $(LEARNSTRUCT)/feature.cpp 

$(OBJDIR)/featureArray.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/featureArray.o -c $(LEARNSTRUCT)/featureArray.cpp 

$(OBJDIR)/markovBlanket.o:
	$(GPP) $(CFLAGS) -o $(OBJDIR)/markovBlanket.o -c $(LEARNSTRUCT)/markovBlanket.cpp 



###############################################################################


clean:
	@rm -f $(OBJDIR)/*.o $(OBJDIR)/*.oo

depend:
	@echo 'updating the dependencies for:'
	@echo '    ' $(CPPFILES)
	@( \
	< $(MF) sed -n '1,/^###.*SUDDEN DEATH/p'; \
	    echo '#' ; \
	    echo '# dependencies generated on: ' `date` ; \
	    echo '#' ; \
	    for i in $(CPPFILES); do \
	        echo -n "$(OBJDIR)/" ; \
	        $(GPP) -MM $(CFLAGS) $$i ; \
	        echo; \
	    done \
	) > $(MF).new
	@mv $(MF) $(MF).last
	@mv $(MF).new $(MF)

############ Everything below here is subject to SUDDEN DEATH #####
#
# dependencies generated on:  Mon Feb 23 09:23:42 PST 2009
#
../bin/obj/arguments.o: ../src/util/arguments.cpp ../src/util/arguments.h

../bin/obj/powerset.o: ../src/util/powerset.cpp ../src/util/powerset.h \
  ../src/util/array.h ../src/util/random.h

../bin/obj/fol.o: ../src/parser/fol.cpp ../src/parser/fol.h ../src/logic/mln.h \
  ../src/logic/clause.h ../src/logic/domain.h ../src/util/equalstr.h \
  ../src/util/dualmap.h ../src/util/array.h ../src/util/random.h \
  ../src/util/equalstr.h ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicatetemplate.h ../src/util/array.h \
  ../src/logic/functiontemplate.h ../src/logic/predicate.h \
  ../src/logic/term.h ../src/util/hash.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/util/hashstring.h \
  ../src/util/hasharray.h ../src/util/hashlist.h \
  ../src/util/arraysaccessor.h ../src/util/powerset.h \
  ../src/util/multdarray.h ../src/util/hashint.h ../src/logic/database.h \
  ../src/logic/groundpreds.h ../src/infer/groundpredicate.h \
  ../src/logic/domain.h ../src/logic/predicate.h \
  ../src/infer/groundclause.h ../src/util/hasharray.h ../src/util/array.h \
  ../src/util/hash.h ../src/logic/clausesampler.h \
  ../src/logic/truefalsegroundingsstore.h ../src/util/random.h \
  ../src/logic/clausehelper.h ../src/logic/mlnhelper.h \
  ../src/logic/domain.h ../src/logic/database.h ../src/parser/follex.cpp \
  ../src/parser/folhelper.h ../src/parser/strfifolist.h \
  ../src/logic/predicate.h ../src/logic/function.h ../src/logic/clause.h \
  ../src/logic/clausefactory.h ../src/util/hashstring.h \
  ../src/parser/listobj.h ../src/util/array.h \
  ../src/util/arraysaccessor.h ../src/util/timer.h \
  ../src/parser/internals.h

../bin/obj/listobj.o: ../src/parser/listobj.cpp ../src/parser/listobj.h \
  ../src/util/array.h ../src/util/random.h ../src/util/hashstring.h \
  ../src/util/hasharray.h ../src/logic/domain.h ../src/util/equalstr.h \
  ../src/util/dualmap.h ../src/util/array.h ../src/util/equalstr.h \
  ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicatetemplate.h ../src/util/array.h \
  ../src/logic/functiontemplate.h ../src/logic/predicate.h \
  ../src/logic/term.h ../src/util/hash.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/util/arraysaccessor.h

../bin/obj/predicatetemplate.o: ../src/logic/predicatetemplate.cpp \
  ../src/logic/predicatetemplate.h ../src/util/array.h \
  ../src/util/random.h ../src/logic/domain.h ../src/util/equalstr.h \
  ../src/util/dualmap.h ../src/util/array.h ../src/util/equalstr.h \
  ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/functiontemplate.h ../src/logic/predicate.h \
  ../src/logic/term.h ../src/util/hash.h ../src/util/hasharray.h \
  ../src/logic/function.h

../bin/obj/functiontemplate.o: ../src/logic/functiontemplate.cpp \
  ../src/logic/functiontemplate.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/util/random.h ../src/logic/domain.h \
  ../src/util/equalstr.h ../src/util/dualmap.h ../src/util/array.h \
  ../src/util/equalstr.h ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicate.h ../src/logic/term.h ../src/util/hash.h \
  ../src/util/hasharray.h ../src/logic/function.h

../bin/obj/function.o: ../src/logic/function.cpp ../src/logic/function.h \
  ../src/logic/functiontemplate.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/util/random.h ../src/util/hash.h \
  ../src/util/array.h ../src/logic/term.h ../src/logic/domain.h \
  ../src/util/equalstr.h ../src/util/dualmap.h ../src/util/equalstr.h \
  ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicate.h ../src/util/hasharray.h

../bin/obj/predicate.o: ../src/logic/predicate.cpp ../src/logic/predicate.h \
  ../src/logic/predicatetemplate.h ../src/util/array.h \
  ../src/util/random.h ../src/logic/term.h ../src/util/hash.h \
  ../src/util/array.h ../src/util/hasharray.h ../src/logic/function.h \
  ../src/logic/functiontemplate.h ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/equalstr.h ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/util/hashstring.h ../src/util/hasharray.h ../src/util/hashlist.h \
  ../src/util/arraysaccessor.h ../src/util/powerset.h \
  ../src/util/multdarray.h ../src/util/hashint.h ../src/logic/database.h \
  ../src/logic/groundpreds.h ../src/infer/groundpredicate.h \
  ../src/logic/domain.h ../src/logic/predicate.h \
  ../src/infer/groundclause.h ../src/util/hasharray.h ../src/util/array.h \
  ../src/util/hash.h ../src/logic/clausesampler.h \
  ../src/logic/truefalsegroundingsstore.h ../src/util/random.h \
  ../src/logic/clausehelper.h

../bin/obj/term.o: ../src/logic/term.cpp ../src/logic/term.h ../src/util/array.h \
  ../src/util/random.h ../src/logic/function.h \
  ../src/logic/functiontemplate.h ../src/logic/predicatetemplate.h \
  ../src/util/hash.h ../src/util/array.h ../src/logic/domain.h \
  ../src/util/equalstr.h ../src/util/dualmap.h ../src/util/equalstr.h \
  ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicate.h ../src/util/hasharray.h

../bin/obj/domain.o: ../src/logic/domain.cpp ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/array.h ../src/util/random.h ../src/util/equalstr.h \
  ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicatetemplate.h ../src/util/array.h \
  ../src/logic/functiontemplate.h ../src/logic/predicate.h \
  ../src/logic/term.h ../src/util/hash.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/util/hashstring.h \
  ../src/util/hasharray.h ../src/util/hashlist.h \
  ../src/util/arraysaccessor.h ../src/util/powerset.h \
  ../src/util/multdarray.h ../src/util/hashint.h ../src/logic/database.h \
  ../src/logic/groundpreds.h ../src/infer/groundpredicate.h \
  ../src/logic/domain.h ../src/logic/predicate.h \
  ../src/infer/groundclause.h ../src/util/hasharray.h ../src/util/array.h \
  ../src/util/hash.h ../src/logic/clausesampler.h \
  ../src/logic/truefalsegroundingsstore.h ../src/util/random.h \
  ../src/logic/clausehelper.h ../src/logic/mln.h ../src/logic/mlnhelper.h

../bin/obj/clause.o: ../src/logic/clause.cpp ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/array.h ../src/util/random.h ../src/util/equalstr.h \
  ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicatetemplate.h ../src/util/array.h \
  ../src/logic/functiontemplate.h ../src/logic/predicate.h \
  ../src/logic/term.h ../src/util/hash.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/util/hashstring.h \
  ../src/util/hasharray.h ../src/util/hashlist.h \
  ../src/util/arraysaccessor.h ../src/util/powerset.h \
  ../src/util/multdarray.h ../src/util/hashint.h ../src/logic/database.h \
  ../src/logic/groundpreds.h ../src/infer/groundpredicate.h \
  ../src/logic/domain.h ../src/logic/predicate.h \
  ../src/infer/groundclause.h ../src/util/hasharray.h ../src/util/array.h \
  ../src/util/hash.h ../src/logic/clausesampler.h \
  ../src/logic/truefalsegroundingsstore.h ../src/util/random.h \
  ../src/logic/clausehelper.h ../src/infer/mrf.h ../src/util/timer.h \
  ../src/logic/mln.h ../src/logic/mlnhelper.h \
  ../src/infer/groundpredicate.h

../bin/obj/clausesampler.o: ../src/logic/clausesampler.cpp \
  ../src/logic/clausesampler.h ../src/logic/database.h ../src/util/hash.h \
  ../src/util/array.h ../src/util/random.h ../src/logic/groundpreds.h \
  ../src/logic/predicate.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/logic/term.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/logic/functiontemplate.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/equalstr.h ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/util/arraysaccessor.h ../src/infer/groundpredicate.h \
  ../src/logic/domain.h ../src/logic/predicate.h \
  ../src/infer/groundclause.h ../src/util/hasharray.h ../src/util/array.h \
  ../src/util/hash.h ../src/logic/truefalsegroundingsstore.h \
  ../src/util/random.h ../src/logic/clause.h ../src/util/hashstring.h \
  ../src/util/hasharray.h ../src/util/hashlist.h ../src/util/powerset.h \
  ../src/util/multdarray.h ../src/util/hashint.h \
  ../src/logic/clausehelper.h

../bin/obj/groundclause.o: ../src/infer/groundclause.cpp ../src/infer/groundclause.h \
  ../src/util/hasharray.h ../src/util/random.h ../src/util/array.h \
  ../src/util/hash.h ../src/util/array.h ../src/infer/groundpredicate.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/equalstr.h ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicatetemplate.h ../src/util/array.h \
  ../src/logic/functiontemplate.h ../src/logic/predicate.h \
  ../src/logic/term.h ../src/util/hash.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/logic/predicate.h ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/hashstring.h ../src/util/hasharray.h \
  ../src/util/hashlist.h ../src/util/arraysaccessor.h \
  ../src/util/powerset.h ../src/util/multdarray.h ../src/util/hashint.h \
  ../src/logic/database.h ../src/logic/groundpreds.h \
  ../src/infer/groundpredicate.h ../src/logic/clausesampler.h \
  ../src/logic/truefalsegroundingsstore.h ../src/util/random.h \
  ../src/logic/clausehelper.h ../src/logic/mln.h ../src/logic/clause.h \
  ../src/logic/mlnhelper.h

../bin/obj/learnwts.o: ../src/learnwts/learnwts.cpp ../src/util/arguments.h \
  ../src/infer/inferenceargs.h ../src/learnwts/lbfgsb.h \
  ../src/util/timer.h ../src/learnwts/pseudologlikelihood.h \
  ../src/util/array.h ../src/util/random.h ../src/util/random.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/array.h ../src/util/equalstr.h ../src/util/constdualmap.h \
  ../src/util/strint.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/logic/functiontemplate.h \
  ../src/logic/predicate.h ../src/logic/term.h ../src/util/hash.h \
  ../src/util/hasharray.h ../src/logic/function.h ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/hashstring.h ../src/util/hasharray.h \
  ../src/util/hashlist.h ../src/util/arraysaccessor.h \
  ../src/util/powerset.h ../src/util/multdarray.h ../src/util/hashint.h \
  ../src/logic/database.h ../src/logic/groundpreds.h \
  ../src/infer/groundpredicate.h ../src/logic/domain.h \
  ../src/logic/predicate.h ../src/infer/groundclause.h \
  ../src/util/hasharray.h ../src/util/array.h ../src/util/hash.h \
  ../src/logic/clausesampler.h ../src/logic/truefalsegroundingsstore.h \
  ../src/util/random.h ../src/logic/clausehelper.h ../src/logic/mln.h \
  ../src/logic/clause.h ../src/logic/mlnhelper.h \
  ../src/learnwts/indextranslator.h \
  ../src/learnwts/discriminativelearner.h ../src/infer/infer.h \
  ../src/util/util.h ../src/parser/fol.h ../src/logic/mln.h \
  ../src/logic/domain.h ../src/logic/database.h ../src/infer/mrf.h \
  ../src/util/timer.h ../src/logic/mln.h ../src/infer/groundpredicate.h \
  ../src/learnwts/learnwts.h ../src/util/util.h ../src/parser/fol.h \
  ../src/infer/inferenceargs.h ../src/infer/maxwalksat.h \
  ../src/infer/sat.h ../src/infer/inference.h \
  ../src/logic/variablestate.h ../src/infer/mrf.h ../src/util/timer.h \
  ../src/infer/maxwalksatparams.h ../src/infer/samplesatparams.h \
  ../src/infer/mcsat.h ../src/infer/mcmc.h ../src/infer/mcmcparams.h \
  ../src/infer/mcsatparams.h ../src/infer/unitpropagation.h \
  ../src/infer/gibbssampler.h ../src/infer/gibbsparams.h \
  ../src/infer/convergencetest.h ../src/util/meanvariance.h \
  ../src/infer/gelmanconvergencetest.h ../src/infer/simulatedtempering.h \
  ../src/infer/simulatedtemperingparams.h ../src/infer/maxwalksat.h \
  ../src/learnwts/learnwts.h ../src/infer/mcsat.h \
  ../src/infer/gibbssampler.h ../src/infer/simulatedtempering.h

../bin/obj/transferwts.o: ../src/learnwts/transferwts.cpp ../src/util/arguments.h \
  ../src/infer/inferenceargs.h ../src/learnwts/lbfgsb.h \
  ../src/util/timer.h ../src/learnwts/pseudologlikelihood.h \
  ../src/util/array.h ../src/util/random.h ../src/util/random.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/array.h ../src/util/equalstr.h ../src/util/constdualmap.h \
  ../src/util/strint.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/logic/functiontemplate.h \
  ../src/logic/predicate.h ../src/logic/term.h ../src/util/hash.h \
  ../src/util/hasharray.h ../src/logic/function.h ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/hashstring.h ../src/util/hasharray.h \
  ../src/util/hashlist.h ../src/util/arraysaccessor.h \
  ../src/util/powerset.h ../src/util/multdarray.h ../src/util/hashint.h \
  ../src/logic/database.h ../src/logic/groundpreds.h \
  ../src/infer/groundpredicate.h ../src/logic/domain.h \
  ../src/logic/predicate.h ../src/infer/groundclause.h \
  ../src/util/hasharray.h ../src/util/array.h ../src/util/hash.h \
  ../src/logic/clausesampler.h ../src/logic/truefalsegroundingsstore.h \
  ../src/util/random.h ../src/logic/clausehelper.h ../src/logic/mln.h \
  ../src/logic/clause.h ../src/logic/mlnhelper.h \
  ../src/learnwts/indextranslator.h \
  ../src/learnwts/discriminativelearner.h ../src/infer/infer.h \
  ../src/util/util.h ../src/parser/fol.h ../src/logic/mln.h \
  ../src/logic/domain.h ../src/logic/database.h ../src/infer/mrf.h \
  ../src/util/timer.h ../src/logic/mln.h ../src/infer/groundpredicate.h \
  ../src/learnwts/learnwts.h ../src/util/util.h ../src/parser/fol.h \
  ../src/infer/inferenceargs.h ../src/infer/maxwalksat.h \
  ../src/infer/sat.h ../src/infer/inference.h \
  ../src/logic/variablestate.h ../src/infer/mrf.h ../src/util/timer.h \
  ../src/infer/maxwalksatparams.h ../src/infer/samplesatparams.h \
  ../src/infer/mcsat.h ../src/infer/mcmc.h ../src/infer/mcmcparams.h \
  ../src/infer/mcsatparams.h ../src/infer/unitpropagation.h \
  ../src/infer/gibbssampler.h ../src/infer/gibbsparams.h \
  ../src/infer/convergencetest.h ../src/util/meanvariance.h \
  ../src/infer/gelmanconvergencetest.h ../src/infer/simulatedtempering.h \
  ../src/infer/simulatedtemperingparams.h ../src/infer/maxwalksat.h \
  ../src/learnwts/learnwts.h ../src/infer/mcsat.h \
  ../src/infer/gibbssampler.h ../src/infer/simulatedtempering.h

../bin/obj/atominfer.o: ../src/learnwts/atominfer.cpp ../src/util/arguments.h \
  ../src/infer/inferenceargs.h ../src/learnwts/lbfgsb.h \
  ../src/util/timer.h ../src/learnwts/pseudologlikelihood.h \
  ../src/util/array.h ../src/util/random.h ../src/util/random.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/array.h ../src/util/equalstr.h ../src/util/constdualmap.h \
  ../src/util/strint.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/logic/functiontemplate.h \
  ../src/logic/predicate.h ../src/logic/term.h ../src/util/hash.h \
  ../src/util/hasharray.h ../src/logic/function.h ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/hashstring.h ../src/util/hasharray.h \
  ../src/util/hashlist.h ../src/util/arraysaccessor.h \
  ../src/util/powerset.h ../src/util/multdarray.h ../src/util/hashint.h \
  ../src/logic/database.h ../src/logic/groundpreds.h \
  ../src/infer/groundpredicate.h ../src/logic/domain.h \
  ../src/logic/predicate.h ../src/infer/groundclause.h \
  ../src/util/hasharray.h ../src/util/array.h ../src/util/hash.h \
  ../src/logic/clausesampler.h ../src/logic/truefalsegroundingsstore.h \
  ../src/util/random.h ../src/logic/clausehelper.h ../src/logic/mln.h \
  ../src/logic/clause.h ../src/logic/mlnhelper.h \
  ../src/learnwts/indextranslator.h \
  ../src/learnwts/discriminativelearner.h ../src/infer/infer.h \
  ../src/util/util.h ../src/parser/fol.h ../src/logic/mln.h \
  ../src/logic/domain.h ../src/logic/database.h ../src/infer/mrf.h \
  ../src/util/timer.h ../src/logic/mln.h ../src/infer/groundpredicate.h \
  ../src/learnwts/learnwts.h ../src/util/util.h ../src/parser/fol.h \
  ../src/infer/inferenceargs.h ../src/infer/maxwalksat.h \
  ../src/infer/sat.h ../src/infer/inference.h \
  ../src/logic/variablestate.h ../src/infer/mrf.h ../src/util/timer.h \
  ../src/infer/maxwalksatparams.h ../src/infer/samplesatparams.h \
  ../src/infer/mcsat.h ../src/infer/mcmc.h ../src/infer/mcmcparams.h \
  ../src/infer/mcsatparams.h ../src/infer/unitpropagation.h \
  ../src/infer/gibbssampler.h ../src/infer/gibbsparams.h \
  ../src/infer/convergencetest.h ../src/util/meanvariance.h \
  ../src/infer/gelmanconvergencetest.h ../src/infer/simulatedtempering.h \
  ../src/infer/simulatedtemperingparams.h ../src/infer/maxwalksat.h \
  ../src/learnwts/learnwts.h ../src/infer/mcsat.h \
  ../src/infer/gibbssampler.h ../src/infer/simulatedtempering.h

../bin/obj/fb.o: ../src/learnwts/fb.cpp ../src/util/arguments.h \
  ../src/infer/inferenceargs.h ../src/learnwts/lbfgsb.h \
  ../src/util/timer.h ../src/learnwts/pseudologlikelihood.h \
  ../src/util/array.h ../src/util/random.h ../src/util/random.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/array.h ../src/util/equalstr.h ../src/util/constdualmap.h \
  ../src/util/strint.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/logic/functiontemplate.h \
  ../src/logic/predicate.h ../src/logic/term.h ../src/util/hash.h \
  ../src/util/hasharray.h ../src/logic/function.h ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/hashstring.h ../src/util/hasharray.h \
  ../src/util/hashlist.h ../src/util/arraysaccessor.h \
  ../src/util/powerset.h ../src/util/multdarray.h ../src/util/hashint.h \
  ../src/logic/database.h ../src/logic/groundpreds.h \
  ../src/infer/groundpredicate.h ../src/logic/domain.h \
  ../src/logic/predicate.h ../src/infer/groundclause.h \
  ../src/util/hasharray.h ../src/util/array.h ../src/util/hash.h \
  ../src/logic/clausesampler.h ../src/logic/truefalsegroundingsstore.h \
  ../src/util/random.h ../src/logic/clausehelper.h ../src/logic/mln.h \
  ../src/logic/clause.h ../src/logic/mlnhelper.h \
  ../src/learnwts/indextranslator.h \
  ../src/learnwts/discriminativelearner.h ../src/infer/infer.h \
  ../src/util/util.h ../src/parser/fol.h ../src/logic/mln.h \
  ../src/logic/domain.h ../src/logic/database.h ../src/infer/mrf.h \
  ../src/util/timer.h ../src/logic/mln.h ../src/infer/groundpredicate.h \
  ../src/learnwts/learnwts.h ../src/util/util.h ../src/parser/fol.h \
  ../src/infer/inferenceargs.h ../src/infer/maxwalksat.h \
  ../src/infer/sat.h ../src/infer/inference.h \
  ../src/logic/variablestate.h ../src/infer/mrf.h ../src/util/timer.h \
  ../src/infer/maxwalksatparams.h ../src/infer/samplesatparams.h \
  ../src/infer/mcsat.h ../src/infer/mcmc.h ../src/infer/mcmcparams.h \
  ../src/infer/mcsatparams.h ../src/infer/unitpropagation.h \
  ../src/infer/gibbssampler.h ../src/infer/gibbsparams.h \
  ../src/infer/convergencetest.h ../src/util/meanvariance.h \
  ../src/infer/gelmanconvergencetest.h ../src/infer/simulatedtempering.h \
  ../src/infer/simulatedtemperingparams.h ../src/infer/maxwalksat.h \
  ../src/learnwts/learnwts.h ../src/infer/mcsat.h \
  ../src/infer/gibbssampler.h ../src/infer/simulatedtempering.h

../bin/obj/d_at.o: ../src/learnwts/d_at.cpp ../src/util/arguments.h \
  ../src/infer/inferenceargs.h ../src/learnwts/lbfgsb.h \
  ../src/util/timer.h ../src/learnwts/pseudologlikelihood.h \
  ../src/util/array.h ../src/util/random.h ../src/util/random.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/array.h ../src/util/equalstr.h ../src/util/constdualmap.h \
  ../src/util/strint.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/logic/functiontemplate.h \
  ../src/logic/predicate.h ../src/logic/term.h ../src/util/hash.h \
  ../src/util/hasharray.h ../src/logic/function.h ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/hashstring.h ../src/util/hasharray.h \
  ../src/util/hashlist.h ../src/util/arraysaccessor.h \
  ../src/util/powerset.h ../src/util/multdarray.h ../src/util/hashint.h \
  ../src/logic/database.h ../src/logic/groundpreds.h \
  ../src/infer/groundpredicate.h ../src/logic/domain.h \
  ../src/logic/predicate.h ../src/infer/groundclause.h \
  ../src/util/hasharray.h ../src/util/array.h ../src/util/hash.h \
  ../src/logic/clausesampler.h ../src/logic/truefalsegroundingsstore.h \
  ../src/util/random.h ../src/logic/clausehelper.h ../src/logic/mln.h \
  ../src/logic/clause.h ../src/logic/mlnhelper.h \
  ../src/learnwts/indextranslator.h \
  ../src/learnwts/discriminativelearner.h ../src/infer/infer.h \
  ../src/util/util.h ../src/parser/fol.h ../src/logic/mln.h \
  ../src/logic/domain.h ../src/logic/database.h ../src/infer/mrf.h \
  ../src/util/timer.h ../src/logic/mln.h ../src/infer/groundpredicate.h \
  ../src/learnwts/learnwts.h ../src/util/util.h ../src/parser/fol.h \
  ../src/infer/inferenceargs.h ../src/infer/maxwalksat.h \
  ../src/infer/sat.h ../src/infer/inference.h \
  ../src/logic/variablestate.h ../src/infer/mrf.h ../src/util/timer.h \
  ../src/infer/maxwalksatparams.h ../src/infer/samplesatparams.h \
  ../src/infer/mcsat.h ../src/infer/mcmc.h ../src/infer/mcmcparams.h \
  ../src/infer/mcsatparams.h ../src/infer/unitpropagation.h \
  ../src/infer/gibbssampler.h ../src/infer/gibbsparams.h \
  ../src/infer/convergencetest.h ../src/util/meanvariance.h \
  ../src/infer/gelmanconvergencetest.h ../src/infer/simulatedtempering.h \
  ../src/infer/simulatedtemperingparams.h ../src/infer/maxwalksat.h \
  ../src/learnwts/learnwts.h ../src/infer/mcsat.h \
  ../src/infer/gibbssampler.h ../src/infer/simulatedtempering.h

../bin/obj/printClauses.o: ../src/learnwts/printClauses.cpp ../src/util/arguments.h \
  ../src/infer/inferenceargs.h ../src/learnwts/lbfgsb.h \
  ../src/util/timer.h ../src/learnwts/pseudologlikelihood.h \
  ../src/util/array.h ../src/util/random.h ../src/util/random.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/array.h ../src/util/equalstr.h ../src/util/constdualmap.h \
  ../src/util/strint.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/logic/functiontemplate.h \
  ../src/logic/predicate.h ../src/logic/term.h ../src/util/hash.h \
  ../src/util/hasharray.h ../src/logic/function.h ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/hashstring.h ../src/util/hasharray.h \
  ../src/util/hashlist.h ../src/util/arraysaccessor.h \
  ../src/util/powerset.h ../src/util/multdarray.h ../src/util/hashint.h \
  ../src/logic/database.h ../src/logic/groundpreds.h \
  ../src/infer/groundpredicate.h ../src/logic/domain.h \
  ../src/logic/predicate.h ../src/infer/groundclause.h \
  ../src/util/hasharray.h ../src/util/array.h ../src/util/hash.h \
  ../src/logic/clausesampler.h ../src/logic/truefalsegroundingsstore.h \
  ../src/util/random.h ../src/logic/clausehelper.h ../src/logic/mln.h \
  ../src/logic/clause.h ../src/logic/mlnhelper.h \
  ../src/learnwts/indextranslator.h \
  ../src/learnwts/discriminativelearner.h ../src/infer/infer.h \
  ../src/util/util.h ../src/parser/fol.h ../src/logic/mln.h \
  ../src/logic/domain.h ../src/logic/database.h ../src/infer/mrf.h \
  ../src/util/timer.h ../src/logic/mln.h ../src/infer/groundpredicate.h \
  ../src/learnwts/learnwts.h ../src/util/util.h ../src/parser/fol.h \
  ../src/infer/inferenceargs.h ../src/infer/maxwalksat.h \
  ../src/infer/sat.h ../src/infer/inference.h \
  ../src/logic/variablestate.h ../src/infer/mrf.h ../src/util/timer.h \
  ../src/infer/maxwalksatparams.h ../src/infer/samplesatparams.h \
  ../src/infer/mcsat.h ../src/infer/mcmc.h ../src/infer/mcmcparams.h \
  ../src/infer/mcsatparams.h ../src/infer/unitpropagation.h \
  ../src/infer/gibbssampler.h ../src/infer/gibbsparams.h \
  ../src/infer/convergencetest.h ../src/util/meanvariance.h \
  ../src/infer/gelmanconvergencetest.h ../src/infer/simulatedtempering.h \
  ../src/infer/simulatedtemperingparams.h ../src/infer/maxwalksat.h \
  ../src/learnwts/learnwts.h ../src/infer/mcsat.h \
  ../src/infer/gibbssampler.h ../src/infer/simulatedtempering.h

../bin/obj/fastinfer.o: ../src/learnwts/fastinfer.cpp ../src/util/arguments.h \
  ../src/infer/inferenceargs.h ../src/learnwts/lbfgsb.h \
  ../src/util/timer.h ../src/learnwts/pseudologlikelihood.h \
  ../src/util/array.h ../src/util/random.h ../src/util/random.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/array.h ../src/util/equalstr.h ../src/util/constdualmap.h \
  ../src/util/strint.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/logic/functiontemplate.h \
  ../src/logic/predicate.h ../src/logic/term.h ../src/util/hash.h \
  ../src/util/hasharray.h ../src/logic/function.h ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/hashstring.h ../src/util/hasharray.h \
  ../src/util/hashlist.h ../src/util/arraysaccessor.h \
  ../src/util/powerset.h ../src/util/multdarray.h ../src/util/hashint.h \
  ../src/logic/database.h ../src/logic/groundpreds.h \
  ../src/infer/groundpredicate.h ../src/logic/domain.h \
  ../src/logic/predicate.h ../src/infer/groundclause.h \
  ../src/util/hasharray.h ../src/util/array.h ../src/util/hash.h \
  ../src/logic/clausesampler.h ../src/logic/truefalsegroundingsstore.h \
  ../src/util/random.h ../src/logic/clausehelper.h ../src/logic/mln.h \
  ../src/logic/clause.h ../src/logic/mlnhelper.h \
  ../src/learnwts/indextranslator.h \
  ../src/learnwts/discriminativelearner.h ../src/infer/infer.h \
  ../src/util/util.h ../src/parser/fol.h ../src/logic/mln.h \
  ../src/logic/domain.h ../src/logic/database.h ../src/infer/mrf.h \
  ../src/util/timer.h ../src/logic/mln.h ../src/infer/groundpredicate.h \
  ../src/learnwts/learnwts.h ../src/util/util.h ../src/parser/fol.h \
  ../src/infer/inferenceargs.h ../src/infer/maxwalksat.h \
  ../src/infer/sat.h ../src/infer/inference.h \
  ../src/logic/variablestate.h ../src/infer/mrf.h ../src/util/timer.h \
  ../src/infer/maxwalksatparams.h ../src/infer/samplesatparams.h \
  ../src/infer/mcsat.h ../src/infer/mcmc.h ../src/infer/mcmcparams.h \
  ../src/infer/mcsatparams.h ../src/infer/unitpropagation.h \
  ../src/infer/gibbssampler.h ../src/infer/gibbsparams.h \
  ../src/infer/convergencetest.h ../src/util/meanvariance.h \
  ../src/infer/gelmanconvergencetest.h ../src/infer/simulatedtempering.h \
  ../src/infer/simulatedtemperingparams.h ../src/infer/maxwalksat.h \
  ../src/learnwts/learnwts.h ../src/infer/mcsat.h \
  ../src/infer/gibbssampler.h ../src/infer/simulatedtempering.h

../bin/obj/infer.o: ../src/infer/infer.cpp ../src/parser/fol.h ../src/logic/mln.h \
  ../src/logic/clause.h ../src/logic/domain.h ../src/util/equalstr.h \
  ../src/util/dualmap.h ../src/util/array.h ../src/util/random.h \
  ../src/util/equalstr.h ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicatetemplate.h ../src/util/array.h \
  ../src/logic/functiontemplate.h ../src/logic/predicate.h \
  ../src/logic/term.h ../src/util/hash.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/util/hashstring.h \
  ../src/util/hasharray.h ../src/util/hashlist.h \
  ../src/util/arraysaccessor.h ../src/util/powerset.h \
  ../src/util/multdarray.h ../src/util/hashint.h ../src/logic/database.h \
  ../src/logic/groundpreds.h ../src/infer/groundpredicate.h \
  ../src/logic/domain.h ../src/logic/predicate.h \
  ../src/infer/groundclause.h ../src/util/hasharray.h ../src/util/array.h \
  ../src/util/hash.h ../src/logic/clausesampler.h \
  ../src/logic/truefalsegroundingsstore.h ../src/util/random.h \
  ../src/logic/clausehelper.h ../src/logic/mlnhelper.h \
  ../src/logic/domain.h ../src/logic/database.h ../src/util/arguments.h \
  ../src/util/util.h ../src/infer/infer.h ../src/infer/mrf.h \
  ../src/util/timer.h ../src/logic/mln.h ../src/infer/groundpredicate.h \
  ../src/learnwts/learnwts.h ../src/util/util.h ../src/util/timer.h \
  ../src/parser/fol.h ../src/logic/mln.h \
  ../src/learnwts/indextranslator.h ../src/infer/inferenceargs.h \
  ../src/infer/maxwalksat.h ../src/infer/sat.h ../src/infer/inference.h \
  ../src/logic/variablestate.h ../src/infer/mrf.h ../src/util/timer.h \
  ../src/infer/maxwalksatparams.h ../src/infer/samplesatparams.h \
  ../src/infer/mcsat.h ../src/infer/mcmc.h ../src/infer/mcmcparams.h \
  ../src/infer/mcsatparams.h ../src/infer/unitpropagation.h \
  ../src/infer/gibbssampler.h ../src/infer/gibbsparams.h \
  ../src/infer/convergencetest.h ../src/util/meanvariance.h \
  ../src/infer/gelmanconvergencetest.h ../src/infer/simulatedtempering.h \
  ../src/infer/simulatedtemperingparams.h

../bin/obj/learnstruct.o: ../src/learnstruct/learnstruct.cpp ../src/util/arguments.h \
  ../src/parser/fol.h ../src/logic/mln.h ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/array.h ../src/util/random.h ../src/util/equalstr.h \
  ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicatetemplate.h ../src/util/array.h \
  ../src/logic/functiontemplate.h ../src/logic/predicate.h \
  ../src/logic/term.h ../src/util/hash.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/util/hashstring.h \
  ../src/util/hasharray.h ../src/util/hashlist.h \
  ../src/util/arraysaccessor.h ../src/util/powerset.h \
  ../src/util/multdarray.h ../src/util/hashint.h ../src/logic/database.h \
  ../src/logic/groundpreds.h ../src/infer/groundpredicate.h \
  ../src/logic/domain.h ../src/logic/predicate.h \
  ../src/infer/groundclause.h ../src/util/hasharray.h ../src/util/array.h \
  ../src/util/hash.h ../src/logic/clausesampler.h \
  ../src/logic/truefalsegroundingsstore.h ../src/util/random.h \
  ../src/logic/clausehelper.h ../src/logic/mlnhelper.h \
  ../src/logic/domain.h ../src/logic/database.h \
  ../src/learnwts/learnwts.h ../src/util/util.h ../src/util/timer.h \
  ../src/parser/fol.h ../src/logic/mln.h \
  ../src/learnwts/indextranslator.h ../src/learnstruct/structlearn.h \
  ../src/logic/clausefactory.h ../src/logic/mln.h \
  ../src/learnwts/lbfgsb.h ../src/learnwts/pseudologlikelihood.h \
  ../src/util/array.h ../src/util/random.h ../src/logic/domain.h \
  ../src/logic/clause.h ../src/learnwts/discriminativelearner.h \
  ../src/infer/infer.h ../src/util/util.h ../src/parser/fol.h \
  ../src/infer/mrf.h ../src/util/timer.h ../src/logic/mln.h \
  ../src/infer/groundpredicate.h ../src/learnwts/learnwts.h \
  ../src/infer/inferenceargs.h ../src/infer/maxwalksat.h \
  ../src/infer/sat.h ../src/infer/inference.h \
  ../src/logic/variablestate.h ../src/infer/mrf.h ../src/util/timer.h \
  ../src/infer/maxwalksatparams.h ../src/infer/samplesatparams.h \
  ../src/infer/mcsat.h ../src/infer/mcmc.h ../src/infer/mcmcparams.h \
  ../src/infer/mcsatparams.h ../src/infer/unitpropagation.h \
  ../src/infer/gibbssampler.h ../src/infer/gibbsparams.h \
  ../src/infer/convergencetest.h ../src/util/meanvariance.h \
  ../src/infer/gelmanconvergencetest.h ../src/infer/simulatedtempering.h \
  ../src/infer/simulatedtemperingparams.h ../src/infer/maxwalksat.h \
  ../src/learnstruct/rise.h

../bin/obj/busl-learnstruct.o: ../src/learnstruct/busl-learnstruct.cpp \
  ../src/util/arguments.h ../src/parser/fol.h ../src/logic/mln.h \
  ../src/logic/clause.h ../src/logic/domain.h ../src/util/equalstr.h \
  ../src/util/dualmap.h ../src/util/array.h ../src/util/random.h \
  ../src/util/equalstr.h ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicatetemplate.h ../src/util/array.h \
  ../src/logic/functiontemplate.h ../src/logic/predicate.h \
  ../src/logic/term.h ../src/util/hash.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/util/hashstring.h \
  ../src/util/hasharray.h ../src/util/hashlist.h \
  ../src/util/arraysaccessor.h ../src/util/powerset.h \
  ../src/util/multdarray.h ../src/util/hashint.h ../src/logic/database.h \
  ../src/logic/groundpreds.h ../src/infer/groundpredicate.h \
  ../src/logic/domain.h ../src/logic/predicate.h \
  ../src/infer/groundclause.h ../src/util/hasharray.h ../src/util/array.h \
  ../src/util/hash.h ../src/logic/clausesampler.h \
  ../src/logic/truefalsegroundingsstore.h ../src/util/random.h \
  ../src/logic/clausehelper.h ../src/logic/mlnhelper.h \
  ../src/logic/domain.h ../src/logic/database.h \
  ../src/learnwts/learnwts.h ../src/util/util.h ../src/util/timer.h \
  ../src/parser/fol.h ../src/logic/mln.h \
  ../src/learnwts/indextranslator.h ../src/learnstruct/ut-structlearn.h \
  ../src/logic/clausefactory.h ../src/logic/mln.h \
  ../src/learnwts/pseudologlikelihood.h ../src/util/array.h \
  ../src/util/random.h ../src/logic/domain.h ../src/logic/clause.h \
  ../src/learnwts/lbfgsb.h ../src/learnwts/pseudologlikelihood.h \
  ../src/learnstruct/structlearn.h \
  ../src/learnwts/discriminativelearner.h ../src/infer/infer.h \
  ../src/util/util.h ../src/parser/fol.h ../src/infer/mrf.h \
  ../src/util/timer.h ../src/logic/mln.h ../src/infer/groundpredicate.h \
  ../src/learnwts/learnwts.h ../src/infer/inferenceargs.h \
  ../src/infer/maxwalksat.h ../src/infer/sat.h ../src/infer/inference.h \
  ../src/logic/variablestate.h ../src/infer/mrf.h ../src/util/timer.h \
  ../src/infer/maxwalksatparams.h ../src/infer/samplesatparams.h \
  ../src/infer/mcsat.h ../src/infer/mcmc.h ../src/infer/mcmcparams.h \
  ../src/infer/mcsatparams.h ../src/infer/unitpropagation.h \
  ../src/infer/gibbssampler.h ../src/infer/gibbsparams.h \
  ../src/infer/convergencetest.h ../src/util/meanvariance.h \
  ../src/infer/gelmanconvergencetest.h ../src/infer/simulatedtempering.h \
  ../src/infer/simulatedtemperingparams.h ../src/infer/maxwalksat.h \
  ../src/learnstruct/feature.h ../src/logic/predicate.h \
  ../src/util/array.h ../src/learnstruct/featureArray.h \
  ../src/learnstruct/markovBlanket.h ../src/logic/clause.h

../bin/obj/feature.o: ../src/learnstruct/feature.cpp ../src/learnstruct/feature.h \
  ../src/logic/predicate.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/util/random.h ../src/logic/term.h \
  ../src/util/hash.h ../src/util/array.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/logic/functiontemplate.h \
  ../src/util/array.h

../bin/obj/featureArray.o: ../src/learnstruct/featureArray.cpp \
  ../src/learnstruct/featureArray.h ../src/learnstruct/feature.h \
  ../src/logic/predicate.h ../src/logic/predicatetemplate.h \
  ../src/util/array.h ../src/util/random.h ../src/logic/term.h \
  ../src/util/hash.h ../src/util/array.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/logic/functiontemplate.h \
  ../src/util/array.h

../bin/obj/markovBlanket.o: ../src/learnstruct/markovBlanket.cpp \
  ../src/learnstruct/markovBlanket.h ../src/logic/clause.h \
  ../src/logic/domain.h ../src/util/equalstr.h ../src/util/dualmap.h \
  ../src/util/array.h ../src/util/random.h ../src/util/equalstr.h \
  ../src/util/constdualmap.h ../src/util/strint.h \
  ../src/logic/predicatetemplate.h ../src/util/array.h \
  ../src/logic/functiontemplate.h ../src/logic/predicate.h \
  ../src/logic/term.h ../src/util/hash.h ../src/util/hasharray.h \
  ../src/logic/function.h ../src/util/hashstring.h \
  ../src/util/hasharray.h ../src/util/hashlist.h \
  ../src/util/arraysaccessor.h ../src/util/powerset.h \
  ../src/util/multdarray.h ../src/util/hashint.h ../src/logic/database.h \
  ../src/logic/groundpreds.h ../src/infer/groundpredicate.h \
  ../src/logic/domain.h ../src/logic/predicate.h \
  ../src/infer/groundclause.h ../src/util/hasharray.h ../src/util/array.h \
  ../src/util/hash.h ../src/logic/clausesampler.h \
  ../src/logic/truefalsegroundingsstore.h ../src/util/random.h \
  ../src/logic/clausehelper.h ../src/learnstruct/feature.h \
  ../src/logic/predicate.h ../src/util/array.h \
  ../src/learnstruct/featureArray.h

