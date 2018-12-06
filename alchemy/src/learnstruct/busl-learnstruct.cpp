#include <fstream>
#include <iostream>
#include "arguments.h"
#include "fol.h"
#include "learnwts.h"
//#include "infer.h"
#include "ut-structlearn.h"
#include "structlearn.h"


char* inMLNFiles = NULL;
char* outMLNFile = NULL;
char* dbFiles = NULL;
char* nonEvidPredsStr = NULL; 
bool multipleDatabases = false;

double minWt = 0.01;
double penalty = 0.01;
int maxVars = 6;
int maxNumPredicates = 6;
int cacheSize = 500;

bool noSampleClauses = false;
double ddelta = 0.05;
double epsilon = 0.2;
int minClauseSamples = -1;
int maxClauseSamples = -1;

bool noSampleGndPreds = false;
double fraction = 0.8;
int minGndPredSamples = -1;
int maxGndPredSamples = -1;

bool noPrior = false;
double priorMean = 0;
double priorStdDev = 100;

int lbMaxIter = 10000;
double lbConvThresh = 1e-5;
int looseMaxIter = 10;
double looseConvThresh = 1e-3;

bool noWtPredsEqually = false;
bool startFromEmptyMLN = false;
bool tryAllFlips = false;

bool structGradDescent = false;
bool withEM = false;

int countIsAtLeast = 0;
double indepThreshold = 0.15;
int maxNumFreeVars = 1;
double minimumTrueProportion = 0.01;

