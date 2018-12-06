#ifndef UTSTRUCTLEARN_H_APR_12_2005
#define UTSTRUCTLEARN_H_APR_12_2005

#include <string.h>
#include <stdlib.h>
#include <time.h> //for seeding the random num generator
#include <set>
#include <map>
#include <math.h>
#include "clausefactory.h"
#include "mln.h"
#include "pseudologlikelihood.h"
#include "lbfgsb.h"
//#include "ut-mrf.h"
#include "structlearn.h"
#include "feature.h"
#include "featureArray.h"
#include "markovBlanket.h"

class ExistFormula;

class UtStructLearn {
public:
	//If there are more than one domain, there must be a one-to-one 
	//correspondence among the clauses in mlns.
	UtStructLearn(Array<MLN*>* const & mlns, const bool& startFromEmptyMLN,
			const string& outMLNFileName, Array<Domain*>* const & domains,
			const Array<string>* const & nonEvidPredNames, const int& maxVars,
			const int& maxNumPredicates, const bool& cacheClauses,
			const double& maxCacheSizeMB, const bool& tryAllFlips,
			const bool& sampleClauses, const double& delta,
			const double& epsilon, const int& minClauseSamples,
			const int& maxClauseSamples, const bool& hasPrior,
			const double& priorMean, const double& priorStdDev,
			const bool& wtPredsEqually, const int& lbMaxIter,
			const double& lbConvThresh, const int& looseMaxIter,
			const double& looseConvThresh, const double& minWt,
			const double& penalty, const bool& sampleGndPreds,
			const double& fraction, const int& minGndPredSamples,
			const int& maxGndPredSamples) :
		mln0_((*mlns)[0]), mlns_(mlns), startFromEmptyMLN_(startFromEmptyMLN),
				outMLNFileName_(outMLNFileName), domains_(domains),
				preds_(new Array<Predicate*>),
				areNonEvidPreds_(new Array<bool>),
				clauseFactory_(new ClauseFactory(maxVars, maxNumPredicates, (*domains_)[0])), cacheClauses_(cacheClauses),
				origCacheClauses_(cacheClauses),
				cachedClauses_((cacheClauses) ? (new ClauseHashArray) : NULL),
				cacheSizeMB_(0), maxCacheSizeMB_(maxCacheSizeMB),
				tryAllFlips_(tryAllFlips), sampleClauses_(sampleClauses),
				origSampleClauses_(sampleClauses), pll_(NULL),
				hasPrior_(hasPrior), priorMean_(priorMean),
				priorStdDev_(priorStdDev), wtPredsEqually_(wtPredsEqually),
				lbfgsb_(NULL), lbMaxIter_(lbMaxIter),
				lbConvThresh_(lbConvThresh), looseMaxIter_(looseMaxIter),
				looseConvThresh_(looseConvThresh), minGain_(0), minWt_(minWt),
				penalty_(penalty), sampleGndPreds_(sampleGndPreds),
				fraction_(fraction), minGndPredSamples_(minGndPredSamples),
				maxGndPredSamples_(maxGndPredSamples), candCnt_(0), iter_(-1),
				startSec_(-1), indexTrans_(NULL)
	{
	  assert(minWt_ >= 0);
	  assert(domains_->size() == mlns_->size());
	  seen24hr = false;
	  seen48hr = false;
	  srand(time(NULL));
	  jMaxVars = maxVars;
	  jMaxPreds = maxNumPredicates;
		areNonEvidPreds_->growToSize((*domains_)[0]->getNumPredicates(), false);
		for (int i = 0; i < nonEvidPredNames->size(); i++) {
			int predId=(*domains_)[0]->getPredicateId((*nonEvidPredNames)[i].c_str());
			if (predId < 0) {
				cout << "ERROR: in UtStructLearn::UtStructLearn(). Predicate "
						<< (*nonEvidPredNames)[i] << " undefined." << endl;
				exit(-1);
			}
			(*areNonEvidPreds_)[predId] = true;
		}

		(*domains_)[0]->createPredicates(preds_, true);

		if (origSampleClauses_) {
			ClauseSampler* cs = new ClauseSampler(delta, epsilon, minClauseSamples,
					maxClauseSamples);
			Clause::setClauseSampler(cs);
			for (int i = 0; i < domains_->size(); i++)
				(*domains_)[i]->newTrueFalseGroundingsStore();
		}
	}

	~UtStructLearn()
	{
		if (pll_)
			delete pll_;
		if (lbfgsb_)
			delete lbfgsb_;
		preds_->deleteItemsAndClear();
		delete preds_;
		delete areNonEvidPreds_;
		delete clauseFactory_;
		if (cachedClauses_) {
			cachedClauses_->deleteItemsAndClear();
			delete cachedClauses_;
		}
		if (origSampleClauses_)
			delete Clause::getClauseSampler();
		if (indexTrans_)
			delete indexTrans_;
	}

	bool learnAndSetMLNWeights(double& score)
	{
		Array<double> priorMeans, priorStdDevs;
		double tmpScore = score;
		pllSetPriorMeansStdDevs(priorMeans, priorStdDevs, 0, NULL);
		int numClausesFormulas = getNumClausesFormulas();
		Array<double> wts;
		wts.growToSize(numClausesFormulas + 1);
		//indexTrans_ is already created
		int iter;
		bool error;
		double elapsedSec;
		tmpScore = maximizeScore(numClausesFormulas, 0, &wts, NULL, NULL, iter,
				error, elapsedSec);

	/*	cout <<"****The weights learned at this point are :\n";
		for (int i = 0; i < wts.size(); i++)
			cout << wts[i] << " ";
		cout << endl;*/

		if (error) {
		  cout << "LBFGSB failed to find wts" << endl;
		  return false;
		} 
		else {
		  score = tmpScore;
		  //printNewScore((Clause*)NULL, NULL, iter, elapsedSec, score, 0, 0);
		}

		updateWts(wts, NULL, NULL);

		return true;
	}

	//the arrays need to be allocated, but caller is responsible for their contents
	void preparePredGndingStorage(Domain * currDomain, int numPreds,
			Array<PredicateHashArray*>* truePredicateGndingsArray,
			Array<PredicateHashArray*>* falsePredicateGndingsArray)
	{

		for (int pred = 0; pred < numPreds; pred++) {
			const char * nameOfCurrPred = currDomain->getPredicateName(pred);
			if (strstr(nameOfCurrPred, "SK")) {
				PredicateHashArray * dummy= NULL;
				truePredicateGndingsArray->append(dummy);
				falsePredicateGndingsArray->append(dummy);
				continue; //equals predicates
			}
			PredicateHashArray * truePredicateGroundings =
					new PredicateHashArray;
			PredicateHashArray * falsePredicateGroundings =
					new PredicateHashArray;

			splitAllPredGndings(pred, currDomain, truePredicateGroundings,
					falsePredicateGroundings);

			truePredicateGndingsArray->append(truePredicateGroundings);
			falsePredicateGndingsArray->append(falsePredicateGroundings);
		}
	}

	void prepareConstToPredicateMap(Domain * currDomain, int numPreds,
			Array<PredicateHashArray*>* truePredicateGndingsArray,
			Array<PredicateHashArray*>* falsePredicateGndingsArray,
			map<int, PredicateHashArray*>* constIdToPred)
	{
		for (int pred = 0; pred < numPreds; pred++) {
			const char * nameOfCurrPred = currDomain->getPredicateName(pred);
			if (strstr(nameOfCurrPred, "SK"))
				continue; //equals predicates

			PredicateHashArray *currPredTrueGndings = (*truePredicateGndingsArray)[pred];
			assert(currPredTrueGndings != NULL);
			for (int g = 0; g < currPredTrueGndings->size(); g++) {
				Predicate * currGnding = (*currPredTrueGndings)[g];
				for (int t = 0; t < currGnding->getNumTerms(); t++) {

					int termId = currGnding->getTerm(t)->getId();
					assert(termId >= 0);

					if (constIdToPred->find(termId) == constIdToPred->end()) {
						constIdToPred->insert(make_pair(termId,
								new PredicateHashArray));
					}

					(*constIdToPred)[termId]->append(currGnding);
				}
			}
		}
	}

