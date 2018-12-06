/*
 * All of the documentation and software included in the
 * Alchemy Software is copyrighted by Stanley Kok, Parag
 * Singla, Matthew Richardson, Pedro Domingos, Marc
 * Sumner, Hoifung Poon, and Daniel Lowd.
 * 
 * Copyright [2004-07] Stanley Kok, Parag Singla, Matthew
 * Richardson, Pedro Domingos, Marc Sumner, Hoifung
 * Poon, and Daniel Lowd. All rights reserved.
 * 
 * Contact: Pedro Domingos, University of Washington
 * (pedrod@cs.washington.edu).
 * 
 * Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that
 * the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above
 * copyright notice, this list of conditions and the
 * following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the
 * above copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use
 * of this software must display the following
 * acknowledgment: "This product includes software
 * developed by Stanley Kok, Parag Singla, Matthew
 * Richardson, Pedro Domingos, Marc Sumner, Hoifung
 * Poon, and Daniel Lowd in the Department of Computer Science and
 * Engineering at the University of Washington".
 * 
 * 4. Your publications acknowledge the use or
 * contribution made by the Software to your research
 * using the following citation(s): 
 * Stanley Kok, Parag Singla, Matthew Richardson and
 * Pedro Domingos (2005). "The Alchemy System for
 * Statistical Relational AI", Technical Report,
 * Department of Computer Science and Engineering,
 * University of Washington, Seattle, WA.
 * http://www.cs.washington.edu/ai/alchemy.
 * 
 * 5. Neither the name of the University of Washington nor
 * the names of its contributors may be used to endorse or
 * promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY OF WASHINGTON
 * AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
 * OF WASHINGTON OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <fstream>
#include <iostream>
#include <sstream>
//#include <gsl/gsl_cdf.h>
#include <cmath>
#include "arguments.h"
#include "inferenceargs.h"
#include "lbfgsb.h"
#include "discriminativelearner.h"
#include "learnwts.h"
#include "maxwalksat.h"
#include "mcsat.h"
#include "gibbssampler.h"
#include "simulatedtempering.h"

  //set to false to disable printing of clauses when they are counted during 
  //generative learning
bool PRINT_CLAUSE_DURING_COUNT = true;

const double DISC_DEFAULT_STD_DEV = 2;
const double GEN_DEFAULT_STD_DEV = 100;

  // Variables for holding inference command line args are in inferenceargs.h
bool discLearn = false;
bool genLearn = false;
char* outMLNFile = NULL;
char* dbFiles = NULL;
char* nonEvidPredsStr = NULL;
bool noAddUnitClauses = false;
bool multipleDatabases = false;
bool initWithLogOdds = false;
bool isQueryEvidence = false;

bool aPeriodicMLNs = false;

bool noPrior = false;
double priorMean = 0;
double priorStdDev = -1;

  // Generative learning args
int maxIter = 10000;
double convThresh = 1e-5;
bool noEqualPredWt = false;

  // Discriminative learning args
int numIter = 100;
double maxSec  = 0;
double maxMin  = 0;
double maxHour = 0;
double learningRate = 0.001;
double momentum = 0.0;
bool withEM = false;
char* aInferStr = NULL;
bool noUsePerWeight = false;
bool useNewton = false;
bool useCG = false;
bool useVP = false;
int  discMethod = DiscriminativeLearner::CG;
double cg_lambda = 100;
double cg_max_lambda = DBL_MAX;
bool   cg_noprecond = false;
int amwsMaxSubsequentSteps = -1;


  // Inference arguments needed for disc. learning defined in inferenceargs.h
  // TODO: List the arguments common to learnwts and inference in
  // inferenceargs.h. This can't be done with a static array.
ARGS ARGS::Args[] = 
{
    // BEGIN: Common arguments
  ARGS("i", ARGS::Req, ainMLNFiles, 
       "Comma-separated input .mln files. (With the -multipleDatabases "
       "option, the second file to the last one are used to contain constants "
       "from different databases, and they correspond to the .db files "
       "specified with the -t option.)"),

  ARGS("cw", ARGS::Opt, aClosedWorldPredsStr,
       "Specified non-evidence atoms (comma-separated with no space) are "
       "closed world, otherwise, all non-evidence atoms are open world. Atoms "
       "appearing here cannot be query atoms and cannot appear in the -o "
       "option."),

  ARGS("ow", ARGS::Opt, aOpenWorldPredsStr,
       "Specified evidence atoms (comma-separated with no space) are open "
       "world, while other evidence atoms are closed-world. "
       "Atoms appearing here cannot appear in the -c option."),
    // END: Common arguments

    // BEGIN: Common inference arguments
  ARGS("m", ARGS::Tog, amapPos, 
       "(Embed in -infer argument) "
       "Run MAP inference and return only positive query atoms."),

  ARGS("a", ARGS::Tog, amapAll, 
       "(Embed in -infer argument) "
       "Run MAP inference and show 0/1 results for all query atoms."),

  ARGS("p", ARGS::Tog, agibbsInfer, 
       "(Embed in -infer argument) "
       "Run inference using MCMC (Gibbs sampling) and return probabilities "
       "for all query atoms."),
  
  ARGS("ms", ARGS::Tog, amcsatInfer,
       "(Embed in -infer argument) "
       "Run inference using MC-SAT and return probabilities "
       "for all query atoms"),

  ARGS("simtp", ARGS::Tog, asimtpInfer,
       "(Embed in -infer argument) "
       "Run inference using simulated tempering and return probabilities "
       "for all query atoms"),

  ARGS("seed", ARGS::Opt, aSeed,
       "(Embed in -infer argument) "
       "[2350877] Seed used to initialize the randomizer in the inference "
       "algorithm. If not set, seed is initialized from a fixed random number."),

  ARGS("lazy", ARGS::Opt, aLazy, 
       "(Embed in -infer argument) "
       "[false] Run lazy version of inference if this flag is set."),
  
  ARGS("lazyNoApprox", ARGS::Opt, aLazyNoApprox, 
       "(Embed in -infer argument) "
       "[false] Lazy version of inference will not approximate by deactivating "
       "atoms to save memory. This flag is ignored if -lazy is not set."),
  
  ARGS("memLimit", ARGS::Opt, aMemLimit, 
       "(Embed in -infer argument) "
       "[-1] Maximum limit in kbytes which should be used for inference. "
       "-1 means main memory available on system is used."),
    // END: Common inference arguments

    // BEGIN: MaxWalkSat args
  ARGS("mwsMaxSteps", ARGS::Opt, amwsMaxSteps,
       "(Embed in -infer argument) "
       "[100000] (MaxWalkSat) The max number of steps taken."),

  ARGS("tries", ARGS::Opt, amwsTries, 
       "(Embed in -infer argument) "
       "[1] (MaxWalkSat) The max number of attempts taken to find a solution."),

  ARGS("targetWt", ARGS::Opt, amwsTargetWt,
       "(Embed in -infer argument) "
       "[the best possible] (MaxWalkSat) MaxWalkSat tries to find a solution "
       "with weight <= specified weight."),

  ARGS("hard", ARGS::Opt, amwsHard, 
       "(Embed in -infer argument) "
       "[false] (MaxWalkSat) MaxWalkSat never breaks a hard clause in order to "
       "satisfy a soft one."),
  
  ARGS("heuristic", ARGS::Opt, amwsHeuristic,
       "(Embed in -infer argument) "
       "[2] (MaxWalkSat) Heuristic used in MaxWalkSat (0 = RANDOM, 1 = BEST, "
       "2 = TABU, 3 = SAMPLESAT)."),
  
  ARGS("tabuLength", ARGS::Opt, amwsTabuLength,
       "(Embed in -infer argument) "
       "[5] (MaxWalkSat) Minimum number of flips between flipping the same "
       "atom when using the tabu heuristic in MaxWalkSat." ),

  ARGS("lazyLowState", ARGS::Opt, amwsLazyLowState, 
       "(Embed in -infer argument) "
       "[false] (MaxWalkSat) If false, the naive way of saving low states "
       "(each time a low state is found, the whole state is saved) is used; "
       "otherwise, a list of variables flipped since the last low state is "
       "kept and the low state is reconstructed. This can be much faster for "
       "very large data sets."),  
    // END: MaxWalkSat args

    // BEGIN: MCMC args
  ARGS("burnMinSteps", ARGS::Opt, amcmcBurnMinSteps,
       "(Embed in -infer argument) "
       "[0] (MCMC) Minimun number of burn in steps (-1: no minimum)."),

  ARGS("burnMaxSteps", ARGS::Opt, amcmcBurnMaxSteps,
       "(Embed in -infer argument) "
       "[0] (MCMC) Maximum number of burn-in steps (-1: no maximum)."),

  ARGS("minSteps", ARGS::Opt, amcmcMinSteps, 
       "(Embed in -infer argument) "
       "[-1] (MCMC) Minimum number of MCMC sampling steps."),

  ARGS("maxSteps", ARGS::Opt, amcmcMaxSteps, 
       "(Embed in -infer argument) "
       "[optimal] (MCMC) Maximum number of MCMC sampling steps."),

  ARGS("maxSeconds", ARGS::Opt, amcmcMaxSeconds, 
       "(Embed in -infer argument) "
       "[-1] (MCMC) Max number of seconds to run MCMC (-1: no maximum)."),
    // END: MCMC args
  
    // BEGIN: Simulated tempering args
  ARGS("subInterval", ARGS::Opt, asimtpSubInterval,
       "(Embed in -infer argument) "
        "[2] (Simulated Tempering) Selection interval between swap attempts"),

  ARGS("numRuns", ARGS::Opt, asimtpNumST,
       "(Embed in -infer argument) "
        "[3] (Simulated Tempering) Number of simulated tempering runs"),

  ARGS("numSwap", ARGS::Opt, asimtpNumSwap,
       "(Embed in -infer argument) "
        "[10] (Simulated Tempering) Number of swapping chains"),
    // END: Simulated tempering args

    // BEGIN: SampleSat args
  ARGS("numSolutions", ARGS::Opt, amwsNumSolutions,
       "(Embed in -infer argument) "
       "[10] (MC-SAT) Return nth SAT solution in SampleSat"),

  ARGS("saRatio", ARGS::Opt, assSaRatio,
       "(Embed in -infer argument) "
       "[50] (MC-SAT) Ratio of sim. annealing steps mixed with WalkSAT in "
       "MC-SAT"),

  ARGS("saTemperature", ARGS::Opt, assSaTemp,
       "(Embed in -infer argument) "
        "[10] (MC-SAT) Temperature (/100) for sim. annealing step in "
        "SampleSat"),

  ARGS("lateSa", ARGS::Tog, assLateSa,
       "(Embed in -infer argument) "
       "[false] Run simulated annealing from the start in SampleSat"),
    // END: SampleSat args

    // BEGIN: Gibbs sampling args
  ARGS("numChains", ARGS::Opt, amcmcNumChains, 
       "(Embed in -infer argument) "
       "[10] (Gibbs) Number of MCMC chains for Gibbs sampling (there must be "
       "at least 2)."),

  ARGS("delta", ARGS::Opt, agibbsDelta,
       "(Embed in -infer argument) "
       "[0.05] (Gibbs) During Gibbs sampling, probabilty that epsilon error is "
       "exceeded is less than this value."),

  ARGS("epsilonError", ARGS::Opt, agibbsEpsilonError,
       "(Embed in -infer argument) "
       "[0.01] (Gibbs) Fractional error from true probability."),

  ARGS("fracConverged", ARGS::Opt, agibbsFracConverged, 
       "(Embed in -infer argument) "
       "[0.95] (Gibbs) Fraction of ground atoms with probabilities that "
       "have converged."),

  ARGS("walksatType", ARGS::Opt, agibbsWalksatType, 
       "(Embed in -infer argument) "
       "[1] (Gibbs) Use Max Walksat to initialize ground atoms' truth values "
       "in Gibbs sampling (1: use Max Walksat, 0: random initialization)."),

  ARGS("samplesPerTest", ARGS::Opt, agibbsSamplesPerTest, 
       "(Embed in -infer argument) "
       "[100] Perform convergence test once after this many number of samples "
       "per chain."),
    // END: Gibbs sampling args

    // BEGIN: Weight learning specific args
  ARGS("periodic", ARGS::Tog, aPeriodicMLNs,
       "Write out MLNs after 1, 2, 5, 10, 20, 50, etc. iterations"),

  ARGS("infer", ARGS::Opt, aInferStr,
       "Specified inference parameters when using discriminative learning. "
       "The arguments are to be encapsulated in \"\" and the syntax is "
       "identical to the infer command (run infer with no commands to see "
       "this). If not specified, 5 steps of MC-SAT with no burn-in is used."),

  ARGS("d", ARGS::Tog, discLearn, "Discriminative weight learning."),

  ARGS("g", ARGS::Tog, genLearn, "Generative weight learning."),

  ARGS("o", ARGS::Req, outMLNFile, 
       "Output .mln file containing formulas with learned weights."),

  ARGS("t", ARGS::Req, dbFiles, 
       "Comma-separated .db files containing the training database "
       "(of true/false ground atoms), including function definitions, "
       "e.g. ai.db,graphics.db,languages.db."),

  ARGS("ne", ARGS::Opt, nonEvidPredsStr, 
       "First-order non-evidence predicates (comma-separated with no space),  "
       "e.g., cancer,smokes,friends. For discriminative learning, at least "
       "one non-evidence predicate must be specified. For generative learning, "
       "the specified predicates are included in the (weighted) pseudo-log-"
       "likelihood computation; if none are specified, all are included."),
    
  ARGS("noAddUnitClauses", ARGS::Tog, noAddUnitClauses,
       "If specified, unit clauses are not included in the .mln file; "
       "otherwise they are included."),

  ARGS("multipleDatabases", ARGS::Tog, multipleDatabases,
       "If specified, each .db file belongs to a separate database; "
       "otherwise all .db files belong to the same database."),

  ARGS("withEM", ARGS::Tog, withEM,
       "If set, EM is used to fill in missing truth values; "
       "otherwise missing truth values are set to false."),

  ARGS("dNumIter", ARGS::Opt, numIter, 
       "[100] (For discriminative learning only.) "
       "Number of iterations to run discriminative learning method."),

  ARGS("dMaxSec", ARGS::Opt, maxSec,
       "[-1] Maximum number of seconds to spend learning"),

  ARGS("dMaxMin", ARGS::Opt, maxMin,
       "[-1] Maximum number of minutes to spend learning"),

  ARGS("dMaxHour", ARGS::Opt, maxHour,
       "[-1] Maximum number of hours to spend learning"),
  
  ARGS("dLearningRate", ARGS::Opt, learningRate, 
       "[0.001] (For discriminative learning only) "
       "Learning rate for the gradient descent in disc. learning algorithm."),

  ARGS("dMomentum", ARGS::Opt, momentum, 
       "[0.0] (For discriminative learning only) "
       "Momentum term for the gradient descent in disc. learning algorithm."),
       
  ARGS("dNoPW", ARGS::Tog, noUsePerWeight,
       "[false] (For voted perceptron only.) "
       "Do not use per-weight learning rates, based on the number of true "
       "groundings per weight."),
  
  ARGS("dVP", ARGS::Tog, useVP,
       "[false] (For discriminative learning only) "
       "Use voted perceptron to learn the weights."),

  ARGS("dNewton", ARGS::Tog, useNewton,
       "[false] (For discriminative learning only) "
       "Use diagonalized Newton's method to learn the weights."),

  ARGS("dCG", ARGS::Tog, useCG,
       "[false] (For discriminative learning only) "
       "Use rescaled conjugate gradient to learn the weights."),

  ARGS("cgLambda", ARGS::Opt, cg_lambda,
       "[100] (For CG only) (For CG only) Starting value of parameter to limit "
       "step size"),

  ARGS("cgMaxLambda", ARGS::Opt, cg_max_lambda,
       "[no limit] (For CG only) Maximum value of parameter to limit step size"),

  ARGS("cgNoPrecond", ARGS::Tog, cg_noprecond,
       "[false] (For CG only) precondition with the diagonal Hessian"),
       
  ARGS("queryEvidence", ARGS::Tog, isQueryEvidence, 
       "[false] If this flag is set, then all the groundings of query preds not "
       "in db are assumed false evidence."),

  ARGS("dInitWithLogOdds", ARGS::Tog, initWithLogOdds,
       "[false] (For discriminative learning only.) "
       "Initialize clause weights to their log odds instead of zero."),

  ARGS("dMwsMaxSubsequentSteps", ARGS::Opt, amwsMaxSubsequentSteps,
       "[Same as mwsMaxSteps] (For discriminative learning only.) The max "
       "number of MaxWalkSat steps taken in subsequent iterations (>= 2) of "
       "disc. learning. If not specified, mwsMaxSteps is used in each "
       "iteration"),
  
  ARGS("gMaxIter", ARGS::Opt, maxIter, 
       "[10000] (For generative learning only.) "
       "Max number of iterations to run L-BFGS-B, "
       "the optimization algorithm for generative learning."),
  
  ARGS("gConvThresh", ARGS::Opt, convThresh, 
       "[1e-5] (For generative learning only.) "
       "Fractional change in pseudo-log-likelihood at which "
       "L-BFGS-B terminates."),

  ARGS("gNoEqualPredWt", ARGS::Opt, noEqualPredWt, 
       "[false] (For generative learning only.) "
       "If specified, the predicates are not weighted equally in the "
       "pseudo-log-likelihood computation; otherwise they are."),
  
  ARGS("noPrior", ARGS::Tog, noPrior, "No Gaussian priors on formula weights."),

  ARGS("priorMean", ARGS::Opt, priorMean, 
       "[0] Means of Gaussian priors on formula weights. By default, "
       "for each formula, it is the weight given in the .mln input file, " 
       "or fraction thereof if the formula turns into multiple clauses. "
       "This mean applies if no weight is given in the .mln file."),

  ARGS("priorStdDev", ARGS::Opt, priorStdDev, 
       "[2 for discriminative learning. 100 for generative learning] "
       "Standard deviations of Gaussian priors on clause weights."),

  ARGS()
};

//bool extractPredNames(...) defined in infer.h

int main(int argc, char* argv[])
{
  ARGS::parse(argc,argv,&cout);

  if (!discLearn && !genLearn) 
  { 
      // If nothing specified, then use disc. learning
    discLearn = true;
    
    //cout << "must specify either -d or -g "
    //     <<"(discriminative or generative learning) " << endl; 
    //return -1;
  }

  Timer timer;
  double startSec = timer.time();
  double begSec;

  if (priorStdDev < 0)
  {
    if (discLearn) 
    { 
      cout << "priorStdDev set to (discriminative learning's) default of " 
           << DISC_DEFAULT_STD_DEV << endl;
      priorStdDev = DISC_DEFAULT_STD_DEV;
    }
    else
    {
      cout << "priorStdDev set to (generative learning's) default of " 
           << GEN_DEFAULT_STD_DEV << endl;
      priorStdDev = GEN_DEFAULT_STD_DEV;      
    }
  }


  ///////////////////////// check and extract the parameters //////////////////
  if (discLearn && nonEvidPredsStr == NULL)
  {
    cout << "ERROR: you must specify non-evidence predicates for "
         << "discriminative learning" << endl;
    return -1;
  }

  if (maxIter <= 0)  { cout << "maxIter must be > 0" << endl; return -1; }
  if (convThresh <= 0 || convThresh > 1)  
  { cout << "convThresh must be > 0 and <= 1" << endl; return -1;  }
  if (priorStdDev <= 0) { cout << "priorStdDev must be > 0" << endl; return -1;}

  if (amwsMaxSteps <= 0)
  { cout << "ERROR: mwsMaxSteps must be positive" << endl; return -1; }
  
    // If max. subsequent steps not specified, use amwsMaxSteps
  if (amwsMaxSubsequentSteps <= 0) amwsMaxSubsequentSteps = amwsMaxSteps;

  if (amwsTries <= 0)
  { cout << "ERROR: tries must be positive" << endl; return -1; }

  if (aMemLimit <= 0 && aMemLimit != -1)
  { cout << "ERROR: limit must be positive (or -1)" << endl; return -1; }

  if (!discLearn && aLazy)
  {
    cout << "ERROR: lazy can only be used with discriminative learning"
         << endl;
    return -1;
  }

  ofstream out(outMLNFile);
  if (!out.good())
  {
    cout << "ERROR: unable to open " << outMLNFile << endl;
    return -1;
  }

    // Parse the inference parameters, if given
  if (discLearn)
  {
      // If no method given, then use CG
    if (!useVP && !useCG && !useNewton)
      useCG = true;
      // Per-weight can not be used with SCG or Newton
    if ((useCG || useNewton) && !noUsePerWeight)
    {
      noUsePerWeight = true;
    }

      // maxSteps is optimized after domains are built
    amcmcMaxSteps = -1;
    amcmcBurnMaxSteps = -1;
    if (!aInferStr)
    {
        // Set defaults of inference inside disc. weight learning:
        // MC-SAT with no burn-in, 5 steps
      amcsatInfer = true;
    }
      // If inference method given, we need to parse the parameters
    else
    {
      int inferArgc = 0;
      char **inferArgv = new char*[200];
      for (int i = 0; i < 200; i++)
      {
        inferArgv[i] = new char[500];
      }

        // Have to add program name (which is not used) to start of string
      string inferString = "infer ";
      inferString.append(aInferStr);
      extractArgs(inferString.c_str(), inferArgc, inferArgv);
      cout << "extractArgs " << inferArgc << endl;
      for (int i = 0; i < inferArgc; i++)
      {
        cout << i << ": " << inferArgv[i] << endl;
      }

      ARGS::parseFromCommandLine(inferArgc, inferArgv);

        // Delete memory allocated for args
      for (int i = 0; i < 200; i++)
      {
        delete[] inferArgv[i];
      }
      delete[] inferArgv; 
    }
    
    if (!asimtpInfer && !amapPos && !amapAll && !agibbsInfer && !amcsatInfer)
    {
        // If nothing specified, use MC-SAT
      amcsatInfer = true;
    }    
  }


  //the second .mln file to the last one in ainMLNFiles _may_ be used 
  //to hold constants, so they are held in constFilesArr. They will be
  //included into the first .mln file.

    //extract .mln and .db, file names
  Array<string> constFilesArr;
  Array<string> dbFilesArr;
  extractFileNames(ainMLNFiles, constFilesArr);
  assert(constFilesArr.size() >= 1);
  string inMLNFile = constFilesArr[0];
  constFilesArr.removeItem(0);
  extractFileNames(dbFiles, dbFilesArr);

  if (dbFilesArr.size() <= 0)
  {cout<<"ERROR: must specify training data with -t option."<<endl; return -1;}
 
    // if multiple databases, check the number of .db/.func files
  if (multipleDatabases) 
  {
      //if # .mln files containing constants/.func files and .db files are diff
    if ((constFilesArr.size() > 0 && constFilesArr.size() != dbFilesArr.size()))
    {
      cout << "ERROR: when there are multiple databases, if .mln files "
           << "containing constants are specified, there must " 
           << "be the same number of them as .db files" << endl;
      return -1;
    }
  }

  StringHashArray nonEvidPredNames;
  if (nonEvidPredsStr)
  {
    if (!extractPredNames(nonEvidPredsStr, NULL, nonEvidPredNames))
    {
      cout << "ERROR: failed to extract non-evidence predicate names." << endl;
      return -1;
    }
  }

  StringHashArray owPredNames;
  StringHashArray cwPredNames;

  ////////////////////////// create domains and mlns //////////////////////////

  cout << "Parsing MLN and creating domains..." << endl;
  StringHashArray* nePredNames = (discLearn) ? &nonEvidPredNames : NULL;
  Array<Domain*> domains;
  Array<MLN*> mlns;
  begSec = timer.time();
  bool allPredsExceptQueriesAreCW = true;
  if (discLearn)
  {
      //extract names of open-world evidence predicates
    if (aOpenWorldPredsStr)
    {
      if (!extractPredNames(string(aOpenWorldPredsStr), NULL, owPredNames)) 
        return -1;
      assert(owPredNames.size() > 0);
    }

      //extract names of closed-world non-evidence predicates
    if (aClosedWorldPredsStr)
    {
      if (!extractPredNames(string(aClosedWorldPredsStr), NULL, cwPredNames)) 
        return -1;
      assert(cwPredNames.size() > 0);
      if (!checkQueryPredsNotInClosedWorldPreds(nonEvidPredNames, cwPredNames))
        return -1;
    }
 
    //allPredsExceptQueriesAreCW = owPredNames.empty();
    allPredsExceptQueriesAreCW = false;
  }
    // Parse as if lazy inference is set to true to set evidence atoms in DB
    // If lazy is not used, this is removed from DB
  createDomainsAndMLNs(domains, mlns, multipleDatabases, inMLNFile, 
                       constFilesArr, dbFilesArr, nePredNames,
                       !noAddUnitClauses, priorMean, true,
                       allPredsExceptQueriesAreCW, &owPredNames, &cwPredNames);
  cout << "Parsing MLN and creating domains took "; 
  Timer::printTime(cout, timer.time() - begSec); cout << endl;

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
  */

  //////////////////// set the prior means & standard deviations //////////////

    //we need an index translator if clauses do not line up across multiple DBs
  IndexTranslator* indexTrans 
    = (IndexTranslator::needIndexTranslator(mlns, domains)) ?
       new IndexTranslator(&mlns, &domains) : NULL;  

  if (indexTrans) 
    cout << endl << "the weights of clauses in the CNFs of existential"
         << " formulas will be tied" << endl;

  Array<double> priorMeans, priorStdDevs;
  if (!noPrior)
  {
    if (indexTrans)
    {
      indexTrans->setPriorMeans(priorMeans);
      priorStdDevs.growToSize(priorMeans.size());
      for (int i = 0; i < priorMeans.size(); i++)
        priorStdDevs[i] = priorStdDev;
    }
    else
    {
      const ClauseHashArray* clauses = mlns[0]->getClauses();
      int numClauses = clauses->size();
      for (int i = 0; i < numClauses; i++)
      {
        priorMeans.append((*clauses)[i]->getWt());
        priorStdDevs.append(priorStdDev);
      }
    }
  }
  // HACK -- not sure if this is right... but the old version was broke!
  // This may fail when there's an indexTrans.  [Daniel]
  //int numClausesFormulas = priorMeans.size();
  int numClausesFormulas = mlns[0]->getClauses()->size();


  //////////////////////  discriminative/generative learning /////////////////
  Array<double> wts;
  //gsl_cdf_binomial_P 
  //— Function: double gsl_cdf_binomial_P (unsigned int k, double p, unsigned int n)
  //— Function: double gsl_cdf_binomial_Q (unsigned int k, double p, unsigned int n)
  //FLAG - Only works for one input db
  Array< double > * predTrue = new Array< double > (domains[0]->getDB()->getNumPredIds());
  Array< double > * predPoss = new Array< double > (domains[0]->getDB()->getNumPredIds());
  //double checkGuass = gsl_cdf_gaussian_P(0.8,1);
  //cout << checkGuass << endl;
  //exit(-1);
  //int position = 0;
  //cout << M_E << endl;
  //cout << M_PI << endl;
  //exit(-1);
  for(int ix = 0;  ix < mlns[0]->getNumClauses(); ix++){
    Clause* toEvaluate = (Clause*)mlns[0]->getClause(ix);
    int numPreds = toEvaluate->getNumPredicates();
    if (numPreds == 1){
      Predicate* currPred = toEvaluate->getPredicate(0);
      double numTrueGroundings = toEvaluate->getNumTrueGroundings(domains[0], domains[0]->getDB(), false);
      double pG = toEvaluate->getNumGroundings(domains[0]); 
      (*predPoss)[currPred->getId()] = pG;
      (*predTrue)[currPred->getId()] = numTrueGroundings;// / pG;
      currPred->print(cout, domains[0]);
      cout << " True " << numTrueGroundings << " Possible " << pG << endl;
    }
  }
  cout << endl;
  cout << "Number of Clauses " << mlns[0]->getNumClauses() << endl;
  for(int ix = 0; ix < mlns[0]->getNumClauses(); ix++){
    Clause* toEvaluate = (Clause*)mlns[0]->getClause(ix);
    int numPreds = toEvaluate->getNumPredicates();
    if (numPreds == 2){
      double numTrueGroundings = toEvaluate->getNumTrueGroundings(domains[0], domains[0]->getDB(), false);
      double numGroundings = toEvaluate->getNumGroundings(domains[0]);
      cout << "Under Review ";
      toEvaluate->printWithoutWt(cout, domains[0]);
      cout << endl;
      Clause* c1 = new Clause();
      Clause* u1 = new Clause();

      const Array< Predicate * > * predicates = toEvaluate->getPredicates();

      c1->appendPredicate((*predicates)[0]);
      c1->canonicalize();
      u1->appendPredicate((*predicates)[1]);
      u1->canonicalize();

      double ntg1 = c1->getNumTrueGroundings(domains[0], domains[0]->getDB(), false);
      double ng1 = c1->getNumGroundings(domains[0]);
      double prob1 = ntg1 / ng1;
      double tmp1 = 0;
      double tmp2 = 0;
      double u_ntg1 = u1->getNumTrueGroundings(domains[0], domains[0]->getDB(), false);
      double u_ng1 = u1->getNumGroundings(domains[0]);
      double up1 = u_ntg1 / u_ng1;

      tmp1 = (1 - prob1);
      tmp2 = 1 - up1;
      prob1 = 1 - (tmp1 * tmp2);

      cout << "Clause " << ix << " Decomposition into unit clauses" << endl;
      c1->printWithoutWt(cout, domains[0]);
      cout << " " << tmp1 << " ";
      u1->printWithoutWt(cout, domains[0]);
      cout << " " << tmp2 << endl;

      double thisProb = numTrueGroundings / numGroundings;
      double sqrtPoss = sqrt(numGroundings);

      double d1 = sqrt(prob1 * (1 - prob1));

      double v1 = (sqrtPoss * (thisProb - prob1)) / d1;

      if (v1 < 0){
	v1 = 0 - v1;
      }

      double pValue1 = 0;//2 * (1 - gsl_cdf_gaussian_P(v1 ,1));//gsl_cdf_binomial_P(ntg,prob1,uPoss);

      double nFact = numGroundings * log(numGroundings) - numGroundings + log(sqrt(2 * M_PI * numGroundings));
      double kFact = numTrueGroundings * log(numTrueGroundings) - numTrueGroundings + log(sqrt(2 * M_PI * numTrueGroundings));
      double diff = (numGroundings - numTrueGroundings);
      double dFact = diff * log(diff) - diff + log(sqrt(2 * M_PI * diff));
      double constFact = nFact - kFact -dFact;
      double diff1 = numTrueGroundings * log(prob1) + diff * log(1 - prob1);

      double kl1 = thisProb * log(thisProb/prob1) + (1-thisProb) * log((1-thisProb)/(1-prob1));


      double val1 = nFact - kFact - dFact + numTrueGroundings * log(prob1) + diff * log(1 - prob1);

      cout << nFact << " " << kFact << " " << dFact << endl;
      cout << "True " << numTrueGroundings << " Possible " << numGroundings << " Difference " << diff << endl; //ntg << " " << uPoss << endl;
      cout << "const    = " << (unsigned int) constFact << endl;
      cout << "Decomp 1 = " << (unsigned int) diff1 << endl;

      cout << "Probs: " << thisProb << " ";
      
      cout << prob1 << endl;

      cout << "Init p-values " << pValue1 << endl;
      cout << "Average Clause " << ix << ": " <<  pValue1 << " " << pValue1 <<  endl;
      cout << "Log Average " << ix << ": " << val1 << " " << val1 << endl;
      double pDiff1 = fabs(thisProb - prob1);
      cout << "Average diff " << pDiff1 << " " << pDiff1 << endl;
      cout << "Average KL " <<  kl1 << " " << kl1 << endl << endl;

      
    }
    if (numPreds == 3 ){
      double numTrueGroundings = toEvaluate->getNumTrueGroundings(domains[0], domains[0]->getDB(), false);
      double numGroundings = toEvaluate->getNumGroundings(domains[0]);
      int total = 0;
      //    cout << numPreds << endl; 	getNumGroundings 
      unsigned int poss = 1;
      double dPoss = 1;
      //Flag -> should be a pointer
      //int[] tots = new int[3];
      for(int jx = 0; jx < numPreds; jx++){
	Predicate* currPred = toEvaluate->getPredicate(jx);
	//cout << "Clause " << ix << " Pred " << jx << " ";
	//ostream & 	print (ostream &out, const Domain *const &domain) const
	//currPred->print(cout, domains[0]);
	int predId = currPred->getId();
	total += domains[0]->getDB()->getNumTrueGndPreds(predId);

	
	const PredicateTemplate* pTemplate = currPred->getTemplate();
	const Array<int> * pTypes = pTemplate->getTermTypesAsInt();
	//const Array< int > * pTypes = pTemplate->getTermTypesAsInt();
	//const Array< const char * > * pStr = pTemplate->getTermTypesAsStr();
	double predGrounds = 1;
	unsigned int predGroundsI = 1;
	for(int jx = 0; jx < pTemplate->getNumTerms(); jx++){
	  predGroundsI = predGrounds * ((unsigned int)domains[0]->getNumConstantsByType((*pTypes)[jx]));
	  predGrounds = predGrounds * ((double)domains[0]->getNumConstantsByType((*pTypes)[jx]));
	}
      
	poss = poss * predGroundsI;//domains[0]->getDB()->getNumTrueGndPreds(predId);
	dPoss = dPoss * predGrounds;//domains[0]->getDB()->getNumTrueGndPreds(predId);
      }
      cout << "Under Review ";
      toEvaluate->printWithoutWt(cout, domains[0]);
      cout << endl;
      Clause* c1 = new Clause(*toEvaluate);
      Clause* c2 = new Clause(*toEvaluate);
      Clause* c3 = new Clause(*toEvaluate);
      Clause* u1 = new Clause();
      Clause* u2 = new Clause();
      Clause* u3 = new Clause();
      const Array< Predicate * > * predicates = toEvaluate->getPredicates();
      //cout << "Here 1" << endl;

      /*
      c1->appendPredicate((*predicates)[0]);
      c1->appendPredicate((*predicates)[1]);
      */
      c1->removePredicate(2);
      c1->canonicalize();
      u1->appendPredicate((*predicates)[2]);
      u1->canonicalize();
      //cout << "Created ";
      //c1->print(cout, domains[0]);
      //cout << endl;
      //cout << "Here 2" << endl;
      double ntg1 = c1->getNumTrueGroundings(domains[0], domains[0]->getDB(), false);
      double ng1 = c1->getNumGroundings(domains[0]);
      double prob1 = ntg1 / ng1;
      double tmp1 = 0;
      double tmp2 = 0;
      double u_ntg1 = u1->getNumTrueGroundings(domains[0], domains[0]->getDB(), false);
      double u_ng1 = u1->getNumGroundings(domains[0]);
      double up1 = u_ntg1 / u_ng1;
      //int id1 = toEvaluate->getPredicate(2)->getId();
      //if (toEvaluate->getPredicate(2)->getSense()){
      tmp1 = (1 - prob1);
      tmp2 = 1 - up1; //((*predTrue)[id1] / (*predPoss)[id1]);
      prob1 = 1 - (tmp1 * tmp2);

      cout << "Clause " << ix << " Decomposition " << 1;
      
      cout << " " << endl;
      c1->printWithoutWt(cout, domains[0]);
      cout << " " << tmp1 << " ";
      toEvaluate->getPredicate(2)->print(cout, domains[0]);
      cout << " " << tmp2 << endl;

      //cout << "Here 3" << endl;
      /*
      c2->appendPredicate((*predicates)[0]);
      c2->appendPredicate((*predicates)[2]);
      */
      //cout << "Here 4" << endl;
      c2->removePredicate(1);
      c2->canonicalize();
      u2->appendPredicate((*predicates)[1]);
      u2->canonicalize();
      double ntg2 = c2->getNumTrueGroundings(domains[0], domains[0]->getDB(), false);
      double ng2 = c2->getNumGroundings(domains[0]);
  
      double prob2 = ntg2 / ng2;
      double u_ntg2 = u2->getNumTrueGroundings(domains[0], domains[0]->getDB(), false);
      double u_ng2 = u2->getNumGroundings(domains[0]);
      double up2 = u_ntg2 / u_ng2;
      //int id2 = toEvaluate->getPredicate(1)->getId();
      //if (toEvaluate->getPredicate(1)->getSense()){
      tmp1 = (1 - prob2);
      tmp2 = 1 - up2;
      prob2 = 1 - (tmp1 * tmp2);
	//}
	/*
	  else{
	  tmp1 = (1 - prob2);
	  tmp2 = 1- (((*predPoss)[id2] - (*predTrue)[id2]) / (*predPoss)[id2]);
	  prob2= 1 - (tmp1 * tmp2);
	  //prob1 = 1 - ((1 - prob1) * (1 - ((( (*predPoss)[id1] -  (*predTrue)[id1]) / (*predPoss)[id1]))));
	  }
	*/
      cout << "Clause " << ix << " Decomposition " << 2 << endl;
      c2->printWithoutWt(cout, domains[0]);
      cout << " " << tmp1 << " ";
      toEvaluate->getPredicate(1)->print(cout, domains[0]);
      cout << " " << tmp2 << endl;
      
   
      /*

	if (toEvaluate->getPredicate(1)->getSense()){
	prob2 = prob2 * ((*predTrue)[id2] / (*predPoss)[id2]);
	}
	else{
	prob2 = prob2 * (( (*predPoss)[id2] -  (*predTrue)[id2]) / (*predPoss)[id2]);
	}
	cout << "Here 5" << endl;
      */
      //cout << "Begin to create" << endl;
      /*
      c3->appendPredicate((*predicates)[1]);
      c3->appendPredicate((*predicates)[2]);
      */
      c3->removePredicate(0);
      c3->canonicalize();
      u3->appendPredicate((*predicates)[0]);
      u3->canonicalize();
      //cout << "Created Clause 3" << endl;
      c3->print(cout, domains[0]);
      cout << endl;
      double ntg3 = c3->getNumTrueGroundings(domains[0], domains[0]->getDB(), false);
      //cout << "True Groundings" << endl;
      double ng3 = c3->getNumGroundings(domains[0]);
      double prob3 = ntg3 / ng3;
      double u_ntg3 = u3->getNumTrueGroundings(domains[0], domains[0]->getDB(), false);
      double u_ng3 = u3->getNumGroundings(domains[0]);
      double up3 = u_ntg3 / u_ng3;
      tmp1 = (1 - prob3);
      tmp2 = 1- up3;
      prob3 = 1 - (tmp1 * tmp2);

      //cout << "Total Groundings" << endl;
      //int id3 = toEvaluate->getPredicate(0)->getId();
      //cout << "Got Id" << endl;
      //cout << "Here 1: Id = " << id3 << endl;

      //if (toEvaluate->getPredicate(0)->getSense()){

      /*
	}
	else{
	tmp1 = (1 - prob3);
	tmp2 = 1- (((*predPoss)[id3] - (*predTrue)[id3]) / (*predPoss)[id3]);
	prob3= 1 - (tmp1 * tmp2);
	}
      */
      cout << "Clause " << ix << " Decomposition " << 3 << endl;
      c3->printWithoutWt(cout, domains[0]);
      cout << " " << tmp1 << " ";
      toEvaluate->getPredicate(0)->print(cout, domains[0]);
      cout << " " << tmp2 << endl;

      /*
	if (toEvaluate->getPredicate(0)->getSense()){
	prob3 = prob3 * ((*predTrue)[id3] / (*predPoss)[id3]);
	}
	else{
	prob3 = prob3 * (( (*predPoss)[id3] -  (*predTrue)[id3]) / (*predPoss)[id3]);
	}
	cout << "Here done" << endl;
      */
      //delete c1;
      //delete c2;
      //delete c3;
      //cout << "done deleting " << endl;
      //toEvaluate->printWithoutWt(cout, domains[0]);


      //<< << endl;
      //double sigma = sqrt( 0.125 * 0.875 * dPoss);
      //double z1 = ((numTrueGroundings - mean) - .5)/sigma;
      //double z2 = ((numTrueGroundings - mean) + .5)/sigma;
      //cout << "Normal approx " << z1 << " " << z2 << endl;
      //unsigned int ntg = (unsigned int) numTrueGroundings;
      //unsigned int uPoss = (unsigned int)numGroundings;
      double thisProb = numTrueGroundings / numGroundings;
      double sqrtPoss = sqrt(numGroundings);

      double d1 = sqrt(prob1 * (1 - prob1));
      double d2 = sqrt(prob2 * (1 - prob2));
      double d3 = sqrt(prob3 * (1 - prob3));

      double v1 = (sqrtPoss * (thisProb - prob1)) / d1;
      double v2 = (sqrtPoss * (thisProb - prob2)) / d2;
      double v3 = (sqrtPoss * (thisProb - prob3)) / d3;
      if (v1 < 0){
	v1 = 0 - v1;
      }
      if (v2 < 0){
	v2 = 0 - v2;
      }
      if (v3 < 0){
	v3 = 0 - v3;
      }
      double pValue1 = 0;//2 * (1 - gsl_cdf_gaussian_P(v1 ,1));//gsl_cdf_binomial_P(ntg,prob1,uPoss);
      double pValue2 = 0;//2 * (1 - gsl_cdf_gaussian_P(v2 ,1)); //gsl_cdf_binomial_P(ntg,prob2,uPoss);
      double pValue3 = 0;//2 * (1 - gsl_cdf_gaussian_P(v3 ,1));  //gsl_cdf_binomial_P(ntg,prob3,uPoss);
      double nFact = numGroundings * log(numGroundings) - numGroundings + log(sqrt(2 * M_PI * numGroundings));
      double kFact = numTrueGroundings * log(numTrueGroundings) - numTrueGroundings + log(sqrt(2 * M_PI * numTrueGroundings));
      double diff = (numGroundings - numTrueGroundings);
      double dFact = diff * log(diff) - diff + log(sqrt(2 * M_PI * diff));
      double constFact = nFact - kFact -dFact;
      double diff1 = numTrueGroundings * log(prob1) + diff * log(1 - prob1);
      double diff2 = numTrueGroundings * log(prob2) + diff * log(1 - prob2);
      double diff3 = numTrueGroundings * log(prob3) + diff * log(1 - prob3);

      double kl1 = thisProb * log(thisProb/prob1) + (1-thisProb) * log((1-thisProb)/(1-prob1));
      double kl2 = thisProb * log(thisProb/prob2) + (1-thisProb) * log((1-thisProb)/(1-prob2));
      double kl3 = thisProb * log(thisProb/prob3) + (1-thisProb) * log((1-thisProb)/(1-prob3));

      double avgKL = (kl1 + kl2 + kl3) / 3;

      double val1 = nFact - kFact - dFact + numTrueGroundings * log(prob1) + diff * log(1 - prob1);
      double val2 = nFact - kFact - dFact + numTrueGroundings * log(prob2) + diff * log(1 - prob2);
      double val3 = nFact - kFact - dFact + numTrueGroundings * log(prob3) + diff * log(1 - prob3);
      double avgVal = (val1 + val2 + val3) / 3;
      //double minVal = min(min(val1,val2),val3);
      /*
	if (pValue1 > .5){
	pValue1 = 1 - pValue1;
	}
	if (pValue2 > .5){
	pValue2 = 1 - pValue2;
	}
	if (pValue3 > .5){
	pValue3 = 1 - pValue3;
	}
      */
      double avg = (pValue1 + pValue2 + pValue3) / 3;
      cout << nFact << " " << kFact << " " << dFact << endl;
      cout << "True " << numTrueGroundings << " Possible " << numGroundings << " Difference " << diff << endl; //ntg << " " << uPoss << endl;
      cout << "const    = " << (unsigned int) constFact << endl;
      cout << "Decomp 1 = " << (unsigned int) diff1 << endl;
      cout << "Decomp 2 = " << (unsigned int) diff2 << endl;
      cout << "Decomp 3 = " << (unsigned int) diff3 << endl;

      cout << "Probs: " << thisProb << " ";
      
      cout << prob1 << " ";
      //c2->printWithoutWt(cout, domains[0]);
      cout << prob2 << " ";
      //c3->printWithoutWt(cout, domains[0]);
      cout << prob3 << endl;
      cout << "Init p-values " << pValue1 << " " << pValue2 << " " << pValue3 << endl;
      cout << "Average Clause " << ix << ": " << avg << " " << pValue1 << " " << pValue2 << " " << pValue3 << endl;
      cout << "Log Average " << ix << ": " << avgVal << " " << val1 << " " << val2 << " " << val3 << endl;
      double pDiff1 = fabs(thisProb - prob1);
      double pDiff2 = fabs(thisProb - prob2);
      double pDiff3 = fabs(thisProb - prob3);
      double avgPDiff = (pDiff1 + pDiff2 + pDiff3) / 3;
      double minDiff = min(min(pDiff1,pDiff2),pDiff3);
      cout << "Average diff " << avgPDiff << " " << minDiff << " " << pDiff1 << " " << pDiff2 << " " << pDiff3 << endl;
      double minKL = min(min(kl1,kl2),kl3);
      cout << "Average KL " << avgKL << " " <<  minKL << " " <<  kl1 << " " << kl2 << " " << kl3<< endl << endl;
      //pred = pred - 100;
      //double checkItP = gsl_cdf_binomial_P(pred, 0.125,poss);
      //double pValue = gsl_cdf_binomial_P(ntg,0.125,poss);
      //double pValue1 = gsl_cdf_binomial_Q(ntg,0.125,poss);
      //cout << "Check " << checkItP << " Pred " << pred << endl;
      //cout << "Init p value " << pValue << " " << pValue1 << endl;
      //if (pValue > 0.5){
      //	pValue = (1 - pValue);
      //}
      //pValue = 2 * pValue;
      //bool delClause = (pValue > 0.05);
      //cout << ntg << " " <<  numTrueGroundings << " possible groudings " << poss << " " <<  dPoss << " num preds " << numPreds << " total " << ix << " pValue " << pValue << " " << delClause << endl;
      /*
	if (!delClause){
	ix++;
	}
	else{
	//deleteClause;
	cout << "removing clause " << ix << endl;
	mlns[0]->removeClause(ix);
	}
      */
     
    }
  }
  cout << "Total time = "; 
  Timer::printTime(cout, timer.time() - startSec); cout << endl;

  exit(-1);
    // Discriminative learning
  if (discLearn) 
  {
    wts.growToSize(numClausesFormulas + 1);
    double* wwts = (double*) wts.getItems();
    wwts++;
      // Non-evid preds as a string
    string nePredsStr = nonEvidPredsStr;

      // Set SampleSat parameters
    SampleSatParams* ssparams = new SampleSatParams;
    ssparams->lateSa = assLateSa;
    ssparams->saRatio = assSaRatio;
    ssparams->saTemp = assSaTemp;

      // Set MaxWalksat parameters
    MaxWalksatParams* mwsparams = NULL;
    mwsparams = new MaxWalksatParams;
    mwsparams->ssParams = ssparams;
    mwsparams->maxSteps = amwsMaxSteps;
    mwsparams->maxTries = amwsTries;
    mwsparams->targetCost = amwsTargetWt;
    mwsparams->hard = amwsHard;
      // numSolutions only applies when used in SampleSat.
      // When just MWS, this is set to 1
    mwsparams->numSolutions = amwsNumSolutions;
    mwsparams->heuristic = amwsHeuristic;
    mwsparams->tabuLength = amwsTabuLength;
    mwsparams->lazyLowState = amwsLazyLowState;

      // Set MC-SAT parameters
    MCSatParams* msparams = new MCSatParams;
    msparams->mwsParams = mwsparams;
      // MC-SAT needs only one chain
    msparams->numChains          = 1;
    msparams->burnMinSteps       = amcmcBurnMinSteps;
    msparams->burnMaxSteps       = amcmcBurnMaxSteps;
    msparams->minSteps           = amcmcMinSteps;
    msparams->maxSteps           = amcmcMaxSteps;
    msparams->maxSeconds         = amcmcMaxSeconds;

      // Set Gibbs parameters
    GibbsParams* gibbsparams = new GibbsParams;
    gibbsparams->mwsParams    = mwsparams;
    gibbsparams->numChains    = amcmcNumChains;
    gibbsparams->burnMinSteps = amcmcBurnMinSteps;
    gibbsparams->burnMaxSteps = amcmcBurnMaxSteps;
    gibbsparams->minSteps     = amcmcMinSteps;
    gibbsparams->maxSteps     = amcmcMaxSteps;
    gibbsparams->maxSeconds   = amcmcMaxSeconds;

    gibbsparams->gamma          = 1 - agibbsDelta;
    gibbsparams->epsilonError   = agibbsEpsilonError;
    gibbsparams->fracConverged  = agibbsFracConverged;
    gibbsparams->walksatType    = agibbsWalksatType;
    gibbsparams->samplesPerTest = agibbsSamplesPerTest;
  
      // Set Sim. Tempering parameters
    SimulatedTemperingParams* stparams = new SimulatedTemperingParams;
    stparams->mwsParams    = mwsparams;
    stparams->numChains    = amcmcNumChains;
    stparams->burnMinSteps = amcmcBurnMinSteps;
    stparams->burnMaxSteps = amcmcBurnMaxSteps;
    stparams->minSteps     = amcmcMinSteps;
    stparams->maxSteps     = amcmcMaxSteps;
    stparams->maxSeconds   = amcmcMaxSeconds;

    stparams->subInterval = asimtpSubInterval;
    stparams->numST       = asimtpNumST;
    stparams->numSwap     = asimtpNumSwap;

    Array<VariableState*> states;
    Array<Inference*> inferences;

    states.growToSize(domains.size());
    inferences.growToSize(domains.size());

      // Build the state for inference in each domain
    Array<int> allPredGndingsAreNonEvid;
    Array<Predicate*> ppreds;
    
    for (int i = 0; i < domains.size(); i++)
    {
      Domain* domain = domains[i];
      MLN* mln = mlns[i];
        // Domains have been built: If user doesn't provide number of MC-SAT
        // steps, then use 10,000 / (min # of gndings of any clause), but not
        // less than 5. This is to insure 10,000 samples of all clauses
      if (amcmcMaxSteps <= 0)
      {
        int minSize = INT_MAX;
        
        for (int c = 0; c < mln->getNumClauses(); c++)
        {
          Clause* clause = (Clause*)mln->getClause(c);
          double size = clause->getNumGroundings(domain);
          if (size < minSize) minSize = (int)size;
        }
        int steps = 10000 / minSize;
        if (steps < 5) steps = 5;
        cout << "Setting number of MCMC steps to " << steps << endl;
        amcmcMaxSteps = steps;
        msparams->maxSteps = amcmcMaxSteps;
        gibbsparams->maxSteps = amcmcMaxSteps;
        stparams->maxSteps = amcmcMaxSteps;        
      }

        // Remove evidence atoms structure from DBs
      if (!aLazy)
        domains[i]->getDB()->setLazyFlag(false);
    
        // Unknown non-ev. preds
      GroundPredicateHashArray* unePreds = NULL;

        // Known non-ev. preds
      GroundPredicateHashArray* knePreds = NULL;
      Array<TruthValue>* knePredValues = NULL;

        // Need to set some dummy weight
      for (int j = 0; j < mln->getNumClauses(); j++)
        ((Clause*) mln->getClause(j))->setWt(1);

		// Make open-world evidence preds into non-evidence
      if (!allPredsExceptQueriesAreCW)
      {
        for (int i = 0; i < owPredNames.size(); i++)
        {
          nePredsStr.append(",");
          nePredsStr.append(owPredNames[i]);
          nonEvidPredNames.append(owPredNames[i]);
        }
      }

      Array<Predicate*> gpreds;
      Array<TruthValue> gpredValues;
        // Eager version: Build query preds from command line and set known
        // non-evidence to unknown for building the states
      if (!aLazy)
      {
        unePreds = new GroundPredicateHashArray;
        knePreds = new GroundPredicateHashArray;
        knePredValues = new Array<TruthValue>;

        allPredGndingsAreNonEvid.growToSize(domain->getNumPredicates(), false);
          //defined in infer.h
        createComLineQueryPreds(nePredsStr, domain, domain->getDB(), 
                                unePreds, knePreds, 
                                &allPredGndingsAreNonEvid);

          // Pred values not set to unknown in DB: unePreds contains
          // unknown, knePreds contains known non-evidence

          // Set known NE to unknown for building state
          // and set blockEvidence to false if this was the true evidence
        knePredValues->growToSize(knePreds->size(), FALSE);
        for (int predno = 0; predno < knePreds->size(); predno++)
        {
            // If this was the true evidence in block, then erase this info
          int blockIdx = domain->getBlock((*knePreds)[predno]);
          if (blockIdx > -1 &&
              domain->getDB()->getValue((*knePreds)[predno]) == TRUE)
          {
            domain->setBlockEvidence(blockIdx, false);
          }
            // Set value to unknown
          (*knePredValues)[predno] =
            domain->getDB()->setValue((*knePreds)[predno], UNKNOWN);
        }

          // If first order query pred groundings are allowed to be evidence
          // - we assume all the predicates not in db to be false
          // evidence - need a better way to code this.
        if (isQueryEvidence)
        {
            // Set unknown NE to false
          for (int predno = 0; predno < unePreds->size(); predno++)
          {
            domain->getDB()->setValue((*unePreds)[predno], FALSE);
            delete (*unePreds)[predno];
          }
          unePreds->clear();
        }
      }
      else
      {
        Array<Predicate*> ppreds;

        domain->getDB()->setPerformingInference(false);

        gpreds.clear();
        gpredValues.clear();
        for (int predno = 0; predno < nonEvidPredNames.size(); predno++) 
        {
          ppreds.clear();
          int predid = domain->getPredicateId(nonEvidPredNames[predno].c_str());
          Predicate::createAllGroundings(predid, domain, ppreds);
          gpreds.append(ppreds);
        }
        //domain->getDB()->alterTruthValue(&gpreds, UNKNOWN, FALSE, &gpredValues);
        domain->getDB()->setValuesToUnknown(&gpreds, &gpredValues);
      }
      
        // Create state for inferred counts using unknown and known (set to
        // unknown in the DB) non-evidence preds
      cout << endl << "constructing state for domain " << i << "..." << endl;
      bool markHardGndClauses = false;
      bool trackParentClauseWts = true;

      VariableState*& state = states[i];
      state = new VariableState(unePreds, knePreds, knePredValues,
                                &allPredGndingsAreNonEvid, markHardGndClauses,
                                trackParentClauseWts, mln, domain, aLazy);

      Inference*& inference = inferences[i];
      bool trackClauseTrueCnts = true;
        // Different inference algorithms
      if (amapPos || amapAll)
      { // MaxWalkSat
          // When standalone MWS, numSolutions is always 1
          // (maybe there is a better way to do this?)
        mwsparams->numSolutions = 1;
        inference = new MaxWalkSat(state, aSeed, trackClauseTrueCnts,
                                   mwsparams);
      }
      else if (amcsatInfer)
      { // MC-SAT
        inference = new MCSAT(state, aSeed, trackClauseTrueCnts, msparams);
      }
      else if (asimtpInfer)
      { // Simulated Tempering
          // When MWS is used in Sim. Temp., numSolutions is always 1
          // (maybe there is a better way to do this?)
        mwsparams->numSolutions = 1;
        inference = new SimulatedTempering(state, aSeed, trackClauseTrueCnts,
                                           stparams);
      }
      else if (agibbsInfer)
      { // Gibbs sampling
          // When MWS is used in Gibbs, numSolutions is always 1
          // (maybe there is a better way to do this?)
        mwsparams->numSolutions = 1;
        inference = new GibbsSampler(state, aSeed, trackClauseTrueCnts,
                                     gibbsparams);
      }

      if (!aLazy)
      {
          // Change known NE to original values
        domain->getDB()->setValuesToGivenValues(knePreds, knePredValues);
          // Set unknown NE to false for weight initialization. This seems to
          // give poor results when using EM. We need to leave these
          // as unknown and do the counts accordingly
        for (int predno = 0; predno < unePreds->size(); predno++)
        {
          domain->getDB()->setValue((*unePreds)[predno], FALSE);
        }
      }
      else
      {
        domain->getDB()->setValuesToGivenValues(&gpreds, &gpredValues);
      
        //cout << "the ground predicates are :" << endl;
        for (int predno = 0; predno < gpreds.size(); predno++) 
        {
          //gpreds[predno]->printWithStrVar(cout, domain);
          //cout << endl;
          delete gpreds[predno];
        }

        domain->getDB()->setPerformingInference(true);
      }
    }
    cout << endl << "done constructing variable states" << endl << endl;
    
    if (useVP)
      discMethod = DiscriminativeLearner::SIMPLE;
    else if (useNewton)
      discMethod = DiscriminativeLearner::DN;
    else
      discMethod = DiscriminativeLearner::CG;

    DiscriminativeLearner dl(inferences, nonEvidPredNames, indexTrans, aLazy,
                             withEM, !noUsePerWeight, discMethod, cg_lambda,
                             !cg_noprecond, cg_max_lambda);

    if (!noPrior) 
      dl.setMeansStdDevs(numClausesFormulas, priorMeans.getItems(),
                         priorStdDevs.getItems());
    else
      dl.setMeansStdDevs(-1, NULL, NULL);
	 
    begSec = timer.time();
    cout << "learning (discriminative) weights .. " << endl;
    double maxTime = maxSec + 60*maxMin + 3600*maxHour;
    dl.learnWeights(wwts, wts.size()-1, numIter, maxTime, learningRate, 
                    momentum, initWithLogOdds, amwsMaxSubsequentSteps,
                    aPeriodicMLNs);
    cout << endl << endl << "Done learning discriminative weights. "<< endl;
    cout << "Time Taken for learning = ";
    Timer::printTime(cout, (timer.time() - begSec)); cout << endl;

    if (mwsparams) delete mwsparams;
    if (ssparams) delete ssparams;
    if (msparams) delete msparams;
    if (gibbsparams) delete gibbsparams;
    if (stparams) delete stparams;
    for (int i = 0; i < inferences.size(); i++)  delete inferences[i];
    for (int i = 0; i < states.size(); i++)  delete states[i];
  } 
  else
  {   
    ////////////// using generative learning

    Array<bool> areNonEvidPreds;
    if (nonEvidPredNames.empty())
    {
      areNonEvidPreds.growToSize(domains[0]->getNumPredicates(), true);
      for (int i = 0; i < domains[0]->getNumPredicates(); i++)
      {
          //prevent equal pred from being non-evidence preds
        if (domains[0]->getPredicateTemplate(i)->isEqualPred())
        {
          const char* pname = domains[0]->getPredicateTemplate(i)->getName();
          int predId = domains[0]->getPredicateId(pname);
          areNonEvidPreds[predId] = false;
        }
          //prevent internal preds from being non-evidence preds
        if (domains[0]->getPredicateTemplate(i)->isInternalPredicateTemplate())
        {
          const char* pname = domains[0]->getPredicateTemplate(i)->getName();
          int predId = domains[0]->getPredicateId(pname);
          areNonEvidPreds[predId] = false;
        }
      }
    } 
    else
    {
      areNonEvidPreds.growToSize(domains[0]->getNumPredicates(), false);
      for (int i = 0; i < nonEvidPredNames.size(); i++)
      {
        int predId = domains[0]->getPredicateId(nonEvidPredNames[i].c_str());
        if (predId < 0)
        {
          cout << "ERROR: Predicate " << nonEvidPredNames[i] << " undefined." 
               << endl;
          exit(-1);
        }
        areNonEvidPreds[predId] = true;
      }
    }

    Array<bool>* nePreds = &areNonEvidPreds;
    PseudoLogLikelihood pll(nePreds, &domains, !noEqualPredWt, false,-1,-1,-1);
    pll.setIndexTranslator(indexTrans);

    if (!noPrior) 
      pll.setMeansStdDevs(numClausesFormulas, priorMeans.getItems(),
                          priorStdDevs.getItems());
    else          
      pll.setMeansStdDevs(-1, NULL, NULL);
    
    ////////////// compute the counts for the clauses

    begSec = timer.time();
    for (int m = 0; m < mlns.size(); m++)
    {
      cout << "Computing counts for clauses in domain " << m << "..." << endl;
      const ClauseHashArray* clauses = mlns[m]->getClauses();
      for (int i = 0; i < clauses->size(); i++)
      {
        if (PRINT_CLAUSE_DURING_COUNT)
        {
          cout << "clause " << i << ": ";
          (*clauses)[i]->printWithoutWt(cout, domains[m]);
          cout << endl; cout.flush();
        }
        MLNClauseInfo* ci = (MLNClauseInfo*) mlns[m]->getMLNClauseInfo(i);
        pll.computeCountsForNewAppendedClause((*clauses)[i], &(ci->index), m, 
                                              NULL, false, NULL);
      }
    }
    pll.compress();
    cout <<"Computing counts took ";
    Timer::printTime(cout, timer.time() - begSec); cout << endl;
    
    ////////////// learn clause wts

      // initialize the clause weights
    wts.growToSize(numClausesFormulas + 1);
    for (int i = 0; i < numClausesFormulas; i++) wts[i+1] = 0;
    //wts[i+1] = priorMeans[i];

      // optimize the clause weights
    cout << "L-BFGS-B is finding optimal weights......" << endl;
    begSec = timer.time();
    LBFGSB lbfgsb(maxIter, convThresh, &pll, numClausesFormulas);
    int iter;
    bool error;
    double pllValue = lbfgsb.minimize((double*)wts.getItems(), iter, error);
    
    if (error) cout << "LBFGSB returned with an error!" << endl;
    cout << "num iterations        = " << iter << endl;
    cout << "time taken            = ";
    Timer::printTime(cout, timer.time() - begSec);
    cout << endl;
    cout << "pseudo-log-likelihood = " << -pllValue << endl;

  } // else using generative learning

  //////////////////////////// output results ////////////////////////////////
  if (indexTrans) assignWtsAndOutputMLN(out, mlns, domains, wts, indexTrans);
  else            assignWtsAndOutputMLN(out, mlns, domains, wts);

  out.close();

  /////////////////////////////// clean up ///////////////////////////////////
  deleteDomains(domains);

  for (int i = 0; i < mlns.size(); i++)
  {
    if (DOMAINS_SHARE_DATA_STRUCT && i > 0)
    {
      mlns[i]->setClauses(NULL);
      mlns[i]->setMLNClauseInfos(NULL);
      mlns[i]->setPredIdToClausesMap(NULL);
      mlns[i]->setFormulaAndClausesArray(NULL);
      mlns[i]->setExternalClause(NULL);
    }
    delete mlns[i];
  }

  PowerSet::deletePowerSet();
  if (indexTrans) delete indexTrans;

  cout << "Total time = "; 
  Timer::printTime(cout, timer.time() - startSec); cout << endl;
}