ARGS ARGS::Args[] = 
{
  ARGS("i", ARGS::Req, inMLNFiles, 
       "Comma-separated input .mln files. (With the -multipleDatabases "
       "option, the second file to the last one are used to contain constants "
       "from different domains, and they correspond to the .db files specified "
       "with the -t option.)"),

  ARGS("o", ARGS::Req, outMLNFile, 
       "Output .mln file containing learned formulas and weights."),

  ARGS("t", ARGS::Req, dbFiles, 
       "Comma-separated .db files containing the training database "
       "(of true/false ground atoms), including function definitions, "
       "e.g. ai.db,graphics.db,languages.db."),
  
  ARGS("ne", ARGS::Opt, nonEvidPredsStr, 
       "[all predicates] Non-evidence predicates "
       "(comma-separated with no space), e.g., cancer,smokes,friends."),
    
  ARGS("multipleDatabases", ARGS::Tog, multipleDatabases,
       "If specified, each .db file belongs to a separate domain; "
       "otherwise all .db files belong to the same domain."),

  ARGS("minWt", ARGS::Opt, minWt, 
       "[0.01] Candidate clauses are discarded if "
       "their absolute weights fall below this."),

  ARGS("penalty", ARGS::Opt, penalty, 
       "[0.01] Each difference between the current and previous version of a "
       "candidate clause penalizes the (weighted) pseudo-log-likelihood "
       "by this amount."),

  ARGS("maxVars", ARGS::Opt, maxVars, 
       "[6] Maximum number of variables in learned clauses."),
  
  ARGS("maxNumPredicates", ARGS::Opt, maxNumPredicates, 
       "[6] Maximum number of predicates in learned clauses."),
  
  ARGS("cacheSize", ARGS::Opt, cacheSize, 
       "[500] Size in megabytes of the cache that is used to store the clauses "
       "(and their counts) that are created during structure learning."),

  ARGS("noSampleClauses", ARGS::Tog, noSampleClauses, 
       "If specified, compute a clause's number of true groundings exactly, "
       "and do not estimate it by sampling its groundings. If not specified, "
       "estimate the number by sampling."),

  ARGS("delta", ARGS::Opt, ddelta, 
       "[0.05] (Used only if sampling clauses.) "
       "The probability that an estimate a clause's number of true groundings "
       "is off by more than epsilon error is less than this value. "
       "Used to determine the number of samples of the clause's groundings "
       "to draw."),

  ARGS("epsilon", ARGS::Opt, epsilon,
       "[0.2] (Used only if sampling clauses.) "
       "Fractional error from a clause's actual number of true groundings. "
       "Used to determine the number of samples of the clause's groundings "
       "to draw."),

  ARGS("minClauseSamples", ARGS::Opt, minClauseSamples,
       "[-1] (Used only if sampling clauses.) "
       "Minimum number of samples of a clause's groundings to draw. "
       "(-1: no minimum)"),

  ARGS("maxClauseSamples", ARGS::Opt, maxClauseSamples,
       "[-1] (Used only if sampling clauses.) "
       "Maximum number of samples of a clause's groundings to draw. "
       "(-1: no maximum)"),

  ARGS("noSampleAtoms", ARGS::Tog, noSampleGndPreds, 
       "If specified, do not estimate the (weighted) pseudo-log-likelihood by "
       "sampling ground atoms; otherwise, estimate the value by sampling."),

  ARGS("fractAtoms", ARGS::Opt, fraction,
       "[0.8] (Used only if sampling ground atoms.) "
       "Fraction of each predicate's ground atoms to draw."),

  ARGS("minAtomSamples", ARGS::Opt, minGndPredSamples,
       "[-1] (Used only if sampling ground atoms.) "
       "Minimum number of each predicate's ground atoms to draw. "
       "(-1: no minimum)"),

  ARGS("maxAtomSamples", ARGS::Opt, maxGndPredSamples,
       "[-1] (Used only if sampling ground atoms.) "
       "Maximum number of each predicate's ground atoms to draw. "
       "(-1: no maximum)"),

  ARGS("noPrior", ARGS::Tog, noPrior, "No Gaussian priors on formula weights."),

  ARGS("priorMean", ARGS::Opt, priorMean, 
       "[0] Means of Gaussian priors on formula weights. By default, "
       "for each formula, it is the weight given in the .mln input file, " 
       "or fraction thereof if the formula turns into multiple clauses. "
       "This mean applies if no weight is given in the .mln file."),

  ARGS("priorStdDev", ARGS::Opt, priorStdDev, 
       "[100] Standard deviations of Gaussian priors on clause weights."),

  ARGS("tightMaxIter", ARGS::Opt, lbMaxIter, 
       "[10000] Max number of iterations to run L-BFGS-B, "
       "the algorithm used to optimize the (weighted) pseudo-log-likelihood."),

  ARGS("tightConvThresh", ARGS::Opt, lbConvThresh, 
       "[1e-5] Fractional change in (weighted) pseudo-log-likelihood at which "
       "L-BFGS-B terminates."),

  ARGS("looseMaxIter", ARGS::Opt, looseMaxIter, 
       "[10] Max number of iterations to run L-BFGS-B "
       "when evaluating candidate clauses."),

  ARGS("looseConvThresh", ARGS::Opt, looseConvThresh, 
       "[1e-3] Fractional change in (weighted) pseudo-log-likelihood at which "
       "L-BFGS-B terminates when evaluating candidate clauses."),

  ARGS("noWtPredsEqually", ARGS::Tog, noWtPredsEqually,
       "If specified, each predicate is not weighted equally. This means that "
       "high-arity predicates contribute more to the pseudo-log-likelihood "
       "than low-arity ones. If not specified, each predicate is given equal "
       "weight in the weighted pseudo-log-likelihood."),

  ARGS("startFromEmptyMLN", ARGS::Tog, startFromEmptyMLN,
       "If specified, start structure learning from an empty MLN. "
       "If the input .mln contains formulas, they will be added to the "
       "candidate clauses created in the first step of beam search. "
       "If not specified, begin structure learning from the input .mln file."),
       
  ARGS("tryAllFlips", ARGS::Tog, tryAllFlips,
       "If specified, the structure learning algorithm tries to flip "
       "the predicate signs of the formulas in the input .mln file "
       "in all possible ways"),

   ARGS("countIsAtLeast", ARGS::Opt, countIsAtLeast,
       "[0] (BUSL Parameter) If specified, uses this number as a criterium for the reliability of Chi2 tests"),
       
   ARGS("indepThreshold", ARGS::Opt, indepThreshold,
        "[0.15] (BUSL parameter) This parameter is used in the chi2 test during markov network template construction."
        "It's the alpha value below which we overthrow the null hypo that two variables are independent."), 
  
   ARGS("maxNumFreeVars", ARGS::Opt, maxNumFreeVars,
        "[1] (BUSL parameter) The maximum number of free variables allowed in a tnode of length > 1."),

   ARGS("minTrueProportion", ARGS::Opt, minimumTrueProportion,
        "[0.01] (BUSL parameter) The minimum proportion of true groundings required to consider a candidate clause."), 
  
  ARGS()
};