	void prepareRelPaths(Array<PredicateHashArray*>* truePredicateGndingsArray,
			Array<PredicateHashArray*>* falsePredicateGndingsArray,
			map<int, PredicateHashArray*>* constIdToPred,
			PredicateToIntMap * relPathIdxForPred,
			Array<Array<RelPath*>*>* relPaths)
	{

		for (int a = 0; a < truePredicateGndingsArray->size(); a++) {
			if ((*truePredicateGndingsArray)[a] == NULL)
				continue;
			PredicateHashArray *truePredicateGroundings = (*truePredicateGndingsArray)[a];
			for (int i = 0; i < truePredicateGroundings->size(); i++) {
				Predicate* currGnding = (*truePredicateGroundings)[i];
				if (currGnding->getNumTerms() < 2)
					continue;

				assert((*relPathIdxForPred).find(currGnding) == (*relPathIdxForPred).end());

				Array<RelPath *> *currRelPaths = new Array<RelPath *>;
				int index = relPaths->append(currRelPaths);
				(*relPathIdxForPred)[currGnding] = index;
				assert((*relPathIdxForPred)[currGnding] == index);

				for (int t = 0; t < currGnding->getNumTerms(); t++) {
					int currTermId = currGnding->getTerm(t)->getId();
					PredicateHashArray *constSharingPreds = (*constIdToPred)[currTermId];

					for (int p = 0; p < constSharingPreds->size(); p++) {
						Predicate *currCSPred = (*constSharingPreds)[p];
						//commented out: this is an artificial limit and will remove it
						//if (currCSPred->getNumTerms() < 2) continue;
						if (currGnding->same(currCSPred))
							continue;
						Array<int>* sharedIndices = new Array<int>;
						sharedIndices->growToSize(currGnding->getNumTerms(), -1);
						int numShared = 0;
						for (int j = 0; j < currGnding->getNumTerms(); j++)
							for (int k = 0; k < currCSPred->getNumTerms(); k++)
								if (currGnding->getTerm(j)->getId() == currCSPred->getTerm(k)->getId()) {
									(*sharedIndices)[j] = k;
									numShared++;
								}
						assert(numShared >= 1);
						RelPath *newRelPath = new RelPath(currCSPred, sharedIndices, numShared);
						bool found = false;
						for (int j = 0; j < currRelPaths->size() && !found; j++)
							found = (*currRelPaths)[j]->same(newRelPath);
						if (!found)
							currRelPaths->append(newRelPath);
						else
							delete newRelPath;
					}//for each gnd pred sharing a value with this term 
				} //for each term
			} //for each true pred gnding
		}//for each predicate
	}

private:

	void getTruePredGndings(int predId, Domain * currDomain,
			PredicateHashArray * trueGroundings)
	{
		Array <Predicate *> allGndings;
		Predicate::createAllGroundings(predId, currDomain, allGndings);

		for (int g = 0; g < allGndings.size(); g++) {
			TruthValue tv = currDomain->getDB()->getValue(allGndings[g]);
			if (tv == TRUE) {
				if (trueGroundings->append(allGndings[g]) < 0)
					delete allGndings[g];
			} else if (tv == FALSE)
				delete allGndings[g];
			else
				//tv must be either true or false
				assert(false);
		}
	}

