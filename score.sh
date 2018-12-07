#!/bin/bash
databases='domains/imdb-fold1.db,domains/imdb-fold2.db,domains/imdb-fold3.db,domains/imdb-fold4.db,domains/imdb-fold5.db';
for inputfile in `find $1 -type f -name "*.mln" ! -name "*output*" ! -empty`; do
 logfile=`echo ${inputfile} | sed 's/.mln/-output.log/g'`;
 outputfile=`echo ${inputfile} | sed 's/.mln/-output.mln/g'`;
 echo "learnstruct -i ${inputfile} -o ${outputfile} -t ${databases} -search 999 -penalty 0.001 -minWt 0.05 -fractAtoms 1 -maxAtomSamples 10000 -maxClauseSamples 10000 -tightConvThresh 1e-3 -looseConvThresh 1e-2 -tightMaxIter 10 -plusType 2 -multipleDatabases -startFromEmptyMLN > ${logfile}";
 ./alchemy/bin/learnstruct -i ${inputfile} -o ${outputfile} -t ${databases} -search 999 -penalty 0.001 -minWt 0.05 -fractAtoms 1 -maxAtomSamples 10000 -maxClauseSamples 10000 -tightConvThresh 1e-3 -looseConvThresh 1e-2 -tightMaxIter 10 -plusType 2 -multipleDatabases -startFromEmptyMLN > ${logfile}
done