bool checkParams()
{
  bool ok = true;

  if (minWt<0) { cout << "ERROR: minWt must be non-negative" << endl;ok =false;}

  if (penalty<0) { cout <<"ERROR: penalty must be non-negative"<<endl;ok=false;}

  if (maxVars<=0) { cout << "ERROR: maxVar must be positive" << endl;ok =false;}

  if (maxNumPredicates <= 0)
  { 
    cout << "ERROR: maxNumPredicates must be positive" << endl; ok = false;
  }
  
  if (cacheSize < 0) 
  { cout << "ERROR: cacheSize must be non-negative" << endl; ok = false; }

  if (ddelta <= 0 || ddelta > 1) 
  { cout << "ERROR: gamma must be between 0 and 1" << endl; ok = false; }

  if (epsilon <= 0 || epsilon >= 1) 
  { cout << "ERROR: epsilon must be between 0 and 1" << endl; ok = false; }

  if (fraction < 0 || fraction > 1) 
  { cout << "ERROR: fraction must be between 0 and 1" << endl; ok = false;}

  if (priorMean < 0) 
  { cout << "ERROR: priorMean must be non-negative" << endl; ok = false; }

  if (priorStdDev <= 0) 
  { cout << "ERROR: priorStdDev must be positive" << endl; ok = false; }

  if (lbMaxIter <= 0) 
  { cout << "ERROR: tightMaxIter must be positive" << endl;  ok = false; }

  if (lbConvThresh <= 0 || lbConvThresh >= 1) 
  { cout << "ERROR: tightConvThresh must be between 0 and 1" << endl; ok=false;}

  if (looseMaxIter <= 0) 
  { cout << "ERROR: looseMaxIter must be positive" << endl; ok = false; }

  if (looseConvThresh <= 0 || looseConvThresh >= 1) 
  { cout << "ERROR: looseConvThresh must be between 0 and 1" << endl; ok=false;}

  if (!structGradDescent && withEM)
  { cout << "ERROR: EM can only be used with structural gradient descent" << endl; ok=false; }
  
  if (structGradDescent && nonEvidPredsStr == NULL)
  {
    cout << "ERROR: you must specify non-evidence predicates for "
         << "structural gradient descent" << endl;
    ok = false;
  }
  
  if (indepThreshold <= 0 || indepThreshold >= 1 ){
	  cout << "Error: the indepThreshold must be in the interval (0, 1)" << endl;
	  ok = false;
  }
  
  if (maxNumFreeVars < 0){
	  cout << "Error: the maxNumFreeVars must be >= 0\n";
	  ok = false;
  }
  
  if (countIsAtLeast < 0){
	  cout << "Error: countIsAtLeast must be >= 0\n";
	  ok = false;
  }

  if (minimumTrueProportion <= 0 || minimumTrueProportion >= 1 ){
	  cout << "Error: the minimumTrueProportion must be in the interval (0, 1)" << endl;
	  ok = false;
  }

  return ok;
}


//void extractFileNames(...) defined in learnwts.h
//void createDomainsAndMLNs(...) defined in learnwts.h 
//void deleteDomains(...) defined in learnwts.h 
//bool extractPredNames(...) defined in infer.h