	//caller is responsible for deleting the contents of the
	//trueGroundigs and falseGroundigns arrays
	void splitAllPredGndings(int predId, Domain * currDomain,
			PredicateHashArray * trueGroundings,
			PredicateHashArray * falseGroundings)
	{

		Array <Predicate *> allGndings;
		Predicate::createAllGroundings(predId, currDomain, allGndings);

		for (int g = 0; g < allGndings.size(); g++) {
			TruthValue tv = currDomain->getDB()->getValue(allGndings[g]);
			if (tv == TRUE) {
				if (trueGroundings->append(allGndings[g]) < 0)
					delete allGndings[g];
			} else if (tv == FALSE) {
				if (falseGroundings->append(allGndings[g]) < 0)
					delete allGndings[g];
			} else
				assert(false);
		}
	}
public:
	void busl(int countIsAtLeast, double indepThreshold, int maxNumFreeVars, double minimumTrueProportion)
	{
	  cout << "Running BUSL.." << endl;
	  cout << "BUSL currently only supports learning from scratch\n";
	  
	  int numCandidatesConsidered = 0;
	  
	  //0000000000000000 Doing the preparations: auxiliary data etc 00000000000000000//
	  startSec_ = timer_.time();
	  seen24hr = false;
	  seen48hr = false;
	  bool needIndexTrans =IndexTranslator::needIndexTranslator(*mlns_,
								    *domains_);
	  
	  //add unit clauses to the MLN
	  cout << "adding unit clauses to MLN..." << endl << endl;
	  addUnitClausesToMLNs();
	  
	  //create auxiliary data for each clause in MLN
	  for (int i = 0; i < mln0_->getNumClauses(); i++) {
	    Clause* c = (Clause*) mln0_->getClause(i);
	    c->newAuxClauseData();
	    //only clauses that are not in any exist. quant. formula's CNF may need 
	    //to be translated across domains
	    if (isModifiableClause(i))
	      c->trackConstants();
	  }
	  
	  //create PseudoLogLikelihood and LBFGSB
	  indexTrans_ = (needIndexTrans) ? new IndexTranslator(mlns_, domains_) : NULL;
	  if (indexTrans_)
	    cout
	      << "The weights of clauses in the CNFs of existential formulas wiil "
	      << "be tied" << endl;
	  
	  pll_ = new PseudoLogLikelihood(areNonEvidPreds_, domains_, wtPredsEqually_, sampleGndPreds_, fraction_, minGndPredSamples_, maxGndPredSamples_);
	  pll_->setIndexTranslator(indexTrans_);
	  
	  int numClausesFormulas = getNumClausesFormulas();
	  lbfgsb_ = new LBFGSB(-1, -1, pll_, numClausesFormulas);
	  
	  useTightParams();

	  //compute initial counts for clauses in all MLNs
	  cout << "computing counts for initial MLN clauses..." << endl;
	  double begSec = timer_.time();
	  pllComputeCountsForInitialMLN();
	  cout << "computing counts for initial MLN clauses took ";
	  timer_.printTime(cout, timer_.time()-begSec);
	  cout << endl << endl;
	  
	  //learn initial wt/score of MLN, and set MLN clauses to learned weights
	  cout << "learning the initial weights and score of MLN..." << endl
	       << endl;
	  begSec = timer_.time();
	  double score;
	  if (!learnAndSetMLNWeights(score))
	    return;
	  //printMLNClausesWithWeightsAndScore(score, -1);
	  
	  cout << "learning the initial weights and score of MLN took ";
	  timer_.printTime(cout, timer_.time()-begSec);
	  cout << endl << endl;
	  
	  //add unit clauses with diff combinations of variables
	  cout
	    <<"trying to add unit clause with diff variable combinations to MLN..."
	    << endl << endl;
	  begSec = timer_.time();
	  appendUnitClausesWithDiffCombOfVar(score); // score is updated
	  //printMLNClausesWithWeightsAndScore(score, -1);
	  cout
	    <<"adding unit clause with diff variable combinations to MLN took ";
	  timer_.printTime(cout, timer_.time()-begSec);
	  cout << endl << endl;
	  
	  //=======This is where the new stuff starts===
	  int numPreds = (*domains_)[0]->getNumPredicates();
	  Array<Array<PredicateHashArray *>*> *allDomainsTruePredGndings =
	    new Array<Array<PredicateHashArray *>*>;
	  Array<Array<PredicateHashArray *>*> *allDomainsFalsePredGndings =
	    new Array<Array<PredicateHashArray *>*>;
	  // Array<Array<PredicateHashArray *>*>* allDomainsConstIdToPred = 
	  //   new Array<Array<PredicateHashArray*>*>;
	  Array<map<int, PredicateHashArray*>* >* allDomainsConstIdToPred =
	    new Array<map<int, PredicateHashArray*>* >;
	  Array<PredicateToIntMap *>* allDomainsRelPathIdxForPred =
	    new Array<PredicateToIntMap *>;
	  Array<Array<Array<RelPath*>*>*> * allDomainsRelPaths =
	    new Array<Array<Array<RelPath*>*>*>;
	  
	  for (int d = 0; d < domains_->size(); d++) {
	    Domain * currDomain = (*domains_)[d];
	    assert(currDomain->getNumPredicates() == numPreds);
	    //cout << "Working on domain " << d << endl;
	    //prepare storage for all predicate gndings
	    Array<PredicateHashArray *>* truePredicateGndingsArray;
	    Array<PredicateHashArray *>* falsePredicateGndingsArray;
	    truePredicateGndingsArray = new Array<PredicateHashArray *>;
	    falsePredicateGndingsArray = new Array<PredicateHashArray *>;
	    preparePredGndingStorage(currDomain, numPreds,
				     truePredicateGndingsArray, falsePredicateGndingsArray);
	    
	    allDomainsTruePredGndings->append(truePredicateGndingsArray);
	    allDomainsFalsePredGndings->append(falsePredicateGndingsArray);
	    
	    //prepare the constant to predicate map
	    //Array <PredicateHashArray *>* constIdToPred;
	    //constIdToPred = new Array<PredicateHashArray*>;
	    map<int, PredicateHashArray*>* constIdToPred =
	      new map<int, PredicateHashArray*>;
	    prepareConstToPredicateMap(currDomain, numPreds,
				       truePredicateGndingsArray, 
				       falsePredicateGndingsArray,
				       constIdToPred);
	    
	    allDomainsConstIdToPred->append(constIdToPred);

			//prepare the relpaths for true predicate gndings
	    PredicateToIntMap * relPathIdxForPred = new PredicateToIntMap;
	    Array<Array<RelPath*>*>* relPaths = new Array<Array<RelPath*>*>; //clean it up
	    prepareRelPaths(truePredicateGndingsArray,
			    falsePredicateGndingsArray, constIdToPred,
			    relPathIdxForPred, relPaths);
	    
	    allDomainsRelPathIdxForPred->append(relPathIdxForPred);
	    allDomainsRelPaths->append(relPaths);
	  }
	  
	  ClauseOpHashArray constructedClauses;
	  
	  for (int pred = 0; pred < numPreds; pred++) {
	    const char * nameOfCurrPred = (*domains_)[0]->getPredicateName(pred);
	    if (strstr(nameOfCurrPred, "SK"))
	      continue; //equals predicates
	    
			//modified this to make it discriminative -- not a good idea
			/* if (!(*areNonEvidPreds_)[pred]) {
			   cout << "Predicate " << nameOfCurrPred << " is evidence. Continuing...\n";
			   continue;
			   }*/
	    
	    cout << "Now doing predicate with id " << pred << " and name "
					<< nameOfCurrPred<< endl;
			//      double bsec1 = timer_.time();

	    FeatureArray* currentFeatureArray = new FeatureArray(maxNumFreeVars);
	    
	    for (int d = 0; d < domains_->size(); d++) {
	      //cout << "Updating the features for domain " << d << endl;
	      Domain * currDomain = (*domains_)[d];
	      //	double bsec2 = timer_.time();
	      
	      Array<PredicateHashArray *>* truePredicateGndingsArray = (*allDomainsTruePredGndings)[d];
	      Array<PredicateHashArray *>* falsePredicateGndingsArray = (*allDomainsFalsePredGndings)[d];
	      map<int, PredicateHashArray*>* constIdToPred = (*allDomainsConstIdToPred)[d];
	      PredicateToIntMap * relPathIdxForPred = (*allDomainsRelPathIdxForPred)[d];
	      Array<Array<RelPath*>*>* relPaths = (*allDomainsRelPaths)[d];
	      
	      PredicateHashArray * truePredGndings = (*truePredicateGndingsArray)[pred];
	      PredicateHashArray * falsePredGndings = (*falsePredicateGndingsArray)[pred];
	      
	      assert(truePredGndings != NULL && falsePredGndings != NULL);
	      /*
	      cout << "Num true gndings is " << truePredGndings->size()
		   << endl;
	      cout << "Num false gndings is " << falsePredGndings->size()
		   << endl;
	      */
	      double bsec3 = timer_.time();
	      for (int gnd = 0; gnd < truePredGndings->size(); gnd++) {
		currentFeatureArray->constructFeaturesForGnding((*truePredGndings)[gnd], currDomain, true, constIdToPred, *relPathIdxForPred, relPaths);
	      }
	      /*
	      cout << "--Constructing features for true gndings took ";
	      timer_.printTime(cout, timer_.time() - bsec3);
	      cout << endl;
	      */
	      double bsec4 = timer_.time();
	      for (int gnd = 0; gnd < falsePredGndings->size(); gnd++) {
		currentFeatureArray->constructFeaturesForGnding((*falsePredGndings)[gnd], currDomain, false, constIdToPred, *relPathIdxForPred, relPaths);
	      }
	      cout << "--Constructing features for false gndings took ";
	      timer_.printTime(cout, timer_.time() - bsec4);
	      cout << endl;
	      
	      //cout << "Printing the constructed features\n";
	      //currentFeatureArray->print(currDomain, false);
	    }
	    
	    if (currentFeatureArray->getTotalNumFeatures() == 0) {
	      delete currentFeatureArray;
	      continue;
	    }
	    
	    //done preparing the features. Next: grow-shrink
	    //MarkovBlanketsFinder *mbf = new MarkovBlanketsFinder(currentFeatureArray, 0.15, countIsAtLeast);
	    MarkovBlanketsFinder *mbf = 
	      new MarkovBlanketsFinder(currentFeatureArray, indepThreshold, countIsAtLeast);
	    mbf->growBlanketAndMakeClauses(constructedClauses, jMaxVars, jMaxPreds);
	    //cout << "Made markovBlanket and clause candidates from it\n";
	    /*
	    for(int jj = 0; jj < constructedClauses.size(); jj++){
	      constructedClauses[jj]->printWithoutWt(cout, (*domains_)[0]);
	      cout << endl;
	      //if constructedClauses[jj]->getNumVariables()) > maxVars)
	    }
	    */
	    delete currentFeatureArray;
	    delete mbf;
	  }

/// Begin Jan Van Haaren

      cout << "Jan: start counting" << endl;
      cout << "Minimum true proportion: " << minimumTrueProportion << endl;
      // cout << constructedClauses.size() << endl;
	  ClauseOpHashArray constructedClauses2;
	  for (int i = 0; i < constructedClauses.size(); i++) {
	     Clause* cl = constructedClauses[i];
         double trueCount = 0;
         double totalCount = 0;
         for (int dx = 0; dx < domains_->size(); dx++) {
	        trueCount += cl->getNumTrueGroundingsAnd((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	        totalCount += cl->getNumDistinctGroundings((*domains_)[dx]);
         }
         double proportion = trueCount / totalCount;
	     // cl->printWithoutWtWithStrVar(cout, (*domains_)[0]);
         // cout << endl;
         // cout << "true count: " << trueCount << endl;
         // cout << "total count: " << totalCount << endl;    
         // cout << "proportion: " << proportion << endl;
         if (proportion > minimumTrueProportion) {
           constructedClauses2.append(cl);
         }
	  }
      constructedClauses = constructedClauses2;
      cout << "Jan: end counting" << endl;

/// End Jan Van Haaren

	  numCandidatesConsidered += constructedClauses.size();
	  
	  bool error = false;
	  Array<double> priorMeans, priorStdDevs;
	  numClausesFormulas = getNumClausesFormulas();
	  Array<double> wts;
	  wts.growToSize(numClausesFormulas + 2); // first slot is unused
	  Array<double> origWts(numClausesFormulas); // current clause/formula wts
	  if (indexTrans_)
	    indexTrans_->getClauseFormulaWts(origWts);
	  else
	    mln0_->getClauseWts(origWts);
	  
	  //set the prior means and std deviations
	  setPriorMeansStdDevs(priorMeans, priorStdDevs, 1, NULL);
	  cout << "J Here..." << endl;
	  cout << "evaluating gain of candidates..." << constructedClauses.size() << endl << endl;
	  double bsec = timer_.time();
	  //FLAG - LIFTED LEARNING
	  for (int i = 0; i < constructedClauses.size(); i++) {
	    constructedClauses[i]->setDirty();
	    countAndMaxScoreEffectCandidate(constructedClauses[i], &wts,
					    &origWts, score, false, priorMeans, priorStdDevs, error, 
					    NULL, NULL);
	  }
	  
	  cout << "evaluating gain of candidates took ";
	  timer_.printTime(cout, timer_.time()-bsec);
	  cout << endl << endl;
	  
	  bsec = timer_.time();
	  //put the clauses in an array 
	  Array<Clause* > candidatesArray;
	  for (int i = 0; i < constructedClauses.size(); i++)
	    candidatesArray.append(constructedClauses[i]);
	  Array<Clause* > bestCandidates;
	  if (sortCandidates(candidatesArray, bestCandidates)) {
	    for (int i = 0; i < bestCandidates.size(); i++) {
	      //cout << "The current candidate is ";
	      //bestCandidates[i]->print(cout, (*domains_)[0]);
	      //cout << "The current status of the mln is\n";
	      //printMLNClausesWithWeightsAndScore(score, -1);
	      
	      appendToAndRemoveFromMLNs(bestCandidates[i], score);
	    }
	  }
	  
	  cout << "effecting high-gain candidates took ";
	  timer_.printTime(cout, timer_.time()-bsec);
	  cout << endl << endl;
	  
	  //clean up 
	  for (int d = 0; d < domains_->size(); d++) {
	    Array<PredicateHashArray *>* truePredicateGndingsArray = (*allDomainsTruePredGndings)[d];
	    Array<PredicateHashArray *>* falsePredicateGndingsArray = (*allDomainsFalsePredGndings)[d];
	    map<int, PredicateHashArray*>* constIdToPred = (*allDomainsConstIdToPred)[d];
	    PredicateToIntMap * relPathIdxForPred = (*allDomainsRelPathIdxForPred)[d];
	    Array<Array<RelPath*>*>* relPaths = (*allDomainsRelPaths)[d];

	    for (int i = 0; i < truePredicateGndingsArray->size(); i++)
	      if ((*truePredicateGndingsArray)[i] != NULL)
		delete (*truePredicateGndingsArray)[i];
	    delete truePredicateGndingsArray;
	    
	    for (int i = 0; i < falsePredicateGndingsArray->size(); i++)
	      if ((*falsePredicateGndingsArray)[i] != NULL)
		delete (*falsePredicateGndingsArray)[i];
	    delete falsePredicateGndingsArray;
	    
	    //for (int i = 0; i < constIdToPred->size(); i++)
	    //	delete (*constIdToPred)[i];
	    //delete constIdToPred;
	    map<int, PredicateHashArray*>::iterator iter;
	    for (iter = constIdToPred->begin(); iter != constIdToPred->end(); iter++) {
	      PredicateHashArray* curr = iter->second;
	      delete curr;
	    }
	    delete constIdToPred;
	    
	    delete relPathIdxForPred;
	    for (int i = 0; i < relPaths->size(); i++) {
	      Array<RelPath*>* currArray = (*relPaths)[i];
	      for (int j = 0; j < currArray->size(); j++)
		delete (*currArray)[j];
	      delete currArray;
	    }
	    delete relPaths;
	  }
	  
	  delete allDomainsTruePredGndings;
	  delete allDomainsFalsePredGndings;
	  delete allDomainsConstIdToPred;
	  delete allDomainsRelPathIdxForPred;
	  delete allDomainsRelPaths;
	  //done cleaning up
	  
	  //Prune each non-unit clause in MLN if it does not decrease score    
	  cout << "pruning clauses from MLN..." << endl << endl;
	  begSec = timer_.time();
	  stricterPruneMLN(score);
	  cout << "pruning clauses from MLN took ";
	  timer_.printTime(cout, timer_.time()-begSec);
	  cout << endl << endl;
	  
	  //printMLNClausesWithWeightsAndScore(score, -1);
	  printMLNToFile(NULL, -2);
	  
	  cout << "time taken for structure learning = ";
	  timer_.printTime(cout, timer_.time()-startSec_);
	  cout << endl << endl;
	  
	  cout << "Number of candidates considered was "
	       << numCandidatesConsidered << endl;
	  
	}

	bool sortCandidates(Array<Clause*>& candidates, Array<Clause*>& bestClauses)
	{
		//sort candidates in order of decreasing gain
		rquicksort(candidates);

		for (int i = 0; i < candidates.size(); i++) {
			double candGain = candidates[i]->getAuxClauseData()->gain;
			double candWeight = candidates[i]->getWt();
			
			if (candGain > minGain_) {
			  candidates[i]->printWithoutWtWithStrVar(cout, (*domains_)[0]);
			  cout <<  " candGain " << candGain << endl;
				if (candWeight > minWt_)
					bestClauses.append(candidates[i]);
			} else
				break;
		}
		return bestClauses.size() > 0;
	}

	/////////////// functions dealing with counts and maximizing score ///////////
private:

	double computeGainOfClauses(ClauseOpHashArray &newClauses, double & score)
	{
		double maxGain = 0;
		for (int i = 0; i < newClauses.size(); i++) {
			Array <double> weights;
			Array <double> priorMeans, priorStdDevs;
			setPriorMeansStdDevs(priorMeans, priorStdDevs, 1, NULL);
			weights.growToSize(getNumClausesFormulas() + 2);
			bool error = false;
			bool resetPriors = false;

			//cout << "The score before is " << score << endl;

			//this function returns the score, not the gain. We need to get the gain from the aux data of new clauses
			countAndMaxScoreEffectCandidate(newClauses[i], &weights, NULL,
					score, resetPriors, priorMeans, priorStdDevs, error, NULL, 
					NULL);

			//cout << "The score after is " << score << endl;

			double currGain = newClauses[i]->getAuxClauseData()->gain;

			//  cout << "The curr score is " << newScore << endl;
			//cout << "The curr gain is " << currGain << endl;

			if (currGain > maxGain || i == 0)
				maxGain = currGain;
		}
		return maxGain;
	}

	//size of wts array must be numClauses+1, its first slot is unused
	//if indexTrans_ is used, the sizes of its instance variables must be 
	//correctly set before calling this function
	double maximizeScore(const int& numClausesFormulas, const int& numExtraWts,
			Array<double>* const & wts, const Array<double>* const & origWts,
			const Array<int>* const & removedClauseFormulaIdxs, int& iter,
			bool& error, double& elapsedSec)
	{
		if (origWts) {
			for (int i=1; i<=numClausesFormulas; i++)
				(*wts)[i] = (*origWts)[i-1];
		} else {
			for (int i=1; i<=numClausesFormulas; i++)
				(*wts)[i] = 0;
		}

		for (int i = 1; i <= numExtraWts; i++)
			(*wts)[numClausesFormulas+i] = 0;

		if (removedClauseFormulaIdxs)
			for (int i = 0; i < removedClauseFormulaIdxs->size(); i++)
				(*wts)[ (*removedClauseFormulaIdxs)[i]+1 ] = 0;

		//commented out: this is done before this function
		//if (indexTrans_) 
		//  indexTrans_->appendClauseIdxToClauseFormulaIdxs(numExtraWts, 1);

		double* wwts = (double*) wts->getItems();
		double begSec = timer_.time();
		double newScore = lbfgsb_->minimize(numClausesFormulas + numExtraWts,
				wwts, iter, error);
		newScore = -newScore;
		elapsedSec = timer_.time() - begSec;

		//commented out: this is done after this function
		//if (indexTrans_) 
		//  indexTrans_->removeClauseIdxToClauseFormulaIdxs(numExtraWts, 1);

		return newScore;
	}

	//Compute/remove counts and maximize score when all the candidates are 
	//effected upon MLN. It computes/removes the counts in PseudoLogLikelihood 
	//and then finds the optimal pseudoLogLikelihood and weights, but it  
	//does not add/remove/replace clauses in the MLN.
	//if resetPriors is false, then priorMeans and priorStdDevs must 
	//already contain the mean/std dev of the existing MLN clauses, and be
	//of size #MLN clauses + #candidates
	double countAndMaxScoreEffectAllCandidates(
			const Array<Clause*>& candidates, Array<double>* const & wts,
			const Array<double>*const& origWts, const double& prevScore,
			const bool& resetPriors, Array<double>& priorMeans,
			Array<double>& priorStdDevs, bool& error,
			Array<UndoInfo*>* const& uundoInfos,
			Array<ClauseAndICDArray*>* const & appendedClauseInfos)
	{
		Array<UndoInfo*>* undoInfos = (uundoInfos) ? uundoInfos
				: new Array<UndoInfo*>;
		Array<int> remClauseFormulaIdxs; //for domain 0 only
		Array<int*> idxPtrs; //for clean up
		Array<Clause*> toBeRemovedClauses;
		Array<Clause*> toBeAppendedClauses;
		int numClausesFormulas = getNumClausesFormulas();

		for (int i = 0; i < candidates.size(); i++) {
			Clause* cand = candidates[i];
			AuxClauseData* acd = cand->getAuxClauseData();
			int op = acd->op;

			//remove counts for candidate
			if (op == OP_REMOVE || op == OP_REPLACE || op == OP_REPLACE_ADDPRED
					|| op == OP_REPLACE_REMPRED) {
				Clause
						* remClause =
								(mlns_->size() > 1) ? (Clause*) mln0_->getClause(acd->removedClauseIdx)
										: NULL;
				Array<int*> idxs;
				idxs.growToSize(mlns_->size());
				Array<Clause*> remClauses;
				remClauses.growToSize(mlns_->size());
				for (int d = 0; d < mlns_->size(); d++) {
					int remIdx;
					if (d == 0) {
						remIdx = acd->removedClauseIdx;
						remClauseFormulaIdxs.append(remIdx);
					} else {
						if (remClause->containsConstants())
							remClause->translateConstants((*domains_)[d-1], (*domains_)[d]);
						remIdx = (*mlns_)[d]->findClauseIdx(remClause);
					}
					idxs[d] = (*mlns_)[d]->getMLNClauseInfoIndexPtr(remIdx);
					remClauses[d] = (Clause*) (*mlns_)[d]->getClause(remIdx);
				}
				if (mlns_->size() > 1 && remClause->containsConstants())
					remClause->translateConstants((*domains_)[mlns_->size()-1], (*domains_)[0]);
				pllRemoveCountsForClause(remClauses, idxs, undoInfos);

				if (op == OP_REMOVE)
					toBeRemovedClauses.append(cand);
			}

			//compute counts for candidate
			if (op == OP_ADD || op == OP_REPLACE || op == OP_REPLACE_ADDPRED
					|| op == OP_REPLACE_REMPRED) {
				Array<int*> tmpIdxs;
				tmpIdxs.growToSize(mlns_->size());
				for (int d = 0; d < mlns_->size(); d++) {
					int* tmpClauseIdxInMLN = new int( (*mlns_)[d]->getNumClauses() +
							toBeAppendedClauses.size() );
					tmpIdxs[d] = tmpClauseIdxInMLN;
					idxPtrs.append(tmpClauseIdxInMLN);
				}

				Array<UndoInfo*>* tmpInfos = (appendedClauseInfos)?
				new Array<UndoInfo*> : undoInfos;
				pllComputeCountsForClause(cand, tmpIdxs, tmpInfos);
				toBeAppendedClauses.append(cand);

				if (appendedClauseInfos) {
					undoInfos->append(*tmpInfos);
					ClauseAndICDArray* ca = new ClauseAndICDArray;
					ca->clause = cand;
					appendedClauseInfos->append(ca);
					for (int j = 0; j < tmpInfos->size(); j++) {
						ca->icdArray.append(IndexCountDomainIdx());
						ca->icdArray.lastItem().iac=(*tmpInfos)[j]->affectedArr->lastItem();
						ca->icdArray.lastItem().domainIdx = (*tmpInfos)[j]->domainIdx;
					}
					delete tmpInfos;
				}
			}
		}

		//find optimal weights and score

		// find the clause/formula indexes for indexes in remClauseFormulaIdxs 
		if (indexTrans_)
			indexTrans_->getClauseFormulaIndexes(remClauseFormulaIdxs, 0);

		Array<double> removedValues;
		if (resetPriors) {
			setPriorMeansStdDevs(priorMeans, priorStdDevs,
					toBeAppendedClauses.size(), &remClauseFormulaIdxs);
			if (indexTrans_)
				indexTrans_->appendClauseIdxToClauseFormulaIdxs(
						toBeAppendedClauses.size(), 1);
		} else {
			assert(priorMeans.size() == numClausesFormulas + candidates.size());
			assert(priorStdDevs.size() == numClausesFormulas
					+ candidates.size());
			if (indexTrans_)
				assert(indexTrans_->checkCIdxWtsGradsSize(candidates.size()));

			setRemoveAppendedPriorMeans(numClausesFormulas, priorMeans,
					remClauseFormulaIdxs, toBeAppendedClauses.size(),
					removedValues);
		}

		if (hasPrior_)
			pll_->setMeansStdDevs(priorMeans.size(), priorMeans.getItems(),
					priorStdDevs.getItems());
		else
			pll_->setMeansStdDevs(-1, NULL, NULL);

		wts->growToSize(numClausesFormulas + toBeAppendedClauses.size() + 1);

		int iter;
		double elapsedSec;
		double newScore = maximizeScore(numClausesFormulas,
				toBeAppendedClauses.size(), wts, origWts,
				&remClauseFormulaIdxs, iter, error, elapsedSec);

		//set the weights of the candidates
		for (int i = 0; i < toBeAppendedClauses.size(); i++)
			toBeAppendedClauses[i]->setWt((*wts)[numClausesFormulas+i+1]);
		//set to large weight so that they are filtered out because of small wt
		for (int i = 0; i < toBeRemovedClauses.size(); i++)
			toBeRemovedClauses[i]->setWt(111111111);

		Array<double> penalties;
		for (int i = 0; i < candidates.size(); i++)
			penalties.append(getPenalty(candidates[i]));

		if (error) {
			newScore=prevScore;
			cout<<"LBFGSB failed to find wts"<<endl;
		}
		/*
		  printNewScore(candidates, (*domains_)[0], iter, elapsedSec, newScore, newScore-prevScore,
				penalties);
		*/
		for (int i = 0; i < candidates.size(); i++)
			candidates[i]->getAuxClauseData()->gain = newScore-prevScore-penalties[i];

		if (uundoInfos == NULL) {
			pll_->undoAppendRemoveCounts(undoInfos);
			delete undoInfos;
		}

		if (resetPriors) {
			if (indexTrans_)
				indexTrans_->removeClauseIdxToClauseFormulaIdxs(
						toBeAppendedClauses.size(), 1);
		} else { //restore the prior means that were set to zero
			for (int i = 0; i < remClauseFormulaIdxs.size(); i++)
				priorMeans[remClauseFormulaIdxs[i]] = removedValues[i];
		}

		idxPtrs.deleteItemsAndClear();

		return newScore;
	} //countAndMaxScoreEffectAllCandidates()


	double countAndMaxScoreEffectCandidate(Clause* const & candidate,
			Array<double>* const & wts, const Array<double>* const & origWts,
			const double& prevScore, const bool& resetPriors,
			Array<double>& priorMeans, Array<double>& priorStdDevs,
			bool& error, Array<UndoInfo*>* const& undoInfos,
			Array<ClauseAndICDArray*>* const & appendedClauseInfos)
	{
		Array<Clause*> candidates;
		candidates.append(candidate);
		return countAndMaxScoreEffectAllCandidates(candidates, wts, origWts,
				prevScore, resetPriors, priorMeans, priorStdDevs, error,
				undoInfos, appendedClauseInfos);
	}

	//returns true if the candidates are all effected on MLN resulting in a
	//better score
	bool appendToAndRemoveFromMLNs(const Array<Clause*>& candidates,
			double& score, const bool& makeChangeForEqualScore=false)
	{
	  Array<double> wts;
	  Array<UndoInfo*> undoInfos;
	  Array<ClauseAndICDArray*> appendedClauseInfos;
	  Array<double> priorMeans, priorStdDevs;
	  bool error;
	  
	  double newScore = countAndMaxScoreEffectAllCandidates(candidates, &wts, 
								NULL, score, true, priorMeans, priorStdDevs, error, &undoInfos,
								&appendedClauseInfos);
	  
	  bool improve = (makeChangeForEqualScore) ? (newScore >= score)
	    : (newScore > score);
	  
	  if (!error && improve) {
	    score = newScore;
	    
	    //set MLN clauses/formulas to new weights
	    //weights of appended clauses are set when they are added to the MLN 
	    updateWts(wts, NULL, NULL);
	    
	    int numClausesFormulas = getNumClausesFormulas();
	    
	    for (int i = 0; i < appendedClauseInfos.size(); i++)
	      appendedClauseInfos[i]->clause->setWt(wts[numClausesFormulas+i+1]);
	    
	    //effect candidates on MLN
	    int appClauseIdx = 0;
	    for (int i = 0; i < candidates.size(); i++) {
	      Clause* cand = candidates[i];
	      AuxClauseData* acd = cand->getAuxClauseData();
	      int op = acd->op;
	      
	      // NOTE: cand was not canonicalized so as to easily compare it to the
	      //       mln clause it is to replace. Canonicalize it now before 
	      //       appending to MLN.
	      if (op == OP_REPLACE)
		cand->canonicalize();
	      
	      if (op == OP_REMOVE || op == OP_REPLACE || op
		  == OP_REPLACE_ADDPRED || op == OP_REPLACE_REMPRED) {
		Clause* remClause =
		  (Clause*) mln0_->getClause(acd->removedClauseIdx);
		
		for (int d = 0; d < mlns_->size(); d++) {
		  if (d == 0) {
		    //Flag print out
		    Clause* r = removeClauseFromMLN(acd->removedClauseIdx, d);
		    if (!seen24hr){
		      //86400
		      if ((timer_.time() - startSec_) > 86400){ 
			seen24hr = true;
			printMLNToFile("24hr",-2);
		      }
		    }
		    else if (!seen48hr){
		      if ((timer_.time() - startSec_) > 172800){ 
			seen48hr = true;
			printMLNToFile("48hr",-2);
		      }
		    }
		    cout << "Modified MLN: Removed clause from MLN: ";
		    r->printWithoutWtWithStrVar(cout, (*domains_)[0]);
		    cout << " " << r->getAuxClauseData()->gain << " " << newScore << " " << score << endl;
		    cout << endl;
		    if (op == OP_REMOVE && cand != r)
		      delete cand;
		    //delete r; //this is remClause which has to be used below
		  } else {
		    if (remClause->containsConstants())
		      remClause->translateConstants((*domains_)[d-1], (*domains_)[d]);
		    int remIdx = (*mlns_)[d]->findClauseIdx(remClause);
		    delete removeClauseFromMLN(remIdx, d);
		  }
		}
		delete remClause;
	      }
	      
	      if (op == OP_ADD || op == OP_REPLACE || op
		  == OP_REPLACE_ADDPRED || op == OP_REPLACE_REMPRED) {
		Array<int*> idxPtrs;
		idxPtrs.growToSize(mlns_->size());
		for (int d = 0; d < mlns_->size(); d++) {
		  Clause* c = cand;
		  if (d > 0) {
		    if (cand->containsConstants())
		      cand->translateConstants((*domains_)[d-1], (*domains_)[d]);
		    c = new Clause(*cand);
		  }
		  int idx = appendClauseToMLN(c, d);
		  idxPtrs[d] = (*mlns_)[d]->getMLNClauseInfoIndexPtr(idx);
		}
		
		Array<IndexCountDomainIdx>& icds =
		  appendedClauseInfos[appClauseIdx++]->icdArray;
		for (int j = 0; j < icds.size(); j++)
		  icds[j].iac->index = idxPtrs[ icds[j].domainIdx ];
		
		//Flag print out
		if (!seen24hr){
		  //86400
		  if ((timer_.time() - startSec_) > 86400){ 
		    seen24hr = true;
		    printMLNToFile("24hr",-2);
		  }
		}
		else if (!seen48hr){
		  if ((timer_.time() - startSec_) > 172800){ 
		    seen48hr = true;
		    printMLNToFile("48hr",-2);
		  }
		}

		cout << "Modified MLN: Appended clause to MLN: ";
		cand->printWithoutWtWithStrVar(cout, (*domains_)[0]);
		cout << " gain " << cand->getAuxClauseData()->gain;
		cout << endl;
	      }
	    }
	    
	    assert(pll_->checkNoRepeatedIndex());
	    assert(appClauseIdx == appendedClauseInfos.size());
	    
	    undoInfos.deleteItemsAndClear();
	    
	    //MLNs has changed, the index translation must be recreated
	    if (indexTrans_)
	      indexTrans_->createClauseIdxToClauseFormulaIdxsMap();
	  } 
	  else {
	    /*
	    cout << "Keep: ";
	    candidates[0]->printWithoutWtWithStrVar(cout, (*domains_)[0]);
	    cout << " " << candidates.size() << " score " << score << " new score " << newScore << " gain " << candidates[0]->getAuxClauseData()->gain << endl;
	    */
	    //cout << "undoing candidates because score did not improve..."<<endl
	    //<<endl;
	    pll_->undoAppendRemoveCounts(&undoInfos);
	  }
	  
	  appendedClauseInfos.deleteItemsAndClear();
	  return improve;
	}//appendToAndRemoveFromMLNs()
	
	//returns true if candidate is effected on MLN resulting in a better score
	bool appendToAndRemoveFromMLNs(Clause* const & candidate, double& score,
			const bool& makeChangeForEqualScore=false)
	{
		Array<Clause*> candidates;
		candidates.append(candidate);
		return appendToAndRemoveFromMLNs(candidates, score,
				makeChangeForEqualScore);
	}

	//Clause c enters & exits  function with its AuxClauseData's cache == NULL
	void pllComputeCountsForClause(Clause* const & c,
			const Array<int*>& clauseIdxInMLNs,
			Array<UndoInfo*>* const & undoInfos)
	{
		assert(c->getAuxClauseData()->cache == NULL);
		assert(clauseIdxInMLNs.size() == domains_->size());
		double begSec = timer_.time();

		if (cacheClauses_) {
			int i;
			if ((i = cachedClauses_->find(c)) >= 0) // if clause and counts are cached
			{
				//cout << "using cached counts ";
				//cout << "for "; c->printWithoutWtWithStrVar(cout, (*domains_)[0]); 
				//cout << endl;
				pll_->insertCounts(clauseIdxInMLNs, undoInfos, (*cachedClauses_)[i]->getAuxClauseData()->cache);
				cout << "using cached counts took ";
				timer_.printTime(cout, timer_.time()-begSec);
				cout << endl;
				return;
			} else {
				assert(c->getAuxClauseData()->cache == NULL);
				if (cacheSizeMB_ < maxCacheSizeMB_)
					c->newCache(domains_->size(), (*domains_)[0]->getNumPredicates());
				else {
					static bool printCacheFull = true;
					if (printCacheFull) {
						cout << "Cache is full, approximate size = "
								<< cacheSizeMB_ << " MB" << endl;
						printCacheFull = false;
					}
				}
				//cout << "cache size (MB) = " << cacheSizeMB_ << endl;
			}
		}

		//we do not have the counts

		//if we don't need to translate constant ids from one domain to another
		if (!c->containsConstants()) {
			for (int i = 0; i < domains_->size(); i++) {
				int* clauseIdxInMLN = clauseIdxInMLNs[i];
				pll_->computeCountsForNewAppendedClause(c, clauseIdxInMLN, i,
						undoInfos, sampleClauses_, c->getAuxClauseData()->cache);
			}
		} else { //we need to translate constant ids from one domain to another
			int i;
			for (i = 0; i < domains_->size(); i++) {
				if (i > 0)
					c->translateConstants((*domains_)[i-1], (*domains_)[i]);
				int* clauseIdxInMLN = clauseIdxInMLNs[i];
				pll_->computeCountsForNewAppendedClause(c, clauseIdxInMLN, i,
						undoInfos, sampleClauses_, c->getAuxClauseData()->cache);
			}
			if (i > 1)
				c->translateConstants((*domains_)[i-1], (*domains_)[0]);
		}

		if (c->getAuxClauseData()->cache) {
			if (cacheSizeMB_ < maxCacheSizeMB_) {
				cacheSizeMB_ += c->sizeMB();
				Array<Array<Array<CacheCount*>*>*>* cache =c->getAuxClauseData()->cache;
				c->getAuxClauseData()->cache = NULL;
				Clause* copyClause = new Clause(*c);
				copyClause->getAuxClauseData()->cache = cache;
				copyClause->compress();
				cachedClauses_->append(copyClause);
			} else {
				c->getAuxClauseData()->deleteCache();
				c->getAuxClauseData()->cache = NULL;
			}
		}
	}

	void pllRemoveCountsForClause(const Array<Clause*>& remClauses,
			const Array<int*>& clauseIdxInMLNs,
			Array<UndoInfo*>* const & undoInfos)
	{
		assert(clauseIdxInMLNs.size() == domains_->size());
		//    double begSec = timer_.time();        
		for (int i = 0; i < domains_->size(); i++)
			pll_->removeCountsForClause(remClauses[i], clauseIdxInMLNs[i], i,
					undoInfos);
	}

	void pllComputeCountsForInitialMLN()
	{
		for (int i = 0; i < mlns_->size(); i++) {
			cout << "computing counts for clauses in domain " << i << "..."
					<< endl;
			MLN* mln = (*mlns_)[i];
			for (int j = 0; j < mln->getNumClauses(); j++) {
				Clause* c = (Clause*)mln->getClause(j);
				cout << "Clause " << j << ": ";
				c->printWithoutWtWithStrVar(cout, (*domains_)[i]);
				cout << endl;
				int* clauseIdxInMLN = mln->getMLNClauseInfoIndexPtr(j);
				pll_->computeCountsForNewAppendedClause(c, clauseIdxInMLN, i, 
				NULL, sampleClauses_, NULL);
			}
		}
	}

	//////////////////// adding/removing/replacing MLN clauses /////////////////
private:
	void addUnitClausesToMLNs()
	{
		Array<Predicate*> nonEvidPreds;
		for (int i = 0; i < preds_->size(); i++)
			if ((*areNonEvidPreds_)[(*preds_)[i]->getId()])
				nonEvidPreds.append((*preds_)[i]);

		Array<Clause*> unitClauses;
		bool allowEqualPreds = true;
		ClauseFactory::createUnitClauses(unitClauses, nonEvidPreds,
				allowEqualPreds);

		for (int i = 0; i < unitClauses.size(); i++) {
			if (mln0_->containsClause(unitClauses[i])) {
				delete unitClauses[i];
				continue;
			}
			ostringstream oss;
			int idx;
			unitClauses[i]->printWithoutWtWithStrVar(oss, (*domains_)[0]);

			for (int j = 0; j < mlns_->size(); j++) {
				Clause* c = (j == 0) ? unitClauses[i] : new Clause(*unitClauses[i]);
				(*mlns_)[j]->appendClause(oss.str(), false, c,
						priorMean_, false, idx);
				((MLNClauseInfo*)(*mlns_)[j]->getMLNClauseInfo(idx))->priorMean = priorMean_;
			}
		}
	}

	void appendUnitClausesWithDiffCombOfVar(double& score)
	{
		bool allowEqualPreds = false;
		for (int i = 0; i < preds_->size(); i++) {
			if (!(*areNonEvidPreds_)[(*preds_)[i]->getId()])
				continue;

			Clause* origClause = ClauseFactory::createUnitClause((*preds_)[i], allowEqualPreds);
			if (origClause == NULL)
				continue;
			assert(origClause->getAuxClauseData() == NULL);
			origClause->setAuxClauseData(new AuxClauseData);
			origClause->trackConstants();

			ClauseOpHashArray newUnitClauses;
			clauseFactory_->createUnitClausesWithDiffCombOfVar((*preds_)[i], OP_ADD, -1, newUnitClauses);
			for (int j = 0; j < newUnitClauses.size(); j++) {
				Clause* newClause = newUnitClauses[j];

				if (origClause->same(newClause)
						|| !clauseFactory_->validClause(newClause)
						|| mln0_->containsClause(newClause)) {
					newUnitClauses.removeItemFastDisorder(j);
					delete newClause;
					j--;
					continue;
				}
				//score is updated
				if (!appendToAndRemoveFromMLNs(newClause, score))
					delete newClause;
			}
			delete origClause;
		} // for each predicate
	}

	void flipMLNClauses(double& score)
	{
		Array<Clause*> mlnClauses;
		for (int i = 0; i < mln0_->getNumClauses(); i++) {
			//do not flip unit clauses or those derived from existential formulas
			if (mln0_->getClause(i)->getNumPredicates()==1 || !isModifiableClause(i))
				continue;
			mlnClauses.append((Clause*)mln0_->getClause(i));
		}

		for (int i = 0; i < mlnClauses.size(); i++) {
			Clause* origClause = mlnClauses[i];
			int origIdx = mln0_->findClauseIdx(origClause);

			// NOTE: new clauses are not canonicalized so that they can be easily 
			//       compared with the original to determine its penalty.
			bool canonicalizeNewClauses = false;
			ClauseOpHashArray newClauses;
			clauseFactory_->flipSensesInClause(origClause, OP_REPLACE, origIdx,
					newClauses, canonicalizeNewClauses);

			Array<double> priorMeans, priorStdDevs;
			setPriorMeansStdDevs(priorMeans, priorStdDevs, 1, NULL);
			Array<double> wts;
			wts.growToSize(getNumClausesFormulas()+2);
			Clause* bestClause= NULL;
			double bestScore = score;
			bool error;

			for (int j = 0; j < newClauses.size(); j++) // for each 'flipped' clause
			{
				Clause* newClause = newClauses[j];
				if (origClause->same(newClause)
						|| newClause->hasRedundantPredicates()
						|| mln0_->containsClause(newClause)) {
					delete newClause;
					continue;
				}

				double newScore = countAndMaxScoreEffectCandidate(newClause,
						&wts, NULL, score, false, priorMeans, priorStdDevs,
						error, 
						NULL, NULL);
				if (newScore > bestScore) {
					bestScore = newScore;
					if (bestClause)
						delete bestClause;
					bestClause = newClause;
				} else
					delete newClause;
			}

			if (bestClause && !appendToAndRemoveFromMLNs(bestClause, score))
				delete bestClause;
		}// for each MLN clause
	}//flipMLNClauses()

	void stricterPruneMLN(double& score)
	{
		Array<Clause*> mlnClauses;
		for (int i = 0; i < mln0_->getNumClauses(); i++) {
			//do not prune unit clauses or clauses derived from existential formulas
			if (mln0_->getClause(i)->getNumPredicates() == 1 || !isModifiableClause(i))
				continue;
			mlnClauses.append((Clause*)mln0_->getClause(i));
		}

		for (int i = 0; i < mlnClauses.size(); i++) {
			Clause* origClause = mlnClauses[i];
			int origIdx = mln0_->findClauseIdx(origClause);
			Clause* copy = new Clause(*origClause);
			copy->setAuxClauseData(new AuxClauseData(0, OP_REMOVE, origIdx));
			copy->trackConstants();

			if (origClause->getWt() < minWt_) {
				double tempScore = -10000; //put some big score to ensure it can be improved
				if (!appendToAndRemoveFromMLNs(copy, tempScore, true))
					delete copy;
				score = tempScore;
			} else {
				if (!appendToAndRemoveFromMLNs(copy, score, true))
					delete copy;
			}
		}
	}

	int appendClauseToMLN(Clause* const c, const int& domainIdx)
	{
		ostringstream oss;
		int idx;
		c->printWithoutWtWithStrVar(oss, (*domains_)[domainIdx]);
		MLN* mln = (*mlns_)[domainIdx];
		bool ok =
				mln->appendClause(oss.str(), false, c, c->getWt(), false, idx);
		if (!ok) {
			cout << "ERROR: failed to insert " << oss.str() <<" into MLN"
					<< endl;
			exit(-1);
		}
		((MLNClauseInfo*)mln->getMLNClauseInfo(idx))->priorMean = priorMean_;
		return idx;
	}

	Clause* removeClauseFromMLN(const int& remClauseIdx, const int& domainIdx)
	{
		Clause* remClause = (*mlns_)[domainIdx]->removeClause(remClauseIdx);
		if (remClause == NULL) {
			cout << "ERROR: failed to remove ";
			remClause->printWithoutWtWithStrVar(cout, (*domains_)[0]);
			cout << " from MLN" << endl;
			exit(-1);
		}
		return remClause;
	}

	//////////////////////// helper functions ////////////////////////////////

private:
	void useTightParams()
	{
		sampleClauses_ = false;
		pll_->setSampleGndPreds(false);
		lbfgsb_->setMaxIter(lbMaxIter_);
		lbfgsb_->setFtol(lbConvThresh_);
		cacheClauses_ = false;
	}

	void useLooseParams()
	{
		sampleClauses_ = origSampleClauses_;
		pll_->setSampleGndPreds(sampleGndPreds_);
		if (looseMaxIter_ >= 0)
			lbfgsb_->setMaxIter(looseMaxIter_);
		if (looseConvThresh_ >= 0)
			lbfgsb_->setFtol(looseConvThresh_);
		cacheClauses_ = origCacheClauses_;
	}

	//i is the clause's index in mln_[0]
	bool isModifiableClause(const int& i) const
	{
		return (!mln0_->isExistClause(i) && !mln0_->isExistUniqueClause(i));
	}

	bool isNonModifiableFormula(const FormulaAndClauses* const & fnc) const
	{
		return (fnc->hasExist || fnc->isExistUnique);
	}

	void printIterAndTimeElapsed(const double& begSec)
	{
		cout << "Iteration " << iter_ << " took ";
		timer_.printTime(cout, timer_.time()-begSec);
		cout << endl << endl;
		cout << "Time elapsed = ";
		timer_.printTime(cout, timer_.time()-startSec_);
		cout << endl << endl;
	}

	void removeClausesFromMLNs()
	{
		for (int i = 0; i < mlns_->size(); i++)
		  (*mlns_)[i]->removeAllClauses(NULL,false);
	}

	bool effectBestCandidateOnMLNs(Clause* const & cand, double& score)
	{
	  if (appendToAndRemoveFromMLNs(cand, score)) {
	    //printMLNClausesWithWeightsAndScore(score, iter_);
	    //printMLNToFile(NULL, iter_);
	    return true;
	  }
	  return false;
	}

	bool effectBestCandidateOnMLNs(Array<Clause*>& bestCandidates, double& score)
	{
		cout << "effecting best candidate on MLN..." << endl << endl;
		bool ok = false;
		int i;
		for (i = 0; i < bestCandidates.size(); i++) {
			cout << "effecting best candidate " << i << " on MLN..." << endl
					<< endl;
			if (ok=effectBestCandidateOnMLNs(bestCandidates[i], score)) {
				cout << "!!!!!!!!!!Added Best Candidate\n";
				break;
			}
			cout << "failed to effect candidate on MLN." << endl;
			delete bestCandidates[i];
		}
		for (int j = i+1; j < bestCandidates.size(); j++)
			delete bestCandidates[j];
		return ok;
	}

	//same as original method except that it returns the index of the
	//candidate that was added. Used to keep stuff found during
	//relational pathfinding out of the beam. Returns -1 if not ok
	int effectBestCandidateOnMLNsReturnIndex(Array<Clause*>& bestCandidates,
			double& score)
	{
		cout << "effecting best candidate on MLN..." << endl << endl;
		int index = -1;
		int i;
		for (i = 0; i < bestCandidates.size(); i++) {
			cout << "effecting best candidate " << i << " on MLN..." << endl
					<< endl;
			if (effectBestCandidateOnMLNs(bestCandidates[i], score)) {
				cout << "Added Best Candidate\n";
				index = i;
				break;
			}
			cout << "failed to effect candidate on MLN." << endl;
			delete bestCandidates[i];
		}
		for (int j = i+1; j < bestCandidates.size(); j++)
			delete bestCandidates[j];
		return index;
	}

	double getPenalty(const Clause* const & cand)
	{
		int op = cand->getAuxClauseData()->op;

		if (op == OP_ADD)
			return cand->getNumPredicates() * penalty_;

		if (op == OP_REPLACE_ADDPRED) {
			int remIdx = cand->getAuxClauseData()->removedClauseIdx;
			int origlen = mln0_->getClause(remIdx)->getNumPredicates();
			int diff = cand->getNumPredicates() - origlen;
			assert(diff > 0);
			return diff * penalty_;
		}

		if (op == OP_REPLACE_REMPRED) {
			int remIdx = cand->getAuxClauseData()->removedClauseIdx;
			int origlen = mln0_->getClause(remIdx)->getNumPredicates();
			int diff = origlen - cand->getNumPredicates();
			assert(diff > 0);
			return diff * penalty_;
		}

		if (op == OP_REPLACE) {
			//NOTE: op is REPLACE only when an MLN clause is to be replaced with
			//      one that is identical except for its predicates' senses
			int remIdx = cand->getAuxClauseData()->removedClauseIdx;
			const Clause* mlnClause = mln0_->getClause(remIdx);
			assert(cand->getNumPredicates() == mlnClause->getNumPredicates());
			int diff = 0;
			for (int i = 0; i < cand->getNumPredicates(); i++) {
				Predicate* cpred = (Predicate*) cand->getPredicate(i);
				Predicate* mpred = (Predicate*) mlnClause->getPredicate(i);
				assert(cpred->same(mpred));
				if (cpred->getSense() != mpred->getSense())
					diff++;
			}
			return diff * penalty_;
		}

		if (op == OP_REMOVE)
			return cand->getNumPredicates() * penalty_;

		assert(false);
		return 88888;
	}

	void printNewScore(const Array<Clause*>& carray,
			const Domain* const & domain, const int& lbfgsbIter,
			const double& lbfgsbSec, const double& newScore,
			const double& gain, const Array<double>& penalties)
	{
	  cout << "*************************** " << candCnt_++ << ", iter "
	       << iter_ << endl;
	  // << ", beam search iter " <<  bsiter_ << endl;
	  for (int i = 0; i < carray.size(); i++) {
	    if (carray[i]) {
	      cout << "candidate     : ";
	      carray[i]->printWithoutWtWithStrVar(cout, domain);
	      cout << endl;
	      cout << "op            : ";
	      cout << Clause::getOpAsString(carray[i]->getAuxClauseData()->op) <<endl;
	      cout << "removed clause: ";
	      int remIdx = carray[i]->getAuxClauseData()->removedClauseIdx;
	      if (remIdx < 0)
		cout << "NULL";
	      else {
		mln0_->getClause(remIdx)->printWithoutWtWithStrVar(cout, domain);
	      }
	      cout << endl;
	      if (carray[i]->getAuxClauseData()->prevClauseStr.length() > 0) {
		cout << "prevClause    : ";
		cout << carray[i]->getAuxClauseData()->prevClauseStr << endl;
	      }
	      if (carray[i]->getAuxClauseData()->addedPredStr.length() > 0) {
		cout << "addedPred     : ";
		cout << carray[i]->getAuxClauseData()->addedPredStr << endl;
	      }
	      if (carray[i]->getAuxClauseData()->removedPredIdx >= 0) {
		cout << "removedPredIdx: ";
		carray[i]->getAuxClauseData()->removedPredIdx;
	      }
	      
	      cout << "score    : " << newScore << endl;
	      cout << "gain     : " << gain << endl;
	      cout << "penalty  : " << penalties[i] << endl;
	      cout << "net gain : " << gain - penalties[i] << endl;
	      if (carray[i]->getAuxClauseData()->op != OP_REMOVE)
		cout << "wt       : " << carray[i]->getWt() << endl;
	    }
	  }
	  cout << "num LBFGSB iter      = " << lbfgsbIter << endl;
	  cout << "time taken by LBFGSB = ";
	  Timer::printTime(cout, lbfgsbSec);
	  cout << endl;
	  cout << "*************************** " << endl << endl;;
	}

	void printNewScore(const Clause* const & c, const Domain* const & domain,
			   const int& lbfgsbIter, const double& lbfgsbSec,
			   const double& newScore, const double& gain, const double& penalty)
	{
	  Array<Clause*> carray;
	  Array<double> parray;
	  if (c) {
	    carray.append((Clause*)c);
			parray.append(penalty);
	  }
	  printNewScore(carray, domain, lbfgsbIter, lbfgsbSec, newScore, gain,
			parray);
	}
	
	void printMLNClausesWithWeightsAndScore(const double& score, const int& iter)
	{
	  if (iter >= 0)
	    cout << "MLN in iteration " << iter << ":" << endl;
	  else
	    cout << "MLN:" << endl;
	  cout << "------------------------------------" << endl;
	  if (indexTrans_)
	    indexTrans_->printClauseFormulaWts(cout, true);
	  else
	    mln0_->printMLNClausesFormulas(cout, (*domains_)[0], true);
	  cout << "------------------------------------" << endl;
	  cout << "score = "<< score << endl << endl;
	}

	void printMLNToFile(const char* const & appendStr, const int& iter)
	{
	  string fname = outMLNFileName_;
	  
	  if (appendStr)
	    fname.append(".").append(appendStr);
	  
	  if (iter >= -1) {
	    char buf[100];
	    sprintf(buf, "%d", iter);
	    fname.append(".iter").append(buf);
	  }
	  
	  if (appendStr || iter >= -1)
	    fname.append(".mln");
	  
	  ofstream out(fname.c_str());
	  if (!out.good()) {
	    cout << "ERROR: failed to open " <<fname<<endl;
	    exit(-1);
	  }
	  // output the predicate declaration
	  out << "//predicate declarations" << endl;
	  (*domains_)[0]->printPredicateTemplates(out);
	  out << endl;
	  
	  if (indexTrans_)
	    indexTrans_->printClauseFormulaWts(out, false);
	  else
	    mln0_->printMLNClausesFormulas(out, (*domains_)[0], false);
	  
	  out << endl;
	  out.close();
	}

	void setPriorMeansStdDevs(Array<double>& priorMeans,
			Array<double>& priorStdDevs, const int& addSlots,
			const Array<int>* const& removedSlotIndexes)
	{
	  priorMeans.clear();
	  priorStdDevs.clear();
	  
	  if (indexTrans_) {
	    indexTrans_->setPriorMeans(priorMeans);
	    priorStdDevs.growToSize(priorMeans.size());
	    for (int i = 0; i < priorMeans.size(); i++)
				priorStdDevs[i] = priorStdDev_;
		} else {
			for (int i = 0; i < mln0_->getNumClauses(); i++) {
				priorMeans.append(mln0_->getMLNClauseInfo(i)->priorMean);
				priorStdDevs.append(priorStdDev_);
			}
		}

		if (removedSlotIndexes) {
			for (int i = 0; i < removedSlotIndexes->size(); i++)
				priorMeans[ (*removedSlotIndexes)[i] ] = 0;
		}

		for (int i = 0; i < addSlots; i++) {
			priorMeans.append(priorMean_);
			priorStdDevs.append(priorStdDev_);
		}
	}

	void pllSetPriorMeansStdDevs(Array<double>& priorMeans,
			Array<double>& priorStdDevs, const int& addSlots,
			const Array<int>* const & removedSlotIndexes)
	{
		if (hasPrior_) {
			setPriorMeansStdDevs(priorMeans, priorStdDevs, addSlots,
					removedSlotIndexes);
			pll_->setMeansStdDevs(priorMeans.size(), priorMeans.getItems(),
					priorStdDevs.getItems());
		} else
			pll_->setMeansStdDevs(-1, NULL, NULL);
	}

	void setRemoveAppendedPriorMeans(const int& numClausesFormulas,
			Array<double>& priorMeans, const Array<int>& removedSlotIndexes,
			const int& addSlots, Array<double>& removedValues)
	{
		for (int i = 0; i < removedSlotIndexes.size(); i++) {
			removedValues.append(priorMeans[ removedSlotIndexes[i] ]);
			priorMeans[ removedSlotIndexes[i] ] = 0;
		}

		for (int i = 0; i < addSlots; i++)
			priorMeans[numClausesFormulas+i] = priorMean_;
	}

	int getNumClausesFormulas()
	{
		if (indexTrans_)
			return indexTrans_->getNumClausesAndExistFormulas();
		return mln0_->getNumClauses();
	}

	void updateWts(const Array<double>& wts,
			const Array<Clause*>* const & appendedClauses,
			const Array<string>* const & appendedFormulas)
	{
		if (indexTrans_) {
			Array<double> tmpWts;
			tmpWts.growToSize(wts.size()-1);
			for (int i = 1; i < wts.size(); i++)
				tmpWts[i-1] = wts[i];
			indexTrans_->updateClauseFormulaWtsInMLNs(tmpWts, appendedClauses,
					appendedFormulas);
		} else {
			for (int i = 0; i < mlns_->size(); i++)
				for (int j = 0; j < (*mlns_)[i]->getNumClauses(); j++)
					((Clause*)(*mlns_)[i]->getClause(j))->setWt(wts[j+1]);
		}
	}

	/////////////////////////// quicksort ////////////////////////////////

private:
	//sort clauses in decreasing order of gain
	void rquicksort(Array<Clause*>& clauses, const int& l, const int& r)
	{
		Clause** items = (Clause**) clauses.getItems();
		if (l >= r)
			return;
		Clause* tmp = items[l];
		items[l] = items[(l+r)/2];
		items[(l+r)/2] = tmp;

		int last = l;
		for (int i = l+1; i <= r; i++)
			if (items[i]->getAuxClauseData()->gain > items[l]->getAuxClauseData()->gain) {
				++last;
				Clause* tmp = items[last];
				items[last] = items[i];
				items[i] = tmp;
			}

		tmp = items[l];
		items[l] = items[last];
		items[last] = tmp;
		rquicksort(clauses, l, last-1);
		rquicksort(clauses, last+1, r);
	}

	void rquicksort(Array<Clause*>& ca)
	{
		rquicksort(ca, 0, ca.size()-1);
	}

	////////////////// functions to handle existential formulas ///////////////
private:
	void deleteExistFormulas(Array<ExistFormula*>& existFormulas);

	void getMLNClauses(Array<Clause*>& initialMLNClauses,
			Array<ExistFormula*>& existFormulas);

	//cnfClauses enter & exit function with its AuxClauseData's cache == NULL
	inline void pllCountsForExistFormula(Clause* cnfClause,
			const int& domainIdx, int* clauseIdxInMLN,
			Array<UndoInfo*>* const & undoInfos);

	inline void printNewScore(const string existFormStr, const int& lbfgsbIter,
			const double& lbfgsbSec, const double& newScore,
			const double& gain, const double& penalty, const double& wt);

	inline void rquicksort(Array<ExistFormula*>& existFormulas, const int& l,
			const int& r);

	inline void rquicksort(Array<ExistFormula*>& ef);

	inline void evaluateExistFormula(ExistFormula* const& ef,
			const bool& computeCountsOnly,
			Array<Array<Array<IndexAndCount*> > >* const & iacArraysPerDomain,
			const double& prevScore);

	double evaluateExistFormulas(Array<ExistFormula*>& existFormulas,
			Array<ExistFormula*>& highGainWtFormulas, const double& prevScore);

	inline void appendExistFormulaToMLNs(ExistFormula* const & ef);

	inline bool effectExistFormulaOnMLNs(ExistFormula* ef,
			Array<ExistFormula*>& existFormulas, double& score);

	bool effectBestCandidateOnMLNs(Array<Clause*>& bestCandidates,
			Array<ExistFormula*>& existFormulas,
			Array<ExistFormula*>& highGainWtFormulas, double& score);

/////////////////////////////////////////////////////////////////////////
private:
  // mln0_ and mlns_ are not owned by StructLearn; do not delete;
  // if there are more than one domains, mln0_ corresponds to domains_[0]
   MLN* mln0_;
Array<MLN*>* mlns_;
bool startFromEmptyMLN_;
string outMLNFileName_;
Array<Domain*>* domains_; // not owned by StructLearn; do not delete;
Array<Predicate*>* preds_;
	Array<bool>* areNonEvidPreds_;
	ClauseFactory* clauseFactory_;

	bool cacheClauses_;
	bool origCacheClauses_;
	ClauseHashArray* cachedClauses_;
	double cacheSizeMB_;
	double maxCacheSizeMB_;

	bool tryAllFlips_;
	bool sampleClauses_;
	bool origSampleClauses_;

	PseudoLogLikelihood* pll_;
	bool hasPrior_;
	double priorMean_;
	double priorStdDev_;
	bool wtPredsEqually_;

	LBFGSB* lbfgsb_;
	int lbMaxIter_;
	double lbConvThresh_;
	int looseMaxIter_;
	double looseConvThresh_;

	double minGain_;
	double minWt_;
	double penalty_;

 
 	bool seen24hr;
	bool seen48hr;
	int jMaxVars;
	int jMaxPreds;
	bool sampleGndPreds_;
	double fraction_;
	int minGndPredSamples_;
	int maxGndPredSamples_;

	// bool reEvalBestCandsWithTightParams_;

	Timer timer_;
	int candCnt_;

	int iter_;
	//int bsiter_;
	double startSec_;

	//Used to map clause indexes in MLNs to the weights indexes that are 
	//presented to LBFGSB. This is required if the clauses do not line up
	//across databases (as when an existentially quantified formula has a 
	//different number of formulas in its CNF for different databases).
	IndexTranslator* indexTrans_;
};
#endif