int main(int argc, char* argv[])
{
  ARGS::parse(argc,argv,&cout);
  Timer timer;
  double begSec, startSec = timer.time();

    //Compute the size in MB of the components of Term, Predicate, and Clause
    //that do not change. The sizes will be used when computing the running 
    //size of the cache of clauses.
  Term::computeFixedSizeB();
  Predicate::computeFixedSizeB();
  Clause::computeFixedSizeB();
  AuxClauseData::computeFixedSizeB();
  

  ///////////////////////// check and extract the parameters //////////////////

  if (!checkParams()) return -1;
  
  //the second .mln file to the last one in inMLNFiles _may_ be used 
  //to hold constants, so they are held in constFilesArr. They will be
  //included into the first .mln file.

    //extract .mln and .db, file names
  Array<string> constFilesArr, dbFilesArr;
  extractFileNames(inMLNFiles, constFilesArr);
  assert(constFilesArr.size() >= 1);
  string inMLNFile = constFilesArr[0];
  constFilesArr.removeItem(0);
  extractFileNames(dbFiles, dbFilesArr);

  if (dbFilesArr.size() <= 0)
  { cout << "ERROR: must specify training data with -t flag."<<endl; return -1;}

    // if multiple databases, check the number of .db/.func files
  if (multipleDatabases) 
  {
      //if # .mln files containing constants and .db files are diff
    if ( (constFilesArr.size() > 0 && constFilesArr.size() != dbFilesArr.size()))
    {
      cout << "ERROR: when there are multiple databases, if .mln files "
           << "containing constants are specified, there must " 
           << "be the same number of them as .db files" << endl;
      return -1;
    }
  }

  StringHashArray tmpNEPredNames;
  Array<string> nonEvidPredNames;
  if (nonEvidPredsStr)
  {
    if(!extractPredNames(nonEvidPredsStr, NULL, tmpNEPredNames))
    {
      cout << "ERROR: failed to extract non-evidence predicate names." << endl;
      return -1;
    }
    for (int i = 0; i < tmpNEPredNames.size(); i++) 
      nonEvidPredNames.append(tmpNEPredNames[i]);
  }

  ////////////////////////// create domains and mlns ///////////////////////////

  Array<Domain*> domains;
  Array<MLN*> mlns;
  StringHashArray* queryPredNames = NULL;
  if (structGradDescent)
  {
    queryPredNames = new StringHashArray();
    for (int i = 0; i < nonEvidPredNames.size(); i++) 
      queryPredNames->append(nonEvidPredNames[i]);
  }

  bool addUnitClauses = false;
 
  bool allPredsExceptQueriesAreCW = true;
  bool mwsLazy = true;
  if (structGradDescent && withEM) allPredsExceptQueriesAreCW = false;
  begSec = timer.time();
  cout << "Parsing MLN and creating domains..." << endl;
  createDomainsAndMLNs(domains, mlns, multipleDatabases, inMLNFile, 
                       constFilesArr, dbFilesArr, queryPredNames,
                       addUnitClauses, priorMean, mwsLazy,
                       allPredsExceptQueriesAreCW, NULL, NULL);
  cout << "Parsing MLN and creating domains took ";
  Timer::printTime(cout, timer.time()-begSec); cout << endl << endl;

  /*
  cout << "Clause prior means:" << endl;
  cout << "_________________________________" << endl;
  mlns[0]->printClausePriorMeans(cout, domains[0]);
  cout << "_________________________________" << endl;
  cout << endl;

  cout << "Formula prior means:" << endl;
  cout << "_________________________________" << endl;
  mlns[0]->printFormulaPriorMeans(cout);
  cout << "_________________________________" << endl;
  cout << endl;
  //*/


  //////////////////////////// structure learning //////////////////////////////

  if (nonEvidPredNames.size() == 0) 
    domains[0]->getNonEqualPredicateNames(nonEvidPredNames);
  bool cacheClauses = (cacheSize > 0);
  
  UtStructLearn sl(&mlns, startFromEmptyMLN, outMLNFile, &domains, 
                 &nonEvidPredNames, maxVars, maxNumPredicates, cacheClauses,
                 cacheSize, tryAllFlips,
                 !noSampleClauses, ddelta, epsilon, 
                 minClauseSamples, maxClauseSamples,
                 !noPrior, priorMean, priorStdDev, 
                 !noWtPredsEqually, 
                 lbMaxIter, lbConvThresh, looseMaxIter, looseConvThresh, 
                 minWt, penalty, 
                 !noSampleGndPreds,fraction,minGndPredSamples,maxGndPredSamples);
  				
  sl.busl(countIsAtLeast, indepThreshold, maxNumFreeVars, minimumTrueProportion);
  
  ////////////////////////////// clean up ////////////////////////////////////
  
  deleteDomains(domains);
  for (int i = 0; i < mlns.size(); i++) delete mlns[i];
  PowerSet::deletePowerSet();

  cout << "Total time taken = "; 
  Timer::printTime(cout, timer.time()-startSec); cout << endl;
}


