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
#ifndef STRUCTLEARN_H_NOV_2_2005
#define STRUCTLEARN_H_NOV_2_2005

#include "clausefactory.h"
#include "mln.h"
//#include "pseudologlikelihood.h"
#include "lbfgsb.h"
#include "discriminativelearner.h"

//class ExistFormula;
/////////////////////// code to handle existential formulas //////////////////
struct ExistFormula
{
  ExistFormula(const string& fformula) 
    : formula(fformula), gain(0), wt(0), newScore(0), numPreds(0) {}
  ~ExistFormula() 
  { 
    for (int i = 0; i < cnfClausesForDomains.size(); i++)
      cnfClausesForDomains[i].deleteItemsAndClear();
  }

  string formula;
    //cnfClausesForDomains[d] are the CNF clauses formed with the constants 
    //in domain d
  Array<Array<Clause*> > cnfClausesForDomains;
  double gain; 
  double wt;
  Array<double> wts;
  double newScore;
  int numPreds;
};

struct IndexCountDomainIdx { IndexAndCount* iac; int domainIdx; };
struct ClauseAndICDArray {Clause* clause;Array<IndexCountDomainIdx> icdArray;};

//typedef HashArray<Term*, HashTerm, EqualTerm> TermHashArray;

class StructLearn
{
 public:
    //If there are more than one domain, there must be a one-to-one 
    //correspondence among the clauses in mlns.
  StructLearn(Array<MLN*>* const & mlns, const bool& startFromEmptyMLN,
              const string& outMLNFileName, Array<Domain*>* const & domains,
              const Array<string>* const & nonEvidPredNames,
              const int& maxVars, const int& maxNumPredicates,
              const bool& cacheClauses, const double& maxCacheSizeMB,
              const bool& tryAllFlips, const bool& sampleClauses, 
              const double& delta, const double& epsilon, 
              const int& minClauseSamples, const int& maxClauseSamples,
              const bool& hasPrior, const double& priorMean, 
              const double& priorStdDev, const bool& wtPredsEqually, 
              const int& lbMaxIter, const double& lbConvThresh,
              const int& looseMaxIter, const double& looseConvThresh, 
              const int& beamSize, const int& bestGainUnchangedLimit,
              const int& numEstBestClauses, 
              const double& minWt, const double& penalty,
              const bool& sampleGndPreds, const double& fraction, 
              const int& minGndPredSamples, const int& maxGndPredSamples,
              const bool& reEvaluateBestCandidatesWithTightParams,
              const bool& structGradDescent, const bool& withEM,
	      const bool& partialPrint, const bool& timeBased,
	      const double& maxRunTime, const int plusType, const int hrs, const int& numClauses, const double& minPllGain)

    : mln0_((*mlns)[0]), mlns_(mlns), startFromEmptyMLN_(startFromEmptyMLN), 
      outMLNFileName_(outMLNFileName), domains_(domains), 
      preds_(new Array<Predicate*>), areNonEvidPreds_(new Array<bool>),
      clauseFactory_(new ClauseFactory(maxVars, maxNumPredicates,
                     (*domains_)[0])), 
      cacheClauses_(cacheClauses), origCacheClauses_(cacheClauses), 
      cachedClauses_((cacheClauses) ? (new ClauseHashArray) : NULL), 
      cacheSizeMB_(0), maxCacheSizeMB_(maxCacheSizeMB), 
      tryAllFlips_(tryAllFlips), 
      sampleClauses_(sampleClauses), origSampleClauses_(sampleClauses),
      pll_(NULL), hasPrior_(hasPrior), priorMean_(priorMean), 
      priorStdDev_(priorStdDev), wtPredsEqually_(wtPredsEqually), 
      lbfgsb_(NULL), lbMaxIter_(lbMaxIter), lbConvThresh_(lbConvThresh), 
      looseMaxIter_(looseMaxIter), looseConvThresh_(looseConvThresh),
      beamSize_(beamSize), bestGainUnchangedLimit_(bestGainUnchangedLimit),
      numEstBestClauses_(numEstBestClauses), 
      minGain_(0), minWt_(minWt), penalty_(penalty), 
      sampleGndPreds_(sampleGndPreds), fraction_(fraction), 
      minGndPredSamples_(minGndPredSamples), 
      maxGndPredSamples_(maxGndPredSamples),
      reEvalBestCandsWithTightParams_(reEvaluateBestCandidatesWithTightParams),
    structVerbose(false) ,
      candCnt_(0), iter_(-1), bsiter_(-1), startSec_(-1), indexTrans_(NULL),
    structGradDescent_(structGradDescent), withEM_(withEM), 
    partialPrint_(partialPrint), timeBased_(timeBased), 
    maxRunTime_(maxRunTime), plusSearch(plusType), hrs_(hrs), numClauses_(numClauses), minPllGain_(minPllGain), jdebug(false)
  { 
    //cout << "Done with the init\n";
    assert(minWt_ >= 0);
    assert(domains_->size() == mlns_->size());
    
    areNonEvidPreds_->growToSize((*domains_)[0]->getNumPredicates(), false);
    for (int i = 0; i < nonEvidPredNames->size(); i++)
    {
      int predId=(*domains_)[0]->getPredicateId((*nonEvidPredNames)[i].c_str());
      if (predId < 0)
      {
        cout << "ERROR: in StructLearn::StructLearn(). Predicate " 
             << (*nonEvidPredNames)[i] << " undefined." << endl;
        exit(-1);
      }
      (*areNonEvidPreds_)[predId] = true;
    }
    //cout << "for loop\n";
    //for(int ix = 0; ix < domains_->size(); ix++){
    (*domains_)[0]->createPredicates(preds_, true);
    //}
    //cout << "Preds\n";
    if (origSampleClauses_)
    {
      ClauseSampler* cs = new ClauseSampler(delta, epsilon, minClauseSamples, 
                                            maxClauseSamples);
      //cout << "Created clause sampler\n";
      Clause::setClauseSampler(cs);
      for (int i = 0; i < domains_->size(); i++)
        (*domains_)[i]->newTrueFalseGroundingsStore();
      //cout << "2nd for loop\n";
    }    
  }

  
  ~StructLearn() 
  {
    if (pll_) delete pll_;
    if (lbfgsb_) delete lbfgsb_;
    preds_->deleteItemsAndClear();
    delete preds_;
    delete areNonEvidPreds_;
    delete clauseFactory_;
    if (cachedClauses_)
    {
      cachedClauses_->deleteItemsAndClear();
      delete cachedClauses_;
    }
    if (origSampleClauses_) delete Clause::getClauseSampler();
    if (indexTrans_) delete indexTrans_;
    for(int ix = 0; ix < addedClauses.size(); ix++){
      if (addedClauses[ix]){
	delete addedClauses[ix];
      }
    }
  }

  void run(int searchStrategy)
  {
    startSec_ = timer_.time();

    bool needIndexTrans = IndexTranslator::needIndexTranslator(*mlns_,*domains_);

      //if we are starting from an empty MLN, and including MLN clauses among 
      //the candidates in first step of beam search
    Array<Clause*> initialMLNClauses; 
    Array<ExistFormula*> existFormulas;
    if (startFromEmptyMLN_)
    {
      getMLNClauses(initialMLNClauses, existFormulas);
      removeClausesFromMLNs();
      for (int i = 0; i < initialMLNClauses.size(); i++) 
      {
        Clause* c = initialMLNClauses[i];
        c->newAuxClauseData(); 
        c->trackConstants();
        c->getAuxClauseData()->gain = 0;
        c->getAuxClauseData()->op = OP_ADD;
        c->getAuxClauseData()->removedClauseIdx = -1;
        c->getAuxClauseData()->hasBeenExpanded = false;
        c->getAuxClauseData()->lastStepExpanded = -1;
        c->getAuxClauseData()->lastStepOverMinWeight = -1;
      }
    }

      //add unit clauses to the MLN
    cout << "adding unit clauses to MLN..." << endl << endl;
    addUnitClausesToMLNs();
    //cerr << "added unit clause\n";
    //if (mln0_ == NULL){
    //  cerr << "mln0 is null\n";
    //}
    //else{
    //  cerr << "OK" << endl;
    //}
    //cerr << mln0_->getNumClauses() << endl;
      //create auxiliary data for each clause in MLN
    for (int i = 0; i < mln0_->getNumClauses(); i++) 
    {
      Clause* c = (Clause*) mln0_->getClause(i);
      c->newAuxClauseData(); 
        //only clauses that are not in any exist. quant. formula's CNF may need 
        //to be translated across domains
      if (isModifiableClause(i)) c->trackConstants();
    }

    indexTrans_ = (needIndexTrans)? new IndexTranslator(mlns_, domains_) : NULL;
    if (indexTrans_) 
      cout << "The weights of clauses in the CNFs of existential formulas wiil "
           << "be tied" << endl;
        
    //if (structGradDescent_) runStructGradDescent(initialMLNClauses, existFormulas);
    if (searchStrategy == 1){
      cout << "Beam Search - Stanley" << endl << endl;
      runStructLearning(initialMLNClauses, existFormulas);
    }
    else if (searchStrategy == 666){
      cout << "PLL weight learning" << endl << endl;
      runWeightLearning(initialMLNClauses, existFormulas);
    }
    else if (searchStrategy == 0 || searchStrategy == -1){
      cout << "Debugging :) " << endl << endl;
      debugStructLearning( initialMLNClauses, existFormulas, searchStrategy);
    }
    else if (searchStrategy == 2){
      cout << "Pick Clauses - Jesse" << endl << endl;
      greedy(initialMLNClauses, existFormulas, searchStrategy);
    }
    else if (searchStrategy == 4){
      //cout << "Patterns " << initialMLNClauses.size() << endl;
      
      patterns();
    }
    else if (searchStrategy == 14){
      jPatterns(initialMLNClauses);
    }
    else if (searchStrategy == 5 || searchStrategy == 6){
      newTransfer(initialMLNClauses, searchStrategy);
    }
    else if (searchStrategy == 7){
      cout << "New New Transfer :) " << domains_->size() << "\n";
      newNewTransfer(initialMLNClauses);
    }
    else if (searchStrategy == 8){
      cout << "tot Transfer :) " << domains_->size() << "\n";
      totTransfer(initialMLNClauses);
    }
    else if (searchStrategy == 10){
      rename(initialMLNClauses);
    }
    else if (searchStrategy == 3){
      cout << "Transfer " << initialMLNClauses.size() << endl;
      transfer(initialMLNClauses);
      for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	if (initialMLNClauses[ix]){
	  delete initialMLNClauses[ix];
	}
      }
    }
    else if (searchStrategy == 11){
      cout << "Fast Transfer " << initialMLNClauses.size() << endl;
      fastTransfer(initialMLNClauses);
      for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	if (initialMLNClauses[ix]){
	  delete initialMLNClauses[ix];
	}
      }
    }
    else if (searchStrategy == 12){
      cout << "Fast Transfer " << initialMLNClauses.size() << endl;
      superFastTransfer(initialMLNClauses);
      for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	if (initialMLNClauses[ix]){
	  delete initialMLNClauses[ix];
	}
      }
    }
    else if (searchStrategy == 13){
      cout << "Fast Likelihood " << initialMLNClauses.size() << endl;
      superFastLikelihood(initialMLNClauses);
      for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	if (initialMLNClauses[ix]){
	  delete initialMLNClauses[ix];
	}
      }
    }
    else if (searchStrategy == 20){
      cout << "Ind Super Fast Likelihood " << initialMLNClauses.size() << endl;
      indConjunctSuperFastLikelihood(initialMLNClauses);
      for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	if (initialMLNClauses[ix]){
	  delete initialMLNClauses[ix];
	}
      }
    }
    else if (searchStrategy == 15){
      cout << "Fast Likelihood Constants " << initialMLNClauses.size() << endl;
      //  Array<Clause*> plusType(Clause* clause){
      /*
      Array<Clause*> newClauses;
      for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	Array<Clause*> tmp = plusType(initialMLNClauses[ix]);
	for(int jx = 0; jx < tmp.size(); jx++){
	  newClauses.append(tmp[jx]);
	}
      }
      */
      superFastLikelihood(initialMLNClauses);
      //superFastLikelihood(newClauses);
      for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	if (initialMLNClauses[ix]){
	  delete initialMLNClauses[ix];
	}
      }
    }
    else if (searchStrategy == -13){
      cout << "Fast Transfer " << initialMLNClauses.size() << endl;
      testTransfer(initialMLNClauses);
      for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	if (initialMLNClauses[ix]){
	  delete initialMLNClauses[ix];
	}
      }
    }
    else if (searchStrategy == 15){
      cout << "Cog Transfer " << initialMLNClauses.size() << endl;
      cogTransfer(initialMLNClauses);
      for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	if (initialMLNClauses[ix]){
	  delete initialMLNClauses[ix];
	}
      }
    }
    else if (searchStrategy == 16){
      cout << "2nd order formuals " << initialMLNClauses.size() << endl;
      buildStates(initialMLNClauses);
      for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	if (initialMLNClauses[ix]){
	  delete initialMLNClauses[ix];
	}
      }
    }
    else if (searchStrategy == 17){
     cout << "2nd order formuals " << initialMLNClauses.size() << endl;
     getHier(initialMLNClauses,3,1);
     for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	if (initialMLNClauses[ix]){
	  delete initialMLNClauses[ix];
	}
      }
    }
    else if (searchStrategy == 18){
     cout << "2nd order formulas " << initialMLNClauses.size() << endl;
     getHier(initialMLNClauses,3);
     for(int ix = 0; ix < initialMLNClauses.size(); ix++){
	  if (initialMLNClauses[ix]){
	   delete initialMLNClauses[ix];
	  }
     }
    }
    else if (searchStrategy == 30) {
      cout << "Formula transfer - Jan" << endl << endl;
      newGreedy(initialMLNClauses, existFormulas, searchStrategy);
    }
    else if (searchStrategy == 31) {
      cout << "Template transfer - Jan" << endl << endl;
      newGreedyTemplate(initialMLNClauses, existFormulas, searchStrategy);
    }
    else if (searchStrategy == 998) {
      cout << "Clause Learn - Jan" << endl << endl;
      greedyLearn(initialMLNClauses, existFormulas, searchStrategy);
    }
    else if (searchStrategy == 999) {
      cout << "Clause Evaluation - Jan" << endl << endl;
      greedyEvaluation(initialMLNClauses, existFormulas, searchStrategy);
    }
  }

  void rename(Array<Clause*> initClauses){

    cout << "Here 1 has " << initClauses.size() << "\n";
    for(int ix = 0; ix < initClauses.size(); ix++){
      cout << "Here 1\n";
      IntHashArray tmp;    
      ArraysAccessor<int> pred;
      ArraysAccessor<int> vars;
      Array<int>* pids = new Array<int>;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      for(int jx = 0; jx < preds->size(); jx++){
	cout << (*preds)[jx]->getId() << "(";
	const PredicateTemplate* ptmp = (*preds)[jx]->getTemplate();
	const Array<int>* termTypes =  ptmp->getTermTypesAsInt();
	for(int kx = 0; kx < termTypes->size(); kx++){
	  cout << (*termTypes)[kx];
	  if (kx < termTypes->size()-1){
	    cout << ",";
	  }
	}
	cout << ") ";
	if (!tmp.contains((*preds)[jx]->getId())){
	  tmp.append((*preds)[jx]->getId());
	  pids->append((*preds)[jx]->getId());
	}

      }
      cout << endl;
      for(int jx = 0; jx < tmp.size(); jx++){
	pred.appendArray(pids);
      }
      int smallestVarId;
      Array<Array<int>*>* varArrays = initClauses[ix]->getTypeIdToVarIdsMapAndSmallestVarId(smallestVarId);
      cout << pids->size() << " " << varArrays->size() << " size\n";
      for(int jx = 0; jx < varArrays->size(); jx++){
	if ((*varArrays)[jx] != NULL){
	  cout << "type " << jx << " " << (*varArrays)[jx]->size() << " size";
	  for(int kx = 0; kx < (*varArrays)[jx]->size(); kx++){
	    vars.appendArray((*varArrays)[jx]);
	    cout << (*(*varArrays)[jx])[kx] << " ";
	  }
	  cout << endl;
	}
      }
      Array<int> predMap;
      Array<int> varMap;
      Array<int> varBase;
      vars.getDistinctNextCombination(varMap);
      varBase.growToSize(varMap.size());
      for(int jx = 0; jx < varMap.size(); jx++){
	varBase[jx] = varMap[jx];
      }
      vars.reset();
      while (pred.hasDistinctNextCombination()){
	pred.getDistinctNextCombination(predMap);
	bool legal = true;
	for(int jx = 0; jx < predMap.size(); jx++){
	  cout << predMap[jx] << " ";
	}
	for(int jx = 0; jx < predMap.size() && legal; jx++){
	  const PredicateTemplate* ptmp1 = (*domains_)[0]->getPredicateTemplate((*pids)[jx]);
	  const PredicateTemplate* ptmp2 = (*domains_)[0]->getPredicateTemplate(predMap[jx]);
	  const Array<int>* termTypes1 =  ptmp1->getTermTypesAsInt();
	  const Array<int>* termTypes2 =  ptmp2->getTermTypesAsInt();
	  if ((((*termTypes1)[0] == (*termTypes1)[1]) &&
	       ((*termTypes2)[0] != (*termTypes2)[1]))||
	      (((*termTypes1)[0] != (*termTypes1)[1]) &&
	       ((*termTypes2)[0] == (*termTypes2)[1]))){
	    legal = false;
	    cout << " illegal ";

	    
	  }
	  
	}
	if (legal){
	  cout << "patterns";
	}
	cout << endl;
	if (legal){
	  vars.reset();
	  cout << "here\n";
	  while (vars.hasDistinctNextCombination()){
	    vars.getDistinctNextCombination(varMap);
	    /*
	    for(int jx = 0; jx < predMap.size(); jx++){
	      cout << predMap[jx] << " ";
	    }
	    cout << "<> ";
	    for(int jx = 0; jx < varMap.size(); jx++){
	      cout << varMap[jx] << " ";
	    }
	    */
	    cout << "RENAMED:";
	    for(int jx = 0; jx < preds->size(); jx++){
	      int currId = (*preds)[jx]->getId();
	      bool found = false;
	      for(int kx = 0; kx < pids->size(); kx++){
		if ((*pids)[kx] == currId){
		  found = true;
		  const PredicateTemplate* ptmp1 = (*domains_)[0]->getPredicateTemplate(predMap[kx]);
		  cout << ptmp1->getName() << "(";
		}
	      }
	      for(int tx = 0; tx < (*preds)[jx]->getNumTerms(); tx++){
		bool found = false;
		int termId = (*preds)[jx]->getTerm(tx)->getId();
		for(int kx = 0; kx < varBase.size(); kx++){
		  if (varBase[kx] == termId){
		    found = true;
		    cout << varMap[kx];
		    //const PredicateTemplate* ptmp1 = (*domains_)[0]->getPredicateTemplate((*pids)[jx]);
		    //cout << ptmp1->getName() << "(";
		    if (tx < (*preds)[jx]->getNumTerms()-1){
		      cout << ",";
		    }
		  }
		}
	      }
	      cout << ")";
	      if (jx < preds->size() -1){
		cout << " v ";
	      }
	    }
	    cout << endl;
	  }
	}
      }
    }
  }

  void jPatterns(Array<Clause*> jClause){
    Array<Clause*> initClauses;
    for (int i = 0; i < addedClauses.size(); i++){
      Clause* c = addedClauses[i];
      c->newAuxClauseData();
      c->getAuxClauseData()->reset();
      if (isModifiableClause(i))
        {
          c->trackConstants();
          initClauses.append(new Clause(*c));
        }
    }

    cout << "J clause size " << jClause.size() << endl;
    ClauseOpHashArray* beam = new ClauseOpHashArray;
    for (int i = 0; i < jClause.size(); i++){
      beam->append(jClause[i]);
    }
    StringHashArray patterns;

    Array<Clause*> candidates;
    for(int px = 1; px < clauseFactory_->getMaxNumPredicates(); px++){
      candidates.clear();
      cout << beam->size() << " beam size\n";
      createCandidateClauses(beam, candidates, NULL);

      
      cout << "Created: " << candidates.size() << " candidates\n";
      for(int ix = 0; ix < candidates.size(); ix++){
        cout << ix << " " << candidates[ix]->printWithoutWt(cout, (*domains_)[0]);
	AuxClauseData* beamacd = candidates[ix]->getAuxClauseData();
	int op = beamacd->op;
	cout << "\t" << "op = " << op << endl;
            
        if (candidates[ix] == NULL){

	}
      }
    }
  }

  void patterns(){
    Array<Clause*> initClauses;
    for (int i = 0; i < addedClauses.size(); i++){
      Clause* c = addedClauses[i];
      c->newAuxClauseData();
      c->getAuxClauseData()->reset();
      if (isModifiableClause(i)) 
	{
	  c->trackConstants();
	  initClauses.append(new Clause(*c));
	}
    }
  

    ClauseOpHashArray* beam = new ClauseOpHashArray;
    for (int i = 0; i < initClauses.size(); i++){
      beam->append(initClauses[i]);
    }
    StringHashArray patterns;
   
    Array<Clause*> candidates;
    for(int px = 1; px < clauseFactory_->getMaxNumPredicates(); px++){
      candidates.clear();
      cout << beam->size() << " beam size\n";
      createCandidateClauses(beam, candidates, NULL);
      
      int removed = 0;
      cout << "Created: " << candidates.size() << " candidates\n";
      for(int ix = 0; ix < candidates.size(); ix++){
	//cout << ix << " " << candidates.size() << endl;
	if (candidates[ix] == NULL){
	  cerr << "Null candidate\n";
	}
	candidates[ix]->canonicalize();
	//Flag find mln      
	const Array<Predicate*>* preds = candidates[ix]->getPredicates();
	bool hasNegation = false;
	for(int jx = 0; jx < preds->size(); jx++){
	  if (!(*preds)[jx]->getSense() ||
	      (*preds)[jx]->isEqualPred()){
	    hasNegation = true;
	  }
	}
	if (hasNegation){
	  delete candidates[ix];
	  candidates[ix] = NULL;
	  removed++;
	}
      }
      candidates.removeAllNull();
      cout << "Removed: " << removed << endl;
      for(int ix = 0; ix < candidates.size(); ix++){
	const Array<Predicate*>* preds = candidates[ix]->getPredicates();
	IntHashArray tmp;
	int dup = -1;
	for(int jx = 0; jx < preds->size(); jx++){
	  if (tmp.contains((*preds)[jx]->getId())){
	    dup = (*preds)[jx]->getId();
	  }
	  tmp.append((*preds)[jx]->getId());
	  
	}
	cout << "<clause>";
	if (tmp.size() == 2 && preds->size() >= 3){
	  Clause* c1 = new Clause();
	  int last = -1;
	  for(int jx = 0; jx < preds->size(); jx++){
	    if ((*preds)[jx]->getId() == dup){
	      c1->appendPredicate(new Predicate(*(*preds)[jx]));
	      
	    }
	    else{
	      last = jx;
	    }
	  }
	  //candidates[ix]->printWithoutWt(cout,(*domains_)[0]);
	  //cout << " ";
	  c1->appendPredicate(new Predicate(*(*preds)[last]));
	  c1->canonicalize(false);
	  
	  c1->printWithoutWt(cout, (*domains_)[0]);
	  AuxClauseData* beamacd = candidates[ix]->getAuxClauseData();
          int op = beamacd->op;
          cout << "\t" << "op = " << op;

	  cout << "\t" << "type" << tmp.size() << "-pat";
	  string patType = "type" + Util::intToString(tmp.size()) + "-pat";
	  const Array<Predicate*>* cpreds = c1->getPredicates();
	  for(int jx = 0; jx < cpreds->size(); jx++){
	    for(int kx = 0; kx < (*cpreds)[jx]->getNumTerms(); kx++){
	      const Term* term = (*cpreds)[jx]->getTerm(kx);
	      cout << term->getId();
	      patType = patType + Util::intToString(term->getId());
	    }
	  }
	  patterns.append(patType);
	  cout << "\t";
	  preds = c1->getPredicates();
	  for(int jx = 0; jx < preds->size(); jx++){
	    if (jx < (preds->size()-1)){
	      cout << "p(";
	    }
	    else{
	      cout << "q(";
	    }
	    for(int kx = 0; kx < (*preds)[jx]->getNumTerms(); kx++){
	      const Term* term = (*preds)[jx]->getTerm(kx);
	      cout << term->getId();
	      if (kx < ((*preds)[jx]->getNumTerms()-1)){
		cout << ",";
	      }
	    }
	    cout << ")";
	    if (jx < (preds->size() -1)){
	      cout << " v ";
	    }
	  }
	  cout << "\t";
	  IntHashArray letter; 
	  int letterIdx = -1;
	  
	  for(int jx = 0; jx < preds->size(); jx++){
	    if (!letter.contains((*preds)[jx]->getId())){
	      letter.append((*preds)[jx]->getId());
	      letterIdx++;
	      if (letterIdx == 0){
		cout << "p(";
	      }
	      else{
		cout << "<>q(";
	      }
	      const Array<int>* termType = (*preds)[jx]->getTemplate()->getTermTypesAsInt();
	      for(int kx = 0; kx < termType->size(); kx++){
		cout << "type" << (*termType)[kx];
		if (kx < (termType->size() -1)){
		  cout << ",";
		}
	      }
	      cout << ")";
	    }
	  }
	  cout << endl;
	  delete c1;
	}
	else{// if (false){
	  candidates[ix]->printWithoutWt(cout, (*domains_)[0]);
	  AuxClauseData* beamacd = candidates[ix]->getAuxClauseData();
	  int op = beamacd->op;
	  cout << "\t" << "op = " << op;
	  cout << "\t" << "type" << tmp.size() << "-pat";
	  string patType = "type" + Util::intToString(tmp.size()) + "-pat";
	  for(int jx = 0; jx < preds->size(); jx++){
	    for(int kx = 0; kx < (*preds)[jx]->getNumTerms(); kx++){
	      const Term* term = (*preds)[jx]->getTerm(kx);
	      cout << term->getId();
	      patType = patType + Util::intToString(term->getId());
	    }
	  }
	  patterns.append(patType);
	  cout << "\t"; 
	  IntHashArray letter; 
	  int letterIdx = -1;
	  for(int jx = 0; jx < preds->size(); jx++){
	    if (!letter.contains((*preds)[jx]->getId())){
	      letterIdx++;
	      letter.append((*preds)[jx]->getId());
	    }
	    if (letterIdx == 0){
	      cout << "p(";
	    }
	    else if (letterIdx == 1){
	      cout << "q(";
	    }
	    else if (letterIdx == 2){
	      cout << "r(";
	    }
	    else if (letterIdx == 3){
	      cout << "s(";
	    }
	    else if (letterIdx == 4){
	      cout << "t(";
	    }
	    else{
	      cout << "u(";
	    }

	    /*
	    if (jx == 0 || tmp.size() == 1){
	      cout << "p(";
	    }
	    else if (jx == 1){
	      cout << "q(";
	    }
	    else{
	      cout << "r(";
	    }
	    */
	    for(int kx = 0; kx < (*preds)[jx]->getNumTerms(); kx++){
	      const Term* term = (*preds)[jx]->getTerm(kx);
	      cout << term->getId();
	      if (kx < ((*preds)[jx]->getNumTerms()-1)){
		cout << ",";
	      }
	    }
	    cout << ")";
	    if (jx < (preds->size()-1)){
	      cout << " v ";
	    }
	  }
	  IntHashArray letter1; 
	  letterIdx = -1;
	  cout << "\t";
	  for(int jx = 0; jx < preds->size(); jx++){
	    if (!letter1.contains((*preds)[jx]->getId())){
	      letterIdx++;
	      letter1.append((*preds)[jx]->getId());
	    
	      if (letterIdx == 0){
		cout << "p(";
	      }
	      else if (letterIdx == 1){
		cout << "<>q(";
	      }
	      else if (letterIdx == 2){
		cout << "<>r(";
	      }
	      else if (letterIdx == 3){
		cout << "<>s(";
	      }
	      else if (letterIdx == 4){
		cout << "<>t(";
	      }
	      else{
		cout << "<>u(";
	      }

	      const Array<int>* termType = (*preds)[jx]->getTemplate()->getTermTypesAsInt();
	      for(int kx = 0; kx < termType->size(); kx++){
		cout << "type" << (*termType)[kx];
		if (kx < (termType->size() -1)){
		  cout << ",";
		}
	      }
	      cout << ")";

	    }
	  }

	  cout << endl;

	}
      }
      beam->deleteItemsAndClear();
      for(int ix = 0; ix < candidates.size(); ix++){
	beam->append(candidates[ix]);
      }
    }
    cout << "Generated " << patterns.size() << " patterns\n";
    for(int ix = 0; ix < patterns.size(); ix++){
      cout << "<pattern>" << patterns[ix] << endl;
    }
    
  }


  void patterns1(){
    Array<Clause*> initClauses;
    for (int i = 0; i < addedClauses.size(); i++){
      Clause* c = addedClauses[i];
      c->newAuxClauseData();
      c->getAuxClauseData()->reset();
      if (isModifiableClause(i)) 
	{
	  c->trackConstants();
	  initClauses.append(new Clause(*c));
	}
    }
  

    ClauseOpHashArray* beam = new ClauseOpHashArray;
    for (int i = 0; i < initClauses.size(); i++){
      beam->append(initClauses[i]);
    }
    StringHashArray patterns;
   
    Array<Clause*> candidates;
    for(int px = 1; px < clauseFactory_->getMaxNumPredicates(); px++){
      candidates.clear();
      createCandidateClauses(beam, candidates, NULL);
      
      int removed = 0;
      cout << "Created: " << candidates.size() << " candidates\n";
      for(int ix = 0; ix < candidates.size(); ix++){
	//cout << ix << " " << candidates.size() << endl;
	if (candidates[ix] == NULL){
	  cerr << "Null candidate\n";
	}
	candidates[ix]->canonicalize();
	//Flag find mln      
	const Array<Predicate*>* preds = candidates[ix]->getPredicates();
	bool hasNegation = false;
	for(int jx = 0; jx < preds->size(); jx++){
	  if (!(*preds)[jx]->getSense() ||
	      (*preds)[jx]->isEqualPred()){
	    hasNegation = true;
	  }
	}
	if (hasNegation){
	  delete candidates[ix];
	  candidates[ix] = NULL;
	  removed++;
	}
      }
      candidates.removeAllNull();
      cout << "Removed: " << removed << endl;
      for(int ix = 0; ix < candidates.size(); ix++){
	const Array<Predicate*>* preds = candidates[ix]->getPredicates();
	IntHashArray tmp;
	int dup = -1;
	for(int jx = 0; jx < preds->size(); jx++){
	  if (tmp.contains((*preds)[jx]->getId())){
	    dup = (*preds)[jx]->getId();
	  }
	  tmp.append((*preds)[jx]->getId());
	  
	}
	cout << "<clause>";
	if (tmp.size() == 2 && preds->size() >= 3){
	  Clause* c1 = new Clause();
	  int last = -1;
	  for(int jx = 0; jx < preds->size(); jx++){
	    if ((*preds)[jx]->getId() == dup){
	      c1->appendPredicate(new Predicate(*(*preds)[jx]));
	      
	    }
	    else{
	      last = jx;
	    }
	  }
	  //candidates[ix]->printWithoutWt(cout,(*domains_)[0]);
	  //cout << " ";
	  c1->appendPredicate(new Predicate(*(*preds)[last]));
	  c1->canonicalize(false);
	  
	  c1->printWithoutWt(cout, (*domains_)[0]);
	  cout << "\t" << "type" << tmp.size() << "-pat";
	  string patType = "type" + Util::intToString(tmp.size()) + "-pat";
	  const Array<Predicate*>* cpreds = c1->getPredicates();
	  for(int jx = 0; jx < cpreds->size(); jx++){
	    for(int kx = 0; kx < (*cpreds)[jx]->getNumTerms(); kx++){
	      const Term* term = (*cpreds)[jx]->getTerm(kx);
	      cout << term->getId();
	      patType = patType + Util::intToString(term->getId());
	    }
	  }
	  patterns.append(patType);
	  cout << "\t";
	  preds = c1->getPredicates();
	  for(int jx = 0; jx < preds->size(); jx++){
	    if (jx < (preds->size()-1)){
	      cout << "p(";
	    }
	    else{
	      cout << "q(";
	    }
	    for(int kx = 0; kx < (*preds)[jx]->getNumTerms(); kx++){
	      const Term* term = (*preds)[jx]->getTerm(kx);
	      cout << term->getId();
	      if (kx < ((*preds)[jx]->getNumTerms()-1)){
		cout << ",";
	      }
	    }
	    cout << ")";
	    if (jx < (preds->size() -1)){
	      cout << " v ";
	    }
	  }
	  cout << "\t";
	  IntHashArray letter; 
	  int letterIdx = -1;
	  
	  for(int jx = 0; jx < preds->size(); jx++){
	    if (!letter.contains((*preds)[jx]->getId())){
	      letter.append((*preds)[jx]->getId());
	      letterIdx++;
	      if (letterIdx == 0){
		cout << "p(";
	      }
	      else{
		cout << "<>q(";
	      }
	      const Array<int>* termType = (*preds)[jx]->getTemplate()->getTermTypesAsInt();
	      for(int kx = 0; kx < termType->size(); kx++){
		cout << "type" << (*termType)[kx];
		if (kx < (termType->size() -1)){
		  cout << ",";
		}
	      }
	      cout << ")";
	    }
	  }
	  cout << endl;
	  delete c1;
	}
	else{// if (false){
	  candidates[ix]->printWithoutWt(cout, (*domains_)[0]);
	  cout << "\t" << "type" << tmp.size() << "-pat";
	  string patType = "type" + Util::intToString(tmp.size()) + "-pat";
	  for(int jx = 0; jx < preds->size(); jx++){
	    for(int kx = 0; kx < (*preds)[jx]->getNumTerms(); kx++){
	      const Term* term = (*preds)[jx]->getTerm(kx);
	      cout << term->getId();
	      patType = patType + Util::intToString(term->getId());
	    }
	  }
	  patterns.append(patType);
	  cout << "\t"; 
	  IntHashArray letter; 
	  int letterIdx = -1;
	  for(int jx = 0; jx < preds->size(); jx++){
	    if (!letter.contains((*preds)[jx]->getId())){
	      letterIdx++;
	      letter.append((*preds)[jx]->getId());
	    }
	    if (letterIdx == 0){
	      cout << "p(";
	    }
	    else if (letterIdx == 1){
	      cout << "q(";
	    }
	    else if (letterIdx == 2){
	      cout << "r(";
	    }
	    else if (letterIdx == 3){
	      cout << "s(";
	    }
	    else if (letterIdx == 4){
	      cout << "t(";
	    }
	    else{
	      cout << "u(";
	    }

	    /*
	    if (jx == 0 || tmp.size() == 1){
	      cout << "p(";
	    }
	    else if (jx == 1){
	      cout << "q(";
	    }
	    else{
	      cout << "r(";
	    }
	    */
	    for(int kx = 0; kx < (*preds)[jx]->getNumTerms(); kx++){
	      const Term* term = (*preds)[jx]->getTerm(kx);
	      cout << term->getId();
	      if (kx < ((*preds)[jx]->getNumTerms()-1)){
		cout << ",";
	      }
	    }
	    cout << ")";
	    if (jx < (preds->size()-1)){
	      cout << " v ";
	    }
	  }
	  IntHashArray letter1; 
	  letterIdx = -1;
	  cout << "\t";
	  for(int jx = 0; jx < preds->size(); jx++){
	    if (!letter1.contains((*preds)[jx]->getId())){
	      letterIdx++;
	      letter1.append((*preds)[jx]->getId());
	    
	      if (letterIdx == 0){
		cout << "p(";
	      }
	      else if (letterIdx == 1){
		cout << "<>q(";
	      }
	      else if (letterIdx == 2){
		cout << "<>r(";
	      }
	      else if (letterIdx == 3){
		cout << "<>s(";
	      }
	      else if (letterIdx == 4){
		cout << "<>t(";
	      }
	      else{
		cout << "<>u(";
	      }

	      const Array<int>* termType = (*preds)[jx]->getTemplate()->getTermTypesAsInt();
	      for(int kx = 0; kx < termType->size(); kx++){
		cout << "type" << (*termType)[kx];
		if (kx < (termType->size() -1)){
		  cout << ",";
		}
	      }
	      cout << ")";

	    }
	  }

	  cout << endl;

	}
      }
      beam->deleteItemsAndClear();
      for(int ix = 0; ix < candidates.size(); ix++){
	beam->append(candidates[ix]);
      }
    }
    cout << "Generated " << patterns.size() << " patterns\n";
    for(int ix = 0; ix < patterns.size(); ix++){
      cout << "<pattern>" << patterns[ix] << endl;
    }
    
  }

  void buildClique(Clause* clause, Array<double>* probs, Array<int>* idx, 
		   ClauseHashArray* clauses){
    const Array<Predicate*>* preds = clause->getPredicates();
    int numNonEqual = 0;
    ArraysAccessor<int> acc;
    Array<int> pos;
    for(int jx = 0; jx < preds->size(); jx++){
      if (!(*preds)[jx]->isEqualPred()){
	numNonEqual++;
	pos.append(jx);
	Array<int>* vals = new Array<int>;
	vals->append(0);
	vals->append(1);
	acc.appendArray(vals);
      }
    }
    Array<double> trueCnts;
    Array<double> possCnts;
    ClauseHashArray cliques;
    Array<int> sense;
    //Array<double>* probs = new Array<double>;
    while(acc.getNextCombination(sense)){
      Clause* c1 = new Clause(*clause);
      for(int jx = 0; jx < pos.size(); jx++){
	Predicate* pred = c1->getPredicate(pos[jx]);
	if (sense[jx] == 0){
	  pred->setSense(false);
	}
	else{
	  pred->setSense(true);
	}
      }
      c1->setDirty();
      c1->canonicalize();
      if (cliques.contains(c1)){
	/*
	  cout << "Dup: ";
	  c1->printWithoutWt(cout, (*domains_)[0]);
	  cout << endl;
	*/
	//fix this here please
	//idx->append(cliques.find(c1));
	(*idx)[cliques.find(c1)]++;
	delete c1;
	continue;
      }
      cliques.append(c1);
      clauses->append(c1);
      idx->append(1);//cliques.find(c1));

      double ntg_c1 = 1;
      double ng_c1 = 0; //c1->getNumDistinctGroundings((*domains_)[0]);      
      for(int dx = 0; dx < domains_->size(); dx++){
	ntg_c1 += c1->getNumTrueGroundingsAnd((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	ng_c1 += c1->getNumDistinctGroundings((*domains_)[dx]);      
      }
      c1->printWithoutWt(cout, (*domains_)[0]);
      cout << " " << ntg_c1 << endl;//" " << ng_c1 << endl;
      trueCnts.append(ntg_c1);
      possCnts.append(ng_c1);
      probs->append(ntg_c1);//(ntg_c1/ng_c1));
    }
    //cliques.deleteItemsAndClear();

    //Do calculations
    double norm = 0;
    for(int ix = 0; ix < probs->size(); ix++){
      norm += (*probs)[ix];
    }
    for(int ix = 0; ix < probs->size(); ix++){
      (*probs)[ix] = (*probs)[ix] / norm;
      
      cliques[ix]->printWithoutWt(cout, (*domains_)[0]);
      cout << " " << (*probs)[ix] << endl;
      
    }
    cout << endl;
    acc.deleteArraysAndClear();
  }
  void buildCliqueFast(Clause* clause, Array<double>* probs, Array<int>* idx, 
		       ClauseHashArray* clauses){
    const Array<Predicate*>* preds = clause->getPredicates();
    int numNonEqual = 0;
    ArraysAccessor<int> acc;
    Array<int> pos;
    for(int jx = 0; jx < preds->size(); jx++){
      if (!(*preds)[jx]->isEqualPred()){
	numNonEqual++;
	pos.append(jx);
	Array<int>* vals = new Array<int>;
	vals->append(0);
	vals->append(1);
	acc.appendArray(vals);
      }
    }

    Array<Array<double>*> counts;
    for(int dx = 0; dx < domains_->size(); dx++){
      Array<double>* dCounts = clause->templateGroundings((*domains_)[dx], (*domains_)[dx]->getDB());
      counts.append(dCounts);
    }
    
    Array<double> trueCnts;
    ClauseHashArray cliques;
    Array<int> sense;
    while(acc.getNextCombination(sense)){
      Clause* c1 = new Clause(*clause);
      int index = 0;
      for(int jx = 0; jx < pos.size(); jx++){
	Predicate* pred = c1->getPredicate(pos[jx]);
	if (sense[jx] == 0){
	  pred->setSense(false);
	}
	else{
	  index += Clause::pow(2, (pos.size() - jx - 1));
	  pred->setSense(true);
	}
      }
      c1->setDirty();
      c1->canonicalize();
      //c1->printWithoutWt(cout, (*domains_)[0]);
      //cout << endl;
      if (cliques.contains(c1)){
	(*idx)[cliques.find(c1)]++;
	delete c1;
	continue;
      }
      cliques.append(c1);
      clauses->append(c1);
      idx->append(1);//cliques.find(c1));
      
      double ntg_c1 = 0;
      for(int dx = 0; dx < counts.size(); dx++){
	
	ntg_c1 += (*(counts[dx]))[index];
	/*
	cout << "ntg_c1 " << ntg_c1 << " dx ";
	cout << dx << " index " << index << " value ";
	cout << (*(counts[dx]))[index] << endl;
	*/
      }
      ntg_c1 = ntg_c1 - (counts.size()-1);
      //double ng_c1 = 0; //c1->getNumDistinctGroundings((*domains_)[0]); 
      //cerr << "here\n";
      c1->printWithoutWt(cout, (*domains_)[0]);
      cout << " " << ntg_c1 << endl;//" " << ng_c1 << endl;
      trueCnts.append(ntg_c1);
      //possCnts.append(ng_c1);
      probs->append(ntg_c1);//(ntg_c1/ng_c1));
    }
    //cliques.deleteItemsAndClear();

    //Do calculations
    double norm = 0;
    for(int ix = 0; ix < probs->size(); ix++){
      norm += (*probs)[ix];
    }
    for(int ix = 0; ix < probs->size(); ix++){
      (*probs)[ix] = (*probs)[ix] / norm;
      
      cliques[ix]->printWithoutWt(cout, (*domains_)[0]);
      cout << " " << (*probs)[ix] << endl;
      
    }
    cout << endl;
    acc.deleteArraysAndClear();
    //counts.deleteItemsAndClear();
  }

  //fast
  void fastTransfer(Array<Clause*> initClauses){
    double maxScore = 0;
    string fname = outMLNFileName_;
    ofstream out(fname.c_str());

    if (!out.good()) { cout << "ERROR: failed to open " <<fname<<endl;exit(-1);}


    for(int ix = 0; ix < initClauses.size(); ix++){
      //Flag Eval Time
      double begSec = timer_.time();
      out << "Evaluate: ";
      initClauses[ix]->printWithoutWt(out, (*domains_)[0]);
      out << endl;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      int numNonEqual = 0;
      ArraysAccessor<int> acc;
      Array<int> pos;
      for(int jx = 0; jx < preds->size(); jx++){
	if (!(*preds)[jx]->isEqualPred()){
	  numNonEqual++;
	  pos.append(jx);
	  Array<int>* vals = new Array<int>;
	  vals->append(0);
	  vals->append(1);
	  acc.appendArray(vals);
	}
      }
      Array<Array<double>*> dProbs1;
      Array<Array<int>*> idx1;
      Array<Array<double>*> dProbs2;
      Array<Array<int>*> idx2;
      Array<ClauseHashArray*> clause1;
      Array<ClauseHashArray*> clause2;
      if (numNonEqual == 2){
	Clause* d1 = new Clause();
	Clause* d2 = new Clause();
	Predicate* p1 = new Predicate(*(*preds)[pos[0]]);
	Predicate* p2 = new Predicate(*(*preds)[pos[1]]);
	
	d1->appendPredicate(p1);
	d2->appendPredicate(p2);
	d1->canonicalize();
	d2->canonicalize();
	Array<double>* tmpDouble = new Array<double>;
	Array<int>* tmpInt = new Array<int>;
	ClauseHashArray* tmpClause = new ClauseHashArray;
	buildCliqueFast(d1, tmpDouble, tmpInt, tmpClause);
	dProbs1.append(tmpDouble);
	idx1.append(tmpInt);
	clause1.append(tmpClause);
	tmpDouble = new Array<double>;
	tmpInt = new Array<int>;
	tmpClause = new ClauseHashArray;
	buildCliqueFast(d2, tmpDouble, tmpInt, tmpClause);
	dProbs2.append(tmpDouble);
	idx2.append(tmpInt);
	clause2.append(tmpClause);
	delete d1;
	delete d2;
      }
      else if (numNonEqual == 3){

	for(int jx = 0; jx < 3; jx++){
	  Clause* d1 = new Clause();
	  Clause* d2 = new Clause();
	  Predicate* p1 = new Predicate(*(*preds)[pos[0]]);
	  Predicate* p2 = new Predicate(*(*preds)[pos[1]]);
	  Predicate* p3 = new Predicate(*(*preds)[pos[2]]);
	    if (jx == 0){
	      d1->appendPredicate(p1);
	      d1->appendPredicate(p2);
	      d2->appendPredicate(p3);
	    }
	    else if (jx == 1){
	      d1->appendPredicate(p1);
	      d1->appendPredicate(p3);
	      d2->appendPredicate(p2);
	    }
	    else{
	      d1->appendPredicate(p2);
	      d1->appendPredicate(p3);
	      d2->appendPredicate(p1);
	    }
	

	  d1->canonicalize();
	  d2->canonicalize();
	  Array<double>* tmpDouble = new Array<double>;
	  Array<int>* tmpInt = new Array<int>;
	  ClauseHashArray* tmpClause = new ClauseHashArray;
	  buildCliqueFast(d1, tmpDouble, tmpInt, tmpClause);
	  dProbs1.append(tmpDouble);
	  idx1.append(tmpInt);
	  clause1.append(tmpClause);
	  tmpDouble = new Array<double>;
	  tmpInt = new Array<int>;
	  tmpClause = new ClauseHashArray;
	  buildCliqueFast(d2, tmpDouble, tmpInt, tmpClause);
	  dProbs2.append(tmpDouble);
	  idx2.append(tmpInt);
	  clause2.append(tmpClause);
	  delete d1;
	  delete d2;
	}
      }
      else{
	for(int jx = 0; jx < 7; jx++){
	  Clause* d1 = new Clause();
	  Clause* d2 = new Clause();
	  Predicate* p1 = new Predicate(*(*preds)[pos[0]]);
	  Predicate* p2 = new Predicate(*(*preds)[pos[1]]);
	  Predicate* p3 = new Predicate(*(*preds)[pos[2]]);
	  Predicate* p4 = new Predicate(*(*preds)[pos[3]]);
	  if (jx == 0){
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p2);
	    d1->appendPredicate(p3);
	    d2->appendPredicate(p4);
	  }
	  else if (jx == 1){
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p2);
	    d1->appendPredicate(p4);
	    d2->appendPredicate(p3);
	  }
	  else if (jx == 2){
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p3);
	    d1->appendPredicate(p4);
	    d2->appendPredicate(p2);
	  }
	  else if (jx == 3){
	    d1->appendPredicate(p2);
	    d1->appendPredicate(p3);
	    d1->appendPredicate(p4);
	    d2->appendPredicate(p1);
	  }
	  else if (jx == 4){
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p2);
	    d2->appendPredicate(p3);
	    d2->appendPredicate(p4);
	  }
	  else if (jx == 5){
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p3);
	    d2->appendPredicate(p2);
	    d2->appendPredicate(p4);
	  }
	  else{
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p4);
	    d2->appendPredicate(p2);
	    d2->appendPredicate(p3);
	  }
	  

	  d1->canonicalize();
	  /*
	  if (jx < 4){
	    cout << "jx: ";
	    d1->printWithoutWt(cout, (*domains_)[0]);
	    cout << endl;
	  }
	  */
	  d2->canonicalize();
	  Array<double>* tmpDouble = new Array<double>;
	  Array<int>* tmpInt = new Array<int>;
	  ClauseHashArray* tmpClause = new ClauseHashArray;
	  buildCliqueFast(d1, tmpDouble, tmpInt, tmpClause);
	  dProbs1.append(tmpDouble);
	  idx1.append(tmpInt);
	  clause1.append(tmpClause);
	  tmpDouble = new Array<double>;
	  tmpInt = new Array<int>;
	  tmpClause = new ClauseHashArray;
	  buildCliqueFast(d2, tmpDouble, tmpInt, tmpClause);
	  dProbs2.append(tmpDouble);
	  idx2.append(tmpInt);
	  clause2.append(tmpClause);
	  delete d1;
	  delete d2;
	}
      }

      Array<double> trueCnts;
      Array<double> possCnts;
      Array<double> probs;
      Array<int> sense;
      double sum = 0;
      double total = 0;
      ClauseHashArray cliques;
      Array<Clause*> forDecomp;
      Array<ClauseHashArray*> decomp;
      Array<int> mult;
      Array<Array<double>*> counts;
      for(int dx = 0; dx < domains_->size(); dx++){
	Array<double>* dCounts = initClauses[ix]->templateGroundings((*domains_)[dx], (*domains_)[dx]->getDB());
	counts.append(dCounts);
      }
    /*
    */

      while(acc.getNextCombination(sense)){
	Clause* c1 = new Clause(*initClauses[ix]);
	int index = 0;
	for(int jx = 0; jx < pos.size(); jx++){
	  Predicate* pred = c1->getPredicate(pos[jx]);
	  if (sense[jx] == 0){
	    pred->setSense(false);
	  }
	  else{
	    index += Clause::pow(2, (pos.size() - jx - 1));
	    pred->setSense(true);
	  }
	}
	Clause* r1 = new Clause(*c1);
	c1->setDirty();
	c1->canonicalize();
	if (cliques.contains(c1)){
	  int dupIndex = cliques.find(c1);
	  mult[dupIndex]++;
	  delete c1;
	  continue;
	}
	cliques.append(c1);
	forDecomp.append(r1);
	mult.append(1);

	double ntg_c1 = 0;//c1->getNumTrueGroundings((*domains_)[0], (*domains_)[0]->getDB(), false);
	double ng_c1 = 0;// c1->getNumDistinctGroundings((*domains_)[0]);

	for(int dx = 0; dx < counts.size(); dx++){
	  ntg_c1 += (*(counts[dx]))[index];
	}
	ntg_c1 = ntg_c1 - (counts.size()-1);
	/*
	for(int dx = 0; dx < domains_->size(); dx++){
	  ntg_c1 += c1->getNumTrueGroundingsAnd((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	  ng_c1 = c1->getNumDistinctGroundings((*domains_)[dx]);      
	}
	*/
	c1->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << ntg_c1 << endl;//" " << ng_c1 << endl;
	//cout << " Hash Code " << c1->hashCode() << endl;
	sum += (ng_c1 - ntg_c1);
	total = ng_c1;
	trueCnts.append(ntg_c1);
	possCnts.append(ng_c1);
	probs.append(ntg_c1);
      }

      //Do calculations
      double norm = 0;
      for(int jx = 0; jx < probs.size(); jx++){
	norm += probs[jx];
      }
      ClauseHashArray full1;
      ClauseHashArray full2;
      Array<double> newProbs1;
      Array<double> newProbs2;
      Array<int> mult1;
      Array<int> mult2;

      /*
	cout << clause1.size() << " " << clause2.size() << " "
	<< dProbs1.size() << " " << dProbs2.size() << endl;
      */
      for(int jx = 0; jx < clause1.size(); jx++){
	
	ClauseHashArray* c1Array = clause1[jx];
	ClauseHashArray* c2Array = clause2[jx];
	Array<double>* probs1 = dProbs1[jx];
	Array<double>* probs2 = dProbs2[jx];
	Array<int>* m1 = idx1[jx];
	Array<int>* m2 = idx2[jx];
	/*
	  cout << c1Array->size() << " " << probs1->size()
	  << " " << c2Array->size() << " " << probs2->size() << endl;
	*/
	for(int kx = 0; kx < c1Array->size(); kx++){
	  if (!full1.contains((*c1Array)[kx])){
	    full1.append((*c1Array)[kx]);
	    newProbs1.append((*probs1)[kx]);
	    if (kx >= m1->size()){
	      cout << "Size mismatch\n";
	    }
	    if ((*m1)[kx] == 0){
	      cout << "bad " << jx << " " << kx << " ";
	      (*c1Array)[kx]->printWithoutWt(cout, (*domains_)[0]);
	      cout << endl;
	    }
	    mult1.append((*m1)[kx]);
	  }
	  else{
	    //cout << "Here\n";
	    int index = full1.find((*c1Array)[kx]);
	    if (newProbs1[index] != (*probs1)[kx]){
	      cout << "Probability mismatch\n";
	    }
	    if (mult1[index] != (*m1)[kx]){
	      cout << "Multiplier mismatch\n";
	    }
	  }
	}
	for(int kx = 0; kx < c2Array->size(); kx++){
	  if (!full2.contains((*c2Array)[kx])){
	    full2.append((*c2Array)[kx]);
	    newProbs2.append((*probs2)[kx]);
	    mult2.append((*m2)[kx]);
	  }
	}
      }
      Array<double> klScores;
      for(int jx = 0; jx < dProbs1.size(); jx++){
	klScores.append(0.0);
      }
      Array<Array<double>*> probNorm;
      for(int jx = 0; jx < dProbs1.size(); jx++){
	Array<double>* tmpD = new Array<double>;
	probNorm.append(tmpD);
      }

      for(int jx = 0; jx < probs.size(); jx++){

	probs[jx] = probs[jx] / norm;
	//cliques[jx]->printWithoutWt(cout, (*domains_)[0]);
	//cout << " " << probs[jx] << " | ";
	//cout << dProbs1.size() << " " << dProbs2.size() << " -> " << endl;
	
	for(int kx = 0; kx < dProbs1.size(); kx++){
	  Clause* d1 = new Clause(*forDecomp[jx]);
	  Clause* d2 = new Clause(*forDecomp[jx]);
	  if (dProbs1.size() == 1){
	    Predicate* rmed = d1->removePredicate(1);
	    delete rmed;
	    rmed = d2->removePredicate(0);
	    delete rmed;
	  }
	  else if (dProbs1.size() == 3){
	    if (kx == 0){
	      Predicate* rmed = d1->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    else if (kx == 1){
	      Predicate* rmed = d1->removePredicate(1);
	      delete rmed;
	      rmed =d2->removePredicate(2);
	      delete rmed;
	      rmed =d2->removePredicate(0);
	      delete rmed;
	    }
	    else{
	      Predicate* rmed = d1->removePredicate(0);
	      delete rmed;
	      rmed = d2->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(1);
	      delete rmed;
	    }
	  }
	  else{
	    if (kx == 0){
	      Predicate* rmed = d1->removePredicate(3);
	      delete rmed;
	      rmed = d2->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    else if (kx == 1){
	      Predicate* rmed = d1->removePredicate(2);
	      delete rmed;
	      rmed =d2->removePredicate(3);
	      delete rmed;
	      rmed =d2->removePredicate(1);
	      delete rmed;
	      rmed =d2->removePredicate(0);
	      delete rmed;
	    }
	    else if (kx == 2){
	      Predicate* rmed = d1->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(3);
	      delete rmed;
	      rmed = d2->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    else if (kx == 3){
	      Predicate* rmed = d1->removePredicate(0);
	      delete rmed;
	      rmed =d2->removePredicate(3);
	      delete rmed;
	      rmed =d2->removePredicate(2);
	      delete rmed;
	      rmed =d2->removePredicate(1);
	      delete rmed;
	    }
	    else if (kx == 4){
	      Predicate* rmed = d1->removePredicate(3);
	      delete rmed;
	      rmed = d1->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    else if (kx == 5){
	      Predicate* rmed = d1->removePredicate(3);
	      delete rmed;
	      rmed = d1->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    else{
	      Predicate* rmed = d1->removePredicate(2);
	      delete rmed;
	      rmed = d1->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(3);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }

	  }
	  d1->canonicalize();
	  d2->canonicalize();
	  int index1 = full1.find(d1);
	  int index2 = full2.find(d2);
	  //Array<int>* c1Mult = idx1[kx];
	  //Array<int>* c2Mult = idx2[kx];
	  //cout << idx1.size() << " " << c1Mult->size() << " " << idx2.size() << " " << c2Mult->size() << endl;
	  if (mult1[index1] == 0){
	    cout << "This is bad!" << endl;
	    cout << "Missing: " << d1->printWithoutWt(cout, (*domains_)[0]);
	    cout << endl;
	  }
	  //cout << "mults: " << kx << " mult1 " << (1.0/mult1[index1]) << " mult2 " << (1.0/mult2[index2]) << " index1 " << index1 << " index2 " << index2 << " " << mult1.size() << " " << mult2.size() << endl;
	  cout << "kx: " << kx << " ";
	  d1->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";
	  d2->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";

	  double predProb = newProbs1[index1] * newProbs2[index2] * (1.0/mult1[index1]) * (1.0/mult2[index2]);// * (1/(*c1Mult)[index1]) * (1/(*c2Mult)[index2]);//1 - ((1-newProbs1[index1]) * (1-newProbs2[index2]));
	  Array<double>* dp = probNorm[kx];
	  dp->append(predProb);
	  cout << " " << newProbs1[index1] << " " << newProbs2[index2] << " " << predProb << endl;
	  //cout << "kx: " << kx << " " << predProb << " ";
	  //double kl1 = thisProb * log(thisProb/prob1) + (1-thisProb) * log((1-thisProb)/(1-prob1));
	  //double kl1 = probs[jx] * log(probs[jx]/predProb) + (1-probs[jx]) * log((1-probs[jx])/(1-predProb));
	  //cout << newProbs1[index1] << " " << newProbs2[index2] << " (" << kl1 <<  "): "; //newProbs1[index1] << " " << newProbs2[index2] << ": ";
	  delete d1;
	  delete d2;
	  //klScores[kx] += kl1;
	} 
	cout << endl;
      }
      for(int jx = 0; jx < probNorm.size(); jx++){
	Array<double>* td = probNorm[jx];
	double z = 0;
	//cout << td->size() << " " << mult.size() << endl;
	
	for(int kx = 0; kx < td->size(); kx++){
	  z += (*td)[kx] * mult[kx];
	}
	cout << "check z = " << z;
	out << "check z = " << z << endl;
	if (z > 1.0 || z < 1.0){
	  cout << " z badness ";
	}
	cout << endl;
	/*
	  for(int kx = 0; kx < td->size(); kx++){
	  (*td)[kx] = (*td)[kx]  / z;
	  }
	*/
      }
      for(int jx = 0; jx < probs.size(); jx++){
	out << "feature ";
      	forDecomp[jx]->printWithoutWt(out, (*domains_)[0]);
	out << " " << probs[jx] << " | ";
	
	//cout << "feature ";
      	forDecomp[jx]->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << probs[jx] << " | ";
	
	for(int kx = 0; kx < probNorm.size(); kx++){
	  Array<double>* predProb = probNorm[kx];
	  double predFinal = mult[jx] * (*predProb)[jx];
	  //flag
	  double kl1 = probs[jx] * log(probs[jx]/predFinal);// + (1-probs[jx]) * log((1-probs[jx])/(1-predFinal));
	  //double kl1 = probs[jx] * log(probs[jx]/(*predProb)[jx]) + (1-probs[jx]) * log((1-probs[jx])/(1-(*predProb)[jx]));
	  //cout << newProbs1[index1] << " " << newProbs2[index2] << " (" << kl1 <<  "): "; //newProbs1[index1] << " " << newProbs2[index2] << ": ";
	  /*
	  out << (*predProb)[jx] << " (" << kl1 << "): ";
	  cout << (*predProb)[jx] << " (" << kl1 << "): ";
	  */

	  out << predFinal << " (" << kl1 << "): ";
	  cout << predFinal << " (" << kl1 << "): ";
	  klScores[kx] += kl1;
	}
	cout << endl;
	out << endl;
      }
      cout << "KL Scores: ";
      out << "KL Scores: ";
      double minScore = klScores[0];
      for(int jx = 0; jx < klScores.size(); jx++){
	out << klScores[jx] << " ";
	cout << klScores[jx] << " ";
	if (klScores[jx] < minScore){
	  minScore = klScores[jx];
	}
      }
      if (maxScore < minScore){
	maxScore = minScore;
      }
      out << endl;
      cout << endl;
      out << "Min " << minScore << endl;
      for(int jx = 0; jx < probNorm.size(); jx++){
	Array<double>* tmp = probNorm[jx];
	delete tmp;
      } 
      for(int jx = 0; jx < clause1.size(); jx++){
	
	ClauseHashArray* ca1 = clause1[jx];
	ClauseHashArray* ca2 = clause2[jx];
	ca1->deleteItemsAndClear();
	ca2->deleteItemsAndClear();
	Array<double>* pr1 = dProbs1[jx];
	Array<double>* pr2 = dProbs2[jx];
	Array<int>* id1 = idx1[jx];
	Array<int>* id2 = idx2[jx];
	delete id1;
	delete id2;
	delete pr1;
	delete pr2;
	delete ca1;
	delete ca2;
      }
      /*
      for(int dx = 0; dx < trueDCnts.size(); dx++){
	Array<double>* tmp =  trueDCnts[dx];
	delete tmp;
      }
      for(int dx = 0; dx < possDCnts.size(); dx++){
	Array<double>* tmp =  possDCnts[dx];
	delete tmp;
      }
      */
      forDecomp.deleteItemsAndClear();
      cliques.deleteItemsAndClear();
      acc.deleteArraysAndClear();
      out << endl;
      double endSec = timer_.time();
      cout << "Clique Score Time: " << (endSec - begSec) << endl << endl;
    }//end of ix
    out << "Max: " << maxScore << endl;
    out.close();
  }

  //end fast
  void setUpCounts(int templateSize, Array<Array<double>*>* d1Counts, Array<Array<double>*>* d2Counts, double value ){
    PowerSet* jset = new PowerSet;
    jset->create(templateSize);
    jset->prepareAccess(templateSize,false);
    const Array<int>* part;// = new Array<int>;
    int pos = 0;
    int setCount = 0;


    while(jset->getNextSet(part)){
      if (part->size() < templateSize){
	Array<double>* tmp = new Array<double>;
	double initValue = value;// / Clause::pow(2,part->size());
	//cout << "Init value = " << initValue << " " << value << " " << Clause::pow(2,part->size()) << endl;
	tmp->growToSize(Clause::pow(2,part->size()),initValue);
	if (setCount < d1Counts->size()){
	  (*d1Counts)[pos] = tmp;
	  if (pos < (d1Counts->size()-1)){
	    pos++;
	  }
	}
	else{
	  (*d2Counts)[pos] = tmp;
	  pos--;
	}
	setCount++;
      }
    }
    delete jset;

  }
  
  void cogTransfer(Array<Clause*> initClauses){
    //double maxScore = 0;
    string fname = outMLNFileName_;
    ofstream out(fname.c_str());
    
    if (!out.good()) { 
      cout << "ERROR: failed to open " <<fname<<endl;exit(-1);
    }
    
    
    for(int ix = 0; ix < initClauses.size(); ix++){
      //Flag Eval Time
      //double begSec = timer_.time();
      out << "Evaluate: ";
      initClauses[ix]->printWithoutWt(out, (*domains_)[0]);
      out << endl;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      int numNonEqual = 0;
      ArraysAccessor<int> acc;
      Array<int> pos;
      Array<int> termArr;
      Array<int> typeArr;
      for(int jx = 0; jx < preds->size(); jx++){
	if (!(*preds)[jx]->isEqualPred()){
	  numNonEqual++;
	  pos.append(jx);
	  Array<int>* vals = new Array<int>;
	  vals->append(0);
	  vals->append(1);
	  acc.appendArray(vals);
	}
	for(int kx = 0; kx < (*preds)[jx]->getNumTerms(); kx++){
	  const Term* term = (*preds)[jx]->getTerm(kx);

	  if (!termArr.contains(term->getId())){
	    int termType = (*preds)[jx]->getTermTypeAsInt(kx);
	    termArr.append(term->getId());
	    typeArr.append(termType);
	  }
	}
      }
      PowerSet* jset = new PowerSet;
      jset->create(numNonEqual);
      jset->prepareAccess(numNonEqual,false);
      const Array<int>* part;// = new Array<int>;
      int currPos = 0;
      int setCount = 0;
      Array<Array<int>*> d1Pos;
      Array<Array<int>*> d2Pos;
      d1Pos.growToSize(Clause::pow(2,numNonEqual-1)-1);
      d2Pos.growToSize(d1Pos.size());
      while(jset->getNextSet(part)){
	if (part->size() < numNonEqual){
	  Array<int>* tmp = new Array<int>;
	  tmp->growToSize(part->size());
	  //cout << "Cur Pos: " << currPos << " ";
	  for(int jx = 0; jx < tmp->size(); jx++){
	    (*tmp)[jx] = (*part)[jx];
	    //cout << "PredIdx = " << (*part)[jx] << " ";
	  }                                            
	  //cout << endl;
	  if (setCount < d1Pos.size()){
	    d1Pos[currPos] = tmp;
	    if (currPos < (d1Pos.size()-1)){
	      currPos++;
	    }
	  }
	  else{
	    d2Pos[currPos] = tmp;
	    currPos--;
	  }
	  setCount++;
	}
      }
      delete jset;
      jset = new PowerSet;
      jset->create(numNonEqual);
      jset->prepareAccess(typeArr.size(),false);
      while(jset->getNextSet(part)){
	ArraysAccessor<int> varTypes;
	for(int jx = 0; jx < part->size(); jx++){
	  cout << (*part)[jx] << " ";
	  const Array<int>* types = (*domains_)[0]->getConstantsByType(typeArr[jx]);
	  varTypes.appendArray(types);
	}
	cout << endl;
	Array<int> constants;
	while(varTypes.getDistinctNextCombination(constants)){
	  Clause* newClause = new Clause(*initClauses[ix]);
	  
	  for(int jx = 0; jx < constants.size(); jx++){
	    Clause::substitute(newClause, termArr[(*part)[jx]], constants[jx]);
	  }
	  double ntg  = newClause->getNumTrueGroundingsAnd((*domains_)[0], (*domains_)[0]->getDB(), false);
	  newClause->printWithoutWt(cout, (*domains_)[0]);
	  cout << " = " << ntg << endl;
	  delete newClause;
	}
      }
     
      /*
      for(int jx = 0; jx < typeArr.size(); jx++){
	const Array<int>* types = (*domains_)[0]->getConstantsByType(typeArr[jx]);
	for(int kx = 0; kx < types->size(); kx++){
	  Clause* newClause = new Clause(*initClauses[ix]);
	  
	  Clause::substitute(newClause, termArr[jx], (*types)[kx]);
	  double ntg  = newClause->getNumTrueGroundingsAnd((*domains_)[0], (*domains_)[0]->getDB(), false);
	  newClause->printWithoutWt(cout, (*domains_)[0]);
	  cout << " = " << ntg << endl;
	  delete newClause;
	}
      }
      */
    }
  }
  void getHier(Array<Clause*> initClauses, int num2nd){
     ArraysAccessor<int> acc;
     for(int ix = 0; ix < num2nd; ix++){
       Array<int>* tmp = new Array<int>;
       tmp->growToSize(initClauses.size(), 0);
       for(int jx = 0; jx < tmp->size(); jx++){
	 (*tmp)[jx] = jx;
       }
       acc.appendArray(tmp);
     }
     Array<int> picked;
     while(acc.getDistinctNextCombinationUnordered(picked)){
       /*
       cout << "Picked: ";
       for(int ix = 0; ix < picked.size(); ix++){
	 cout << picked[ix] << "=" << initClauses[picked[ix]]->getWt() << " "; 
       }
       cout << endl;
       */
       ArraysAccessor<int> internal;
       Array<int> offSet;
       Array<int> clique;
       //int index = 0;
       for(int ix = 0; ix < picked.size(); ix++){
	 cout << picked[ix];
	 if (ix < (picked.size()-1)){
	   cout << "-";
	 }
       }
       cout << endl;
     }
  }
      
  void getHier(Array<Clause*> initClauses, int num2nd, int num1st){
     ArraysAccessor<int> acc;
     for(int ix = 0; ix < num2nd; ix++){
       Array<int>* tmp = new Array<int>;
       tmp->growToSize(initClauses.size(), 0);
       for(int jx = 0; jx < tmp->size(); jx++){
	 (*tmp)[jx] = jx;
       }
       acc.appendArray(tmp);
     }
     Array<int> picked;
     while(acc.getDistinctNextCombinationUnordered(picked)){
       /*
       cout << "Picked: ";
       for(int ix = 0; ix < picked.size(); ix++){
	 cout << picked[ix] << "=" << initClauses[picked[ix]]->getWt() << " "; 
       }
       cout << endl;
       */
       ArraysAccessor<int> internal;
       Array<int> offSet;
       Array<int> clique;
       int index = 0;
       for(int ix = 0; ix < picked.size(); ix++){
	 
	 double bound = num1st;
	 if (bound > initClauses[picked[ix]]->getWt() ){
	   bound = initClauses[picked[ix]]->getWt();
	 }
	 for(int jx = 0; jx < bound; jx++){
	   Array<int>* indexes = new Array<int>;
	   
	   for(int kx = 0; kx < initClauses[picked[ix]]->getWt(); kx++){
	     indexes->append(kx + index);
	   }
	   internal.appendArray(indexes);
	   offSet.append(index);
	   clique.append(picked[ix]);
	 }
	 index += (int)initClauses[picked[ix]]->getWt();
       }
       Array<int> firstOrder;
       while(internal.getDistinctNextCombinationUnordered(firstOrder)){
	 for(int ix = 0; ix < firstOrder.size(); ix++){
	   cout << clique[ix] << "-" << (firstOrder[ix] - offSet[ix]);
	   if (ix < firstOrder.size()-1){
	     cout << " ";
	   }
	   else{
	     cout << "<>";
	   }
	 }
	 for(int ix = 0; ix < firstOrder.size(); ix++){
	   cout << "t" << clique[ix] << "-c" << (firstOrder[ix] - offSet[ix]);
	   if (ix < firstOrder.size()-1){
	     cout << "_";
	   }
	   else{
	     cout << ".mln";
	   }
	 }
	 cout << endl;
       }
       internal.deleteArraysAndClear();
       //cout << endl;
     }
  }
      
 //Super Fast
  void buildStates(Array<Clause*> initClauses){
    //double maxScore = 0;

    ArraysAccessor<int> acc;
    for(int jx = 0; jx < initClauses[0]->getNumPredicates(); jx++){
	  Array<int>* vals = new Array<int>;
	  vals->append(0);
	  vals->append(1);
	  acc.appendArray(vals);
    }
    Array<int> sense;

    ClauseHashArray* cliques = new ClauseHashArray;
    while(acc.getNextCombination(sense)){
      string fname = outMLNFileName_;
      string appendStr = "";
      for(int ix = 0; ix < sense.size(); ix++){
	if (sense[ix] == 0){
	  appendStr.append("0");
	}
	else{
	  appendStr.append("1");
	}
      }
      fname.append(".").append(appendStr).append(".txt");
      ofstream out(fname.c_str());

      
      if (!out.good()) { cout << "ERROR: failed to open " <<fname<<endl;exit(-1);}
      for(int ix = 0; ix < initClauses.size(); ix++){
	Clause* c1 = new Clause(*initClauses[ix]);
	
	for(int jx = 0; jx < c1->getNumPredicates(); jx++){
	  Predicate* pred = c1->getPredicate(jx);
	  if (sense[jx] == 0){
	    pred->setSense(false);
	  }
	  else{
	    pred->setSense(true);
	  }
	}
	Clause* c2 = new Clause(*c1);
	c1->canonicalize();

	if (cliques->contains(c1)){

	}
	else{
	  cliques->append(c1);
	  c2->printWithoutWt(out, (*domains_)[0]);
	  out << endl;
	}
	delete c2;
      }
      out.close();
    }
  }

  //Super Fast
  void superFastTransfer(Array<Clause*> initClauses){
    double maxScore = 0;
    string fname = outMLNFileName_;
    ofstream out(fname.c_str());

    if (!out.good()) { cout << "ERROR: failed to open " <<fname<<endl;exit(-1);}


    for(int ix = 0; ix < initClauses.size(); ix++){
      //Flag Eval Time
      double begSec = timer_.time();
      out << "Evaluate: ";
      initClauses[ix]->printWithoutWt(out, (*domains_)[0]);
      out << endl;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      int numNonEqual = 0;
      ArraysAccessor<int> acc;
      Array<int> pos;
      for(int jx = 0; jx < preds->size(); jx++){
	if (!(*preds)[jx]->isEqualPred()){
	  numNonEqual++;
	  pos.append(jx);
	  Array<int>* vals = new Array<int>;
	  vals->append(0);
	  vals->append(1);
	  acc.appendArray(vals);
	}
      }
      PowerSet* jset = new PowerSet;
      jset->create(numNonEqual);
      jset->prepareAccess(numNonEqual,false);
      const Array<int>* part;// = new Array<int>;
      int currPos = 0;
      int setCount = 0;
      Array<Array<int>*> d1Pos;
      Array<Array<int>*> d2Pos;
      d1Pos.growToSize(Clause::pow(2,numNonEqual-1)-1);
      d2Pos.growToSize(d1Pos.size());
      while(jset->getNextSet(part)){
	if (part->size() < numNonEqual){
	  Array<int>* tmp = new Array<int>;
	  tmp->growToSize(part->size());
	  //cout << "Cur Pos: " << currPos << " ";
	  for(int jx = 0; jx < tmp->size(); jx++){
	    (*tmp)[jx] = (*part)[jx];
	    //cout << "PredIdx = " << (*part)[jx] << " ";
	  }
	  //cout << endl;
	  if (setCount < d1Pos.size()){
	    d1Pos[currPos] = tmp;
	    if (currPos < (d1Pos.size()-1)){
	      currPos++;
	    }
	  }
	  else{
	    d2Pos[currPos] = tmp;
	    currPos--;
	  }
	  setCount++;
	}
      }
      delete jset;

      Array<Array<double>*> counts;
      Array<Array<double>*>* d1Counts = new Array<Array<double>*>;
      Array<Array<double>*>* d2Counts = new Array<Array<double>*>;
      d1Counts->growToSize(Clause::pow(2,numNonEqual-1)-1);
      d2Counts->growToSize(Clause::pow(2,numNonEqual-1)-1);
      //set up flag
      //cerr << "Here 1\n";
      //double initCount = 
      setUpCounts(numNonEqual, d1Counts,d2Counts, 0);
       //cerr << "Here 2\n";
      for(int dx = 0; dx < domains_->size(); dx++){
	//cerr << "Here Domains " << dx << "\n";
	Array<Array<double>*>* d1Cnt = new Array<Array<double>*>;
	Array<Array<double>*>* d2Cnt = new Array<Array<double>*>;
	d1Cnt->growToSize(Clause::pow(2,numNonEqual-1)-1);
	d2Cnt->growToSize(Clause::pow(2,numNonEqual-1)-1);
	//set up flag
	setUpCounts(numNonEqual, d1Cnt,d2Cnt,0);

	Array<double>* dCounts = initClauses[ix]->allTemplateGroundings((*domains_)[dx], (*domains_)[dx]->getDB(), d1Cnt, d2Cnt, &d1Pos, &d2Pos, numNonEqual);
	for(int ix = 0; ix < d1Cnt->size(); ix++){
	  Array<double>* tmpArr = (*d1Cnt)[ix];
	  for(int jx = 0; jx < tmpArr->size(); jx++){
	    (*((*d1Counts)[ix]))[jx] += (*tmpArr)[jx];
	  }
	  tmpArr = (*d2Cnt)[ix];
	  for(int jx = 0; jx < tmpArr->size(); jx++){
	    (*((*d2Counts)[ix]))[jx] += (*tmpArr)[jx];
	  }
	}
	d1Cnt->deleteItemsAndClear();
	d2Cnt->deleteItemsAndClear();
	delete d1Cnt;
	delete d2Cnt;
	/*
	  for(int xx = 0; xx < dCounts->size(); xx++){
	  cerr << xx << " " << (*dCounts)[xx] << endl;
	  }
	*/
	counts.append(dCounts);
      }

      Array<Array<double>*> dProbs1;
      Array<Array<int>*> idx1;
      Array<Array<double>*> dProbs2;
      Array<Array<int>*> idx2;
      Array<ClauseHashArray*> clause1;
      Array<ClauseHashArray*> clause2;


      for(int jx = 0; jx < d1Pos.size(); jx++){

	Clause* d1 = new Clause();
	Clause* d2 = new Clause();
	Array<int>* predPos = d1Pos[jx];
	for(int px = 0; px < predPos->size(); px++){
	  Predicate* p1 = new Predicate(*(*preds)[(*predPos)[px]]);
	  p1->setClausePos(px);
	  d1->appendPredicate(p1);
	}
	predPos = d2Pos[jx];
	for(int px = 0; px < predPos->size(); px++){
	  Predicate* p1 = new Predicate(*(*preds)[(*predPos)[px]]);
	  p1->setClausePos(px);
	  d2->appendPredicate(p1);
	}
	d1->canonicalize();
	d2->canonicalize();
	Array<double>* tmpDouble = new Array<double>;
	Array<int>* tmpInt = new Array<int>;
	ClauseHashArray* tmpClause = new ClauseHashArray;
	buildCliqueFast(d1, tmpDouble, tmpInt, tmpClause, (*d1Counts)[jx]);
	dProbs1.append(tmpDouble);
	idx1.append(tmpInt);
	clause1.append(tmpClause);
	tmpDouble = new Array<double>;
	tmpInt = new Array<int>;
	tmpClause = new ClauseHashArray;
	buildCliqueFast(d2, tmpDouble, tmpInt, tmpClause, (*d2Counts)[jx]);
	dProbs2.append(tmpDouble);
	idx2.append(tmpInt);
	clause2.append(tmpClause);
	delete d1;
	delete d2;
 	
      }
      //cout << "HERE\n";
      Array<double> trueCnts;
      Array<double> possCnts;
      Array<double> probs;
      Array<int> sense;
      double sum = 0;
      double total = 0;
      ClauseHashArray cliques;
      Array<Clause*> forDecomp;
      Array<ClauseHashArray*> decomp;
      Array<int> mult;
    /*
    */

      while(acc.getNextCombination(sense)){
	Clause* c1 = new Clause(*initClauses[ix]);
	int index = 0;
	for(int jx = 0; jx < pos.size(); jx++){
	  Predicate* pred = c1->getPredicate(pos[jx]);
	  if (sense[jx] == 0){

	    pred->setSense(false);
	  }
	  else{
	    index += Clause::pow(2, (pos.size() - jx - 1));	    
	    pred->setSense(true);
	  }
	}
	Clause* r1 = new Clause(*c1);
	c1->setDirty();
	c1->canonicalize();
	if (cliques.contains(c1)){
	  int dupIndex = cliques.find(c1);
	  mult[dupIndex]++;
	  delete c1;
	  continue;
	}
	cliques.append(c1);
	forDecomp.append(r1);
	mult.append(1);

	double ntg_c1 = 0;
	double ng_c1 = 0;

	for(int dx = 0; dx < counts.size(); dx++){
	  ntg_c1 += (*(counts[dx]))[index];
	}
	ntg_c1 = ntg_c1 - (counts.size()-1);
	c1->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << ntg_c1 << endl;
	sum += (ng_c1 - ntg_c1);
	total = ng_c1;
	trueCnts.append(ntg_c1);
	possCnts.append(ng_c1);
	probs.append(ntg_c1);
      }

      //Do calculations
      double norm = 0;
      for(int jx = 0; jx < probs.size(); jx++){
	norm += probs[jx];
      }
      ClauseHashArray full1;
      ClauseHashArray full2;
      Array<double> newProbs1;
      Array<double> newProbs2;
      Array<int> mult1;
      Array<int> mult2;

      /*
	cout << clause1.size() << " " << clause2.size() << " "
	<< dProbs1.size() << " " << dProbs2.size() << endl;
      */
      for(int jx = 0; jx < clause1.size(); jx++){
	
	ClauseHashArray* c1Array = clause1[jx];
	ClauseHashArray* c2Array = clause2[jx];
	Array<double>* probs1 = dProbs1[jx];
	Array<double>* probs2 = dProbs2[jx];
	Array<int>* m1 = idx1[jx];
	Array<int>* m2 = idx2[jx];
	for(int kx = 0; kx < c1Array->size(); kx++){
	  if (!full1.contains((*c1Array)[kx])){
	    full1.append((*c1Array)[kx]);
	    newProbs1.append((*probs1)[kx]);
	    if (kx >= m1->size()){
	      cout << "Size mismatch\n";
	    }
	    if ((*m1)[kx] == 0){
	      cout << "bad " << jx << " " << kx << " ";
	      (*c1Array)[kx]->printWithoutWt(cout, (*domains_)[0]);
	      cout << endl;
	    }
	    mult1.append((*m1)[kx]);
	  }
	  else{
	    //cout << "Here\n";
	    int index = full1.find((*c1Array)[kx]);
	    //super mismatch
	    if (newProbs1[index] != (*probs1)[kx]){
	      cout << "Probability mismatch\n";
	      cout << "new probs: " << newProbs1[index] << " probs1 " << (*probs1)[kx] << endl;
	      cout << "index " << index << " kx " << kx << "\n";
	      (*c1Array)[kx]->printWithoutWt(cout, (*domains_)[0]);
	      cout << endl;
	    }
	    if (mult1[index] != (*m1)[kx]){
	      cout << "Multiplier mismatch\n";
	    }
	  }
	}
	for(int kx = 0; kx < c2Array->size(); kx++){
	  if (!full2.contains((*c2Array)[kx])){
	    full2.append((*c2Array)[kx]);
	    newProbs2.append((*probs2)[kx]);
	    mult2.append((*m2)[kx]);
	  }
	}
      }
      Array<double> klScores;
      for(int jx = 0; jx < dProbs1.size(); jx++){
	klScores.append(0.0);
      }
      Array<Array<double>*> probNorm;
      for(int jx = 0; jx < dProbs1.size(); jx++){
	Array<double>* tmpD = new Array<double>;
	probNorm.append(tmpD);
      }

      for(int jx = 0; jx < probs.size(); jx++){

	probs[jx] = probs[jx] / norm;
	//cliques[jx]->printWithoutWt(cout, (*domains_)[0]);
	//cout << " " << probs[jx] << " | ";
	//cout << dProbs1.size() << " " << dProbs2.size() << " -> ";
	//Work Here Jesse
	for(int kx = 0; kx < dProbs1.size(); kx++){
	  Clause* d1 = new Clause(*forDecomp[jx]);
	  Clause* d2 = new Clause(*forDecomp[jx]);
	  if (dProbs1.size() == 1){
	    Predicate* rmed = d1->removePredicate(1);
	    delete rmed;
	    rmed = d2->removePredicate(0);
	    delete rmed;
	  }
	  else{
	    //for(int dx = (d2Pos.size()-1); dx >= 0; dx--){
	    //for(int dx = 0; dx < d2Pos.size(); dx++){
	    Array<int>* predIdx = d2Pos[kx];
	    for(int px = (predIdx->size()-1); px >=0; px--){
	      Predicate* rmed = d1->removePredicate((*predIdx)[px]);
	      delete rmed;
	    }
	    
	    predIdx = d1Pos[kx];
	    for(int px = (predIdx->size()-1); px >=0; px--){
	      Predicate* rmed = d2->removePredicate((*predIdx)[px]);
	      delete rmed;
	    }
	  }
	  d1->canonicalize();
	  d2->canonicalize();
	  int index1 = full1.find(d1);
	  int index2 = full2.find(d2);
	  /*
	  cerr << "index1 " << index1 << " " << mult1.size() << endl;
	  cout << "kx: " << kx << " ";
	  d1->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";
	  d2->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";

	  exit(-1);
	  */
	  //Array<int>* c1Mult = idx1[kx];
	  //Array<int>* c2Mult = idx2[kx];
	  //cout << idx1.size() << " " << c1Mult->size() << " " << idx2.size() << " " << c2Mult->size() << endl;
	  if (mult1[index1] == 0){
	    cout << "This is bad!" << endl;
	    cout << "Missing: ";
	    d1->printWithoutWt(cout, (*domains_)[0]);
	    cout << endl;
	  }
	  //cout << "mults: " << kx << " mult1 " << (1.0/mult1[index1]) << " mult2 " << (1.0/mult2[index2]) << " index1 " << index1 << " index2 " << index2 << " " << mult1.size() << " " << mult2.size() << endl;
	  cout << "kx: " << kx << " ";
	  d1->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";
	  d2->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";

	  double predProb = newProbs1[index1] * newProbs2[index2] * (1.0/mult1[index1]) * (1.0/mult2[index2]);// * (1/(*c1Mult)[index1]) * (1/(*c2Mult)[index2]);//1 - ((1-newProbs1[index1]) * (1-newProbs2[index2]));
	  Array<double>* dp = probNorm[kx];
	  dp->append(predProb);
	  cout << " " << newProbs1[index1] << " " << newProbs2[index2] << " " << predProb << endl;
	  //cout << "kx: " << kx << " " << predProb << " ";
	  //double kl1 = thisProb * log(thisProb/prob1) + (1-thisProb) * log((1-thisProb)/(1-prob1));
	  //double kl1 = probs[jx] * log(probs[jx]/predProb) + (1-probs[jx]) * log((1-probs[jx])/(1-predProb));
	  //cout << newProbs1[index1] << " " << newProbs2[index2] << " (" << kl1 <<  "): "; //newProbs1[index1] << " " << newProbs2[index2] << ": ";
	  delete d1;
	  delete d2;
	  //klScores[kx] += kl1;
	} 
	cout << endl;
      }
      for(int jx = 0; jx < probNorm.size(); jx++){
	Array<double>* td = probNorm[jx];
	double z = 0;
	//cout << td->size() << " " << mult.size() << endl;
	
	for(int kx = 0; kx < td->size(); kx++){
	  z += (*td)[kx] * mult[kx];
	}
	cout << "check z = " << z;
	out << "check z = " << z << endl;
	if (z > 1.0 || z < 1.0){
	  cout << " z badness ";
	}
	cout << endl;
	/*
	  for(int kx = 0; kx < td->size(); kx++){
	  (*td)[kx] = (*td)[kx]  / z;
	  }
	*/
      }
      for(int jx = 0; jx < probs.size(); jx++){
	out << "feature ";
      	forDecomp[jx]->printWithoutWt(out, (*domains_)[0]);
	out << " " << probs[jx] << " | ";
	
	//cout << "feature ";
      	forDecomp[jx]->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << probs[jx] << " | ";
	
	for(int kx = 0; kx < probNorm.size(); kx++){
	  Array<double>* predProb = probNorm[kx];
	  double predFinal = mult[jx] * (*predProb)[jx];
	  //flag
	  double kl1 = probs[jx] * log(probs[jx]/predFinal);// + (1-probs[jx]) * log((1-probs[jx])/(1-predFinal));
	  //double kl1 = probs[jx] * log(probs[jx]/(*predProb)[jx]) + (1-probs[jx]) * log((1-probs[jx])/(1-(*predProb)[jx]));
	  //cout << newProbs1[index1] << " " << newProbs2[index2] << " (" << kl1 <<  "): "; //newProbs1[index1] << " " << newProbs2[index2] << ": ";
	  /*
	  out << (*predProb)[jx] << " (" << kl1 << "): ";
	  cout << (*predProb)[jx] << " (" << kl1 << "): ";
	  */

	  out << predFinal << " (" << kl1 << "): ";
	  cout << predFinal << " (" << kl1 << "): ";
	  klScores[kx] += kl1;
	}
	cout << endl;
	out << endl;
      }
      //Flag J
      double minScore = klScores[0];
      cout << "KL Scores: ";
      out << "KL Scores: ";
      for(int jx = 0; jx < klScores.size(); jx++){
	out << klScores[jx] << " ";
	cout << klScores[jx] << " ";
	if (klScores[jx] < minScore){
	  minScore = klScores[jx];
	}
      }
      if (maxScore < minScore){
	maxScore = minScore;
      }
      out << endl;
      cout << endl;
      out << "Min " << minScore << endl;
      for(int jx = 0; jx < probNorm.size(); jx++){
	Array<double>* tmp = probNorm[jx];
	delete tmp;
      } 
      for(int jx = 0; jx < clause1.size(); jx++){
	
	ClauseHashArray* ca1 = clause1[jx];
	ClauseHashArray* ca2 = clause2[jx];
	ca1->deleteItemsAndClear();
	ca2->deleteItemsAndClear();
	Array<double>* pr1 = dProbs1[jx];
	Array<double>* pr2 = dProbs2[jx];
	Array<int>* id1 = idx1[jx];
	Array<int>* id2 = idx2[jx];
	delete id1;
	delete id2;
	delete pr1;
	delete pr2;
	delete ca1;
	delete ca2;
	counts.deleteItemsAndClear(); 
      }
      /*
      for(int dx = 0; dx < trueDCnts.size(); dx++){
	Array<double>* tmp =  trueDCnts[dx];
	delete tmp;
      }
      for(int dx = 0; dx < possDCnts.size(); dx++){
	Array<double>* tmp =  possDCnts[dx];
	delete tmp;
      }
      */
      forDecomp.deleteItemsAndClear();
      cliques.deleteItemsAndClear();
      acc.deleteArraysAndClear();
      out << endl;
      double endSec = timer_.time();
      cout << "Clique Score Time: " << (endSec - begSec) << endl << endl;
      d1Pos.deleteItemsAndClear();
      d2Pos.deleteItemsAndClear();
      d1Counts->deleteItemsAndClear();
      d2Counts->deleteItemsAndClear();
      delete d1Counts;
      delete d2Counts;
      //delete dCounts;

    }//end of ix
    out << "Max: " << maxScore << endl;
    out.close();
  }


 //Super Fast
  void superFastLikelihood(Array<Clause*> initClauses){
    
    string fname = outMLNFileName_;
    ofstream out(fname.c_str());

    if (!out.good()) { cout << "ERROR: failed to open " <<fname<<endl;exit(-1);}


    for(int ix = 0; ix < initClauses.size(); ix++){
      //Flag Eval Time
      double begSec = timer_.time();
      out << "Evaluate: ";
      initClauses[ix]->printWithoutWt(out, (*domains_)[0]);
      out << endl;
      Array<double> predTrueGround;
      Array<double> totalNumGround;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      for(int jx = 0; jx < preds->size(); jx++){
	Clause* unit = new Clause();
	unit->appendPredicate(new Predicate(*(*preds)[jx]));
	unit->canonicalize();
	double ntg_unit = 1;
	double tot_unit = 2;
	for(int dx = 0; dx < domains_->size(); dx++){
	  ntg_unit += unit->getNumTrueGroundings((*domains_)[dx],
						 (*domains_)[dx]->getDB(),
						 false);
	  tot_unit += unit->getNumDistinctGroundings((*domains_)[dx]);
	}
	predTrueGround.append(ntg_unit);
	totalNumGround.append(tot_unit);
      }
      Array<double> unitProbs;
      for(int jx = 0; jx < predTrueGround.size(); jx++){
	unitProbs.append(predTrueGround[jx]/totalNumGround[jx]);
	cout << "pred " << jx << " " << predTrueGround[jx] << " " << totalNumGround[jx]<< endl;
      }
      int numNonEqual = 0;
      ArraysAccessor<int> acc;
      Array<int> pos;
      for(int jx = 0; jx < preds->size(); jx++){
	if (!(*preds)[jx]->isEqualPred()){
	  numNonEqual++;
	  pos.append(jx);
	  Array<int>* vals = new Array<int>;
	  vals->append(0);
	  vals->append(1);
	  acc.appendArray(vals);
	}
      }
      PowerSet* jset = new PowerSet;
      jset->create(numNonEqual);
      jset->prepareAccess(numNonEqual,false);
      const Array<int>* part;// = new Array<int>;
      int currPos = 0;
      int setCount = 0;
      Array<Array<int>*> d1Pos;
      Array<Array<int>*> d2Pos;
      d1Pos.growToSize(Clause::pow(2,numNonEqual-1)-1);
      d2Pos.growToSize(d1Pos.size());
      while(jset->getNextSet(part)){
	if (part->size() < numNonEqual){
	  Array<int>* tmp = new Array<int>;
	  tmp->growToSize(part->size());
	  //cout << "Cur Pos: " << currPos << " ";
	  for(int jx = 0; jx < tmp->size(); jx++){
	    (*tmp)[jx] = (*part)[jx];
	    //cout << "PredIdx = " << (*part)[jx] << " ";
	  }
	  //cout << endl;
	  if (setCount < d1Pos.size()){
	    d1Pos[currPos] = tmp;
	    if (currPos < (d1Pos.size()-1)){
	      currPos++;
	    }
	  }
	  else{
	    d2Pos[currPos] = tmp;
	    currPos--;
	  }
	  setCount++;
	}
      }
      delete jset;

      Array<Array<double>*> counts;
      Array<Array<double>*>* d1Counts = new Array<Array<double>*>;
      Array<Array<double>*>* d2Counts = new Array<Array<double>*>;
      d1Counts->growToSize(Clause::pow(2,numNonEqual-1)-1);
      d2Counts->growToSize(Clause::pow(2,numNonEqual-1)-1);
  
      setUpCounts(numNonEqual, d1Counts,d2Counts, 0);
  
      for(int dx = 0; dx < domains_->size(); dx++){
	Array<Array<double>*>* d1Cnt = new Array<Array<double>*>;
	Array<Array<double>*>* d2Cnt = new Array<Array<double>*>;
	d1Cnt->growToSize(Clause::pow(2,numNonEqual-1)-1);
	d2Cnt->growToSize(Clause::pow(2,numNonEqual-1)-1);

	setUpCounts(numNonEqual, d1Cnt,d2Cnt,0);

	Array<double>* dCounts = initClauses[ix]->allTemplateGroundings((*domains_)[dx], (*domains_)[dx]->getDB(), d1Cnt, d2Cnt, &d1Pos, &d2Pos, numNonEqual);
	for(int ix = 0; ix < d1Cnt->size(); ix++){
	  Array<double>* tmpArr = (*d1Cnt)[ix];
	  for(int jx = 0; jx < tmpArr->size(); jx++){
	    (*((*d1Counts)[ix]))[jx] += (*tmpArr)[jx];
	  }
	  tmpArr = (*d2Cnt)[ix];
	  for(int jx = 0; jx < tmpArr->size(); jx++){
	    (*((*d2Counts)[ix]))[jx] += (*tmpArr)[jx];
	  }
	}
	d1Cnt->deleteItemsAndClear();
	d2Cnt->deleteItemsAndClear();
	delete d1Cnt;
	delete d2Cnt;

	counts.append(dCounts);
      }
      //cout << "J DAWG Like\n";

      //cout << "HERE\n";
      Array<double> trueCnts;
      Array<double> possCnts;
      Array<double> probs;
      Array<int> sense;
      double sum = 0;
      double total = 0;
      ClauseHashArray cliques;
      Array<Clause*> forDecomp;
      Array<ClauseHashArray*> decomp;
      Array<int> mult;


      while(acc.getNextCombination(sense)){
	Clause* c1 = new Clause(*initClauses[ix]);
	int index = 0;
	for(int jx = 0; jx < pos.size(); jx++){
	  Predicate* pred = c1->getPredicate(pos[jx]);
	  if (sense[jx] == 0){

	    pred->setSense(false);
	  }
	  else{
	    index += Clause::pow(2, (pos.size() - jx - 1));	    
	    pred->setSense(true);
	  }
	}
	Clause* r1 = new Clause(*c1);
	c1->setDirty();
	c1->canonicalize();
	if (cliques.contains(c1)){
	  int dupIndex = cliques.find(c1);
	  mult[dupIndex]++;
	  delete c1;
	  continue;
	}
	cliques.append(c1);
	forDecomp.append(r1);
	mult.append(1);

	double ntg_c1 = 0;
	double ng_c1 = 0;

	for(int dx = 0; dx < counts.size(); dx++){
	  ntg_c1 += (*(counts[dx]))[index];
	}
	ntg_c1 = ntg_c1 - (counts.size()-1);
	c1->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << ntg_c1 << endl;
	sum += (ng_c1 - ntg_c1);
	total = ng_c1;
	trueCnts.append(ntg_c1);
	possCnts.append(ng_c1);
	probs.append(ntg_c1);
      }

      Array<double> indProb;
      double indNorm = 0;
      for(int jx = 0; jx < probs.size(); jx++){
	const Array<Predicate*>* preds = forDecomp[jx]->getPredicates();
	double unitLogScore = 1;
	for(int kx = 0; kx < unitProbs.size(); kx++){
	  //cout << unitProbs[kx] << " (" << (*preds)[kx]->getSense() << ") ";
	  if ( (*preds)[kx]->getSense() == 0){
	    unitLogScore = unitLogScore * (1-unitProbs[kx]);
	  }
	  else{
	    unitLogScore = unitLogScore * (unitProbs[kx]);
	  }
	}
	indNorm += unitLogScore;
	indProb.append(unitLogScore);
      }
      double denom = Clause::pow(2, initClauses[ix]->getNumPredicates());
      for(int jx = 0; jx < probs.size(); jx++){
	probs[jx] += mult[jx] / denom;
      }
      
      //FLAG Likelihood calc
      //Do calculations
      double norm = 0;
      for(int jx = 0; jx < probs.size(); jx++){
	norm += probs[jx];
      }
      Array<double> probsNorm;
      for(int jx = 0; jx < probs.size(); jx++){
	cout << "Jx " << jx << " " << probs[jx] << " " << norm << " ";
	cout << (probs[jx]/norm) << endl;
	probsNorm.append((probs[jx]/norm));
      }

 
      long double logCliq = 0;
      long double logInd =0;
      out.precision(10);
      cout.precision(10);
      for(int jx = 0; jx < probs.size(); jx++){
	long double logScore = probs[jx] * log(probsNorm[jx]);
	logCliq += logScore;
	out << "feature ";
      	forDecomp[jx]->printWithoutWt(out, (*domains_)[0]);
	out << " " << probsNorm[jx] << " " << logScore << " | ";
	
	//cout << "feature ";
      	forDecomp[jx]->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << probsNorm[jx] << " " << logScore << " | ";
	const Array<Predicate*>* preds = forDecomp[jx]->getPredicates();

	long double normalized = indProb[jx]/indNorm;
	long double unitLogScore = probs[jx] * log(normalized);
	cout << normalized << " (" << indProb[jx] << "/" << indNorm << ") ";
	out << normalized << " " << unitLogScore;
	for(int kx = 0; kx < unitProbs.size(); kx++){
	  cout << unitProbs[kx] << " (" << (*preds)[kx]->getSense() << ") ";
	}
	cout << endl;
	logInd+=unitLogScore;
	out << endl;
      }
      //Flag J

      long double logDiff = logCliq - logInd;
      long double normLogDiff = logDiff / norm;
      cout << "Logs " << logCliq << " " << logInd << " ";
      cout << logDiff << " " << normLogDiff << endl;
      out << "Logs: " << logCliq << " " << logInd << " ";
      out << logDiff << " " << normLogDiff << endl;
      
      forDecomp.deleteItemsAndClear();
      cliques.deleteItemsAndClear();
      acc.deleteArraysAndClear();
      out << endl;
      double endSec = timer_.time();
      cout << "Clique Score Time: " << (endSec - begSec) << endl << endl;
      d1Pos.deleteItemsAndClear();
      d2Pos.deleteItemsAndClear();
      d1Counts->deleteItemsAndClear();
      d2Counts->deleteItemsAndClear();
      delete d1Counts;
      delete d2Counts;
      //delete dCounts;

    }//end of ix
    //out << "Max: " << maxScore << endl;
    out.close();
  }
  void indConjunctSuperFastLikelihood(Array<Clause*> initClauses){
    
    string fname = outMLNFileName_;
    ofstream out(fname.c_str());

    if (!out.good()) { cout << "ERROR: failed to open " <<fname<<endl;exit(-1);}


    for(int ix = 0; ix < initClauses.size(); ix++){
      //Flag Eval Time
      double begSec = timer_.time();
      //out << "Evaluate: ";
      initClauses[ix]->printWithoutWt(out, (*domains_)[0]);
      out << endl;
      Array<double> predTrueGround;
      Array<double> totalNumGround;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      for(int jx = 0; jx < preds->size(); jx++){
	Clause* unit = new Clause();
	unit->appendPredicate(new Predicate(*(*preds)[jx]));
	unit->canonicalize();
	double ntg_unit = 1;
	double tot_unit = 2;
	for(int dx = 0; dx < domains_->size(); dx++){
	  ntg_unit += unit->getNumTrueGroundings((*domains_)[dx],
						 (*domains_)[dx]->getDB(),
						 false);
	  tot_unit += unit->getNumDistinctGroundings((*domains_)[dx]);
	}
	predTrueGround.append(ntg_unit);
	totalNumGround.append(tot_unit);
      }
      Array<double> unitProbs;
      for(int jx = 0; jx < predTrueGround.size(); jx++){
	unitProbs.append(predTrueGround[jx]/totalNumGround[jx]);
	cout << "pred " << jx << " " << predTrueGround[jx] << " " << totalNumGround[jx]<< endl;
      }
      int numNonEqual = 0;
      ArraysAccessor<int> acc;
      Array<int> pos;
      for(int jx = 0; jx < preds->size(); jx++){
	if (!(*preds)[jx]->isEqualPred()){
	  numNonEqual++;
	  pos.append(jx);
	  Array<int>* vals = new Array<int>;
	  vals->append(0);
	  vals->append(1);
	  acc.appendArray(vals);
	}
      }
      PowerSet* jset = new PowerSet;
      jset->create(numNonEqual);
      jset->prepareAccess(numNonEqual,false);
      const Array<int>* part;// = new Array<int>;
      int currPos = 0;
      int setCount = 0;
      Array<Array<int>*> d1Pos;
      Array<Array<int>*> d2Pos;
      d1Pos.growToSize(Clause::pow(2,numNonEqual-1)-1);
      d2Pos.growToSize(d1Pos.size());
      while(jset->getNextSet(part)){
	if (part->size() < numNonEqual){
	  Array<int>* tmp = new Array<int>;
	  tmp->growToSize(part->size());
	  //cout << "Cur Pos: " << currPos << " ";
	  for(int jx = 0; jx < tmp->size(); jx++){
	    (*tmp)[jx] = (*part)[jx];
	    //cout << "PredIdx = " << (*part)[jx] << " ";
	  }
	  //cout << endl;
	  if (setCount < d1Pos.size()){
	    d1Pos[currPos] = tmp;
	    if (currPos < (d1Pos.size()-1)){
	      currPos++;
	    }
	  }
	  else{
	    d2Pos[currPos] = tmp;
	    currPos--;
	  }
	  setCount++;
	}
      }
      delete jset;

      Array<Array<double>*> counts;
      Array<Array<double>*>* d1Counts = new Array<Array<double>*>;
      Array<Array<double>*>* d2Counts = new Array<Array<double>*>;
      d1Counts->growToSize(Clause::pow(2,numNonEqual-1)-1);
      d2Counts->growToSize(Clause::pow(2,numNonEqual-1)-1);
  
      setUpCounts(numNonEqual, d1Counts,d2Counts, 0);
  
      for(int dx = 0; dx < domains_->size(); dx++){
	Array<Array<double>*>* d1Cnt = new Array<Array<double>*>;
	Array<Array<double>*>* d2Cnt = new Array<Array<double>*>;
	d1Cnt->growToSize(Clause::pow(2,numNonEqual-1)-1);
	d2Cnt->growToSize(Clause::pow(2,numNonEqual-1)-1);

	setUpCounts(numNonEqual, d1Cnt,d2Cnt,0);

	Array<double>* dCounts = initClauses[ix]->allTemplateGroundings((*domains_)[dx], (*domains_)[dx]->getDB(), d1Cnt, d2Cnt, &d1Pos, &d2Pos, numNonEqual);
	for(int ix = 0; ix < d1Cnt->size(); ix++){
	  Array<double>* tmpArr = (*d1Cnt)[ix];
	  for(int jx = 0; jx < tmpArr->size(); jx++){
	    (*((*d1Counts)[ix]))[jx] += (*tmpArr)[jx];
	  }
	  tmpArr = (*d2Cnt)[ix];
	  for(int jx = 0; jx < tmpArr->size(); jx++){
	    (*((*d2Counts)[ix]))[jx] += (*tmpArr)[jx];
	  }
	}
	d1Cnt->deleteItemsAndClear();
	d2Cnt->deleteItemsAndClear();
	delete d1Cnt;
	delete d2Cnt;

	counts.append(dCounts);
      }
      //cout << "J DAWG Like\n";

      //cout << "HERE\n";
      Array<double> trueCnts;
      Array<double> possCnts;
      Array<double> probs;
      Array<int> sense;
      double sum = 0;
      double total = 0;
      ClauseHashArray cliques;
      Array<Clause*> forDecomp;
      Array<ClauseHashArray*> decomp;
      Array<int> mult;


      while(acc.getNextCombination(sense)){
	Clause* c1 = new Clause(*initClauses[ix]);
	int index = 0;
	for(int jx = 0; jx < pos.size(); jx++){
	  Predicate* pred = c1->getPredicate(pos[jx]);
	  if (sense[jx] == 0){

	    pred->setSense(false);
	  }
	  else{
	    index += Clause::pow(2, (pos.size() - jx - 1));	    
	    pred->setSense(true);
	  }
	}
	Clause* r1 = new Clause(*c1);
	c1->setDirty();
	c1->canonicalize();
	if (cliques.contains(c1)){
	  int dupIndex = cliques.find(c1);
	  mult[dupIndex]++;
	  delete c1;
	  continue;
	}
	cliques.append(c1);
	forDecomp.append(r1);
	mult.append(1);

	double ntg_c1 = 0;
	double ng_c1 = 0;

	for(int dx = 0; dx < counts.size(); dx++){
	  ntg_c1 += (*(counts[dx]))[index];
	}
	ntg_c1 = ntg_c1 - (counts.size()-1);
	c1->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << ntg_c1 << endl;
	sum += (ng_c1 - ntg_c1);
	total = ng_c1;
	trueCnts.append(ntg_c1);
	possCnts.append(ng_c1);
	probs.append(ntg_c1);
      }

      Array<double> indProb;
      double indNorm = 0;
      for(int jx = 0; jx < probs.size(); jx++){
	const Array<Predicate*>* preds = forDecomp[jx]->getPredicates();
	double unitLogScore = 1;
	for(int kx = 0; kx < unitProbs.size(); kx++){
	  //cout << unitProbs[kx] << " (" << (*preds)[kx]->getSense() << ") ";
	  if ( (*preds)[kx]->getSense() == 0){
	    unitLogScore = unitLogScore * (1-unitProbs[kx]);
	  }
	  else{
	    unitLogScore = unitLogScore * (unitProbs[kx]);
	  }
	}
	indNorm += unitLogScore;
	indProb.append(unitLogScore);
      }
      double denom = Clause::pow(2, initClauses[ix]->getNumPredicates());
      for(int jx = 0; jx < probs.size(); jx++){
	probs[jx] += mult[jx] / denom;
      }
      
      //FLAG Likelihood calc
      //Do calculations
      double norm = 0;
      for(int jx = 0; jx < probs.size(); jx++){
	norm += probs[jx];
      }
      Array<double> probsNorm;
      for(int jx = 0; jx < probs.size(); jx++){
	cout << "Jx " << jx << " " << probs[jx] << " " << norm << " ";
	cout << (probs[jx]/norm) << endl;
	probsNorm.append((probs[jx]/norm));
      }

 
      long double logCliq = 0;
      long double logInd =0;
      out.precision(10);
      cout.precision(10);
      for(int jx = 0; jx < probs.size(); jx++){
	long double logScore = probs[jx] * log(probsNorm[jx]);
	logCliq += logScore;
	out << "feature ";
      	forDecomp[jx]->printWithoutWt(out, (*domains_)[0]);

	
	//cout << "feature ";
      	forDecomp[jx]->printWithoutWt(cout, (*domains_)[0]);

	const Array<Predicate*>* preds = forDecomp[jx]->getPredicates();

	long double normalized = indProb[jx]/indNorm;
	long double unitLogScore = probs[jx] * log(normalized);
	long double feat_norm = (logScore - unitLogScore) / norm;
	out << " :" << feat_norm << ": ";
	cout << " " << feat_norm << " :";
	cout << " " << probsNorm[jx] << " " << logScore << " | ";
	cout << normalized << " (" << indProb[jx] << "/" << indNorm << ") ";
	out << " " << probsNorm[jx] << " " << logScore << " | ";
	out << normalized << " " << unitLogScore;
	for(int kx = 0; kx < unitProbs.size(); kx++){
	  cout << unitProbs[kx] << " (" << (*preds)[kx]->getSense() << ") ";
	}
	cout << endl;
	logInd+=unitLogScore;
	out << endl;
      }
      //Flag J

      long double logDiff = logCliq - logInd;
      long double normLogDiff = logDiff / norm;
      cout << "Logs " << logCliq << " " << logInd << " ";
      cout << logDiff << " " << normLogDiff << endl;
      out << "Logs: " << logCliq << " " << logInd << " ";
      out << logDiff << " " << normLogDiff << endl;
      
      forDecomp.deleteItemsAndClear();
      cliques.deleteItemsAndClear();
      acc.deleteArraysAndClear();
      out << endl;
      double endSec = timer_.time();
      cout << "Clique Score Time: " << (endSec - begSec) << endl << endl;
      d1Pos.deleteItemsAndClear();
      d2Pos.deleteItemsAndClear();
      d1Counts->deleteItemsAndClear();
      d2Counts->deleteItemsAndClear();
      delete d1Counts;
      delete d2Counts;
      //delete dCounts;

    }//end of ix
    //out << "Max: " << maxScore << endl;
    out.close();
  }


  void buildCliqueFast(Clause* clause, Array<double>* probs, Array<int>* idx, 
		       ClauseHashArray* clauses,
		      Array<double>* counts){
    const Array<Predicate*>* preds = clause->getPredicates();
    int numNonEqual = 0;
    ArraysAccessor<int> acc;
    Array<int> pos;
    for(int jx = 0; jx < preds->size(); jx++){
      if (!(*preds)[jx]->isEqualPred()){
	numNonEqual++;
	pos.append(jx);
	Array<int>* vals = new Array<int>;
	vals->append(0);
	vals->append(1);
	acc.appendArray(vals);
      }
    }

    Array<double> trueCnts;
    ClauseHashArray cliques;
    Array<int> sense;
    while(acc.getNextCombination(sense)){
      Clause* c1 = new Clause(*clause);
      int index = 0;
      for(int jx = 0; jx < pos.size(); jx++){
	Predicate* pred = c1->getPredicate(pos[jx]);
	if (sense[jx] == 0){
	  
	  pred->setSense(false);
	}
	else{
	  /*
	    I flipped this like an idiot
	    index += Clause::pow(2, (pos.size() - jx - 1));
	  */
	  int predIdx = pred->getClausePos();
	  index += Clause::pow(2, predIdx);//(pos.size() - jx - 1));
	  //cout << "predIdx = " << predIdx << " index " << index << endl;
	  pred->setSense(true);
	}
      }
      c1->setDirty();
      c1->canonicalize();
      if (cliques.contains(c1)){
	(*idx)[cliques.find(c1)]++;
	delete c1;
	continue;
      }
      cliques.append(c1);
      clauses->append(c1);
      idx->append(1);//cliques.find(c1));
      //cout << "counts " << counts->size() << " index " << index << endl;
      double ntg_c1 = (*counts)[index];
      for(int dx = 0; dx < counts->size(); dx++){
	
	//ntg_c1 += (*((*counts)[dx]))[index];
	/*
	cout << "ntg_c1 " << ntg_c1 << " dx ";
	cout << dx << " index " << index << " value ";
	cout << (*(counts[dx]))[index] << endl;
	*/
      }
      //ntg_c1 = ntg_c1 - (counts->size()-1);
      //double ng_c1 = 0; //c1->getNumDistinctGroundings((*domains_)[0]); 
      //cerr << "here\n";
      c1->printWithoutWt(cout, (*domains_)[0]);
      cout << " " << ntg_c1 << endl;//" " << ng_c1 << endl;
      trueCnts.append(ntg_c1);
      //possCnts.append(ng_c1);
      probs->append(ntg_c1);//(ntg_c1/ng_c1));
    }
    //cliques.deleteItemsAndClear();

    //Do calculations
    double norm = 0;
    for(int ix = 0; ix < probs->size(); ix++){
      norm += (*probs)[ix];
    }
    for(int ix = 0; ix < probs->size(); ix++){
      (*probs)[ix] = (*probs)[ix] / norm;
      
      cliques[ix]->printWithoutWt(cout, (*domains_)[0]);
      cout << " " << (*probs)[ix] << endl;
      
    }
    cout << endl;
    acc.deleteArraysAndClear();
    //counts.deleteItemsAndClear();
  }


  //end Super Fast

  void newTransfer(Array<Clause*> initClauses, int scoreMetric){
    
    string fname = outMLNFileName_;
    ofstream out(fname.c_str());
    if (!out.good()) { cout << "ERROR: failed to open " <<fname<<endl;exit(-1);}
 


    for(int ix = 0; ix < initClauses.size(); ix++){
      out << "Evaluate: ";
      initClauses[ix]->printWithoutWt(out, (*domains_)[0]);
      out << endl;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      int numNonEqual = 0;
      ArraysAccessor<int> acc;
      Array<int> pos;
      for(int jx = 0; jx < preds->size(); jx++){
	if (!(*preds)[jx]->isEqualPred()){
	  numNonEqual++;
	  pos.append(jx);
	  Array<int>* vals = new Array<int>;
	  vals->append(0);
	  vals->append(1);
	  acc.appendArray(vals);
	}
      }
 
      Array<double> trueCnts;
      Array<double> possCnts;
      Array<double> probs;
      Array<int> sense;
      double sum = 0;
      double total = 0;
      ClauseHashArray cliques;
      Array<Clause*> forDecomp;
      Array<ClauseHashArray*> decomp;
      Array<int> mult;
      int indexCnt = 0;
      while(acc.getNextCombination(sense)){
	Clause* c1 = new Clause(*initClauses[ix]);
	for(int jx = 0; jx < pos.size(); jx++){
	  Predicate* pred = c1->getPredicate(pos[jx]);
	  if (sense[jx] == 0){
	    pred->setSense(false);
	  }
	  else{
	    pred->setSense(true);
	  }
	}
	Clause* r1 = new Clause(*c1);
	c1->setDirty();
	c1->canonicalize();
	if (cliques.contains(c1)){
	  int dupIndex = cliques.find(c1);
	  mult.append(dupIndex);
	  //delete c1;
	  //continue;
	}
	else{
	  mult.append(indexCnt);
	  //mult.append(1);
	}
	cliques.append(c1);
	forDecomp.append(r1);
	indexCnt++;

	double ntg_c1 = 1;//c1->getNumTrueGroundings((*domains_)[0], (*domains_)[0]->getDB(), false);
	double ng_c1 = 0;// c1->getNumDistinctGroundings((*domains_)[0]);      
	for(int dx = 0; dx < domains_->size(); dx++){
	  ntg_c1 += c1->getNumTrueGroundingsAnd((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	  ng_c1 = c1->getNumDistinctGroundings((*domains_)[dx]);      
	}
	/*
	r1->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << r1->getNumTrueGroundingsAnd((*domains_)[0], (*domains_)[0]->getDB(), false) << " / ";
	*/
	c1->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << ntg_c1 << endl;//" " << ng_c1 << endl;
	//cout << " Hash Code " << c1->hashCode() << endl;
	sum += (ng_c1 - ntg_c1);
	total = ng_c1;
	trueCnts.append(ntg_c1);
	possCnts.append(ng_c1);
	probs.append(ntg_c1);
      }
      double norm = 0;
      for(int jx = 0; jx < probs.size(); jx++){
	if (mult[jx] == jx){
	  norm += probs[jx];
	}
      }
      for(int jx = 0; jx < probs.size(); jx++){
	probs[jx] = probs[jx] / norm;
	forDecomp[jx]->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << probs[jx] << endl;
      }
      if (preds->size() == 2){
	int featureIdx = 0;
	double klScore = 0;
	for(int jx = 0; jx < forDecomp.size(); jx++){
	  const Array<Predicate*>* topPreds = forDecomp[jx]->getPredicates();
	  if (mult[jx] == jx){
	    Array<double> subProbs;
	    
	    subProbs.growToSize(topPreds->size());
	    for(int kx = 0; kx < subProbs.size(); kx++){
	      subProbs[kx] = 0;
	    }
	    for(int kx = 0; kx < forDecomp.size(); kx++){
	      const Array<Predicate*>* currPreds = forDecomp[kx]->getPredicates();
	      //for(int px = 0; px < currPreds->size(); px++){
	      if ((*topPreds)[0]->getSense() == (*currPreds)[0]->getSense()){
		  subProbs[0] += probs[kx];
	      }
	      if ((*topPreds)[1]->getSense() == (*currPreds)[1]->getSense()){
		  subProbs[1] += probs[kx];
	      }
	      
	    }
	    double ratio = probs[jx] / (subProbs[0] * subProbs[1]);
	    double nRatio = ratio;
	    klScore += probs[jx] * log(ratio);
	    if (scoreMetric == 6){
	      ratio = probs[jx] * log( probs[jx] / (subProbs[0] * subProbs[1]));
	      nRatio = ratio;
	      if (ratio < 0){
		ratio = 0 - ratio;
	      }
	    }
	    else{
	      if (ratio > 1){
		ratio = ratio - 1;
	      }
	      else{
		ratio = 1 - ratio;
	      }
	    }
	    out << "Feature-" << featureIdx << ": ";
	    cout << "Feature-" << featureIdx << ": ";
	    featureIdx++;
	    forDecomp[jx]->printConjunction(cout, (*domains_)[0]);
	    cout << " :" << ratio << ": (" << nRatio << ") " << probs[jx] << " " << (subProbs[0] * subProbs[1]) << " "
		 << subProbs[0] << " " << subProbs[1] << endl;
	    forDecomp[jx]->printConjunction(out, (*domains_)[0]);
	    out << " :" << ratio << ": (" << nRatio << ") "  << probs[jx] << " " << (subProbs[0] * subProbs[1]) << " "
		<< subProbs[0] << " " << subProbs[1] << endl;
	  }
	}
	
	cout << "Min: " << klScore << endl << endl;
	out << "Min: " << klScore << endl << endl;
      }
      else{
	int featureIdx = 0;
	Array<double> klScores;
	for(int jx = 0; jx < 3; jx++){
	  klScores.append(0.0);
	}
	for(int jx = 0; jx < forDecomp.size(); jx++){
	  const Array<Predicate*>* topPreds = forDecomp[jx]->getPredicates();

	  if (mult[jx] == jx){
	    Array<double> ratios;
	    Array<double> iRatio;
	    Array<double> trueProb;
	    Array<double> predProb;
	    Array<double> s1;
	    Array<double> s2;
	    double maxScore = 0;
	    for(int dx = 0; dx < 3; dx++){
	      int i1 = 0;
	      int i2 = 1;
	      int i3 = 2;
	      if (dx == 1){
		i1 = 0;
		i2 = 2;
		i3 = 1;
	      }
	      if (dx == 2){
		i1 = 1;
		i2 = 2;
		i3 = 0;
		
	      }
	      Array<double> subProbs;
	      subProbs.growToSize(topPreds->size());
	      for(int kx = 0; kx < subProbs.size(); kx++){
		subProbs[kx] = 0;
	      }
	      for(int kx = 0; kx < forDecomp.size(); kx++){
		const Array<Predicate*>* currPreds = forDecomp[kx]->getPredicates();
		//for(int px = 0; px < currPreds->size(); px++){
		if ((*topPreds)[i3]->getSense() == (*currPreds)[i3]->getSense()){
		  subProbs[1] += probs[kx];
		}

		if (((*topPreds)[i1]->getSense() == (*currPreds)[i1]->getSense()) &&
		    ((*topPreds)[i2]->getSense() == (*currPreds)[i2]->getSense())){
		  subProbs[0] += probs[kx];
		}

		
	      }
	      double ratio = probs[jx] / (subProbs[0] * subProbs[1]);
	      klScores[dx] += probs[jx] * log(ratio);
	      if (scoreMetric == 6){
		ratio = probs[jx] * log( probs[jx] / (subProbs[0] * subProbs[1]));
		iRatio.append(ratio);
		if (ratio < 0){
		  ratio = 0 - ratio;
		}
	      }
	      else{
		iRatio.append(ratio);
		if (ratio > 1){
		  ratio = ratio - 1;
		}
		else{
		  ratio = 1 - ratio;
		}
	      }
	      ratios.append(ratio);
	      trueProb.append(probs[jx]);
	      predProb.append((subProbs[0] * subProbs[1]));
	      s1.append(subProbs[0]);
	      s2.append(subProbs[1]);
	      if (dx == 0){
		maxScore = ratio;
		out << "Feature-" << featureIdx << ": ";
		cout << "Feature-" << featureIdx << ": ";
		featureIdx++;
		forDecomp[jx]->printConjunction(cout, (*domains_)[0]);
		forDecomp[jx]->printConjunction(out, (*domains_)[0]);
	      }
	      if (ratio < maxScore){
		maxScore = ratio;
	      }

	      //<< " "<< subProbs[0] << " " << subProbs[1] << endl;
	      /*
		out << " " << ratio << " " << probs[jx] << " " << (subProbs[0] * subProbs[1]) << " "
		<< subProbs[0] << " " << subProbs[1] << endl;
	      */
	    }
	    cout << " :" << maxScore << ": ";
	    out << " :" << maxScore << ": ";
	    for(int jj = 0; jj < ratios.size(); jj++){
	      cout << ratios[jj] << " (" << iRatio[jj] << ") " << trueProb[jj] << "/" << predProb[jj] << " " << s1[jj] << "*" << s2[jj] << " | ";
	      out << ratios[jj] << " (" << iRatio[jj] << ") " << trueProb[jj] << "/" << predProb[jj] << " ";
	      
	    }
	    cout << endl;
	    out << endl;
	    
	  }
	}
	
         
	double minKL = klScores[0];
	for(int jj = 0; jj < klScores.size(); jj++){
	  out << klScores[jj] << " ";
	  cout << klScores[jj] << " ";
	  if (klScores[jj] < minKL){
	    minKL = klScores[jj];
	  }
	}
	cout << endl;
	out << endl;
	cout << "Min: " << minKL << endl << endl;
	out << "Min: " << minKL << endl << endl;
      }
    }
  }
  void totTransfer(Array<Clause*> initClauses){
    string fname = outMLNFileName_;
    ofstream out(fname.c_str());
    for(int ix = 0; ix < initClauses.size(); ix++){
      out << "Evaluate: ";
      initClauses[ix]->printWithoutWt(out, (*domains_)[0]);
      out << endl;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      int numNonEqual = 0;
      ArraysAccessor<int> acc;
      Array<int> pos;
      for(int jx = 0; jx < preds->size(); jx++){
        if (!(*preds)[jx]->isEqualPred()){
          numNonEqual++;
          pos.append(jx);
          Array<int>* vals = new Array<int>;
          vals->append(0);
          vals->append(1);
          acc.appendArray(vals);
        }
      }
      Array<int> sense;
      ClauseHashArray cliques;
      //Array
      int featIdx = 0;
      while(acc.getNextCombination(sense)){
	Clause* c1 = new Clause(*initClauses[ix]);
	for(int jx = 0; jx < pos.size(); jx++){
	  Predicate* pred = c1->getPredicate(pos[jx]);
	  if (sense[jx] == 0){
	    pred->setSense(false);
	  }
	  else{
	    pred->setSense(true);
	  }
	}
	//Clause* r1 = new Clause(*c1);
	c1->setDirty();
	c1->canonicalize();
	if (cliques.contains(c1)){
	  delete c1;
	  continue;
	}
	featIdx++;
	cliques.append(c1);
	//forDecomp.append(r1);
	double ntg_c1 = 0;//1;//c1->getNumTrueGroundings((*domains_)[0], (*domains_)[0]->getDB(), false);
	double ng_c1 = 0;//1;// c1->getNumDistinctGroundings((*domains_)[0]); 
	for(int dx = 0; dx < domains_->size(); dx++){
	  ng_c1 += c1->getNumDistinctGroundings((*domains_)[dx]);
	}
	double prob = ntg_c1 / ng_c1;
	c1->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << ntg_c1 << "/" << ng_c1 << " " << (ntg_c1/ng_c1) << endl;      
	if (c1->getNumPredicates() == 2){
	  Clause* d1 = new Clause(*c1);
	  Clause* d2 = new Clause(*c1);
	  d1->removePredicate(1);
	  d2->removePredicate(0);
	  d1->canonicalize();
	  d2->canonicalize();

	  double ntg_d1 = 0, ntg_d2=0, ng_d1=0,ng_d2=0;
	  for(int dx = 0; dx < domains_->size(); dx++){
	    ng_d1  += d1->getNumDistinctGroundings((*domains_)[dx]);
	    ng_d2  += d2->getNumDistinctGroundings((*domains_)[dx]);
	  }
	  out << "Feature-" << featIdx << ":";
	  c1->printWithoutWt(out, (*domains_)[0]); 
	  out << ":";
	  out << ntg_c1 << "[" << ng_c1 << "/";
	  out << (ng_d1) << " " << (ng_d2) << "]" << endl;
	  d1->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " << ntg_d1 << "/" <<  ng_d1 << " " << (ntg_d1/ng_d1) << " ";
	  d2->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " << ntg_d2 << "/" <<  ng_d2 << " " << (ntg_d2/ng_d2);
	  cout << endl;
	  cout << ntg_c1 << "[" << prob;// " delta)";
	  cout << "/" << (ng_d1) << " " << (ng_d2) << "]" <<  endl;
	  delete d1;
	  delete d2;
	}
	else{
	  Array<double> top;
	  Array<double> bottom;
	  for(int jx = 0; jx < c1->getNumPredicates(); jx++){
	    Clause* d1 = new Clause(*c1);
	    Clause* d2 = new Clause();
	    Predicate* predNew = d1->removePredicate(jx);
	    d2->appendPredicate(predNew);
	    d1->canonicalize();
	    d2->canonicalize();
	    double ntg_d1 = 0, ntg_d2=0, ng_d1=0,ng_d2=0;
	    for(int dx = 0; dx < domains_->size(); dx++){
	      ng_d1  += d1->getNumDistinctGroundings((*domains_)[dx]);
	      ng_d2  += d2->getNumDistinctGroundings((*domains_)[dx]);
	    }
	    d1->printWithoutWt(cout, (*domains_)[0]);
	    cout << " " << ntg_d1 << "/" <<  ng_d1 << " ";
	    cout << " ";
	    d2->printWithoutWt(cout, (*domains_)[0]);
	    cout << " " << ntg_d2 << "/" <<  ng_d2 << " "; 
	    cout << endl;

	    bottom.append(ng_d1);
	    top.append(ng_d2);
	    delete d1;
	    delete d2;
	  }
	  out << "Feature-" << featIdx << ":";
	  c1->printWithoutWt(out, (*domains_)[0]); 
	  out << ":";
	  out << ntg_c1 << "[" << ng_c1 << " ";

	  cout << "Feature-" << featIdx << ":";
	  c1->printWithoutWt(cout, (*domains_)[0]);
	  cout << ":" <<  ntg_c1 << "[" << ntg_c1 << " "; 
	  cout << " ";

	  for(int jx = 0; jx < top.size(); jx++){
	    cout << top[jx] << " ";
	    out << top[jx];
	    if (jx < (top.size()-1)){
	      out << " ";
	    }
	  }
	  cout << "/";
	  out << "/";
	  for(int jx = 0; jx < bottom.size(); jx++){
	    cout <<  bottom[jx] << " ";
	    out <<  bottom[jx];// << " ";
	    if (jx < (bottom.size()-1)){
	      out << " ";
	    }
	  }
	  cout << "]" << endl;
	  out << "]" << endl;
	}
      }
    }
  }

  void newNewTransfer(Array<Clause*> initClauses){
    string fname = outMLNFileName_;
    ofstream out(fname.c_str());
    for(int ix = 0; ix < initClauses.size(); ix++){
      out << "Evaluate: ";
      initClauses[ix]->printWithoutWt(out, (*domains_)[0]);
      out << endl;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      int numNonEqual = 0;
      ArraysAccessor<int> acc;
      Array<int> pos;
      for(int jx = 0; jx < preds->size(); jx++){
        if (!(*preds)[jx]->isEqualPred()){
          numNonEqual++;
          pos.append(jx);
          Array<int>* vals = new Array<int>;
          vals->append(0);
          vals->append(1);
          acc.appendArray(vals);
        }
      }
      Array<int> sense;
      ClauseHashArray cliques;
      //Array
      int featIdx = 0;
      while(acc.getNextCombination(sense)){
	Clause* c1 = new Clause(*initClauses[ix]);
	for(int jx = 0; jx < pos.size(); jx++){
	  Predicate* pred = c1->getPredicate(pos[jx]);
	  if (sense[jx] == 0){
	    pred->setSense(false);
	  }
	  else{
	    pred->setSense(true);
	  }
	}
	//Clause* r1 = new Clause(*c1);
	c1->setDirty();
	c1->canonicalize();
	if (cliques.contains(c1)){
	  delete c1;
	  continue;
	}
	featIdx++;
	cliques.append(c1);
	//forDecomp.append(r1);
	double ntg_c1 = 0;//1;//c1->getNumTrueGroundings((*domains_)[0], (*domains_)[0]->getDB(), false);
	double ng_c1 = 0;//1;// c1->getNumDistinctGroundings((*domains_)[0]); 
	for(int dx = 0; dx < domains_->size(); dx++){
	  ntg_c1 += c1->getNumTrueGroundingsAnd((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	  ng_c1 += c1->getNumDistinctGroundings((*domains_)[dx]);
	}
	double prob = ntg_c1 / ng_c1;
	c1->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << ntg_c1 << "/" << ng_c1 << " " << (ntg_c1/ng_c1) << endl;      
	if (c1->getNumPredicates() == 2){
	  Clause* d1 = new Clause(*c1);
	  Clause* d2 = new Clause(*c1);
	  d1->removePredicate(1);
	  d2->removePredicate(0);
	  d1->canonicalize();
	  d2->canonicalize();

	  double ntg_d1 = 0, ntg_d2=0, ng_d1=0,ng_d2=0;
	  for(int dx = 0; dx < domains_->size(); dx++){
	    ntg_d1 += d1->getNumTrueGroundingsAnd((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	    ng_d1  += d1->getNumDistinctGroundings((*domains_)[dx]);
	    ntg_d2 += d2->getNumTrueGroundingsAnd((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	    ng_d2  += d2->getNumDistinctGroundings((*domains_)[dx]);
	  }
	  out << "Feature-" << featIdx << ":";
	  c1->printWithoutWt(out, (*domains_)[0]); 
	  out << ":";
	  out << ntg_c1 << "[" << prob << "-" << ntg_c1 << "/";
	  out << (ntg_d1/ng_d1) << "-" << ntg_d1 << " " << (ntg_d2/ng_d2) << "]" << endl;
	  d1->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " << ntg_d1 << "/" <<  ng_d1 << " " << (ntg_d1/ng_d1) << " ";
	  d2->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " << ntg_d2 << "/" <<  ng_d2 << " " << (ntg_d2/ng_d2);
	  cout << endl;
	  cout << ntg_c1 << "[" << prob;// " delta)";
	  cout << "/" << (ntg_d1/ng_d1) << " " << (ntg_d2/ng_d2) << "]" <<  endl;
	  delete d1;
	  delete d2;
	}
	else{
	  Array<double> top;
	  Array<double> bottom;
	  for(int jx = 0; jx < c1->getNumPredicates(); jx++){
	    Clause* d1 = new Clause(*c1);
	    Clause* d2 = new Clause();
	    Predicate* predNew = d1->removePredicate(jx);
	    d2->appendPredicate(predNew);
	    d1->canonicalize();
	    d2->canonicalize();
	    double ntg_d1 = 0, ntg_d2=0, ng_d1=0,ng_d2=0;
	    for(int dx = 0; dx < domains_->size(); dx++){
	      ntg_d1 += d1->getNumTrueGroundingsAnd((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	      ng_d1  += d1->getNumDistinctGroundings((*domains_)[dx]);
	      ntg_d2 += d2->getNumTrueGroundingsAnd((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	      ng_d2  += d2->getNumDistinctGroundings((*domains_)[dx]);
	    }
	    d1->printWithoutWt(cout, (*domains_)[0]);
	    cout << " " << ntg_d1 << "/" <<  ng_d1 << " " << (ntg_d1/ng_d1);
	    cout << " ";
	    d2->printWithoutWt(cout, (*domains_)[0]);
	    cout << " " << ntg_d2 << "/" <<  ng_d2 << " " << (ntg_d2/ng_d2);
	    cout << endl;

	    bottom.append(ntg_d1/ng_d1);
	    top.append(ntg_d2/ng_d2);
	    delete d1;
	    delete d2;
	  }
	  out << "Feature-" << featIdx << ":";
	  c1->printWithoutWt(out, (*domains_)[0]); 
	  out << ":";
	  out << ntg_c1 << "[" << prob << " ";

	  cout << "Feature-" << featIdx << ":";
	  c1->printWithoutWt(cout, (*domains_)[0]);
	  cout << ":" <<  ntg_c1 << "[" << prob << " "; 
	  cout << " ";

	  for(int jx = 0; jx < top.size(); jx++){
	    cout << top[jx] << " ";
	    out << top[jx];
	    if (jx < (top.size()-1)){
	      out << " ";
	    }
	  }
	  cout << "/";
	  out << "/";
	  for(int jx = 0; jx < bottom.size(); jx++){
	    cout <<  bottom[jx] << " ";
	    out <<  bottom[jx];// << " ";
	    if (jx < (bottom.size()-1)){
	      out << " ";
	    }
	  }
	  cout << "]" << endl;
	  out << "]" << endl;
	}
      }
    }
  }

  void testTransfer(Array<Clause*> initClauses){

    double maxScore = 0;
    string fname = outMLNFileName_;
    ofstream out(fname.c_str());

    if (!out.good()) { cout << "ERROR: failed to open " <<fname<<endl;exit(-1);}


    for(int ix = 0; ix < initClauses.size(); ix++){
      //Flag Eval Time
      double begSec = timer_.time();
      out << "Evaluate: ";
      initClauses[ix]->printWithoutWt(out, (*domains_)[0]);
      out << endl;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      int numNonEqual = 0;
      ArraysAccessor<int> acc;
      Array<int> pos;
      for(int jx = 0; jx < preds->size(); jx++){
	if (!(*preds)[jx]->isEqualPred()){
	  numNonEqual++;
	  pos.append(jx);
	  Array<int>* vals = new Array<int>;
	  vals->append(0);
	  vals->append(1);
	  acc.appendArray(vals);
	}
      }
      PowerSet* jset = new PowerSet;
      jset->create(numNonEqual);
      jset->prepareAccess(numNonEqual,false);
      const Array<int>* part;// = new Array<int>;
      int currPos = 0;
      int setCount = 0;
      Array<Array<int>*> d1Pos;
      Array<Array<int>*> d2Pos;
      d1Pos.growToSize(Clause::pow(2,numNonEqual-1)-1);
      d2Pos.growToSize(d1Pos.size());
      while(jset->getNextSet(part)){
	if (part->size() < numNonEqual){
	  Array<int>* tmp = new Array<int>;
	  tmp->growToSize(part->size());
	  //cout << "Cur Pos: " << currPos << " ";
	  for(int jx = 0; jx < tmp->size(); jx++){
	    (*tmp)[jx] = (*part)[jx];
	    //cout << "PredIdx = " << (*part)[jx] << " ";
	  }
	  //	  cout << endl;
	  if (setCount < d1Pos.size()){
	    d1Pos[currPos] = tmp;
	    if (currPos < (d1Pos.size()-1)){
	      currPos++;
	    }
	  }
	  else{
	    d2Pos[currPos] = tmp;
	    currPos--;
	  }
	  setCount++;
	}
      }
      delete jset;

      Array<Array<double>*> counts;
      Array<Array<double>*>* d1Counts = new Array<Array<double>*>;
      Array<Array<double>*>* d2Counts = new Array<Array<double>*>;
      d1Counts->growToSize(Clause::pow(2,numNonEqual-1)-1);
      d2Counts->growToSize(Clause::pow(2,numNonEqual-1)-1);
      //set up flag
      //cerr << "Here 1\n";
      setUpCounts(numNonEqual, d1Counts,d2Counts, 1);
       //cerr << "Here 2\n";
      for(int dx = 0; dx < domains_->size(); dx++){
	//cerr << "Here Domains " << dx << "\n";
	Array<Array<double>*>* d1Cnt = new Array<Array<double>*>;
	Array<Array<double>*>* d2Cnt = new Array<Array<double>*>;
	d1Cnt->growToSize(Clause::pow(2,numNonEqual-1)-1);
	d2Cnt->growToSize(Clause::pow(2,numNonEqual-1)-1);
	//set up flag
	setUpCounts(numNonEqual, d1Cnt,d2Cnt,0);

	//Array<double>* dCounts = initClauses[ix]->allTemplateGroundings((*domains_)[dx], (*domains_)[dx]->getDB(), d1Cnt, d2Cnt, &d1Pos, &d2Pos, numNonEqual);
	Array<double>* dCounts = buildItJesse(initClauses[ix], Clause::pow(2,numNonEqual), (*domains_)[dx],  d1Cnt, d2Cnt, &d1Pos, &d2Pos);
	for(int ix = 0; ix < d1Cnt->size(); ix++){
	  Array<double>* tmpArr = (*d1Cnt)[ix];
	  for(int jx = 0; jx < tmpArr->size(); jx++){
	    (*((*d1Counts)[ix]))[jx] += (*tmpArr)[jx];
	  }
	  tmpArr = (*d2Cnt)[ix];
	  for(int jx = 0; jx < tmpArr->size(); jx++){
	    (*((*d2Counts)[ix]))[jx] += (*tmpArr)[jx];
	  }
	}
	d1Cnt->deleteItemsAndClear();
	d2Cnt->deleteItemsAndClear();
	delete d1Cnt;
	delete d2Cnt;
	counts.append(dCounts);
      }

      Array<Array<double>*> dProbs1;
      Array<Array<int>*> idx1;
      Array<Array<double>*> dProbs2;
      Array<Array<int>*> idx2;
      Array<ClauseHashArray*> clause1;
      Array<ClauseHashArray*> clause2;


      for(int jx = 0; jx < d1Pos.size(); jx++){

	Clause* d1 = new Clause();
	Clause* d2 = new Clause();
	Array<int>* predPos = d1Pos[jx];
	for(int px = 0; px < predPos->size(); px++){
	  Predicate* p1 = new Predicate(*(*preds)[(*predPos)[px]]);
	  p1->setClausePos(px);
	  d1->appendPredicate(p1);
	}
	predPos = d2Pos[jx];
	for(int px = 0; px < predPos->size(); px++){
	  Predicate* p1 = new Predicate(*(*preds)[(*predPos)[px]]);
	  p1->setClausePos(px);
	  d2->appendPredicate(p1);
	}
	d1->canonicalize();
	d2->canonicalize();
	Array<double>* tmpDouble = new Array<double>;
	Array<int>* tmpInt = new Array<int>;
	ClauseHashArray* tmpClause = new ClauseHashArray;
	buildCliqueFast(d1, tmpDouble, tmpInt, tmpClause, (*d1Counts)[jx]);
	dProbs1.append(tmpDouble);
	idx1.append(tmpInt);
	clause1.append(tmpClause);
	tmpDouble = new Array<double>;
	tmpInt = new Array<int>;
	tmpClause = new ClauseHashArray;
	buildCliqueFast(d2, tmpDouble, tmpInt, tmpClause, (*d2Counts)[jx]);
	dProbs2.append(tmpDouble);
	idx2.append(tmpInt);
	clause2.append(tmpClause);
	delete d1;
	delete d2;
 	
      }

      Array<double> trueCnts;
      Array<double> possCnts;
      Array<double> probs;
      Array<int> sense;
      double sum = 0;
      double total = 0;
      ClauseHashArray cliques;
      Array<Clause*> forDecomp;
      Array<ClauseHashArray*> decomp;
      Array<int> mult;
    /*
    */

      while(acc.getNextCombination(sense)){
	Clause* c1 = new Clause(*initClauses[ix]);
	int index = 0;
	for(int jx = 0; jx < pos.size(); jx++){
	  Predicate* pred = c1->getPredicate(pos[jx]);
	  if (sense[jx] == 0){

	    pred->setSense(false);
	  }
	  else{
	    index += Clause::pow(2, jx);//(pos.size() - jx - 1));	    
	    pred->setSense(true);
	  }
	}
	Clause* r1 = new Clause(*c1);
	c1->setDirty();
	c1->canonicalize();
	if (cliques.contains(c1)){
	  int dupIndex = cliques.find(c1);
	  mult[dupIndex]++;
	  delete c1;
	  continue;
	}
	cliques.append(c1);
	forDecomp.append(r1);
	mult.append(1);

	double ntg_c1 = 0;
	double ng_c1 = 0;

	for(int dx = 0; dx < counts.size(); dx++){
	  ntg_c1 += (*(counts[dx]))[index];
	}
	ntg_c1 = ntg_c1 - (counts.size()-1);
	c1->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << ntg_c1 << endl;
	sum += (ng_c1 - ntg_c1);
	total = ng_c1;
	trueCnts.append(ntg_c1);
	possCnts.append(ng_c1);
	probs.append(ntg_c1);
      }

      //Do calculations
      double norm = 0;
      for(int jx = 0; jx < probs.size(); jx++){
	norm += probs[jx];
      }
      ClauseHashArray full1;
      ClauseHashArray full2;
      Array<double> newProbs1;
      Array<double> newProbs2;
      Array<int> mult1;
      Array<int> mult2;

      /*
	cout << clause1.size() << " " << clause2.size() << " "
	<< dProbs1.size() << " " << dProbs2.size() << endl;
      */
      for(int jx = 0; jx < clause1.size(); jx++){
	
	ClauseHashArray* c1Array = clause1[jx];
	ClauseHashArray* c2Array = clause2[jx];
	Array<double>* probs1 = dProbs1[jx];
	Array<double>* probs2 = dProbs2[jx];
	Array<int>* m1 = idx1[jx];
	Array<int>* m2 = idx2[jx];
	for(int kx = 0; kx < c1Array->size(); kx++){
	  if (!full1.contains((*c1Array)[kx])){
	    full1.append((*c1Array)[kx]);
	    newProbs1.append((*probs1)[kx]);
	    if (kx >= m1->size()){
	      cout << "Size mismatch\n";
	    }
	    if ((*m1)[kx] == 0){
	      cout << "bad " << jx << " " << kx << " ";
	      (*c1Array)[kx]->printWithoutWt(cout, (*domains_)[0]);
	      cout << endl;
	    }
	    mult1.append((*m1)[kx]);
	  }
	  else{
	    //cout << "Here\n";
	    int index = full1.find((*c1Array)[kx]);
	    //super mismatch
	    if (newProbs1[index] != (*probs1)[kx]){
	      cout << "Probability mismatch\n";
	      cout << "new probs: " << newProbs1[index] << " probs1 " << (*probs1)[kx] << endl;
	      cout << "index " << index << " kx " << kx << "\n";
	      (*c1Array)[kx]->printWithoutWt(cout, (*domains_)[0]);
	      cout << endl;
	    }
	    if (mult1[index] != (*m1)[kx]){
	      cout << "Multiplier mismatch\n";
	    }
	  }
	}
	for(int kx = 0; kx < c2Array->size(); kx++){
	  if (!full2.contains((*c2Array)[kx])){
	    full2.append((*c2Array)[kx]);
	    newProbs2.append((*probs2)[kx]);
	    mult2.append((*m2)[kx]);
	  }
	}
      }
      Array<double> klScores;
      for(int jx = 0; jx < dProbs1.size(); jx++){
	klScores.append(0.0);
      }
      Array<Array<double>*> probNorm;
      for(int jx = 0; jx < dProbs1.size(); jx++){
	Array<double>* tmpD = new Array<double>;
	probNorm.append(tmpD);
      }

      for(int jx = 0; jx < probs.size(); jx++){

	probs[jx] = probs[jx] / norm;
	//cliques[jx]->printWithoutWt(cout, (*domains_)[0]);
	//cout << " " << probs[jx] << " | ";
	//cout << dProbs1.size() << " " << dProbs2.size() << " -> ";
	//Work Here Jesse
	for(int kx = 0; kx < dProbs1.size(); kx++){
	  Clause* d1 = new Clause(*forDecomp[jx]);
	  Clause* d2 = new Clause(*forDecomp[jx]);
	  if (dProbs1.size() == 1){
	    Predicate* rmed = d1->removePredicate(1);
	    delete rmed;
	    rmed = d2->removePredicate(0);
	    delete rmed;
	  }
	  else{
	    //for(int dx = (d2Pos.size()-1); dx >= 0; dx--){
	    //for(int dx = 0; dx < d2Pos.size(); dx++){
	    Array<int>* predIdx = d2Pos[kx];
	    for(int px = (predIdx->size()-1); px >=0; px--){
	      Predicate* rmed = d1->removePredicate((*predIdx)[px]);
	      delete rmed;
	    }
	    
	    predIdx = d1Pos[kx];
	    for(int px = (predIdx->size()-1); px >=0; px--){
	      Predicate* rmed = d2->removePredicate((*predIdx)[px]);
	      delete rmed;
	    }
	  }
	  d1->canonicalize();
	  d2->canonicalize();
	  int index1 = full1.find(d1);
	  int index2 = full2.find(d2);
	  /*
	  cerr << "index1 " << index1 << " " << mult1.size() << endl;
	  cout << "kx: " << kx << " ";
	  d1->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";
	  d2->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";

	  exit(-1);
	  */
	  //Array<int>* c1Mult = idx1[kx];
	  //Array<int>* c2Mult = idx2[kx];
	  //cout << idx1.size() << " " << c1Mult->size() << " " << idx2.size() << " " << c2Mult->size() << endl;
	  if (mult1[index1] == 0){
	    cout << "This is bad!" << endl;
	    cout << "Missing: ";
	    d1->printWithoutWt(cout, (*domains_)[0]);
	    cout << endl;
	  }
	  //cout << "mults: " << kx << " mult1 " << (1.0/mult1[index1]) << " mult2 " << (1.0/mult2[index2]) << " index1 " << index1 << " index2 " << index2 << " " << mult1.size() << " " << mult2.size() << endl;
	  cout << "kx: " << kx << " ";
	  d1->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";
	  d2->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";

	  double predProb = newProbs1[index1] * newProbs2[index2] * (1.0/mult1[index1]) * (1.0/mult2[index2]);// * (1/(*c1Mult)[index1]) * (1/(*c2Mult)[index2]);//1 - ((1-newProbs1[index1]) * (1-newProbs2[index2]));
	  Array<double>* dp = probNorm[kx];
	  dp->append(predProb);
	  cout << " " << newProbs1[index1] << " " << newProbs2[index2] << " " << predProb << endl;
	  //cout << "kx: " << kx << " " << predProb << " ";
	  //double kl1 = thisProb * log(thisProb/prob1) + (1-thisProb) * log((1-thisProb)/(1-prob1));
	  //double kl1 = probs[jx] * log(probs[jx]/predProb) + (1-probs[jx]) * log((1-probs[jx])/(1-predProb));
	  //cout << newProbs1[index1] << " " << newProbs2[index2] << " (" << kl1 <<  "): "; //newProbs1[index1] << " " << newProbs2[index2] << ": ";
	  delete d1;
	  delete d2;
	  //klScores[kx] += kl1;
	} 
	cout << endl;
      }
      for(int jx = 0; jx < probNorm.size(); jx++){
	Array<double>* td = probNorm[jx];
	double z = 0;
	//cout << td->size() << " " << mult.size() << endl;
	
	for(int kx = 0; kx < td->size(); kx++){
	  z += (*td)[kx] * mult[kx];
	}
	cout << "check z = " << z;
	out << "check z = " << z << endl;
	if (z > 1.0 || z < 1.0){
	  cout << " z badness ";
	}
	cout << endl;
	/*
	  for(int kx = 0; kx < td->size(); kx++){
	  (*td)[kx] = (*td)[kx]  / z;
	  }
	*/
      }
      for(int jx = 0; jx < probs.size(); jx++){
	out << "feature ";
      	forDecomp[jx]->printWithoutWt(out, (*domains_)[0]);
	out << " " << probs[jx] << " | ";
	
	//cout << "feature ";
      	forDecomp[jx]->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << probs[jx] << " | ";
	
	for(int kx = 0; kx < probNorm.size(); kx++){
	  Array<double>* predProb = probNorm[kx];
	  double predFinal = mult[jx] * (*predProb)[jx];
	  //flag
	  double kl1 = probs[jx] * log(probs[jx]/predFinal);// + (1-probs[jx]) * log((1-probs[jx])/(1-predFinal));
	  //double kl1 = probs[jx] * log(probs[jx]/(*predProb)[jx]) + (1-probs[jx]) * log((1-probs[jx])/(1-(*predProb)[jx]));
	  //cout << newProbs1[index1] << " " << newProbs2[index2] << " (" << kl1 <<  "): "; //newProbs1[index1] << " " << newProbs2[index2] << ": ";
	  /*
	  out << (*predProb)[jx] << " (" << kl1 << "): ";
	  cout << (*predProb)[jx] << " (" << kl1 << "): ";
	  */

	  out << predFinal << " (" << kl1 << "): ";
	  cout << predFinal << " (" << kl1 << "): ";
	  klScores[kx] += kl1;
	}
	cout << endl;
	out << endl;
      }
      //Flag J
      double minScore = klScores[0];
      cout << "KL Scores: ";
      out << "KL Scores: ";
      for(int jx = 0; jx < klScores.size(); jx++){
	out << klScores[jx] << " ";
	cout << klScores[jx] << " ";
	if (klScores[jx] < minScore){
	  minScore = klScores[jx];
	}
      }
      if (maxScore < minScore){
	maxScore = minScore;
      }
      out << endl;
      cout << endl;
      out << "Min " << minScore << endl;
      for(int jx = 0; jx < probNorm.size(); jx++){
	Array<double>* tmp = probNorm[jx];
	delete tmp;
      } 
      for(int jx = 0; jx < clause1.size(); jx++){
	
	ClauseHashArray* ca1 = clause1[jx];
	ClauseHashArray* ca2 = clause2[jx];
	ca1->deleteItemsAndClear();
	ca2->deleteItemsAndClear();
	Array<double>* pr1 = dProbs1[jx];
	Array<double>* pr2 = dProbs2[jx];
	Array<int>* id1 = idx1[jx];
	Array<int>* id2 = idx2[jx];
	delete id1;
	delete id2;
	delete pr1;
	delete pr2;
	delete ca1;
	delete ca2;
	counts.deleteItemsAndClear(); 
      }
      /*
      for(int dx = 0; dx < trueDCnts.size(); dx++){
	Array<double>* tmp =  trueDCnts[dx];
	delete tmp;
      }
      for(int dx = 0; dx < possDCnts.size(); dx++){
	Array<double>* tmp =  possDCnts[dx];
	delete tmp;
      }
      */
      forDecomp.deleteItemsAndClear();
      cliques.deleteItemsAndClear();
      acc.deleteArraysAndClear();
      out << endl;
      double endSec = timer_.time();
      cout << "Clique Score Time: " << (endSec - begSec) << endl << endl;
      d1Pos.deleteItemsAndClear();
      d2Pos.deleteItemsAndClear();
      d1Counts->deleteItemsAndClear();
      d2Counts->deleteItemsAndClear();
      delete d1Counts;
      delete d2Counts;
      //delete dCounts;

    }//end of ix
    out << "Max: " << maxScore << endl;
    out.close();
  }


  Array<double>* buildItJesse(Clause* underEval, int countSize, const Domain * const & domain, Array<Array<double>*>* d1Counts, Array<Array<double>*>* d2Counts,Array<Array<int>*>* d1Idx, Array<Array<int>*>* d2Idx){

    Array<Clause*> decomp;
    PowerSet* jset = new PowerSet;
    int numNonEqual = underEval->getNumPredicates();
    jset->create(underEval->getNumPredicates());
    jset->prepareAccess(numNonEqual,false);
    const Array<int>* part;// = new Array<int>;
    const Array<Predicate*>* preds = underEval->getPredicates();
    //cerr << "HERE\n";
    Array<double> counts;
    Array<double> vals;
    Array<double> d1FirstGroundCnt;
    Array<double> d2FirstGroundCnt;
    d1FirstGroundCnt.growToSize(d1Counts->size());
    d2FirstGroundCnt.growToSize(d1Counts->size());
    Array<double> d1PossGround;
    Array<double> d2PossGround;
    d1PossGround.growToSize(d1Counts->size());
    d2PossGround.growToSize(d1Counts->size());
    Array<double>* finalCnts = new Array<double>;
    finalCnts->growToSize(countSize,1);
    Array<double> numGrounding;
    //vals.growToSize(countSize);
    Array<Array<bool>*> positive;
    //J$ Flag
    double posGround = underEval->getNumDistinctGroundings(domain);
    while(jset->getNextSet(part)){
      Clause* c1 = new Clause;
      double ntg_c1 = 0;
      Array<bool>* present = new Array<bool>;
      present->growToSize(preds->size(), false);
      int index = 0;
      for(int jx = 0; jx < part->size(); jx++){
	int idx = (*part)[jx];
	(*present)[idx] = true;
	//cerr << idx << " ";
	Predicate* p1 = new Predicate(*((*preds)[idx]));
	if ((*present)[idx]){
	  index += Clause::pow(2,idx); 
	}
	c1->appendPredicate(p1);
      }
      positive.append(present);
      c1->canonicalize();
      //      c1->printWithoutWt(cerr, domain);
      //for(int dx = 0; dx < domains_->size(); dx++){
      if (decomp.contains(c1)){
	int dupIndex = decomp.find(c1);
	ntg_c1 = counts[dupIndex];
      }
      else{
	ntg_c1 += c1->getNumTrueGroundingsAnd(domain, domain->getDB(), false);
	//ng_c1 = c1->getNumDistinctGroundings((*domains_)[dx]);      
	//}
	decomp.append(c1);
      }
      counts.append(ntg_c1);
      //}
      //for(){
      double curCount = 0;
      if (part->size() == preds->size()){
	curCount = ntg_c1;
	vals.append(ntg_c1);//[ix] = counts[ix];
      }
      else{
	double posC1 = c1->getNumDistinctGroundings(domain);
	//cout << "posC1 " << posC1 << " posGround " << posGround << " " << domain->getNumConstantsByType(0) << " " << domain->getNumConstantsByType(1) << " " << domain->getNumConstantsByType(2) << endl;
	numGrounding.append(posC1);
	double offset = posGround / posC1;
	curCount = ntg_c1 * offset;//counts[ix];
	for(int jx = 0; jx < d1Counts->size(); jx++){
	  if (matchSub(part, (*d1Idx)[jx])){
	    d1FirstGroundCnt[jx] = offset;
	    d1PossGround[jx] = posC1;
	  }
	  if (matchSub(part, (*d2Idx)[jx])){
	    d2FirstGroundCnt[jx] = offset;
	    d2PossGround[jx] = posC1;
	  }
	}
	int curIdx = counts.size()-1;
	for(int jx = (curIdx-1); jx >= 0; jx--){
	  if (subset(positive[curIdx], positive[jx])){
	    curCount = curCount - vals[jx];
	  }
	}
	vals.append(curCount);
	//vals[ix] = curCount;
      }
      (*finalCnts)[index] = curCount;
      for(int ix =0 ; ix < d1Counts->size(); ix++){
        int dIndex = 0;
        Array<int>* pidx = (*d1Idx)[ix];
        for(int jx = 0; jx < pidx->size(); jx++){
          dIndex += (*present)[(*pidx)[jx]] * Clause::pow(2, jx);
        }
        (*((*d1Counts)[ix]))[dIndex]+= curCount;
        dIndex = 0;
        pidx = (*d2Idx)[ix];
        for(int jx = 0; jx < pidx->size(); jx++){
          dIndex += (*present)[(*pidx)[jx]] * Clause::pow(2, jx);
        }
        (*((*d2Counts)[ix]))[dIndex]+=curCount;
	if (dIndex == 0){
	  //cerr << "D Index = 0\n";
	}
      }
 
    }
    double allNeg = posGround;
    for(int ix = 1; ix < finalCnts->size(); ix++){
      allNeg = allNeg - (*finalCnts)[ix];
    }
    (*finalCnts)[0] = allNeg;
    
    for(int ix = 0; ix < d1Counts->size(); ix++){
      (*((*d1Counts)[ix]))[0] = 0;
      (*((*d2Counts)[ix]))[0] = 0;
    }
    
    for(int ix = 0; ix < d1Counts->size(); ix++){
      Array<double>* tmpArr = (*d1Counts)[ix];
      double tmpTotal = 0;
      for(int jx = 0; jx < tmpArr->size(); jx++){
        //cout << "D1 " << jx << " " << (*tmpArr)[jx] << " / ";
        (*tmpArr)[jx] = (*tmpArr)[jx] / d1FirstGroundCnt[ix];// + 1;                    
        //cout << d1FirstGroundCnt[ix] << " = " << (*tmpArr)[jx] << endl;
	tmpTotal += (*tmpArr)[jx];
      }
      //cerr << "D1: " << (*tmpArr)[0] << " to ";
      (*tmpArr)[0] = d1PossGround[ix] - tmpTotal;
      //cerr << (*tmpArr)[0] << " " << d1PossGround[ix] << "-" << tmpTotal << endl; 
      tmpTotal = 0;
      tmpArr = (*d2Counts)[ix];
      for(int jx = 0; jx < tmpArr->size(); jx++){
	/*
        cout << "D2 " << jx << " " << (*tmpArr)[jx] << " / "
             << d2FirstGroundCnt[ix];
	*/
        (*tmpArr)[jx] = (*tmpArr)[jx] / d2FirstGroundCnt[ix];// + 1;                    
        //cout << " = " << (*tmpArr)[jx] << endl;
	tmpTotal += (*tmpArr)[jx];
      }
      //cerr << "D2: " << (*tmpArr)[0] << " to ";
      (*tmpArr)[0] = d2PossGround[ix] - tmpTotal;
      //cerr << (*tmpArr)[0] << " " << d1PossGround[ix] << "-" << tmpTotal << endl; 
      //cout << endl;
    }

    /*
      vals.growToSize(counts.size(),0);
      for(int ix = 0; ix < counts.size(); ix++){
      
    */
    delete jset;
    decomp.deleteItemsAndClear();
    for(int ix = 0; ix < finalCnts->size(); ix++){
      (*finalCnts)[ix]++;
    }
    return finalCnts;
  }
  
  bool matchSub(const Array<int>* a1, Array<int>* a2){
    if (a1->size() != a2->size()){
      return false;
    }
    for(int ix = 0; ix < a1->size(); ix++){
      if ((*a1)[ix] != (*a2)[ix]){
	return false;
      }
    }
    return true;
  }

  bool subset(Array<bool>* a1, Array<bool>* a2){
    for(int ix = 0; ix < a1->size(); ix++){
      if ((*a1)[ix]){
	if ((*a2)[ix] == false){
	  return false;
	}
      }
    }
    return true;
  }

  void transfer(Array<Clause*> initClauses){
    double maxScore = 0;
    string fname = outMLNFileName_;
    ofstream out(fname.c_str());
    /*
    ArraysAccessor<int> check;
    Array<int>* tmp1 = new Array<int>;
    tmp1->append(1);
    tmp1->append(2);
    tmp1->append(4);
    tmp1->append(5);
    tmp1->append(6);
    check.appendArray(tmp1);

    tmp1 = new Array<int>;
    tmp1->append(1);
    tmp1->append(2);
    tmp1->append(4);
    tmp1->append(5);
    tmp1->append(6);
    check.appendArray(tmp1);

    tmp1 = new Array<int>;
    tmp1->append(1);
    tmp1->append(2);
    tmp1->append(4);
    tmp1->append(5);
    tmp1->append(6);
    check.appendArray(tmp1);

    tmp1 = new Array<int>;
    tmp1->append(1);
    tmp1->append(2);
    tmp1->append(4);
    tmp1->append(5);
    tmp1->append(6);
    check.appendArray(tmp1);
    int countIt = 0;
    //bool hasComb = false;
    while (check.hasNextCombination()){
      
      int constId;
      Array<int> fullComb;
      while(check.getDistinctNextCombination(fullComb)){
	//cout << constId << " ";
	//fullComb.append(constId);
	countIt++;
	cout << "(P" << fullComb[0] << ",P" << fullComb[1] << ") (";
	cout << "P" << fullComb[0] << ",P" << fullComb[2] << ") (";
	cout << "P" << fullComb[1] << ",P" << fullComb[3] << ")";
	cout << endl;
      }
      cout << "countIt " << countIt << endl;

    } 
    check.deleteArraysAndClear();
    exit(-1);
    */
    if (!out.good()) { cout << "ERROR: failed to open " <<fname<<endl;exit(-1);}
 


    for(int ix = 0; ix < initClauses.size(); ix++){
      //Flag Eval Time
      double begSec = timer_.time();
      out << "Evaluate: ";
      initClauses[ix]->printWithoutWt(out, (*domains_)[0]);
      out << endl;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      int numNonEqual = 0;
      ArraysAccessor<int> acc;
      Array<int> pos;
      for(int jx = 0; jx < preds->size(); jx++){
	if (!(*preds)[jx]->isEqualPred()){
	  numNonEqual++;
	  pos.append(jx);
	  Array<int>* vals = new Array<int>;
	  vals->append(0);
	  vals->append(1);
	  acc.appendArray(vals);
	}
      }
      Array<Array<double>*> dProbs1;
      Array<Array<int>*> idx1;
      Array<Array<double>*> dProbs2;
      Array<Array<int>*> idx2;
      Array<ClauseHashArray*> clause1;
      Array<ClauseHashArray*> clause2;
      if (numNonEqual == 2){
	Clause* d1 = new Clause();
	Clause* d2 = new Clause();
	Predicate* p1 = new Predicate(*(*preds)[pos[0]]);
	Predicate* p2 = new Predicate(*(*preds)[pos[1]]);
	
	d1->appendPredicate(p1);
	d2->appendPredicate(p2);
	d1->canonicalize();
	d2->canonicalize();
	Array<double>* tmpDouble = new Array<double>;
	Array<int>* tmpInt = new Array<int>;
	ClauseHashArray* tmpClause = new ClauseHashArray;
	buildClique(d1, tmpDouble, tmpInt, tmpClause);
	dProbs1.append(tmpDouble);
	idx1.append(tmpInt);
	clause1.append(tmpClause);
	tmpDouble = new Array<double>;
	tmpInt = new Array<int>;
	tmpClause = new ClauseHashArray;
	buildClique(d2, tmpDouble, tmpInt, tmpClause);
	dProbs2.append(tmpDouble);
	idx2.append(tmpInt);
	clause2.append(tmpClause);
	delete d1;
	delete d2;
      }
      else if (numNonEqual == 3){
	for(int jx = 0; jx < 3; jx++){
	  Clause* d1 = new Clause();
	  Clause* d2 = new Clause();
	  Predicate* p1 = new Predicate(*(*preds)[pos[0]]);
	  Predicate* p2 = new Predicate(*(*preds)[pos[1]]);
	  Predicate* p3 = new Predicate(*(*preds)[pos[2]]);
	  if (jx == 0){
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p2);
	    d2->appendPredicate(p3);
	  }
	  else if (jx == 1){
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p3);
	    d2->appendPredicate(p2);
	  }
	  else{
	    d1->appendPredicate(p2);
	    d1->appendPredicate(p3);
	    d2->appendPredicate(p1);
	  }
	  d1->canonicalize();
	  d2->canonicalize();
	  Array<double>* tmpDouble = new Array<double>;
	  Array<int>* tmpInt = new Array<int>;
	  ClauseHashArray* tmpClause = new ClauseHashArray;
	  buildClique(d1, tmpDouble, tmpInt, tmpClause);
	  /*
	    for(int ix = 0; ix < tmpDouble->size(); ix++){
	    cout << (*tmpDouble)[ix] << " ";
	    }
	    cout << endl;
	  */
	  dProbs1.append(tmpDouble);
	  idx1.append(tmpInt);
	  clause1.append(tmpClause);
	  tmpDouble = new Array<double>;
	  tmpInt = new Array<int>;
	  tmpClause = new ClauseHashArray;
	  buildClique(d2, tmpDouble, tmpInt, tmpClause);
	  dProbs2.append(tmpDouble);
	  idx2.append(tmpInt);
	  clause2.append(tmpClause);
	  delete d1;
	  delete d2;
	}
      }
      else{
	for(int jx = 0; jx < 7; jx++){
	  Clause* d1 = new Clause();
	  Clause* d2 = new Clause();
	  Predicate* p1 = new Predicate(*(*preds)[pos[0]]);
	  Predicate* p2 = new Predicate(*(*preds)[pos[1]]);
	  Predicate* p3 = new Predicate(*(*preds)[pos[2]]);
	  Predicate* p4 = new Predicate(*(*preds)[pos[3]]);
	  if (jx == 0){
	      d1->appendPredicate(p1);
	      d1->appendPredicate(p2);
	      d1->appendPredicate(p3);
	      d2->appendPredicate(p4);
	  }
	  else if (jx == 1){
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p2);
	    d1->appendPredicate(p4);
	    d2->appendPredicate(p3);
	    }
	  else if (jx == 2){
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p3);
	    d1->appendPredicate(p4);
	    d2->appendPredicate(p2);
	  }
	  else if (jx == 3){
	    d1->appendPredicate(p2);
	    d1->appendPredicate(p3);
	    d1->appendPredicate(p4);
	    d2->appendPredicate(p1);
	  }
	  else if (jx == 4){
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p2);
	    d2->appendPredicate(p3);
	    d2->appendPredicate(p4);
	  }
	  else if (jx == 5){
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p3);
	    d2->appendPredicate(p2);
	    d2->appendPredicate(p4);
	  }
	  else{
	    d1->appendPredicate(p1);
	    d1->appendPredicate(p4);
	    d2->appendPredicate(p2);
	    d2->appendPredicate(p3);
	  }
	  

	  d1->canonicalize();
	  d2->canonicalize();
	  Array<double>* tmpDouble = new Array<double>;
	  Array<int>* tmpInt = new Array<int>;
	  ClauseHashArray* tmpClause = new ClauseHashArray;
	  //cout << "D1 size " << d1->getNumPredicates() << endl;
	  buildCliqueFast(d1, tmpDouble, tmpInt, tmpClause);//, (*d1Counts)[jx]);
	  dProbs1.append(tmpDouble);
	  idx1.append(tmpInt);
	  clause1.append(tmpClause);
	  tmpDouble = new Array<double>;
	  tmpInt = new Array<int>;
	  tmpClause = new ClauseHashArray;
	  buildCliqueFast(d2, tmpDouble, tmpInt, tmpClause);//, (*d2Counts)[jx]);
	  dProbs2.append(tmpDouble);
	  idx2.append(tmpInt);
	  clause2.append(tmpClause);
	  delete d1;
	  delete d2;
	}
      }

      Array<double> trueCnts;
      Array<double> possCnts;
      Array<double> probs;
      Array<int> sense;
      double sum = 0;
      double total = 0;
      ClauseHashArray cliques;
      Array<Clause*> forDecomp;
      Array<ClauseHashArray*> decomp;
      Array<int> mult;
      while(acc.getNextCombination(sense)){
	Clause* c1 = new Clause(*initClauses[ix]);
	for(int jx = 0; jx < pos.size(); jx++){
	  Predicate* pred = c1->getPredicate(pos[jx]);
	  if (sense[jx] == 0){
	    pred->setSense(false);
	  }
	  else{
	    pred->setSense(true);
	  }
	}
	Clause* r1 = new Clause(*c1);
	c1->setDirty();
	c1->canonicalize();
	if (cliques.contains(c1)){
	  /*
	    cout << "Dup: ";
	    c1->printWithoutWt(cout, (*domains_)[0]);
	    cout << endl;
	  */
	  int dupIndex = cliques.find(c1);
	  mult[dupIndex]++;
	  delete c1;
	  continue;
	}
	cliques.append(c1);
	forDecomp.append(r1);
	mult.append(1);

	double ntg_c1 = 1;//c1->getNumTrueGroundings((*domains_)[0], (*domains_)[0]->getDB(), false);
	double ng_c1 = 0;// c1->getNumDistinctGroundings((*domains_)[0]);      
	for(int dx = 0; dx < domains_->size(); dx++){
	  ntg_c1 += c1->getNumTrueGroundingsAnd((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	  ng_c1 = c1->getNumDistinctGroundings((*domains_)[dx]);      
	}
	c1->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << ntg_c1 << endl;//" " << ng_c1 << endl;
	//cout << " Hash Code " << c1->hashCode() << endl;
	sum += (ng_c1 - ntg_c1);
	total = ng_c1;
	trueCnts.append(ntg_c1);
	possCnts.append(ng_c1);
	probs.append(ntg_c1);
      }

      //Do calculations
      double norm = 0;
      for(int jx = 0; jx < probs.size(); jx++){
	norm += probs[jx];
      }
      ClauseHashArray full1;
      ClauseHashArray full2;
      Array<double> newProbs1;
      Array<double> newProbs2;
      Array<int> mult1;
      Array<int> mult2;

      /*
	cout << clause1.size() << " " << clause2.size() << " "
	<< dProbs1.size() << " " << dProbs2.size() << endl;
      */
      for(int jx = 0; jx < clause1.size(); jx++){
	
	ClauseHashArray* c1Array = clause1[jx];
	ClauseHashArray* c2Array = clause2[jx];
	Array<double>* probs1 = dProbs1[jx];
	Array<double>* probs2 = dProbs2[jx];
	Array<int>* m1 = idx1[jx];
	Array<int>* m2 = idx2[jx];
	/*
	  cout << c1Array->size() << " " << probs1->size()
	  << " " << c2Array->size() << " " << probs2->size() << endl;
	*/
	for(int kx = 0; kx < c1Array->size(); kx++){
	  if (!full1.contains((*c1Array)[kx])){
	    full1.append((*c1Array)[kx]);
	    newProbs1.append((*probs1)[kx]);
	    if (kx >= m1->size()){
	      cout << "Size mismatch\n";
	    }
	    if ((*m1)[kx] == 0){
	      cout << "bad " << jx << " " << kx << " ";
	      (*c1Array)[kx]->printWithoutWt(cout, (*domains_)[0]);
	      cout << endl;
	    }
	    mult1.append((*m1)[kx]);
	  }
	  else{
	    //cout << "Here\n";
	    int index = full1.find((*c1Array)[kx]);
	    if (newProbs1[index] != (*probs1)[kx]){
	      cout << "Probability mismatch\n";
	    }
	    if (mult1[index] != (*m1)[kx]){
	      cout << "Multiplier mismatch\n";
	    }
	  }
	}
	for(int kx = 0; kx < c2Array->size(); kx++){
	  if (!full2.contains((*c2Array)[kx])){
	    full2.append((*c2Array)[kx]);
	    newProbs2.append((*probs2)[kx]);
	    mult2.append((*m2)[kx]);
	  }
	}
      }
      Array<double> klScores;
      for(int jx = 0; jx < dProbs1.size(); jx++){
	klScores.append(0.0);
      }
      Array<Array<double>*> probNorm;
      for(int jx = 0; jx < dProbs1.size(); jx++){
	Array<double>* tmpD = new Array<double>;
	probNorm.append(tmpD);
      }

      for(int jx = 0; jx < probs.size(); jx++){

	probs[jx] = probs[jx] / norm;
	//cliques[jx]->printWithoutWt(cout, (*domains_)[0]);
	//cout << " " << probs[jx] << " | ";
	//cout << dProbs1.size() << " " << dProbs2.size() << " -> ";
	
	for(int kx = 0; kx < dProbs1.size(); kx++){
	  Clause* d1 = new Clause(*forDecomp[jx]);
	  Clause* d2 = new Clause(*forDecomp[jx]);
	  if (dProbs1.size() == 1){
	    Predicate* rmed = d1->removePredicate(1);
	    delete rmed;
	    rmed = d2->removePredicate(0);
	    delete rmed;
	  }
	  else if (dProbs1.size() == 3){
	    if (kx == 0){
	      Predicate* rmed = d1->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    else if (kx == 1){
	      Predicate* rmed = d1->removePredicate(1);
	      delete rmed;
	      rmed =d2->removePredicate(2);
	      delete rmed;
	      rmed =d2->removePredicate(0);
	      delete rmed;
	    }
	    else{
	      Predicate* rmed = d1->removePredicate(0);
	      delete rmed;
	      rmed = d2->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(1);
	      delete rmed;
	    }
	  }
	  else{
	    if (kx == 0){
	      Predicate* rmed = d1->removePredicate(3);
	      delete rmed;
	      rmed = d2->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    else if (kx == 1){
	      Predicate* rmed = d1->removePredicate(2);
	      delete rmed;
	      rmed =d2->removePredicate(3);
	      delete rmed;
	      rmed =d2->removePredicate(1);
	      delete rmed;
	      rmed =d2->removePredicate(0);
	      delete rmed;
	    }
	    else if (kx == 2){
	      Predicate* rmed = d1->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(3);
	      delete rmed;
	      rmed = d2->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    else if (kx == 3){
	      Predicate* rmed = d1->removePredicate(0);
	      delete rmed;
	      rmed =d2->removePredicate(3);
	      delete rmed;
	      rmed =d2->removePredicate(2);
	      delete rmed;
	      rmed =d2->removePredicate(1);
	      delete rmed;
	    }
	    else if (kx == 4){
	      Predicate* rmed = d1->removePredicate(3);
	      delete rmed;
	      rmed = d1->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    else if (kx == 5){
	      Predicate* rmed = d1->removePredicate(3);
	      delete rmed;
	      rmed = d1->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    else{
	      Predicate* rmed = d1->removePredicate(2);
	      delete rmed;
	      rmed = d1->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(3);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    
	  }
	  d1->canonicalize();
	  d2->canonicalize();
	  int index1 = full1.find(d1);
	  int index2 = full2.find(d2);
	  //Array<int>* c1Mult = idx1[kx];
	  //Array<int>* c2Mult = idx2[kx];
	  //cout << idx1.size() << " " << c1Mult->size() << " " << idx2.size() << " " << c2Mult->size() << endl;
	  if (mult1[index1] == 0){
	    cout << "This is bad!" << endl;
	  }
	  //cout << "mults: " << kx << " mult1 " << (1.0/mult1[index1]) << " mult2 " << (1.0/mult2[index2]) << " index1 " << index1 << " index2 " << index2 << " " << mult1.size() << " " << mult2.size() << endl;
	  cout << "kx: " << kx << " ";
	  d1->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";
	  d2->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";

	  double predProb = newProbs1[index1] * newProbs2[index2] * (1.0/mult1[index1]) * (1.0/mult2[index2]);// * (1/(*c1Mult)[index1]) * (1/(*c2Mult)[index2]);//1 - ((1-newProbs1[index1]) * (1-newProbs2[index2]));
	  Array<double>* dp = probNorm[kx];
	  dp->append(predProb);
	  cout << " " << newProbs1[index1] << " " << newProbs2[index2] << " " << predProb << endl;
	  //cout << "kx: " << kx << " " << predProb << " ";
	  //double kl1 = thisProb * log(thisProb/prob1) + (1-thisProb) * log((1-thisProb)/(1-prob1));
	  //double kl1 = probs[jx] * log(probs[jx]/predProb) + (1-probs[jx]) * log((1-probs[jx])/(1-predProb));
	  //cout << newProbs1[index1] << " " << newProbs2[index2] << " (" << kl1 <<  "): "; //newProbs1[index1] << " " << newProbs2[index2] << ": ";
	  delete d1;
	  delete d2;
	  //klScores[kx] += kl1;
	} 
	cout << endl;
      }
      for(int jx = 0; jx < probNorm.size(); jx++){
	Array<double>* td = probNorm[jx];
	double z = 0;
	//cout << td->size() << " " << mult.size() << endl;
	
	for(int kx = 0; kx < td->size(); kx++){
	  z += (*td)[kx] * mult[kx];
	}
	cout << "check z = " << z;
	out << "check z = " << z << endl;
	if (z > 1.0 || z < 1.0){
	  cout << " z badness ";
	}
	cout << endl;
	/*
	  for(int kx = 0; kx < td->size(); kx++){
	  (*td)[kx] = (*td)[kx]  / z;
	  }
	*/
      }
      for(int jx = 0; jx < probs.size(); jx++){
	out << "feature ";
      	forDecomp[jx]->printWithoutWt(out, (*domains_)[0]);
	out << " " << probs[jx] << " | ";
	
	//cout << "feature ";
      	forDecomp[jx]->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << probs[jx] << " | ";
	
	for(int kx = 0; kx < probNorm.size(); kx++){
	  Array<double>* predProb = probNorm[kx];
	  double predFinal = mult[jx] * (*predProb)[jx];
	  //flag
	  double kl1 = probs[jx] * log(probs[jx]/predFinal);// + (1-probs[jx]) * log((1-probs[jx])/(1-predFinal));
	  //double kl1 = probs[jx] * log(probs[jx]/(*predProb)[jx]) + (1-probs[jx]) * log((1-probs[jx])/(1-(*predProb)[jx]));
	  //cout << newProbs1[index1] << " " << newProbs2[index2] << " (" << kl1 <<  "): "; //newProbs1[index1] << " " << newProbs2[index2] << ": ";
	  /*
	  out << (*predProb)[jx] << " (" << kl1 << "): ";
	  cout << (*predProb)[jx] << " (" << kl1 << "): ";
	  */

	  out << predFinal << " (" << kl1 << "): ";
	  cout << predFinal << " (" << kl1 << "): ";
	  klScores[kx] += kl1;
	}
	cout << endl;
	out << endl;
      }
      double minScore = klScores[0];
      cout << "KL Scores: ";
      out << "KL Scores: ";
      for(int jx = 0; jx < klScores.size(); jx++){
	out << klScores[jx] << " ";
	cout << klScores[jx] << " ";
	if (klScores[jx] < minScore){
	  minScore = klScores[jx];
	}
      }
      if (maxScore < minScore){
	maxScore = minScore;
      }
      out << endl;
      cout << endl;
      out << "Min " << minScore << endl;
      for(int jx = 0; jx < probNorm.size(); jx++){
	Array<double>* tmp = probNorm[jx];
	delete tmp;
      } 
      for(int jx = 0; jx < clause1.size(); jx++){
	
	ClauseHashArray* ca1 = clause1[jx];
	ClauseHashArray* ca2 = clause2[jx];
	ca1->deleteItemsAndClear();
	ca2->deleteItemsAndClear();
	Array<double>* pr1 = dProbs1[jx];
	Array<double>* pr2 = dProbs2[jx];
	Array<int>* id1 = idx1[jx];
	Array<int>* id2 = idx2[jx];
	delete id1;
	delete id2;
	delete pr1;
	delete pr2;
	delete ca1;
	delete ca2;
      }
      /*
      for(int dx = 0; dx < trueDCnts.size(); dx++){
	Array<double>* tmp =  trueDCnts[dx];
	delete tmp;
      }
      for(int dx = 0; dx < possDCnts.size(); dx++){
	Array<double>* tmp =  possDCnts[dx];
	delete tmp;
      }
      */
      forDecomp.deleteItemsAndClear();
      cliques.deleteItemsAndClear();
      acc.deleteArraysAndClear();
      out << endl;
      double endSec = timer_.time();
      cout << "Clique Score Time: " << (endSec - begSec) << endl << endl;
    }//end of ix
    out << "Max: " << maxScore << endl;
    out.close();
  }


  void greedy(Array<Clause*> initialMLNClauses,
	      Array<ExistFormula*> existFormulas, int search){

    cout << "faction " << fraction_ << " " << sampleGndPreds_ << " " << endl;
    //pll_->setJDebug(true);
    pll_ = new PseudoLogLikelihood(areNonEvidPreds_, domains_, wtPredsEqually_, 
                                   sampleGndPreds_, fraction_, 
                                   minGndPredSamples_, maxGndPredSamples_);
    pll_->setIndexTranslator(indexTrans_);

    int numClausesFormulas = getNumClausesFormulas();
    lbfgsb_ = new LBFGSB(-1, -1, pll_, numClausesFormulas);

    useTightParams();

      //compute initial counts for clauses in all MLNs
    cout << "computing counts for initial MLN clauses..." << endl;
    double begSec = timer_.time();
    pllComputeCountsForInitialMLN();
    cout << "computing counts for initial MLN clauses took "; 
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;


      //learn initial wt/score of MLN, and set MLN clauses to learned weights
    cout << "learning the initial weights and score of MLN..." << endl << endl;
    begSec = timer_.time();
    double score;
    Array<double> wts;
    if (!learnAndSetMLNWeights(score)) return; 
    printMLNClausesWithWeightsAndScore(score, -1);
    cout << "learning the initial weights and score of MLN took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;
    iter_ = -1;
    //printMLNToFile(NULL, iter_);

      //start searching for best candidates
    double bsec;
    //Array<Clause*> initialClauses;
    Array<Clause*> bestCandidates;
    Array<Clause*> candidates;
    candidates.clear();
    buildInitCandidates(candidates, &initialMLNClauses);
    /*
    for(int ix = 0; ix < candidates.size(); ix++){
      candidates[ix]->printWithoutWtWithStrVar(cout, (*domains_)[0]);
      cout << endl;
      double c_tot = 0;
      double ng_tot = 0;
      for(int dx = 0; dx < domains_->size(); dx++){
	
	double count = candidates[ix]->getNumTrueGroundings((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	double ng = candidates[ix]->getNumGroundings((*domains_)[dx]);    
	c_tot += count;
	ng_tot += ng;
	cout << "count = " << count << " / " << ng << endl;
      }
      cout << "totals = " << c_tot << " / " << ng_tot << endl;
      cout  << endl << endl;
    }
    exit(-1);
    */
    while (true)
    {
      begSec = timer_.time();
      iter_++;

      //if (iter_ == 2) break; // for testing purposes only
      cout << "Iteration " << iter_ << endl << endl;

      minGain_ = 0;

      Array<ExistFormula*> highGainWtExistFormulas;
      if (startFromEmptyMLN_ && !existFormulas.empty()) 
      {
        useTightParams();
        cout << "evaluating the gains of existential formulas..." << endl<<endl;
        bsec = timer_.time(); 
          //score not updated
        minGain_ = evaluateExistFormulas(existFormulas, highGainWtExistFormulas,
                                         score);
        cout << "evaluating the gains of existential formulas took ";
        timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
        cout << "setting minGain to min gain among existential formulas. "
             << "minGain = " << minGain_ << endl;
      }

      //useLooseParams();
      useTightParamsCache(search);
 
      bestCandidates.clear();
      // evalClauses(Array<Clause*>& candidates, const double& prevScore, Array<Clause*>& bestClauses){
      evalClauses(candidates, score, bestCandidates);
      //cout << "Best Candidates Size " << bestCandidates.size() << endl;
      
      bool noCand = (startFromEmptyMLN_ && !existFormulas.empty()) ?
          (bestCandidates.empty() && highGainWtExistFormulas.empty()) 
        : bestCandidates.empty();
      if (noCand){ 
        cout << "Beam is empty. Ending search for MLN clauses." << endl; 
        printIterAndTimeElapsed(begSec);
        break;
      }

        // effect the single best candidate on the MLN
      bsec = timer_.time();
      bool ok;

      //Flag - need original weights
      //int numClausesFormulas = getNumClausesFormulas();
      //Array<double> wts;
      //wts.growToSize(numClausesFormulas + 2); // first slot is unused
      //Array<double> origWts(numClausesFormulas); // current clause/formula wts
      Array<double> origWts(getNumClausesFormulas()); 
      if (indexTrans_){
	indexTrans_->getClauseFormulaWts(origWts);
      }
      else{
	mln0_->getClauseWts(origWts);
      }
      //double timeBound = 86400 * 1 * domains_->size();
      double timeBound = 3600 * hrs_ * domains_->size();
      if ((timer_.time() - startSec_ >= (timeBound)) &&
          timeBased_){
        cout << "Out of time (greedy)" << timeBound << endl << endl;
        break;
      }

      if (startFromEmptyMLN_ && !existFormulas.empty())
        ok = effectBestCandidateOnMLNs(bestCandidates, existFormulas,
                                       highGainWtExistFormulas, score); 
      else{
	//cout << "JD" << endl;
        ok = effectBestCandidateOnMLNs(bestCandidates, score, &origWts);
      }
      //score was updated when best candidate was effected on MLN
      cout << "effecting best candidates took ";
      timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
      if (minWt_ > 0.01){
	minWt_ = minWt_ / 10;
	if (minWt_ < 0.01){
	  minWt_ = 0.01;
	}
      }
      if (!ok) 
      { 
        cout << "failed to effect any of the best candidates on MLN" << endl
             << "stopping search for MLN clauses..." << endl;
        break;
      }
      
      printIterAndTimeElapsed(begSec);
 
    } // while (true)
    candidates.deleteItemsAndClear();
    //initialClauses.deleteItemsAndClear();
    cout << "done searching for MLN clauses" << endl << endl;
    int numIterTaken = iter_+1;
    iter_= -1;
    
    //Prune each non-unit clause in MLN if it does not decrease score    
    cout << "pruning clauses from MLN..." << endl << endl;
    begSec = timer_.time();
    pruneMLN(score);
    cout << "pruning clauses from MLN took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;

    printMLNClausesWithWeightsAndScore(score, -1);
    printMLNToFile(NULL, -2);
    cout << "num of iterations taken = " << numIterTaken << endl;

    cout << "time taken for structure learning = ";
    timer_.printTime(cout, timer_.time()-startSec_); cout << endl << endl;

    initialMLNClauses.deleteItemsAndClear();
    deleteExistFormulas(existFormulas);
  }//run()
 

  void evalClauses(Array<Clause*>& candidates, const double& prevScore, Array<Clause*>& bestClauses) {
    
    //flag

    //int iterBestGainUnchanged = 0; 
    int bsiter_ = -1;
    Array<double> priorMeans, priorStdDevs;
    //ClauseOpHashArray* beam = new ClauseOpHashArray;
    int numClausesFormulas = getNumClausesFormulas();
    Array<double> wts;
    wts.growToSize(numClausesFormulas + 2); // first slot is unused
    Array<double> origWts(numClausesFormulas); // current clause/formula wts
    if (indexTrans_) indexTrans_->getClauseFormulaWts(origWts);
    else             mln0_->getClauseWts(origWts);

      //set the prior means and std deviations
    setPriorMeansStdDevs(priorMeans, priorStdDevs, 1, NULL);
    if (indexTrans_) indexTrans_->appendClauseIdxToClauseFormulaIdxs(1, 1);

    bool error;
    double begIterSec, bsec;
    bsec = timer_.time();
    //while (!beam->empty() && iterBestGainUnchanged < bestGainUnchangedLimit_)
    //{
    begIterSec = timer_.time();
    bsiter_++; 
    
    //if (bsiter_ == 1) break; // for testing purposes only
    cout << endl << "BEAM SEARCH ITERATION " << bsiter_ << endl << endl;
    //cout << "num of candidates created = " << candidates.size() << "; took ";
    //timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
    cout << "evaluating gain of candidates..." << endl << endl;
    bsec = timer_.time();
    
    for (int ix = 0; ix < candidates.size(); ix++){
      /*
      countAndMaxScoreEffectCandidate(candidates[ix], &wts, &origWts,prevScore,
				      false, priorMeans, priorStdDevs, error,
				      NULL, NULL);
      */
      countAndMaxScoreEffectCandidate(candidates[ix], &wts, &origWts,prevScore,
				      true, priorMeans, priorStdDevs, error,
				      NULL, NULL);

    }
    cout << "evaluating gain of candidates took ";
    timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
    
    
    cout << "finding best candidates..." << endl;
    bsec = timer_.time();
    //ClauseOpHashArray* newBeam = new ClauseOpHashArray(beamSize_);
    int bestPos = -1;
    double bestGain = 0;
    for(int ix =0 ; ix < candidates.size(); ix++){
      double candGain = candidates[ix]->getAuxClauseData()->gain;
      double candAbsWt = fabs(candidates[ix]->getWt());
      if (candGain > minGain_ 
	  && candAbsWt >= minWt_ 
	  && candGain > bestGain){
	bestGain = candGain;
	bestPos = ix;
      }
    }
    if (bestPos > -1){
      cout << "Best Pos " << bestPos << " Best Gain " << bestGain << endl;
      bestClauses.append(candidates[bestPos]);
      //fix this	 
      //delete candidates[bestPos];
      candidates[bestPos] = NULL;
      //candidates.removeItem(bestPos);
    }
    candidates.removeAllNull();      
    cout << "finding best candidates took ";
    timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
    
    //beam->deleteItemsAndClear();
    //delete beam;
    //beam = newBeam;
    cout << "found new best clause in beam search iter " << bsiter_ << endl;    
      
    //print the best clauses
    cout << "best clauses found in beam search iter " << bsiter_ << endl;
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    for (int i = 0; i < bestClauses.size(); i++){
      cout << i << "\t";
      bestClauses[i]->printWithoutWtWithStrVar(cout,(*domains_)[0]);
      cout << endl
	   << "\tgain = " << bestClauses[i]->getAuxClauseData()->gain
	   << ",  op = " 
	   << Clause::getOpAsString(bestClauses[i]->getAuxClauseData()->op);
      if (bestClauses[i]->getAuxClauseData()->op != OP_REMOVE)
	cout << ",  wt = " << bestClauses[i]->getWt();
      cout << endl;
    }
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl << endl;
    
    
    cout << "BEAM SEARCH ITERATION " << bsiter_ << " took ";
    timer_.printTime(cout, timer_.time()-begIterSec); cout << endl << endl;
    cout << "Time elapsed = ";
    timer_.printTime(cout, timer_.time()-startSec_); cout << endl << endl;
    //}

    //beam->deleteItemsAndClear();
    //delete beam;
    bsiter_ = -1;
   

    if (indexTrans_) indexTrans_->removeClauseIdxToClauseFormulaIdxs(1, 1);
  }//beamSearch()

/// newGreedyTemplate - begin //////////////////////////////////////////////////

  void newGreedyTemplate(Array<Clause*> initialMLNClauses, Array<ExistFormula*> existFormulas, int search) {

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
    timer_.printTime(cout, timer_.time() - begSec); cout << endl;

    //learn initial wt/score of MLN, and set MLN clauses to learned weights
    cout << "learning the initial weights and score of MLN..." << endl;
    begSec = timer_.time();
    double score;
    Array<double> wts;
    if (!learnAndSetMLNWeights(score)) return; 
    printMLNClausesWithWeightsAndScore(score, -1);
    cout << "learning the initial weights and score of MLN took ";
    timer_.printTime(cout, timer_.time() - begSec); cout << endl;
    iter_ = -1;

    //start searching for best candidates
    double bsec;
    Array<Clause*> bestCandidates;

	cout << "initial clauses: " << initialMLNClauses.size() << endl;

    for (int cx = 0; cx < initialMLNClauses.size(); cx++) {
      Array<Clause*> candidates;
      candidates.clear();
      Array<Clause*> templates = plusType(initialMLNClauses[cx]);
      buildInitCandidates(candidates, &templates);
      cout << "Processing: ";
      initialMLNClauses[cx]->printWithoutWtWithStrVar(cout, (*domains_)[0]); 
      cout << endl;
      iter_ = -1;
      while (true)
	  {
	    begSec = timer_.time();
	    iter_++;
	  
        cout << "Iteration " << iter_ << endl << endl;
	  
	    minGain_ = 0;
	  
	    Array<ExistFormula*> highGainWtExistFormulas;
	    if (startFromEmptyMLN_ && !existFormulas.empty()) 
	    {
	      useTightParams();
	      cout << "evaluating the gains of existential formulas..." << endl<<endl;
	      bsec = timer_.time(); 
	      //score not updated
	      minGain_ = evaluateExistFormulas(existFormulas, highGainWtExistFormulas, score);
	      cout << "evaluating the gains of existential formulas took ";
	      timer_.printTime(cout, timer_.time() - bsec); cout << endl << endl;
	      cout << "setting minGain to min gain among existential formulas. " << "minGain = " << minGain_ << endl;
	    }
	  
	  //useLooseParams();
	  useTightParamsCache(search);
	  
	  bestCandidates.clear();
	  newEvalClauses(candidates, score, bestCandidates);
	  
	  cout << "best candidates: " << bestCandidates.size() << endl;

	  bool noCand = (startFromEmptyMLN_ && !existFormulas.empty()) ? (bestCandidates.empty() && highGainWtExistFormulas.empty()) : bestCandidates.empty();
	  if (noCand) { 
	    cout << "Beam is empty. Ending search for MLN clauses." << endl; 
	    printIterAndTimeElapsed(begSec);
	    break;
	  }
	  
	  // effect the single best candidate on the MLN
	  bsec = timer_.time();
	  bool ok;
	  
	  //Flag - need original weights
	  //int numClausesFormulas = getNumClausesFormulas();
	  //Array<double> wts;
	  //wts.growToSize(numClausesFormulas + 2); // first slot is unused
	  //Array<double> origWts(numClausesFormulas); // current clause/formula wts
	  Array<double> origWts(getNumClausesFormulas()); 
	  if (indexTrans_){
	    indexTrans_->getClauseFormulaWts(origWts);
	  }
	  else{
	    mln0_->getClauseWts(origWts);
	  }
	  //double timeBound = 86400 * 1 * domains_->size();
	  double timeBound = 3600 * hrs_ * domains_->size();
	  if ((timer_.time() - startSec_ >= (timeBound)) &&
	      timeBased_){
	    cout << "Out of time (greedy)" << timeBound << endl << endl;
	    break;
	  }
	  
	  // if (startFromEmptyMLN_ && !existFormulas.empty())
	  //  ok = newEffectBestCandidateOnMLNs(bestCandidates, existFormulas, highGainWtExistFormulas, score); 
	  // else{
	  //  ok = newEffectBestCandidateOnMLNs(bestCandidates, score, &origWts);
	  // }

	  if (startFromEmptyMLN_ && !existFormulas.empty())
	  	ok = effectBestCandidateOnMLNs(bestCandidates, existFormulas, highGainWtExistFormulas, score); 
	  else{
	  	ok = effectBestCandidateOnMLNs(bestCandidates, score, &origWts);
	  }

	  //score was updated when best candidate was effected on MLN
	  cout << "effecting best candidates took ";
	  timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
	  if (minWt_ > 0.01){
	    minWt_ = minWt_ / 10;
	    if (minWt_ < 0.01){
	      minWt_ = 0.01;
	    }
	  }
	  if (!ok) 
	    { 
	      cout << "failed to effect any of the best candidates on MLN" << endl
		   << "stopping search for MLN clauses..." << endl;
	      break;
	    }
	  
	  printIterAndTimeElapsed(begSec);
	  
	} // while (true)
    
      candidates.deleteItemsAndClear();
    }
    //initialClauses.deleteItemsAndClear();
    cout << "done searching for MLN clauses" << endl << endl;
    int numIterTaken = iter_+1;
    iter_= -1;
    
    //Prune each non-unit clause in MLN if it does not decrease score    
    cout << "pruning clauses from MLN..." << endl << endl;
    begSec = timer_.time();
    pruneMLN(score);
    cout << "pruning clauses from MLN took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;

    printMLNClausesWithWeightsAndScore(score, -1);
    printMLNToFile(NULL, -2);
    cout << "num of iterations taken = " << numIterTaken << endl;

    cout << "time taken for structure learning = ";
    timer_.printTime(cout, timer_.time()-startSec_); cout << endl << endl;

    initialMLNClauses.deleteItemsAndClear();
    deleteExistFormulas(existFormulas);
  }//run()

/// newGreedy - begin //////////////////////////////////////////////////////////

  void newGreedy(Array<Clause*> initialMLNClauses, Array<ExistFormula*> existFormulas, int search) {

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
    timer_.printTime(cout, timer_.time() - begSec); cout << endl;

    //learn initial wt/score of MLN, and set MLN clauses to learned weights
    cout << "learning the initial weights and score of MLN..." << endl;
    begSec = timer_.time();
    double score;
    Array<double> wts;
    if (!learnAndSetMLNWeights(score)) return; 
    printMLNClausesWithWeightsAndScore(score, -1);
    cout << "learning the initial weights and score of MLN took ";
    timer_.printTime(cout, timer_.time() - begSec); cout << endl;
    iter_ = -1;

    //start searching for best candidates
    double bsec;
    Array<Clause*> bestCandidates;

	cout << "initial clauses: " << initialMLNClauses.size() << endl;

    for (int cx = 0; cx < initialMLNClauses.size(); cx++) {
      Array<Clause*> candidates;
      candidates.clear();
      Array<Clause*> templates = plusType(initialMLNClauses[cx]);
      buildInitCandidates(candidates, &templates);
      cout << "Processing: ";
      initialMLNClauses[cx]->printWithoutWtWithStrVar(cout, (*domains_)[0]); 
      cout << endl;
      iter_ = -1;
      while (true)
	  {
	    begSec = timer_.time();
	    iter_++;
	  
        cout << "Iteration " << iter_ << endl << endl;
	  
	    minGain_ = 0;
	  
	    Array<ExistFormula*> highGainWtExistFormulas;
	    if (startFromEmptyMLN_ && !existFormulas.empty()) 
	    {
	      useTightParams();
	      cout << "evaluating the gains of existential formulas..." << endl<<endl;
	      bsec = timer_.time(); 
	      //score not updated
	      minGain_ = evaluateExistFormulas(existFormulas, highGainWtExistFormulas, score);
	      cout << "evaluating the gains of existential formulas took ";
	      timer_.printTime(cout, timer_.time() - bsec); cout << endl << endl;
	      cout << "setting minGain to min gain among existential formulas. " << "minGain = " << minGain_ << endl;
	    }
	  
	  //useLooseParams();
	  useTightParamsCache(search);
	  
	  bestCandidates.clear();
	  newEvalClauses(candidates, score, bestCandidates);
	  
	  cout << "best candidates: " << bestCandidates.size() << endl;

	  bool noCand = (startFromEmptyMLN_ && !existFormulas.empty()) ? (bestCandidates.empty() && highGainWtExistFormulas.empty()) : bestCandidates.empty();
	  if (noCand) { 
	    cout << "Beam is empty. Ending search for MLN clauses." << endl; 
	    printIterAndTimeElapsed(begSec);
	    break;
	  }
	  
	  // effect the single best candidate on the MLN
	  bsec = timer_.time();
	  bool ok;
	  
	  //Flag - need original weights
	  //int numClausesFormulas = getNumClausesFormulas();
	  //Array<double> wts;
	  //wts.growToSize(numClausesFormulas + 2); // first slot is unused
	  //Array<double> origWts(numClausesFormulas); // current clause/formula wts
	  Array<double> origWts(getNumClausesFormulas()); 
	  if (indexTrans_){
	    indexTrans_->getClauseFormulaWts(origWts);
	  }
	  else{
	    mln0_->getClauseWts(origWts);
	  }
	  //double timeBound = 86400 * 1 * domains_->size();
	  double timeBound = 3600 * hrs_ * domains_->size();
	  if ((timer_.time() - startSec_ >= (timeBound)) &&
	      timeBased_){
	    cout << "Out of time (greedy)" << timeBound << endl << endl;
	    break;
	  }
	  
	  // if (startFromEmptyMLN_ && !existFormulas.empty())
	  //  ok = newEffectBestCandidateOnMLNs(bestCandidates, existFormulas, highGainWtExistFormulas, score); 
	  // else{
	  //  ok = newEffectBestCandidateOnMLNs(bestCandidates, score, &origWts);
	  // }

	  if (startFromEmptyMLN_ && !existFormulas.empty())
	  	ok = effectBestCandidateOnMLNs(bestCandidates, existFormulas, highGainWtExistFormulas, score); 
	  else{
	  	ok = effectBestCandidateOnMLNs(bestCandidates, score, &origWts);
	  }

	  //score was updated when best candidate was effected on MLN
	  cout << "effecting best candidates took ";
	  timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
	  if (minWt_ > 0.01){
	    minWt_ = minWt_ / 10;
	    if (minWt_ < 0.01){
	      minWt_ = 0.01;
	    }
	  }
	  if (!ok) 
	    { 
	      cout << "failed to effect any of the best candidates on MLN" << endl
		   << "stopping search for MLN clauses..." << endl;
	      break;
	    }
	  
	  printIterAndTimeElapsed(begSec);
	  
	} // while (true)
    
      candidates.deleteItemsAndClear();
    }
    //initialClauses.deleteItemsAndClear();
    cout << "done searching for MLN clauses" << endl << endl;
    int numIterTaken = iter_+1;
    iter_= -1;
    
    //Prune each non-unit clause in MLN if it does not decrease score    
    cout << "pruning clauses from MLN..." << endl << endl;
    begSec = timer_.time();
    pruneMLN(score);
    cout << "pruning clauses from MLN took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;

    printMLNClausesWithWeightsAndScore(score, -1);
    printMLNToFile(NULL, -2);
    cout << "num of iterations taken = " << numIterTaken << endl;

    cout << "time taken for structure learning = ";
    timer_.printTime(cout, timer_.time()-startSec_); cout << endl << endl;

    initialMLNClauses.deleteItemsAndClear();
    deleteExistFormulas(existFormulas);
  }//run()

  void newEvalClauses(Array<Clause*>& candidates, const double& prevScore, Array<Clause*>& bestClauses) {
    
    cout << "newEvalClauses: " << candidates.size() << endl;

    int bsiter_ = -1;
    Array<double> priorMeans, priorStdDevs;
    int numClausesFormulas = getNumClausesFormulas();
    Array<double> wts;
    wts.growToSize(numClausesFormulas + 2); // first slot is unused
    Array<double> origWts(numClausesFormulas); // current clause/formula wts
    if (indexTrans_) indexTrans_->getClauseFormulaWts(origWts);
    else             mln0_->getClauseWts(origWts);

      //set the prior means and std deviations
    setPriorMeansStdDevs(priorMeans, priorStdDevs, 1, NULL);
    if (indexTrans_) indexTrans_->appendClauseIdxToClauseFormulaIdxs(1, 1);

    bool error;
    double begIterSec, bsec;
    bsec = timer_.time();
    begIterSec = timer_.time();
    bsiter_++; 

    cout << "evaluating gain of candidates..." << endl << endl;
    bsec = timer_.time();
    
    for (int ix = 0; ix < candidates.size(); ix++){
      countAndMaxScoreEffectCandidate(candidates[ix], &wts, &origWts,prevScore,
				      true, priorMeans, priorStdDevs, error,
				      NULL, NULL);
    }
    cout << "evaluating gain of candidates took ";
    timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
        
    cout << "finding best candidates..." << endl;
    bsec = timer_.time();
    int bestPos = -1;
    double bestGain = 0;
    for (int ix =0 ; ix < candidates.size(); ix++) {
      double candGain = candidates[ix]->getAuxClauseData()->gain;
      double candAbsWt = fabs(candidates[ix]->getWt());
      if (candGain > minGain_ && candAbsWt >= minWt_  && candGain > bestGain){
        bestClauses.append(candidates[ix]);
	    candidates[ix] = NULL;
      }
    }

    candidates.removeAllNull();      
    cout << "finding best candidates took ";
    timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
    
    //beam->deleteItemsAndClear();
    //delete beam;
    //beam = newBeam;
    cout << "found new best clause in beam search iter " << bsiter_ << endl;    
      
    //print the best clauses
    cout << "best clauses found in beam search iter " << bsiter_ << endl;
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    for (int i = 0; i < bestClauses.size(); i++){
      cout << i << "\t";
      bestClauses[i]->printWithoutWtWithStrVar(cout,(*domains_)[0]);
      cout << endl
	   << "\tgain = " << bestClauses[i]->getAuxClauseData()->gain
	   << ",  op = " 
	   << Clause::getOpAsString(bestClauses[i]->getAuxClauseData()->op);
      if (bestClauses[i]->getAuxClauseData()->op != OP_REMOVE)
	cout << ",  wt = " << bestClauses[i]->getWt();
      cout << endl;
    }
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl << endl;
        
    timer_.printTime(cout, timer_.time()-begIterSec); cout << endl << endl;
    cout << "Time elapsed = ";
    timer_.printTime(cout, timer_.time()-startSec_); cout << endl << endl;
    //}

    bsiter_ = -1;   

    if (indexTrans_) indexTrans_->removeClauseIdxToClauseFormulaIdxs(1, 1);
  } // newEvalClauses()

  //Flag
  bool newEffectBestCandidateOnMLNs(Array<Clause*>& bestCandidates, double& score, const Array<double>* const & origWts)
  {
    cout << "effecting best candidates on MLN..." << endl << endl;
    bool ok = true;
    int i;
    for (i = 0; i < bestCandidates.size(); i++)
    {
	  bool test = effectBestCandidateOnMLNs(bestCandidates[i], score, origWts);
	  ok = ok && test;
    }
    // for (int j = i+1; j < bestCandidates.size();j++) delete bestCandidates[j];
    return ok;
  }

/// newGreedy - end ////////////////////////////////////////////////////////////


/// Jan Van Haaren - begin /////////////////////////////////////////////////////

  void greedyEvaluation(Array<Clause*> initialMLNClauses, Array<ExistFormula*> existFormulas, int search) {

    pll_ = new PseudoLogLikelihood(areNonEvidPreds_, domains_, wtPredsEqually_, sampleGndPreds_, fraction_, minGndPredSamples_, maxGndPredSamples_);
    pll_->setIndexTranslator(indexTrans_);

    int numClausesFormulas = getNumClausesFormulas();
    lbfgsb_ = new LBFGSB(-1, -1, pll_, numClausesFormulas);

    useTightParams();

    // compute initial counts for clauses in all MLNs
    cout << "computing counts for initial MLN clauses..." << endl;
    double begSec = timer_.time();
    pllComputeCountsForInitialMLN();
    cout << "computing counts for initial MLN clauses took "; 
    timer_.printTime(cout, timer_.time() - begSec); 
    cout << endl << endl;

    // learn initial wt/score of MLN, and set MLN clauses to learned weights
    cout << "learning the initial weights and score of MLN..." << endl << endl;
    begSec = timer_.time();

    double score;
    Array<double> wts;

    if (!learnAndSetMLNWeights(score)) return; 
    printMLNClausesWithWeightsAndScore(score, -1);
    cout << "learning the initial weights and score of MLN took ";
    timer_.printTime(cout, timer_.time() - begSec);
    cout << endl << endl;
    
    greedyEvalClauses(initialMLNClauses, score);

    printMLNToFile(NULL, -2);

  } // greedyEvaluation()
 

  void greedyEvalClauses(Array<Clause*>& candidates, const double& prevScore) {

    Array<double> priorMeans, priorStdDevs;
    int numClausesFormulas = getNumClausesFormulas();

    Array<double> wts;
    wts.growToSize(numClausesFormulas + 2); // first slot is unused
    Array<double> origWts(numClausesFormulas); // current clause/formula wts

    if (indexTrans_) indexTrans_->getClauseFormulaWts(origWts);
    else             mln0_->getClauseWts(origWts);

    // set the prior means and standard deviations
    setPriorMeansStdDevs(priorMeans, priorStdDevs, 1, NULL);
    if (indexTrans_) indexTrans_->appendClauseIdxToClauseFormulaIdxs(1, 1);

    bool error;
    double bsec;

    cout << "evaluating gain of candidates..." << endl;
    bsec = timer_.time();
    
    for (int ix = 0; ix < candidates.size(); ix++){

      countAndMaxScoreEffectCandidate(candidates[ix], &wts, &origWts, prevScore, true, priorMeans, priorStdDevs, error, NULL, NULL);
      // cout << "gain " << ix << ": " << candidates[ix]->getAuxClauseData()->gain << endl;
      // cout << "operation " << ix << ": " << candidates[ix]->getAuxClauseData()->op << endl;
      // cout << candidates[ix]->getWt() << endl;
      cout << candidates[ix]->getAuxClauseData()->gain << " \t ";
      candidates[ix]->printWithoutWtWithStrVar(cout,(*domains_)[0]);
      cout << endl;
    }

    cout << "evaluating gain of candidates took ";
    timer_.printTime(cout, timer_.time() - bsec); 
    cout << endl << endl; 

    if (indexTrans_) indexTrans_->removeClauseIdxToClauseFormulaIdxs(1, 1);
  } // greedyEvalClauses()

  void greedyLearn(Array<Clause*> initialMLNClauses, Array<ExistFormula*> existFormulas, int search) {

    pll_ = new PseudoLogLikelihood(areNonEvidPreds_, domains_, wtPredsEqually_, sampleGndPreds_, fraction_, minGndPredSamples_, maxGndPredSamples_);
    pll_->setIndexTranslator(indexTrans_);

    int numClausesFormulas = getNumClausesFormulas();
    lbfgsb_ = new LBFGSB(-1, -1, pll_, numClausesFormulas);

    useTightParams();

    // compute initial counts for clauses in all MLNs
    cout << "computing counts for initial MLN clauses..." << endl;
    double begSec = timer_.time();
    pllComputeCountsForInitialMLN();
    cout << "computing counts for initial MLN clauses took "; 
    timer_.printTime(cout, timer_.time() - begSec); 
    cout << endl << endl;

    // learn initial wt/score of MLN, and set MLN clauses to learned weights
    cout << "learning the initial weights and score of MLN..." << endl << endl;
    begSec = timer_.time();

    double score;
    Array<double> wts;

    if (!learnAndSetMLNWeights(score)) return; 
    printMLNClausesWithWeightsAndScore(score, -1);
    cout << "learning the initial weights and score of MLN took ";
    timer_.printTime(cout, timer_.time() - begSec);
    cout << endl << endl;
    
    greedyLearnClauses(initialMLNClauses, score);

    printMLNToFile(NULL, -2);

  } // greedyLearn()
 

  void greedyLearnClauses(Array<Clause*>& candidates, const double& prevScore) {

    Array<double> priorMeans, priorStdDevs;

    // set the prior means and standard deviations
    setPriorMeansStdDevs(priorMeans, priorStdDevs, 1, NULL);
    if (indexTrans_) indexTrans_->appendClauseIdxToClauseFormulaIdxs(1, 1);

    bool error;
    double bsec;
	double initialScore = prevScore;
    double score = prevScore;

    //cout << "evaluating gain of candidates..." << endl;
    bsec = timer_.time();
    
    // cout << "original score: " << score << endl;

    int numClausesAdded = 0;

    for (int ix = 0; ix < candidates.size(); ix++) {

	  if (mln0_->containsClause(candidates[ix])) {
		continue;
	  }

      Array<double> wts;
      wts.growToSize(getNumClausesFormulas() + 2); // first slot is unused
      Array<double> origWts(getNumClausesFormulas()); // current clause/formula wts

      if (indexTrans_) indexTrans_->getClauseFormulaWts(origWts);
      else             mln0_->getClauseWts(origWts);

      countAndMaxScoreEffectCandidate(candidates[ix], &wts, &origWts, score, true, priorMeans, priorStdDevs, error, NULL, NULL);
      cout << "weight: " << candidates[ix]->getWt() << "\t ";
      cout << "gain: " << candidates[ix]->getAuxClauseData()->gain << "\t ";
      candidates[ix]->printWithWtAndStrVar(cout,(*domains_)[0]);
      cout << endl;

      if (candidates[ix]->getAuxClauseData()->gain > minPllGain_ && fabs(candidates[ix]->getWt()) >= minWt_) {
        effectCandidateOnMLNs(candidates[ix], score, &origWts);
        cout << "new score: " << score << endl;
        numClausesAdded++;
      }
      /* if (numClausesAdded >= numClauses_) {
        break;
      } */
      score = initialScore;
    }

    printMLNClausesWithWeightsAndScore(score, iter_);

    cout << "evaluating gain of candidates took ";
    timer_.printTime(cout, timer_.time() - bsec); 
    cout << endl << endl; 
    cout << "pseudo-log-likelihood = " << score << endl;

    if (indexTrans_) indexTrans_->removeClauseIdxToClauseFormulaIdxs(1, 1);
  } // greedyLearnClauses()


  bool effectCandidateOnMLNs(Clause* const & candidate, double& score, const Array<double>* const & origWts)
  {
    Array<Clause*> candidates; 
    candidates.append(candidate);
    appendToMLNs(candidates, score, origWts);

/*
    if (appendToMLNs(candidates, score, origWts)) {
        cout << "Score did improve" << endl;
    }
    else {
        cout << "Score did not improve" << endl;
    }
*/
    // printMLNClausesWithWeightsAndScore(score, iter_);

    return true;
  }

  bool appendToMLNs(const Array<Clause*>& candidates, double& score, const Array<double>* const & origWts)
  {
    Array<double> wts;
    Array<UndoInfo*> undoInfos;
    Array<ClauseAndICDArray*> appendedClauseInfos;
    Array<double> priorMeans, priorStdDevs;
    bool error;

    double newScore = countAndMaxScoreEffectAllCandidates(candidates, &wts, origWts, score, true, priorMeans, priorStdDevs, error, &undoInfos, &appendedClauseInfos);

    bool improve = newScore >= score;

    if (!error)
    {
      score = newScore;

      // set MLN clauses/formulas to new weights
      // weights of appended clauses are set when they are added to the MLN 
      updateWts(wts, NULL, NULL);

      int numClausesFormulas = getNumClausesFormulas();

      for (int i = 0; i < appendedClauseInfos.size(); i++)
        appendedClauseInfos[i]->clause->setWt(wts[numClausesFormulas+i+1]);

      //effect candidates on MLN
      int appClauseIdx = 0;
      for (int i = 0; i < candidates.size(); i++)
      {
        Clause* cand = candidates[i];
        AuxClauseData* acd = cand->getAuxClauseData();
        int op = acd->op;

        // NOTE: cand was not canonicalized so as to easily compare it to the
        //       mln clause it is to replace. Canonicalize it now before 
        //       appending to MLN.
        if (op == OP_REPLACE) cand->canonicalize();

        if (op == OP_REMOVE || op == OP_REPLACE || op == OP_REPLACE_ADDPRED || 
            op == OP_REPLACE_REMPRED)
        {
          Clause* remClause = (Clause*) mln0_->getClause(acd->removedClauseIdx);

          for (int d = 0; d < mlns_->size(); d++)
          {
            if (d == 0)
            {
              Clause* r = removeClauseFromMLN(acd->removedClauseIdx, d);
              cout << "removed clause from MLN: "; 
              r->printWithoutWtWithStrVar(cout,(*domains_)[0]); cout << endl;
              if (op == OP_REMOVE && cand != r) delete cand;
              //delete r; //this is remClause which has to be used below
            }
            else
            {
              if (remClause->containsConstants())
                remClause->translateConstants((*domains_)[d-1], (*domains_)[d]);
              int remIdx = (*mlns_)[d]->findClauseIdx(remClause);
              delete removeClauseFromMLN(remIdx, d);
            }
          }
          delete remClause;
        }
        
        if (op == OP_ADD || op == OP_REPLACE || op == OP_REPLACE_ADDPRED || 
            op == OP_REPLACE_REMPRED)
        {
          Array<int*> idxPtrs; idxPtrs.growToSize(mlns_->size());
          for (int d = 0; d < mlns_->size(); d++)
          {
            Clause* c = cand;
            if (d > 0)
            {
              if (cand->containsConstants()) 
                cand->translateConstants((*domains_)[d-1], (*domains_)[d]);
              c = new Clause(*cand);
            }
            int idx = appendClauseToMLN(c, d);
            idxPtrs[d] = (*mlns_)[d]->getMLNClauseInfoIndexPtr(idx);
          }

          Array<IndexCountDomainIdx>& icds 
            = appendedClauseInfos[appClauseIdx++]->icdArray;
          for (int j = 0; j < icds.size(); j++)
            icds[j].iac->index = idxPtrs[ icds[j].domainIdx ];
            
          cout << "appended clause to MLN: "; 
          cand->printWithoutWtWithStrVar(cout,(*domains_)[0]); 
          cout << endl;
        }
      }

      assert(pll_->checkNoRepeatedIndex());
      assert(appClauseIdx == appendedClauseInfos.size());

      undoInfos.deleteItemsAndClear();

      // MLNs has changed, the index translation must be recreated
      if (indexTrans_) indexTrans_->createClauseIdxToClauseFormulaIdxsMap();
    }
  
    appendedClauseInfos.deleteItemsAndClear();

    return improve;
  } // appendToMLNs()


/// Jan Van Haaren - end ///////////////////////////////////////////////////////


  void buildInitCandidates(Array<Clause*>& candidates,
                              const Array<Clause*>* const & initMLNClauses)
  {
      // new clauses that are thrown out because they are in MLN
    Array<Clause*> thrownOutClauses; 
    ClauseOpHashArray newClauses;

    if (initMLNClauses){
        // add the MLN clauses to the candidates in the first step of beam 
        // search when we start from an empty MLN
      int newClausesBegIdx = newClauses.size();
      for (int i = 0; i < initMLNClauses->size(); i++){
        Clause* c = new Clause(*((*initMLNClauses)[i]));
        if (newClauses.append(c) < 0){
	  delete c;
	}
      }
      for (int i = newClausesBegIdx; i < newClauses.size(); i++){
        addNewClauseToCandidates(newClauses[i], candidates, &thrownOutClauses);
      }
    }
    for (int i = 0; i < thrownOutClauses.size(); i++){ 
      delete thrownOutClauses[i];
    }
  }

  void runStructLearning(Array<Clause*> initialMLNClauses,
                         Array<ExistFormula*> existFormulas)
  {
    structVerbose = true;
    cout << "structVerbose set to true" << endl;


/*
    startSec_ = timer_.time();

    bool needIndexTrans = IndexTranslator::needIndexTranslator(*mlns_,*domains_);

      //if we are starting from an empty MLN, and including MLN clauses among 
      //the candidates in first step of beam search
    Array<Clause*> initialMLNClauses; 
    Array<ExistFormula*> existFormulas;
    if (startFromEmptyMLN_)
    {
      getMLNClauses(initialMLNClauses, existFormulas);
      removeClausesFromMLNs();
      for (int i = 0; i < initialMLNClauses.size(); i++) 
      {
        Clause* c = initialMLNClauses[i];
        c->newAuxClauseData(); 
        c->trackConstants();
        c->getAuxClauseData()->gain = 0;
        c->getAuxClauseData()->op = OP_ADD;
        c->getAuxClauseData()->removedClauseIdx = -1;
      }
    }

      //add unit clauses to the MLN
    cout << "adding unit clauses to MLN..." << endl << endl;
    addUnitClausesToMLNs();

      //create auxiliary data for each clause in MLN
    for (int i = 0; i < mln0_->getNumClauses(); i++) 
    {
      Clause* c = (Clause*) mln0_->getClause(i);
      c->newAuxClauseData(); 
        //only clauses that are not in any exist. quant. formula's CNF may need 
        //to be translated across domains
      if (isModifiableClause(i)) c->trackConstants();
    }

    indexTrans_ = (needIndexTrans)? new IndexTranslator(mlns_, domains_) : NULL;
    if (indexTrans_) 
      cout << "The weights of clauses in the CNFs of existential formulas wiil "
           << "be tied" << endl;
*/
      //create PseudoLogLikelihood and LBFGSB
    pll_ = new PseudoLogLikelihood(areNonEvidPreds_, domains_, wtPredsEqually_, 
                                   sampleGndPreds_, fraction_, 
                                   minGndPredSamples_, maxGndPredSamples_);
    pll_->setIndexTranslator(indexTrans_);
    //pll_->setJDebug(true);
    int numClausesFormulas = getNumClausesFormulas();
    lbfgsb_ = new LBFGSB(-1, -1, pll_, numClausesFormulas);

    useTightParams();

      //compute initial counts for clauses in all MLNs
    cout << "computing counts for initial MLN clauses..." << endl;
    double begSec = timer_.time();
    pllComputeCountsForInitialMLN();
    cout << "computing counts for initial MLN clauses took "; 
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;


      //learn initial wt/score of MLN, and set MLN clauses to learned weights
    cout << "learning the initial weights and score of MLN..." << endl << endl;
    begSec = timer_.time();
    double score;
    double prevScore = 0;
    Array<double> wts;
    if (!learnAndSetMLNWeights(score)) return; 
    printMLNClausesWithWeightsAndScore(score, -1);
    cout << "learning the initial weights and score of MLN took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;
    prevScore = score;
    
      //add unit clauses with diff combinations of variables
    cout <<"trying to add unit clause with diff variable combinations to MLN..."
         << endl << endl;
    begSec = timer_.time();
    appendUnitClausesWithDiffCombOfVar(score); // score is updated
    printMLNClausesWithWeightsAndScore(score, -1);
    cout <<"adding unit clause with diff variable combinations to MLN took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;


      //for each clause in MLN, try all sign flips and keep best one in MLN
    if (tryAllFlips_)
    {
      cout << "trying to flip the senses of MLN clauses..." << endl << endl;
      begSec = timer_.time();
      flipMLNClauses(score); 
      printMLNClausesWithWeightsAndScore(score, -1); // score is updated
      cout << "trying to flip the senses of MLN clauses took ";;
      timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;
    }

    iter_ = -1;
    //printMLNToFile(NULL, iter_);

      //start searching for best candidates
    double bsec;
    Array<Clause*> initialClauses;
    Array<Clause*> bestCandidates;
    while (true)
    {
      begSec = timer_.time();
      iter_++;

      //if (iter_ == 2) break; // for testing purposes only
      cout << "Iteration " << iter_ << endl << endl;

      minGain_ = 0;

      Array<ExistFormula*> highGainWtExistFormulas;
      if (startFromEmptyMLN_ && !existFormulas.empty()) 
      {
        useTightParams();
        cout << "evaluating the gains of existential formulas..." << endl<<endl;
        bsec = timer_.time(); 
          //score not updated
        minGain_ = evaluateExistFormulas(existFormulas, highGainWtExistFormulas,
                                         score);
        cout << "evaluating the gains of existential formulas took ";
        timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
        cout << "setting minGain to min gain among existential formulas. "
             << "minGain = " << minGain_ << endl;
      }

      useLooseParams();
 
      //flag plus modify
        //add clauses in MLN to clauses array
      initialClauses.clear();
      if (plusSearch == 1){
	for (int i = 0; i < addedClauses.size(); i++) 
	  {
	    Clause* c = addedClauses[i];
	    c->newAuxClauseData();
	    c->getAuxClauseData()->reset();
	    if (isModifiableClause(i)) 
	      {
		c->trackConstants();
		initialClauses.append(new Clause(*c));
	      }
	  }
      }
      else{
	for (int i = 0; i < mln0_->getNumClauses(); i++) 
	  {
	    Clause* c = (Clause*) mln0_->getClause(i);
	    c->getAuxClauseData()->reset();
	    if (isModifiableClause(i)) 
	      {
		c->trackConstants();
		initialClauses.append(new Clause(*c));
	      }
	  }
      }

      bestCandidates.clear();

      beamSearch(initialClauses, initialMLNClauses, score, bestCandidates);
      bool noCand = (startFromEmptyMLN_ && !existFormulas.empty()) ?
          (bestCandidates.empty() && highGainWtExistFormulas.empty()) 
        : bestCandidates.empty();
      if (noCand)
      { 
        cout << "Beam is empty. Ending search for MLN clauses." << endl; 
        printIterAndTimeElapsed(begSec);
        break;
      }

      useTightParams();

      if (reEvalBestCandsWithTightParams_) 
      {
        cout << "reevaluating top candidates... " << endl << endl;
        bsec = timer_.time();
        reEvaluateCandidates(bestCandidates, score);
        cout << "reevaluating top candidates took ";
        timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
      }

        // effect the single best candidate on the MLN
      bsec = timer_.time();
      bool ok;

      Array<double> origWts(getNumClausesFormulas()); 
      if (indexTrans_){
	indexTrans_->getClauseFormulaWts(origWts);
      }
      else{
	mln0_->getClauseWts(origWts);
      }
      double timeBound = 3600 * hrs_ * domains_->size();
      //double timeBound = 86400 * 1 * domains_->size();
      if ((timer_.time() - startSec_ >= (timeBound)) &&
	  timeBased_){
	cout << "Out of time " << timeBound << endl << endl;
	break; 
      }

      if (startFromEmptyMLN_ && !existFormulas.empty())
        ok = effectBestCandidateOnMLNs(bestCandidates, existFormulas,
                                       highGainWtExistFormulas, score); 
      else{
        ok = effectBestCandidateOnMLNs(bestCandidates, score, &origWts);
      }
      
      cout << "score " << score << " prevScore " << prevScore << endl;
      double improve = fabs(score - prevScore);
      if (prevScore > score || improve < 0.001){

	cout << "Score instability, quiting\n";
	break;
      }
      prevScore = score;
      //score was updated when best candidate was effected on MLN
      cout << "effecting best candidates took ";
      timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;

      if (!ok) 
      { 
        cout << "failed to effect any of the best candidates on MLN" << endl
             << "stopping search for MLN clauses..." << endl;
        break;
      }
      if (partialPrint_){
	printMLNToFile(NULL, iter_);
      }

      printIterAndTimeElapsed(begSec);

    } // while (true)

    cout << "done searching for MLN clauses" << endl << endl;
    int numIterTaken = iter_+1;
    iter_= -1;

    useTightParams();
                                 
      //Prune each non-unit clause in MLN if it does not decrease score    
    cout << "pruning clauses from MLN..." << endl << endl;
    begSec = timer_.time();
    pruneMLN(score);
    cout << "pruning clauses from MLN took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;

    printMLNClausesWithWeightsAndScore(score, -1);
    printMLNToFile(NULL, -2);
    cout << "num of iterations taken = " << numIterTaken << endl;

    cout << "time taken for structure learning = ";
    timer_.printTime(cout, timer_.time()-startSec_); cout << endl << endl;

    initialMLNClauses.deleteItemsAndClear();
    deleteExistFormulas(existFormulas);
  }//run()


  void runWeightLearning(Array<Clause*> initialMLNClauses,
                         Array<ExistFormula*> existFormulas)
  {

      //create PseudoLogLikelihood and LBFGSB
    pll_ = new PseudoLogLikelihood(areNonEvidPreds_, domains_, wtPredsEqually_, 
                                   sampleGndPreds_, fraction_, 
                                   minGndPredSamples_, maxGndPredSamples_);
    pll_->setIndexTranslator(indexTrans_);
    //pll_->setJDebug(true);
    int numClausesFormulas = getNumClausesFormulas();
    lbfgsb_ = new LBFGSB(-1, -1, pll_, numClausesFormulas);

    useTightParams();

      //compute initial counts for clauses in all MLNs
    cout << "computing counts for initial MLN clauses..." << endl;
    double begSec = timer_.time();
    pllComputeCountsForInitialMLN();
    cout << "computing counts for initial MLN clauses took "; 
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;

      //learn initial wt/score of MLN, and set MLN clauses to learned weights
    cout << "learning the initial weights and score of MLN..." << endl << endl;
    begSec = timer_.time();
    double score;
    double prevScore = 0;
    Array<double> wts;
    if (!learnAndSetMLNWeights(score)) return; 
    printMLNClausesWithWeightsAndScore(score, -1);
    cout << "learning the initial weights and score of MLN took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;

     printMLNClausesWithWeightsAndScore(score, -1);
    printMLNToFile(NULL, -2);


    cout << "time taken for weight learning = ";
    timer_.printTime(cout, timer_.time()-startSec_); cout << endl << endl;
  }//runWL()



  void debugStructLearning(Array<Clause*> initialMLNClauses,
			   Array<ExistFormula*> existFormulas, int dstrat)
  {

      //create PseudoLogLikelihood and LBFGSB
    pll_ = new PseudoLogLikelihood(areNonEvidPreds_, domains_, wtPredsEqually_, 
                                   sampleGndPreds_, fraction_, 
                                   minGndPredSamples_, maxGndPredSamples_);
    pll_->setIndexTranslator(indexTrans_);

    int numClausesFormulas = getNumClausesFormulas();
    lbfgsb_ = new LBFGSB(-1, -1, pll_, numClausesFormulas);

    useTightParams();

      //compute initial counts for clauses in all MLNs
    cout << "computing counts for initial MLN clauses..." << endl;
    double begSec = timer_.time();
    pllComputeCountsForInitialMLN();
    cout << "computing counts for initial MLN clauses took "; 
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;


      //learn initial wt/score of MLN, and set MLN clauses to learned weights
    cout << "learning the initial weights and score of MLN..." << endl << endl;
    begSec = timer_.time();
    double score;
    Array<double> wts;
    if (!learnAndSetMLNWeights(score)) return; 
    printMLNClausesWithWeightsAndScore(score, -1);
    cout << "learning the initial weights and score of MLN took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;

    
      //add unit clauses with diff combinations of variables
    cout <<"trying to add unit clause with diff variable combinations to MLN..."
         << endl << endl;
    begSec = timer_.time();
    appendUnitClausesWithDiffCombOfVar(score); // score is updated
    printMLNClausesWithWeightsAndScore(score, -1);
    cout <<"adding unit clause with diff variable combinations to MLN took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;


      //for each clause in MLN, try all sign flips and keep best one in MLN
    if (tryAllFlips_)
    {
      cout << "trying to flip the senses of MLN clauses..." << endl << endl;
      begSec = timer_.time();
      flipMLNClauses(score); 
      printMLNClausesWithWeightsAndScore(score, -1); // score is updated
      cout << "trying to flip the senses of MLN clauses took ";;
      timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;
    }

    iter_ = -1;
    //printMLNToFile(NULL, iter_);

      //start searching for best candidates
    double bsec;
    Array<Clause*> initialClauses;
    Array<Clause*> bestCandidates;
    while (true)
    {
      begSec = timer_.time();
      iter_++;

      //if (iter_ == 2) break; // for testing purposes only
      cout << "Iteration " << iter_ << endl << endl;

      minGain_ = 0;

      Array<ExistFormula*> highGainWtExistFormulas;
      if (startFromEmptyMLN_ && !existFormulas.empty()) 
      {
        useTightParams();
        cout << "evaluating the gains of existential formulas..." << endl<<endl;
        bsec = timer_.time(); 
          //score not updated
        minGain_ = evaluateExistFormulas(existFormulas, highGainWtExistFormulas,
                                         score);
        cout << "evaluating the gains of existential formulas took ";
        timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
        cout << "setting minGain to min gain among existential formulas. "
             << "minGain = " << minGain_ << endl;
      }

      useLooseParams();
 
        //add clauses in MLN to clauses array
      initialClauses.clear();
      if (plusSearch == 1){

	for (int i = 0; i < addedClauses.size(); i++) {

	  Clause* c = addedClauses[i];
	  c->newAuxClauseData();
	  c->getAuxClauseData()->reset();

	  //if (isModifiableClause(i)){
	  c->trackConstants();

	  initialClauses.append(new Clause(*c));

	}
      }
      else{
	for (int i = 0; i < mln0_->getNumClauses(); i++) 
	  {
	    Clause* c = (Clause*) mln0_->getClause(i);
	    c->getAuxClauseData()->reset();
	    if (isModifiableClause(i)) 
	      {
		c->trackConstants();
		initialClauses.append(new Clause(*c));
	      }
	  }
      }
      
      bestCandidates.clear();
      cout << initialClauses.size() << " " << initialMLNClauses.size() << endl;
      countSearch(initialClauses, initialMLNClauses, score, bestCandidates, dstrat);

      bool noCand = (startFromEmptyMLN_ && !existFormulas.empty()) ?
          (bestCandidates.empty() && highGainWtExistFormulas.empty()) 
        : bestCandidates.empty();

      if (noCand)
      { 
        cout << "Beam is empty. Ending search for MLN clauses." << endl; 
        printIterAndTimeElapsed(begSec);
        break;
      }

      useTightParams();

      if (reEvalBestCandsWithTightParams_) 
      {
        cout << "reevaluating top candidates... " << endl << endl;
        bsec = timer_.time();
        reEvaluateCandidates(bestCandidates, score);
        cout << "reevaluating top candidates took ";
        timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
      }

        // effect the single best candidate on the MLN
      bsec = timer_.time();
      bool ok;

      Array<double> origWts(getNumClausesFormulas()); 
      if (indexTrans_){
	indexTrans_->getClauseFormulaWts(origWts);
      }
      else{
	mln0_->getClauseWts(origWts);
      }

      if (startFromEmptyMLN_ && !existFormulas.empty())
        ok = effectBestCandidateOnMLNs(bestCandidates, existFormulas,
                                       highGainWtExistFormulas, score); 
      else{
        ok = effectBestCandidateOnMLNs(bestCandidates, score, &origWts);
      }
      //score was updated when best candidate was effected on MLN
      cout << "effecting best candidates took ";
      timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;

      if (!ok) 
      { 
        cout << "failed to effect any of the best candidates on MLN" << endl
             << "stopping search for MLN clauses..." << endl;
        break;
      }
      if (partialPrint_){
	printMLNToFile(NULL, iter_);
      }
      double timeBound = 3600 * hrs_ * domains_->size();
      //double timeBound = 86400 * 1 * (domains_->size());
      if ((timer_.time() - startSec_ >= (timeBound)) &&
	   timeBased_){
	cout << "out of time " << timeBound << "\n";
	break; 
      }
      printIterAndTimeElapsed(begSec);

    } // while (true)

    cout << "done searching for MLN clauses" << endl << endl;
    int numIterTaken = iter_+1;
    iter_= -1;

    useTightParams();
                                 
      //Prune each non-unit clause in MLN if it does not decrease score    
    cout << "pruning clauses from MLN..." << endl << endl;
    begSec = timer_.time();
    pruneMLN(score);
    cout << "pruning clauses from MLN took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;

    printMLNClausesWithWeightsAndScore(score, -1);
    printMLNToFile(NULL, -2);
    cout << "num of iterations taken = " << numIterTaken << endl;

    cout << "time taken for structure learning = ";
    timer_.printTime(cout, timer_.time()-startSec_); cout << endl << endl;

    initialMLNClauses.deleteItemsAndClear();
    deleteExistFormulas(existFormulas);
  }//run()
  

  bool learnAndSetMLNWeights(double& score)
  {
    Array<double> priorMeans, priorStdDevs;
    double tmpScore = score;
    pllSetPriorMeansStdDevs(priorMeans, priorStdDevs, 0, NULL);
    int numClausesFormulas = getNumClausesFormulas();
    Array<double> wts;
    wts.growToSize(numClausesFormulas + 1);
      //indexTrans_ is already created
    int iter; bool error; double elapsedSec;
    tmpScore = maximizeScore(numClausesFormulas, 0, &wts, NULL, NULL,
                             iter, error, elapsedSec);
    if (error) 
    { 
      cout << "LBFGSB failed to find wts" << endl; 
      return false;
    }
    else 
    { 
      score = tmpScore;
      printNewScore((Clause*)NULL, NULL, iter, elapsedSec, score, 0, 0);
    }
    
    updateWts(wts, NULL, NULL);

    return true;
  }

  //////////////////////////// beam search ////////////////////////////////
 private:
    // priorMeans and priorStdDevs should be one larger than the current
    // num of clauses in the MLN. 
    // The clauses in initClauses must have their AuxClauseData created.
  void beamSearch(const Array<Clause*>& initClauses, 
                  const Array<Clause*>& initMLNClauses, const double& prevScore,
                  Array<Clause*>& bestClauses)
  {
    int iterBestGainUnchanged = 0; bsiter_ = -1;
    Array<double> priorMeans, priorStdDevs;
    ClauseOpHashArray* beam = new ClauseOpHashArray;
    for (int i = 0; i < initClauses.size(); i++) beam->append(initClauses[i]);

    int numClausesFormulas = getNumClausesFormulas();
    Array<double> wts;
    wts.growToSize(numClausesFormulas + 2); // first slot is unused
    Array<double> origWts(numClausesFormulas); // current clause/formula wts
    if (indexTrans_) indexTrans_->getClauseFormulaWts(origWts);
    else             mln0_->getClauseWts(origWts);

      //set the prior means and std deviations
    setPriorMeansStdDevs(priorMeans, priorStdDevs, 1, NULL);
    if (indexTrans_) indexTrans_->appendClauseIdxToClauseFormulaIdxs(1, 1);

    bool error;
    double begIterSec, bsec;
    bool hasTime = true;
    Array<Clause*> candidates;
    while (!beam->empty() && iterBestGainUnchanged < bestGainUnchangedLimit_
	   && hasTime)
    {
      begIterSec = timer_.time();
      bsiter_++; 

      //if (bsiter_ == 1) break; // for testing purposes only
      cout << endl << "BEAM SEARCH ITERATION " << bsiter_ << endl << endl;

      cout << "creating candidate clauses..." << endl;
      candidates.clear();
      bsec = timer_.time();
      if (bsiter_==0){
	createCandidateClauses(beam, candidates, &initMLNClauses);
      }
      else{
	createCandidateClauses(beam, candidates, NULL);
      }
      int removed = 0;
      for(int ix = 0; ix < candidates.size(); ix++){
	if (candidates[ix] == NULL){
	  cerr << "Null candidate\n";
	}
	candidates[ix]->canonicalize();
	//Flag find mln
	if (mln0_->containsClause(candidates[ix])){
	  delete candidates[ix];
	  candidates[ix] = NULL;
	  removed++;
	}
      }
      candidates.removeAllNull();
      cout << "num of candidates created = " << candidates.size();// << 
      cout << " after removing " << removed << "; took ";
      timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
      
      cout << "evaluating gain of candidates..." << endl << endl;
      bsec = timer_.time();

      if (true){
	for (int i = 0; i < candidates.size(); i++)
	  countAndMaxScoreEffectCandidate(candidates[i], &wts, &origWts,prevScore,
					  true, priorMeans, priorStdDevs, error,
					  NULL, NULL);
      }
      else{
	  countAndMaxScoreEffectAllCandidates(candidates, &wts, &origWts,prevScore,
					  true, priorMeans, priorStdDevs, error,
					  NULL, NULL);
	
      }
      /*
	=======
	for (int i = 0; i < candidates.size(); i++)
        countAndMaxScoreEffectCandidate(candidates[i], &wts, &origWts,prevScore,
                                        true, priorMeans, priorStdDevs, error,
                                        NULL, NULL);
					>>>>>>> 1.13
      */
      cout << "evaluating gain of candidates took ";
      timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;


      cout << "finding best candidates..." << endl;
      bsec = timer_.time();
      ClauseOpHashArray* newBeam = new ClauseOpHashArray(beamSize_);
      bool newBestClause = findBestClausesAmongCandidates(candidates, newBeam, 
                                                          bestClauses);
      cout << "finding best candidates took ";
      timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;

      beam->deleteItemsAndClear();
      delete beam;
      beam = newBeam;

      if (newBestClause)
      {
        iterBestGainUnchanged = 0;
        cout << "found new best clause in beam search iter " << bsiter_ << endl;
      }
      else
        iterBestGainUnchanged++;


        //print the best clauses
      cout << "best clauses found in beam search iter " << bsiter_ << endl;
      cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
      for (int i = 0; i < bestClauses.size(); i++)
      {
        cout << i << "\t";
        bestClauses[i]->printWithoutWtWithStrVar(cout,(*domains_)[0]);
        cout << endl
             << "\tgain = " << bestClauses[i]->getAuxClauseData()->gain
             << ",  op = " 
             << Clause::getOpAsString(bestClauses[i]->getAuxClauseData()->op);
        if (bestClauses[i]->getAuxClauseData()->op != OP_REMOVE)
          cout << ",  wt = " << bestClauses[i]->getWt();
        cout << endl;
      }
      cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl << endl;


      cout << "BEAM SEARCH ITERATION " << bsiter_ << " took ";
      timer_.printTime(cout, timer_.time()-begIterSec); cout << endl << endl;
      cout << "Time elapsed = ";
      timer_.printTime(cout, timer_.time()-startSec_); cout << endl << endl;
      //double timeBound = 86400 * 1 * domains_->size();
      double timeBound = 3600 * hrs_ * domains_->size();
      if ((timer_.time() - startSec_ >= (timeBound)) &&
	  timeBased_){
	cout << "out of time" << timeBound << "\n";
	hasTime = false;
      }
    
    }
   
    if (beam->empty()) cout << "Beam search ended because beam is empty" 
                            << endl << endl;
    else{
     
      cout << "Beam search ended because best clause did not change in "
              << iterBestGainUnchanged << " iterations" << endl << endl;
    
    }
    beam->deleteItemsAndClear();
    delete beam;
    bsiter_ = -1;

    if (indexTrans_) indexTrans_->removeClauseIdxToClauseFormulaIdxs(1, 1);
  }//beamSearch()



  void countSearch(const Array<Clause*>& initClauses, 
		   const Array<Clause*>& initMLNClauses, const double& prevScore,
		   Array<Clause*>& bestClauses, int dstrat)
  {
    jdebug = true;
    //int iterBestGainUnchanged = 0; 
    int bsiter_ = -1;
    Array<double> priorMeans, priorStdDevs;
    ClauseOpHashArray* beam = new ClauseOpHashArray;
    //flag plus modify
    for (int i = 0; i < initClauses.size(); i++) beam->append(initClauses[i]);

    int numClausesFormulas = getNumClausesFormulas();
    Array<double> wts;
    wts.growToSize(numClausesFormulas + 2); // first slot is unused
    Array<double> origWts(numClausesFormulas); // current clause/formula wts
    if (indexTrans_) indexTrans_->getClauseFormulaWts(origWts);
    else             mln0_->getClauseWts(origWts);

      //set the prior means and std deviations
    setPriorMeansStdDevs(priorMeans, priorStdDevs, 1, NULL);
    if (indexTrans_) indexTrans_->appendClauseIdxToClauseFormulaIdxs(1, 1);

    bool error;
    double begIterSec, bsec;
    Array<Clause*> candidates;
    begIterSec = timer_.time();
    bsiter_++; 
    if (dstrat == 0 && false){
      pll_->setJDebug(true);
    }
    //if (bsiter_ == 1) break; // for testing purposes only
    cout << endl << "BEAM SEARCH ITERATION " << bsiter_ << endl << endl;
    
    cout << "creating candidate clauses..." << endl;
    candidates.clear();
    bsec = timer_.time();
    //    if (initMLNClauses)
    //{
    ClauseOpHashArray nClauses;
    Array<Clause*> tOutClauses;  
    // add the MLN clauses to the candidates in the first step of beam 
    // search when we start from an empty MLN
    //int newClausesBegIdx = newClauses.size();
    if (dstrat == -1){
      createCandidateClauses(beam, candidates, NULL);
    }
    else{
      cout << initClauses.size() << " " << initMLNClauses.size() << endl;
      for (int i = 0; i < initMLNClauses.size(); i++){
	cout << "Here\n";
        Clause* c = new Clause(*(initMLNClauses[i]));
	//c->setJDebug(true);
        if (nClauses.append(c) < 0) delete c;
      }
      for (int i = 0; i < nClauses.size(); i++)
	addNewClauseToCandidates(nClauses[i], candidates, &tOutClauses);
    }


    int removed = 0;
    for(int ix = 0; ix < candidates.size(); ix++){
      candidates[ix]->canonicalize();
      //Flag find mln
      if (mln0_->containsClause(candidates[ix])){
	delete candidates[ix];
	candidates[ix] = NULL;
	removed++;
      }
    }
    candidates.removeAllNull();
    cout << "num of candidates created = " << candidates.size();// << 
    cout << " after removing " << removed << "; took ";
    timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
    jdebug = true;
    cout << "evaluating gain of candidates..." << endl << endl;
    bsec = timer_.time();
    if (true){
      for (int i = 0; i < candidates.size(); i++){
	//cerr << "Candidate " << i << endl;
	//candidates[i].printWithoutWt(cout, domains_[0]);
	//cerr << endl;
	countAndMaxScoreEffectCandidate(candidates[i], &wts, &origWts,prevScore,
					true, priorMeans, priorStdDevs, error,
					NULL, NULL);
      }
    }
    else{
	countAndMaxScoreEffectAllCandidates(candidates, &wts, &origWts,prevScore,
					true, priorMeans, priorStdDevs, error,
					NULL, NULL);

    }
    cout << "evaluating gain of candidates took ";
    timer_.printTime(cout, timer_.time()-bsec); 
    cout << endl << endl;
     
    beam->deleteItemsAndClear();
    delete beam;
    for(int ix = 0; ix < candidates.size(); ix++){
      if (candidates[ix]){
	delete candidates[ix];
      }
    }
    //beam = newBeam;
    /*
      for(int i = 0; i < preds_->size(); i++){
      (*preds_)[i]->print(cout,(*domains_)[0]);
      cout << endl;
      }
    */

  }//countSearch()



  /////////////// functions dealing with counts and maximizing score ///////////
 private:
    //size of wts array must be numClauses+1, its first slot is unused
    //if indexTrans_ is used, the sizes of its instance variables must be 
    //correctly set before calling this function
  double maximizeScore(const int& numClausesFormulas, const int& numExtraWts,
                       Array<double>* const & wts, 
                       const Array<double>* const & origWts,
                       const Array<int>* const & removedClauseFormulaIdxs, 
                       int& iter, bool& error, double& elapsedSec)
  {
    if (origWts) 
    { for (int i=1 ; i<=numClausesFormulas; i++) (*wts)[i] = (*origWts)[i-1];}
    else         
    { for (int i=1; i<=numClausesFormulas; i++) (*wts)[i] = 0; }

    for (int i = 1; i <= numExtraWts; i++) (*wts)[numClausesFormulas+i] = 0;


    if (removedClauseFormulaIdxs) 
      for (int i = 0; i < removedClauseFormulaIdxs->size(); i++) 
        (*wts)[ (*removedClauseFormulaIdxs)[i]+1 ] = 0;

    //commented out: this is done before this function
    //if (indexTrans_) 
    //  indexTrans_->appendClauseIdxToClauseFormulaIdxs(numExtraWts, 1);

    double* wwts = (double*) wts->getItems();
    double begSec = timer_.time();
    double newScore 
      = lbfgsb_->minimize(numClausesFormulas + numExtraWts, wwts, iter, error);
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
  double countAndMaxScoreEffectAllCandidates(const Array<Clause*>& candidates, 
                                             Array<double>* const & wts,
                                             const Array<double>*const& origWts,
                                             const double& prevScore,
                                             const bool& resetPriors,
                                             Array<double>& priorMeans,
                                             Array<double>& priorStdDevs,
                                             bool& error,
                                            Array<UndoInfo*>* const& uundoInfos,
                         Array<ClauseAndICDArray*>* const & appendedClauseInfos)
  {
    Array<UndoInfo*>* undoInfos = (uundoInfos)? uundoInfos:new Array<UndoInfo*>;
    Array<int> remClauseFormulaIdxs; //for domain 0 only
    Array<int*> idxPtrs; //for clean up
    Array<Clause*> toBeRemovedClauses;
    Array<Clause*> toBeAppendedClauses;
    int numClausesFormulas = getNumClausesFormulas();
      
    for (int i = 0; i < candidates.size(); i++)
    {

      Clause* cand = candidates[i];
      AuxClauseData* acd = cand->getAuxClauseData();
 
      int op = acd->op;

        //remove counts for candidate
      if (op == OP_REMOVE || op == OP_REPLACE || op == OP_REPLACE_ADDPRED || 
          op == OP_REPLACE_REMPRED)
      {
        Clause* remClause = (mlns_->size() > 1) ? 
          (Clause*) mln0_->getClause(acd->removedClauseIdx) : NULL;
        Array<int*> idxs; idxs.growToSize(mlns_->size());
        Array<Clause*> remClauses; remClauses.growToSize(mlns_->size());
        for (int d = 0; d < mlns_->size(); d++)
        {
          int remIdx;
          if (d == 0)
          {
            remIdx = acd->removedClauseIdx;
            remClauseFormulaIdxs.append(remIdx);
          }
          else
          {
            if (remClause->containsConstants())
              remClause->translateConstants((*domains_)[d-1], (*domains_)[d]);
            remIdx = (*mlns_)[d]->findClauseIdx(remClause);
          }
          idxs[d] = (*mlns_)[d]->getMLNClauseInfoIndexPtr(remIdx);
          remClauses[d] = (Clause*) (*mlns_)[d]->getClause(remIdx);
        }
        if (mlns_->size() > 1 && remClause->containsConstants())
          remClause->translateConstants((*domains_)[mlns_->size()-1], 
                                        (*domains_)[0]);
        pllRemoveCountsForClause(remClauses, idxs, undoInfos);

        if (op == OP_REMOVE) toBeRemovedClauses.append(cand);
      }

        //compute counts for candidate
      if (op == OP_ADD || op == OP_REPLACE || op == OP_REPLACE_ADDPRED || 
          op == OP_REPLACE_REMPRED)
      {
        Array<int*> tmpIdxs; tmpIdxs.growToSize(mlns_->size());
        for (int d = 0; d < mlns_->size(); d++)
        {
          int* tmpClauseIdxInMLN = new int( (*mlns_)[d]->getNumClauses() + 
                                            toBeAppendedClauses.size() );
          tmpIdxs[d] = tmpClauseIdxInMLN;
          idxPtrs.append(tmpClauseIdxInMLN);

        }

        Array<UndoInfo*>* tmpInfos = (appendedClauseInfos)?  
                                     new Array<UndoInfo*> : undoInfos;

	pllComputeCountsForClause(cand, tmpIdxs, tmpInfos); 

        toBeAppendedClauses.append(cand);

        if (appendedClauseInfos) 
        {
          undoInfos->append(*tmpInfos);
          ClauseAndICDArray* ca = new ClauseAndICDArray;
          ca->clause = cand;
          appendedClauseInfos->append(ca);          
          for (int j = 0; j < tmpInfos->size(); j++)
          {
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
    if (resetPriors) 
    {
      setPriorMeansStdDevs(priorMeans, priorStdDevs, toBeAppendedClauses.size(),
                           &remClauseFormulaIdxs);

      if (indexTrans_){ 
	indexTrans_->appendClauseIdxToClauseFormulaIdxs(toBeAppendedClauses.size(), 1);

      }
    }
    else
    {
      assert(priorMeans.size() == numClausesFormulas + candidates.size());
      assert(priorStdDevs.size() == numClausesFormulas + candidates.size());
      if (indexTrans_) 
        assert(indexTrans_->checkCIdxWtsGradsSize(candidates.size()));

      setRemoveAppendedPriorMeans(numClausesFormulas, priorMeans, 
                                  remClauseFormulaIdxs, 
                                  toBeAppendedClauses.size(), removedValues);

    }

    if (hasPrior_) 
      pll_->setMeansStdDevs(priorMeans.size(), priorMeans.getItems(), 
                            priorStdDevs.getItems());
    else
      pll_->setMeansStdDevs(-1, NULL, NULL);

    wts->growToSize(numClausesFormulas + toBeAppendedClauses.size() + 1);

    int iter; double elapsedSec; 
    double newScore = maximizeScore(numClausesFormulas, 
                                    toBeAppendedClauses.size(), 
                                    wts, origWts, &remClauseFormulaIdxs,
                                    iter, error, elapsedSec);

      //set the weights of the candidates
    for (int i = 0; i < toBeAppendedClauses.size(); i++)
      toBeAppendedClauses[i]->setWt( (*wts)[numClausesFormulas+i+1] );
      //set to large weight so that they are filtered out because of small wt
    for (int i = 0; i < toBeRemovedClauses.size(); i++)
      toBeRemovedClauses[i]->setWt(111111111); 
    
    Array<double> penalties;
    for (int i = 0; i < candidates.size(); i++)
      penalties.append(getPenalty(candidates[i]));
                     
    if (error) { newScore=prevScore; cout<<"LBFGSB failed to find wts"<<endl; }
   
    printNewScore(candidates, (*domains_)[0], iter, elapsedSec, 
		  newScore, newScore-prevScore, penalties);
   
    for (int i = 0; i < candidates.size(); i++)
      candidates[i]->getAuxClauseData()->gain = newScore-prevScore-penalties[i];

    if (uundoInfos == NULL) 
    { 
      pll_->undoAppendRemoveCounts(undoInfos); 

      delete undoInfos;
    }

    if (resetPriors) 
    {
      if (indexTrans_) indexTrans_->removeClauseIdxToClauseFormulaIdxs(
                                                 toBeAppendedClauses.size(), 1);
    }
    else
    {   //restore the prior means that were set to zero
      for (int i = 0; i < remClauseFormulaIdxs.size(); i++)
        priorMeans[remClauseFormulaIdxs[i]] = removedValues[i];
    }

    idxPtrs.deleteItemsAndClear();

    return newScore;
  } //countAndMaxScoreEffectAllCandidates()


  Array<Clause*> plusType(Clause* clause){
    Array<Clause*> candidates;
    const Array<Predicate*>* preds = clause->getPredicates();

    //hash_map<int,int>* var2type = new hash_map<int,int>;
    Array<int> positions;
    ArraysAccessor<int> acc;
    bool noPlus = true;
    for(int ix = 0; ix < preds->size(); ix++){
    
      const Array<int>* perConst = (*preds)[ix]->getTemplate()->getPerConstantIndexes();
      //cout << perConst->size() << endl;
      if (perConst->size() > 0){
	noPlus = false;
	int pos = (*perConst)[0];
	if ((*preds)[ix]->getTerm(pos)->getType()  == Term::VARIABLE){
	  int tId = ((*preds)[ix])->getTerm(pos)->getId();
	  //int type = 0;
	  //(*var2type)[tId] = type;
	  positions.append(tId);
	  const  Array<int>* constants = (*domains_)[0]->getConstantsByTypeWithExt((*preds)[ix]->getTemplate()->getTermTypeAsInt(pos));
	  acc.appendArray(constants);
	}
      }
    }
    if (noPlus){
      candidates.append(clause);
    }
    else{
      // Jan-2804: cout << "In else\n";
      // Jan-2804: cout << "Evaluate: ";
      // Jan-2804: clause->printWithoutWt(cout, (*domains_)[0]);
      // Jan-2804: cout << endl;
      Array<int> termIds;
      int count = 0;
      while(acc.getNextCombination(termIds)){// && count < 1){
	// Jan-2804: cout << "HERE: ";
	count++;
	hash_map<int,int>* var2const = new hash_map<int,int>;
	for(int ix = 0; ix < termIds.size(); ix++){
	  (*var2const)[positions[ix]] = termIds[ix];
	  // Jan-2804: cout << termIds[ix] << " ";
	}
	// Jan-2804: cout << endl;
	Clause* newClause = new Clause(*clause);
	Clause::substitute(newClause, var2const);
	newClause->canonicalize();//createVarIdToVarsGroundedType((*domains_)[0]);
	newClause->trackConstants();
	//newClause->computeAndStoreIntArrRep();
	//newClause->computeAndStoreIntArrRep();
	newClause->setDirty();
	candidates.append(newClause);
	delete var2const;
      }
    }
    acc.deleteArraysAndClear();
    return candidates;
  }
  
  double countAndMaxScoreEffectCandidate(Clause* const & candidate, 
                                         Array<double>* const & wts,
                                         const Array<double>* const & origWts, 
                                         const double& prevScore,
                                         const bool& resetPriors,
                                         Array<double>& priorMeans,
                                         Array<double>& priorStdDevs,
                                         bool& error,
                                         Array<UndoInfo*>* const& undoInfos,
					 Array<ClauseAndICDArray*>* const & appendedClauseInfos)
  {
    //flag plus modify
    Array<Clause*> candidates; 
    if (plusSearch == 1){
      candidates = plusType(candidate);
      cout << "Candidates has " << candidates.size() << " clauses\n";
      /*
      */
    }
    else{
      candidates.append(candidate);
    }
    double res = countAndMaxScoreEffectAllCandidates(candidates, wts, origWts, 
						     prevScore, resetPriors,
						     priorMeans, priorStdDevs,
						     error, undoInfos, 
						     appendedClauseInfos);
    if (plusSearch == 1 && candidates.size() > 1){
      for(int ix = 0; ix < candidates.size(); ix++){
	delete candidates[ix];
      }
    }
    return res;
  }


    //returns true if the candidates are all effected on MLN resulting in a
    //better score
  bool appendToAndRemoveFromMLNs(const Array<Clause*>& candidates,double& score,
				 const Array<double>* const & origWts,
				 const bool& makeChangeForEqualScore=false)
				 
  {
    Array<double> wts;
    Array<UndoInfo*> undoInfos;
    Array<ClauseAndICDArray*> appendedClauseInfos;
    Array<double> priorMeans, priorStdDevs;
    bool error;

    double newScore
      = countAndMaxScoreEffectAllCandidates(candidates, &wts, origWts, score,
                                            true, priorMeans,priorStdDevs,error,
                                            &undoInfos, &appendedClauseInfos);

    bool improve = (makeChangeForEqualScore) ? (newScore >= score) 
                                             : (newScore > score);

    if (!error && improve)
    {
      score = newScore;

        //set MLN clauses/formulas to new weights
        //weights of appended clauses are set when they are added to the MLN 
      updateWts(wts, NULL, NULL);

      int numClausesFormulas = getNumClausesFormulas();

      for (int i = 0; i < appendedClauseInfos.size(); i++)
        appendedClauseInfos[i]->clause->setWt(wts[numClausesFormulas+i+1]);

        //effect candidates on MLN
      int appClauseIdx = 0;
      for (int i = 0; i < candidates.size(); i++)
      {
        Clause* cand = candidates[i];
        AuxClauseData* acd = cand->getAuxClauseData();
        int op = acd->op;

          // NOTE: cand was not canonicalized so as to easily compare it to the
          //       mln clause it is to replace. Canonicalize it now before 
          //       appending to MLN.
        if (op == OP_REPLACE) cand->canonicalize();

        if (op == OP_REMOVE || op == OP_REPLACE || op == OP_REPLACE_ADDPRED || 
            op == OP_REPLACE_REMPRED)
        {
          Clause* remClause = (Clause*) mln0_->getClause(acd->removedClauseIdx);

          for (int d = 0; d < mlns_->size(); d++)
          {
            if (d == 0)
            {
              Clause* r = removeClauseFromMLN(acd->removedClauseIdx, d);
              cout << "Modified MLN: Removed clause from MLN: "; 
              r->printWithoutWtWithStrVar(cout,(*domains_)[0]); cout << endl;
              if (op == OP_REMOVE && cand != r) delete cand;
              //delete r; //this is remClause which has to be used below
            }
            else
            {
              if (remClause->containsConstants())
                remClause->translateConstants((*domains_)[d-1], (*domains_)[d]);
              int remIdx = (*mlns_)[d]->findClauseIdx(remClause);
              delete removeClauseFromMLN(remIdx, d);
            }
          }
          delete remClause;
        }
        
        if (op == OP_ADD || op == OP_REPLACE || op == OP_REPLACE_ADDPRED || 
            op == OP_REPLACE_REMPRED)
        {
          Array<int*> idxPtrs; idxPtrs.growToSize(mlns_->size());
          for (int d = 0; d < mlns_->size(); d++)
          {
            Clause* c = cand;
            if (d > 0)
            {
              if (cand->containsConstants()) 
                cand->translateConstants((*domains_)[d-1], (*domains_)[d]);
              c = new Clause(*cand);
            }
            int idx = appendClauseToMLN(c, d);
            idxPtrs[d] = (*mlns_)[d]->getMLNClauseInfoIndexPtr(idx);
          }

          Array<IndexCountDomainIdx>& icds 
            = appendedClauseInfos[appClauseIdx++]->icdArray;
          for (int j = 0; j < icds.size(); j++)
            icds[j].iac->index = idxPtrs[ icds[j].domainIdx ];
            
          cout << "Modified MLN: Appended clause to MLN: "; 
          cand->printWithoutWtWithStrVar(cout,(*domains_)[0]); 
          cout << endl;
        }
      }

      assert(pll_->checkNoRepeatedIndex());
      assert(appClauseIdx == appendedClauseInfos.size());

      undoInfos.deleteItemsAndClear();

        //MLNs has changed, the index translation must be recreated
    if (indexTrans_) indexTrans_->createClauseIdxToClauseFormulaIdxsMap();
    }
    else
    {
      cout << "undoing candidates because score did not improve..."<<endl<<endl;
      pll_->undoAppendRemoveCounts(&undoInfos);
    }
  
    appendedClauseInfos.deleteItemsAndClear();
    return improve;
  } // appendToAndRemoveFromMLNs()


    //returns true if candidate is effected on MLN resulting in a better score
  bool appendToAndRemoveFromMLNs(Clause* const & candidate, double& score,
				 const Array<double>* const & origWts,
                                 const bool& makeChangeForEqualScore=false)
  {
    //flag modify plus
    Array<Clause*> candidates; 
    if (plusSearch == 1){
      candidates = plusType(candidate);
    }
    else{
      candidates.append(candidate);
    }
    return appendToAndRemoveFromMLNs(candidates, score, origWts, makeChangeForEqualScore);
  }


    //Clause c enters & exits  function with its AuxClauseData's cache == NULL
  void pllComputeCountsForClause(Clause* const & c,
                                 const Array<int*>& clauseIdxInMLNs,
                                 Array<UndoInfo*>* const & undoInfos)
  {
    assert(c->getAuxClauseData()->cache == NULL);
    assert(clauseIdxInMLNs.size() == domains_->size());
    double begSec = timer_.time();
 
    if (cacheClauses_)    
    {
      int i;
      if ((i = cachedClauses_->find(c)) >= 0) // if clause and counts are cached
      {
        //cout << "using cached counts ";
        //cout << "for "; c->printWithoutWtWithStrVar(cout, (*domains_)[0]); 
        //cout << endl;
        pll_->insertCounts(clauseIdxInMLNs, undoInfos, 
                           (*cachedClauses_)[i]->getAuxClauseData()->cache);
	//cout << "structVerbose " << structVerbose << endl;
	if (structVerbose){
	  cout << "using cached counts took ";
	  timer_.printTime(cout, timer_.time()-begSec); cout << endl;
	}
        return;
      }
      else
      {
        assert(c->getAuxClauseData()->cache == NULL);
        if (cacheSizeMB_ <  maxCacheSizeMB_)
          c->newCache(domains_->size(), (*domains_)[0]->getNumPredicates());
        else
        {
          static bool printCacheFull = true;
          if (printCacheFull)
          {
            cout << "Cache is full, approximate size = " << cacheSizeMB_ 
                 << " MB" << endl;
            printCacheFull = false;
          }
        }
        //cout << "cache size (MB) = " << cacheSizeMB_ << endl;
      }
    }
    
    //we do not have the counts
    /*
    */
    //if we don't need to translate constant ids from one domain to another
    if (!c->containsConstants())
    {
      for (int i = 0; i < domains_->size(); i++)
      {
        int* clauseIdxInMLN = clauseIdxInMLNs[i];
        pll_->computeCountsForNewAppendedClause(c, clauseIdxInMLN, i,
                                                undoInfos, sampleClauses_, 
                                                c->getAuxClauseData()->cache);
      }
    }
    else
    {  //we need to translate constant ids from one domain to another
      int i;
      for (i = 0; i < domains_->size(); i++)
      {
        if (i > 0) c->translateConstants((*domains_)[i-1], (*domains_)[i]);
        int* clauseIdxInMLN = clauseIdxInMLNs[i];
        pll_->computeCountsForNewAppendedClause(c, clauseIdxInMLN, i,
                                                undoInfos, sampleClauses_, 
                                                c->getAuxClauseData()->cache);

      }

      if (i > 1) c->translateConstants((*domains_)[i-1], (*domains_)[0]);

    }


    if (c->getAuxClauseData()->cache)
    {
      if (cacheSizeMB_ < maxCacheSizeMB_)
      {
        cacheSizeMB_ += c->sizeMB();
        Array<Array<Array<CacheCount*>*>*>* cache =c->getAuxClauseData()->cache;
        c->getAuxClauseData()->cache = NULL;
        Clause* copyClause = new Clause(*c);
        copyClause->getAuxClauseData()->cache = cache;
        copyClause->compress();
        cachedClauses_->append(copyClause);
      }
      else
      {
        c->getAuxClauseData()->deleteCache();
        c->getAuxClauseData()->cache = NULL; 
      }
    }
    if (structVerbose){
      cout << "Computing counts took ";
      timer_.printTime(cout, timer_.time()-begSec); 
      //cout << " for " << endl ;
      //cout << "\t"; c->printWithoutWtWithStrVar(cout, (*domains_)[0]); 
      cout << endl;
    }
  }


  void pllRemoveCountsForClause(const Array<Clause*>& remClauses, 
                                const Array<int*>& clauseIdxInMLNs,
                                Array<UndoInfo*>* const & undoInfos)
  { 
    assert(clauseIdxInMLNs.size() == domains_->size());
    double begSec = timer_.time();        
    for (int i = 0; i < domains_->size(); i++)
      pll_->removeCountsForClause(remClauses[i],clauseIdxInMLNs[i],i,undoInfos);
    if (structVerbose){
      cout << "Removing counts took ";
      timer_.printTime(cout, timer_.time()-begSec); 
      cout << endl;
    }
  }



  void pllComputeCountsForInitialMLN()
  {
    for (int i = 0; i < mlns_->size(); i++)
    {
      cout << "computing counts for clauses in domain " << i << "..." << endl;
      MLN* mln = (*mlns_)[i];
      for (int j = 0; j < mln->getNumClauses(); j++)
      {
        Clause* c = (Clause*)mln->getClause(j);
        cout << "Clause " << j << ": ";
        c->printWithoutWtWithStrVar(cout, (*domains_)[i]); cout << endl;
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

    ClauseFactory::createUnitClauses(addedClauses,nonEvidPreds,allowEqualPreds);
    ClauseFactory::createUnitClauses(unitClauses,nonEvidPreds,allowEqualPreds, domains_, plusSearch >= 1);

    for (int i = 0; i < unitClauses.size(); i++)
    {

      if (mln0_->containsClause(unitClauses[i]))
      { delete unitClauses[i]; continue; }
      ostringstream oss; int idx; 
      unitClauses[i]->printWithoutWtWithStrVar(oss, (*domains_)[0]);
      
      for (int j = 0; j < mlns_->size(); j++)
      {
        Clause* c = (j == 0) ? unitClauses[i] : new Clause(*unitClauses[i]);
        (*mlns_)[j]->appendClause(oss.str(), false, c, priorMean_, false, idx);
        ((MLNClauseInfo*)(*mlns_)[j]->getMLNClauseInfo(idx))->priorMean 
          = priorMean_;
      }
    }
  }


  void appendUnitClausesWithDiffCombOfVar(double& score)
  {
    bool allowEqualPreds = false;
    for (int i = 0; i < preds_->size(); i++)
    {
      if (!(*areNonEvidPreds_)[(*preds_)[i]->getId()]) continue;

      Clause* origClause = ClauseFactory::createUnitClause((*preds_)[i], 
                                                           allowEqualPreds);
      if (origClause == NULL) continue;
      assert(origClause->getAuxClauseData() == NULL);
      origClause->setAuxClauseData(new AuxClauseData);
      origClause->trackConstants();

      ClauseOpHashArray newUnitClauses;
      /*
	cout << "Diff combo ";
	(*preds_)[i]->print(cout, (*domains_)[0]);
	cout << endl;
      */
      clauseFactory_->createUnitClausesWithDiffCombOfVar((*preds_)[i], OP_ADD,
                                                         -1, newUnitClauses);
      for (int j = 0; j < newUnitClauses.size(); j++)
      {
        Clause* newClause = newUnitClauses[j];

        if (origClause->same(newClause) || 
            !clauseFactory_->validClause(newClause) ||
            mln0_->containsClause(newClause))
        {
          newUnitClauses.removeItemFastDisorder(j);
          delete newClause; 
          j--;
          continue;
        }
          //score is updated
        if (!appendToAndRemoveFromMLNs(newClause, score, NULL)) delete newClause;
      }
      delete origClause;
    } // for each predicate
  }


  void flipMLNClauses(double& score)
  {
    Array<Clause*> mlnClauses;
    for (int i = 0; i < mln0_->getNumClauses(); i++)
    {
        //do not flip unit clauses or those derived from existential formulas
      if (mln0_->getClause(i)->getNumPredicates()==1 || !isModifiableClause(i))
        continue;
      mlnClauses.append((Clause*)mln0_->getClause(i));
    }

    for (int i = 0; i < mlnClauses.size(); i++)
    {
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
      Clause* bestClause = NULL;
      double bestScore = score;
      bool error;

      for (int j = 0; j < newClauses.size(); j++) // for each 'flipped' clause
      {
        Clause* newClause = newClauses[j];
        if (origClause->same(newClause) || newClause->hasRedundantPredicates() 
            || mln0_->containsClause(newClause)) { delete newClause; continue; }

        double newScore 
          = countAndMaxScoreEffectCandidate(newClause, &wts, NULL, score, true,
                                            priorMeans, priorStdDevs, error, 
                                            NULL, NULL);
        if (newScore > bestScore)
        {
          bestScore = newScore;
          if (bestClause) delete bestClause;
          bestClause = newClause;          
        }
        else
          delete newClause;
      }

      if (bestClause && !appendToAndRemoveFromMLNs(bestClause, score, NULL)) 
        delete bestClause;
    }// for each MLN clause
  }//flipMLNClauses()


    //NOTE: formulas with existentially quantified variables, or variables
    //      with mutually exclusive and exhaustive values are not pruned.
  void pruneMLN(double& score)
  {
    Array<Clause*> mlnClauses;
    for (int i = 0; i < mln0_->getNumClauses(); i++)
    {
        //do not prune unit clauses or clauses derived from existential formulas
      if (mln0_->getClause(i)->getNumPredicates() == 1 || 
          !isModifiableClause(i)) continue;
      mlnClauses.append((Clause*)mln0_->getClause(i));
    }
    
    for (int i = 0; i < mlnClauses.size(); i++)
    {
      Clause* origClause = mlnClauses[i];
      int origIdx = mln0_->findClauseIdx(origClause);
      Clause* copy = new Clause(*origClause);
      copy->setAuxClauseData(new AuxClauseData(0, OP_REMOVE, origIdx));
      copy->trackConstants();
      if (!appendToAndRemoveFromMLNs(copy, score, NULL, true)) delete copy;
    }
  }


  int appendClauseToMLN(Clause* const c, const int& domainIdx)
  {
    ostringstream oss; int idx;
    c->printWithoutWtWithStrVar(oss, (*domains_)[domainIdx]);
    MLN* mln = (*mlns_)[domainIdx];
    bool ok = mln->appendClause(oss.str(), false, c, c->getWt(), false, idx);
    if (!ok) { cout << "ERROR: failed to insert " << oss.str() <<" into MLN"
                    << endl; exit(-1); }
    ((MLNClauseInfo*)mln->getMLNClauseInfo(idx))->priorMean = priorMean_;
    return idx;
  }


  Clause* removeClauseFromMLN(const int& remClauseIdx, const int& domainIdx)
  {
    Clause* remClause = (*mlns_)[domainIdx]->removeClause(remClauseIdx);
    if (remClause == NULL)
    { 
      cout << "ERROR: failed to remove "; 
      remClause->printWithoutWtWithStrVar(cout, (*domains_)[0]);
      cout << " from MLN" << endl;
      exit(-1);
    }
    return remClause;
  }

  
  ////////////////////// functions for clause creation ////////////////////////
 private:
  void addPredicateToClause(Clause* const & beamClause, const int& op, 
                            const int& removeClauseIdx,
                            ClauseOpHashArray& newClauses)
  {

    if (jdebug){
      cout << "Beam Clause ";
      beamClause->print(cout, (*domains_)[0]);
      cout << endl;
      for(int ix = 0; ix < preds_->size(); ix++){
	cout << "Possible Preds: ";
	(*preds_)[ix]->print(cout, (*domains_)[0]);
	cout << endl;
      }
    }
      // create new clauses by adding predicates to beamClause
    clauseFactory_->addPredicateToClause(*preds_,beamClause,op,removeClauseIdx,
                                         true,newClauses,false,domains_,
					 plusSearch == 2);

      // if this is a unit clause, flip its sense and create new clauses by
      // adding preds to it
    if (beamClause->getNumPredicates() == 1)
    {
      beamClause->getPredicate(0)->invertSense();        
      clauseFactory_->addPredicateToClause(*preds_, beamClause, op,
                                           removeClauseIdx, true, newClauses,
                                           false,domains_, plusSearch==2);
      beamClause->getPredicate(0)->invertSense();
    }    
  }


    // create new clauses by removing a predicate from beamClause
  void removePredicateFromClause(Clause* const & beamClause, const int& op,
                                 const int& removeClauseIdx,
                                 ClauseOpHashArray& newClauses)
  {
    if (beamClause->getNumPredicates() > 2)
      clauseFactory_->removePredicateFromClause(beamClause, op, removeClauseIdx,
                                                newClauses);
  }


  void addNewClauseToCandidates(Clause* const & newClause,
                                Array<Clause*>& candidates, 
                                Array<Clause*>* const & thrownOut)
  {
      // remove any new clauses that are already in MLN
    if (thrownOut && mln0_->containsClause(newClause)) 
    { thrownOut->append(newClause); return;}
    newClause->setWt(0);
    candidates.append(newClause);
  }


  void createCandidateClauses(const ClauseOpHashArray* const & beam, 
                              Array<Clause*>& candidates,
                              const Array<Clause*>* const & initMLNClauses)
  {

    // new clauses that are thrown out because they are in MLN
    Array<Clause*> thrownOutClauses; 
    ClauseOpHashArray newClauses;
 

    for (int i = 0; i < beam->size(); i++)
    {

      Clause* beamClause = (*beam)[i];
      AuxClauseData* beamacd = beamClause->getAuxClauseData();      
      int op = beamacd->op;
      int newClausesBegIdx = newClauses.size();

      if (op == OP_ADD)
      {
        int remIdx = beamacd->removedClauseIdx;
        addPredicateToClause(beamClause, op, remIdx, newClauses);
        removePredicateFromClause(beamClause, op, remIdx, newClauses);
        for (int j = newClausesBegIdx; j < newClauses.size(); j++)
          addNewClauseToCandidates(newClauses[j], candidates,&thrownOutClauses);
      }
      else
      if (op == OP_REPLACE_ADDPRED)
      {
        addPredicateToClause(beamClause, op, beamacd->removedClauseIdx,
                             newClauses);
        for (int j = newClausesBegIdx; j < newClauses.size(); j++)
          addNewClauseToCandidates(newClauses[j], candidates,&thrownOutClauses);
      }
      else
      if (op == OP_REPLACE_REMPRED)
      {
        removePredicateFromClause(beamClause, op, beamacd->removedClauseIdx,
                                  newClauses);
        for (int j = newClausesBegIdx; j < newClauses.size(); j++)
          addNewClauseToCandidates(newClauses[j], candidates,&thrownOutClauses);
      }
      else
      if (op == OP_NONE)
      {
        int idx = mln0_->findClauseIdx(beamClause);
        bool beamClauseInMLN = (idx >= 0);
        bool removeBeamClause = (beamClauseInMLN && 
                                 beamClause->getNumPredicates() > 1);
        bool isModClause = (!beamClauseInMLN || isModifiableClause(idx));

        if (isModClause)
        {
          addPredicateToClause(beamClause, OP_ADD, -1, newClauses);
          for (int j = newClausesBegIdx; j < newClauses.size(); j++)
            addNewClauseToCandidates(newClauses[j], candidates,
                                     &thrownOutClauses);

          if (removeBeamClause)
          {
            newClausesBegIdx = newClauses.size();
            addPredicateToClause(beamClause, OP_REPLACE_ADDPRED, idx, 
                                 newClauses);
            for (int j = newClausesBegIdx; j < newClauses.size(); j++)
              addNewClauseToCandidates(newClauses[j], candidates,
                                       &thrownOutClauses);            
          }

          newClausesBegIdx = newClauses.size();
          removePredicateFromClause(beamClause, OP_ADD, -1, newClauses);
          for (int j = newClausesBegIdx; j < newClauses.size(); j++)
            addNewClauseToCandidates(newClauses[j], candidates,
                                     &thrownOutClauses);          

          if (removeBeamClause)
          {
            newClausesBegIdx = newClauses.size();          
            removePredicateFromClause(beamClause, OP_REPLACE_REMPRED, idx,
                                      newClauses);
            for (int j = newClausesBegIdx; j < newClauses.size(); j++)
              addNewClauseToCandidates(newClauses[j], candidates,
                                       &thrownOutClauses);          
          }

          if (removeBeamClause)
          {
            Clause* c = new Clause(*beamClause);
            c->getAuxClauseData()->op = OP_REMOVE;
            c->getAuxClauseData()->removedClauseIdx = idx;
            addNewClauseToCandidates(c, candidates, NULL);
          }
        }        
      }
      else
        assert(op == OP_REMOVE || op == OP_REPLACE);
    } // for each item in beam


    if (initMLNClauses)
    {
        // add the MLN clauses to the candidates in the first step of beam 
        // search when we start from an empty MLN
      int newClausesBegIdx = newClauses.size();
      for (int i = 0; i < initMLNClauses->size(); i++)
      {
        Clause* c = new Clause(*((*initMLNClauses)[i]));
        if (newClauses.append(c) < 0) delete c;
      }
      for (int i = newClausesBegIdx; i < newClauses.size(); i++)
        addNewClauseToCandidates(newClauses[i], candidates, &thrownOutClauses);
    }

    for (int i = 0; i < thrownOutClauses.size(); i++) 
      delete thrownOutClauses[i];
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
  
  //Flag Jesse
  void useTightParamsCache(int sample)
  {
    if (false && sample == 2){
      sampleClauses_ = false;
      pll_->setSampleGndPreds(false);
    }
    else{
      sampleClauses_ = origSampleClauses_;
      pll_->setSampleGndPreds(sampleGndPreds_);
    }
    lbfgsb_->setMaxIter(lbMaxIter_);
    lbfgsb_->setFtol(lbConvThresh_);
    cacheClauses_ = origCacheClauses_;
  }


  void useLooseParams()
  {
    sampleClauses_ = origSampleClauses_;
    pll_->setSampleGndPreds(sampleGndPreds_);
    if (looseMaxIter_ >= 0) lbfgsb_->setMaxIter(looseMaxIter_);
    if (looseConvThresh_ >= 0) lbfgsb_->setFtol(looseConvThresh_);
    cacheClauses_ = origCacheClauses_;
  }

  
    //i is the clause's index in mln_[0]
  bool isModifiableClause(const int& i) const
  { return (!mln0_->isExistClause(i) && !mln0_->isExistUniqueClause(i)); }


  bool isNonModifiableFormula(const FormulaAndClauses* const & fnc) const
  { return (fnc->hasExist || fnc->isExistUnique); }


  void printIterAndTimeElapsed(const double& begSec)
  {
    cout << "Iteration " << iter_ << " took ";
    timer_.printTime(cout, timer_.time()-begSec); cout << endl << endl;
    cout << "Time elapsed = ";
    timer_.printTime(cout, timer_.time()-startSec_); cout << endl << endl;
  }


  void removeClausesFromMLNs()
  {
    for (int i = 0; i < mlns_->size(); i++)
      {
	bool deleteClauses = (i == 0) ? true : false;
	(*mlns_)[i]->removeAllClauses(NULL, deleteClauses);
      }
  }


  void reEvaluateCandidates(Array<Clause*>& candidates,
                            const double& prevScore)
  {
    int numClausesFormulas = getNumClausesFormulas();
    Array<double> wts; 
    wts.growToSize(numClausesFormulas + 2);//slot 0 is unused
    Array<double> origWts(numClausesFormulas); // current clause/formula wts
    if (indexTrans_) indexTrans_->getClauseFormulaWts(origWts);
    else             mln0_->getClauseWts(origWts);

    Array<double> priorMeans, priorStdDevs;
    setPriorMeansStdDevs(priorMeans, priorStdDevs, 1, NULL);
    if (indexTrans_) indexTrans_->appendClauseIdxToClauseFormulaIdxs(1, 1);






    bool error;
    for (int i = 0; i < candidates.size(); i++)
    {
      candidates[i]->getAuxClauseData()->gain = 0;
      //<<<<<<< structlearn.h
      
      countAndMaxScoreEffectCandidate(candidates[i], &wts, &origWts, prevScore,
                                      true, priorMeans, priorStdDevs, error,
				      NULL,NULL);
				      /*
      countAndMaxScoreEffectCandidate(candidates[i], &wts, NULL, prevScore,
                                      true, priorMeans, priorStdDevs, error,
                                      NULL, NULL);
      */
    }

    if (indexTrans_) indexTrans_->removeClauseIdxToClauseFormulaIdxs(1, 1);

    Array<Clause*> tmpCand(candidates);
    rquicksort(tmpCand);
    candidates.clear();
    for (int i = 0; i < tmpCand.size(); i++)
    {
      cout << "minGain_ " << tmpCand[i]->getAuxClauseData()->gain << endl;
      //cout << tmpCand[i]->getWt()) >= minWt_
      if (tmpCand[i]->getAuxClauseData()->gain > minGain_ && 
          fabs(tmpCand[i]->getWt()) >= minWt_)
        candidates.append(tmpCand[i]);
      else
        delete tmpCand[i];
    }

    cout << "reevaluated top " << candidates.size() 
         << " candidates with tight params:" << endl;
    cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
    for (int i = 0; i < candidates.size(); i++)
    {
      cout << i << "\t";
      candidates[i]->printWithoutWtWithStrVar(cout,(*domains_)[0]);
      cout << endl 
           << "\tgain = " << candidates[i]->getAuxClauseData()->gain 
           << ",  wt = " << candidates[i]->getWt()
           << ",  op = "
           << Clause::getOpAsString(candidates[i]->getAuxClauseData()->op) 
           << endl;
    }
    cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl << endl;;
  }


    //Copy the best beamSize_ candidates into beam, and place the best 
    //numEstBestClauses_ clauses among those in candidates and bestClauses into
    //bestClauses. Returns true if the best clause changed.
  bool findBestClausesAmongCandidates(Array<Clause*>& candidates,
                                      ClauseOpHashArray* const & beam,
                                      Array<Clause*>& bestClauses)
  {
    assert(beam->size() == 0);

    // fill the beam with best beamSize_ candidates

      //sort candidates in order of decreasing gain
    rquicksort(candidates); 

    for (int i = 0; i < candidates.size(); i++)
    {
      if (beam->size() >= beamSize_) break;
      double candGain = candidates[i]->getAuxClauseData()->gain;
      double candAbsWt = fabs(candidates[i]->getWt());
      if (candGain > minGain_ && candAbsWt >= minWt_)
      {
        int a = beam->append(new Clause(*(candidates[i])));
        assert(a >= 0); a = 0; //avoid compilation warning
      }
    }
    
    if (beam->size() == 0)
    {
      for (int i = 0; i < candidates.size(); i++) delete candidates[i];
      return false;
    }


    Clause* prevBestClause = (bestClauses.size() > 0) ? 
                             new Clause(*(bestClauses[0])) : NULL;

    // pick the best numEstBestClauses_ clauses

    ClauseOpSet cset;
    ClauseOpSet::iterator it;
    for (int i = 0; i < candidates.size(); i++) cset.insert(candidates[i]);

    for (int i = 0; i < bestClauses.size(); i++) 
    {
      if ((it=cset.find(bestClauses[i])) == cset.end())
      {
        candidates.append(bestClauses[i]);
        cset.insert(bestClauses[i]);
      }
      else
      {
        assert((*it)->getAuxClauseData()->gain == 
               bestClauses[i]->getAuxClauseData()->gain);
        delete bestClauses[i];
      }
    }
    
    rquicksort(candidates);

    bestClauses.clear();
    for (int i = 0; i < candidates.size(); i++)
    {
      if (bestClauses.size() < numEstBestClauses_)
      {
        double candGain = candidates[i]->getAuxClauseData()->gain;
        double candAbsWt = fabs(candidates[i]->getWt());
        if (candGain > minGain_ && candAbsWt >= minWt_) 
          bestClauses.append(candidates[i]);
        else
        {
          //cout << "\tDiscarding candidate because of low gain or abs weight:";
          //cout << " gain = " << candGain << ", wt = " 
          //     << candidates[i]->getWt() << endl;
          //cout << "\t"; 
          //candidates[i]->printWithoutWtWithStrVar(cout, (*domains_)[0]);
          //cout << endl;
          delete candidates[i];
        }
      }
      else
        delete candidates[i];
    }

      //check whether the best clause has changed
    bool bestClauseChanged;
    if (bestClauses.size() > 0)
    {
      if (prevBestClause == NULL) bestClauseChanged = true;
      else
      {
          // if the prev and cur best clauses are the same 
        if (bestClauses[0]->same(prevBestClause) && 
            bestClauses[0]->getAuxClauseData()->op == 
            prevBestClause->getAuxClauseData()->op) 
          bestClauseChanged =  false;
        else
        { // prev and cur best clauses are different
            //if they have the same gain
          if (bestClauses[0]->getAuxClauseData()->gain >
              prevBestClause->getAuxClauseData()->gain) 
            bestClauseChanged = true;
          else
            bestClauseChanged = false;
        }
      }
    }
    else
      bestClauseChanged = false;

    if (prevBestClause) delete prevBestClause;
    return bestClauseChanged;
  }


  //Flag
  bool effectBestCandidateOnMLNs(Clause* const & cand, double& score, const Array<double>* const & origWts)
  {
    //Flag modify plus
    //cout << "Jesse " << endl;
    if (plusSearch == 1){
      
      Array<Clause*> toAdd = plusType(cand);
      if (appendToAndRemoveFromMLNs(toAdd, score, origWts)){
	addedClauses.append(new Clause(*cand));
	printMLNClausesWithWeightsAndScore(score, iter_); 
	//printMLNToFile(NULL, iter_);
	return true;
      }
      cout << "Did not add" << endl;
      return false;
    }
    else{
      if (appendToAndRemoveFromMLNs(cand, score, origWts)){ 
	printMLNClausesWithWeightsAndScore(score, iter_); 
	//printMLNToFile(NULL, iter_);
	return true;
      }
      cout << "Did not add" << endl;
      return false;
    }
  }


  //Flag
  bool effectBestCandidateOnMLNs(Array<Clause*>& bestCandidates, double& score, const Array<double>* const & origWts)
  {
    cout << "effecting best candidate on MLN..." << endl << endl;
    bool ok = false;
    int i;
    //cout << "JD 1 " << bestCandidates.size() << endl;
    for (i = 0; i < bestCandidates.size(); i++)
    {
      cout << "effecting best candidate " << i << " on MLN..." << endl << endl;
      for(int dx = 0; dx < domains_->size(); dx++){
	
	// double count = bestCandidates[i]->getNumTrueGroundings((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	// double ng = bestCandidates[i]->getNumGroundings((*domains_)[dx]);    
	// cout << "count = " << count << " / " << ng << endl;
      }
      if ((ok=effectBestCandidateOnMLNs(bestCandidates[i], score, origWts))) break;
      cout << "failed to effect candidate on MLN." << endl;
      delete bestCandidates[i];
    }
    for (int j = i+1; j < bestCandidates.size();j++) delete bestCandidates[j];
    return ok;
  }


  double getPenalty(const Clause* const & cand)
  {
    int op = cand->getAuxClauseData()->op;

    if (op == OP_ADD) return cand->getNumPredicates() * penalty_;

    if (op == OP_REPLACE_ADDPRED) 
    {
      int remIdx = cand->getAuxClauseData()->removedClauseIdx;
      int origlen = mln0_->getClause(remIdx)->getNumPredicates();        
      int diff = cand->getNumPredicates() - origlen;
      assert(diff > 0);
      return diff * penalty_;
    }

    if (op == OP_REPLACE_REMPRED) 
    {
      int remIdx = cand->getAuxClauseData()->removedClauseIdx;
      int origlen = mln0_->getClause(remIdx)->getNumPredicates();        
      int diff = origlen - cand->getNumPredicates();
      assert(diff > 0);
      return diff * penalty_;
    }

    if (op == OP_REPLACE)
    {
        //NOTE: op is REPLACE only when an MLN clause is to be replaced with
        //      one that is identical except for its predicates' senses
      int remIdx = cand->getAuxClauseData()->removedClauseIdx;
      const Clause* mlnClause = mln0_->getClause(remIdx);
      assert(cand->getNumPredicates() == mlnClause->getNumPredicates());
      int diff = 0;
      for (int i = 0; i < cand->getNumPredicates(); i++)
      {
        Predicate* cpred = (Predicate*) cand->getPredicate(i);
        Predicate* mpred = (Predicate*) mlnClause->getPredicate(i);
        assert(cpred->same(mpred));
        if (cpred->getSense() != mpred->getSense()) diff++;
      }
      return diff * penalty_;      
    }

    if (op == OP_REMOVE) return cand->getNumPredicates() * penalty_;

    assert(false);
    return 88888;
  }


  void printNewScore(const Array<Clause*>& carray, const Domain* const & domain,
                     const int& lbfgsbIter, const double& lbfgsbSec,
                     const double& newScore, const double& gain,
                     const Array<double>& penalties)
  {
    
    if (structVerbose){
      cout << "*************************** " << candCnt_++ << ", iter " << iter_ 
	   << ", beam search iter " << bsiter_ << endl;
      
      for (int i = 0; i < carray.size(); i++)
	{
	  if (carray[i])
	    { 
	      cout << "candidate     : ";
	      carray[i]->printWithoutWtWithStrVar(cout,domain); cout << endl;
	      cout << "op            : ";
	      cout << Clause::getOpAsString(carray[i]->getAuxClauseData()->op) <<endl;
	      cout << "removed clause: ";
	      int remIdx = carray[i]->getAuxClauseData()->removedClauseIdx;
	      if (remIdx < 0) cout << "NULL";
	      else { mln0_->getClause(remIdx)->printWithoutWtWithStrVar(cout,domain);}
	      cout << endl;
	      if (carray[i]->getAuxClauseData()->prevClauseStr.length() > 0)
		{
		  cout << "prevClause    : ";
		  cout << carray[i]->getAuxClauseData()->prevClauseStr << endl;
		}
	      if (carray[i]->getAuxClauseData()->addedPredStr.length() > 0)
		{
		  cout << "addedPred     : ";
		  cout << carray[i]->getAuxClauseData()->addedPredStr << endl;
		}
	      if (carray[i]->getAuxClauseData()->removedPredIdx >= 0)
		{
		  cout << "removedPredIdx: ";
		  cout << carray[i]->getAuxClauseData()->removedPredIdx << endl;
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
      cout << "time taken by LBFGSB = "; Timer::printTime(cout, lbfgsbSec);
      cout << endl;
      cout << "*************************** " << endl << endl;;
    }
  }


  void printNewScore(const Clause* const & c, const Domain* const & domain, 
                     const int& lbfgsbIter, const double& lbfgsbSec,
                     const double& newScore, const double& gain,
                     const double& penalty)
  {
    Array<Clause*> carray;
    Array<double> parray;
    if (c) { carray.append((Clause*)c); parray.append(penalty); }
    if (structVerbose)
      printNewScore(carray, domain, lbfgsbIter, lbfgsbSec, newScore, gain,parray);
  }


  void printMLNClausesWithWeightsAndScore(const double& score, const int& iter)
  {    
    if (iter >= 0) cout << "MLN in iteration " << iter << ":" << endl;
    else           cout << "MLN:" << endl;
    cout << "------------------------------------" << endl;
    if (indexTrans_) indexTrans_->printClauseFormulaWts(cout, true);
    else             mln0_->printMLNClausesFormulas(cout, (*domains_)[0], true);
    cout << "------------------------------------" << endl;
    cout << "score = "<< score << endl << endl;
  }


  void printMLNToFile(const char* const & appendStr, const int& iter)
  {
    string fname = outMLNFileName_;

    if (appendStr) fname.append(".").append(appendStr);

    if (iter >= -1) 
    { 
      char buf[100]; 
      sprintf(buf, "%d", iter);
      fname.append(".iter").append(buf);
    }

    if (appendStr || iter >= -1) fname.append(".mln");
    
    ofstream out(fname.c_str());
    if (!out.good()) { cout << "ERROR: failed to open " <<fname<<endl;exit(-1);}

      // output the predicate declaration
    out << "//predicate declarations" << endl;
    (*domains_)[0]->printPredicateTemplates(out);
    out << endl;

      // output the function declarations
    if ((*domains_)[0]->getNumFunctions() > 0) 
    {
      out << "//function declarations" << endl;
      (*domains_)[0]->printFunctionTemplates(out);
      out << endl;
    }

    if (indexTrans_) indexTrans_->printClauseFormulaWts(out, false);
    else             mln0_->printMLNClausesFormulas(out, (*domains_)[0], false);

    out << endl;
    out.close();
  } 
  

  void printClausesInBeam(const ClauseOpHashArray* const & beam)
  {
    cout.setf(ios_base::left, ios_base::adjustfield);
    cout << "^^^^^^^^^^^^^^^^^^^ beam ^^^^^^^^^^^^^^^^^^^" << endl;
    for (int i = 0; i < beam->size(); i++)
    {
      cout << i << ":  ";
      cout.width(10); cout << (*beam)[i]->getWt(); cout.width(0); cout << " ";
      (*beam)[i]->printWithoutWtWithStrVar(cout, (*domains_)[0]); cout << endl;
    }
    cout.width(0);
    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
    
  }

  void setPriorMeansStdDevs(Array<double>& priorMeans, 
                            Array<double>& priorStdDevs, const int& addSlots,
                            const Array<int>* const& removedSlotIndexes)
  {
    priorMeans.clear(); priorStdDevs.clear();

    if (indexTrans_)
    {
      indexTrans_->setPriorMeans(priorMeans);
      priorStdDevs.growToSize(priorMeans.size());
      for (int i = 0; i < priorMeans.size(); i++)
        priorStdDevs[i] = priorStdDev_;
    }
    else
    {
      for (int i = 0; i < mln0_->getNumClauses(); i++)
      {
        priorMeans.append(mln0_->getMLNClauseInfo(i)->priorMean);
        priorStdDevs.append(priorStdDev_);
      }
    }

    if (removedSlotIndexes)
    {
      for (int i = 0; i < removedSlotIndexes->size(); i++)
        priorMeans[ (*removedSlotIndexes)[i] ] = 0;
    }

    for (int i = 0; i < addSlots; i++)
    {
      priorMeans.append(priorMean_);
      priorStdDevs.append(priorStdDev_);      
    }
  }


  void pllSetPriorMeansStdDevs(Array<double>& priorMeans, 
                               Array<double>& priorStdDevs, const int& addSlots,
                               const Array<int>* const & removedSlotIndexes)
  {
    if (hasPrior_)
    {
      setPriorMeansStdDevs(priorMeans, priorStdDevs, 
                           addSlots, removedSlotIndexes);
      pll_->setMeansStdDevs(priorMeans.size(), priorMeans.getItems(), 
                            priorStdDevs.getItems());
    }
    else
      pll_->setMeansStdDevs(-1, NULL, NULL);
  }


  void setRemoveAppendedPriorMeans(const int& numClausesFormulas,
                                   Array<double>& priorMeans, 
                                   const Array<int>& removedSlotIndexes, 
                                   const int& addSlots,
                                   Array<double>& removedValues)
  {
    for (int i = 0; i < removedSlotIndexes.size(); i++)
    {
      removedValues.append(priorMeans[ removedSlotIndexes[i] ]);
      priorMeans[ removedSlotIndexes[i] ] = 0;
    }
    for (int i = 0; i < addSlots; i++){
      priorMeans[numClausesFormulas+i] = priorMean_;
    }
  }

  
  int getNumClausesFormulas()
  {
    if (indexTrans_) return indexTrans_->getNumClausesAndExistFormulas();
    return mln0_->getNumClauses();
  }


  void updateWts(const Array<double>& wts,
                 const Array<Clause*>* const & appendedClauses,
                 const Array<string>* const & appendedFormulas)
  {
    if (indexTrans_)
    {
      Array<double> tmpWts; 
      tmpWts.growToSize(wts.size()-1);
      for (int i = 1; i < wts.size(); i++) tmpWts[i-1] = wts[i];
      indexTrans_->updateClauseFormulaWtsInMLNs(tmpWts, appendedClauses,
                                                appendedFormulas);
    }
    else
    {
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
    if (l >= r) return;
    Clause* tmp = items[l];
    items[l] = items[(l+r)/2];
    items[(l+r)/2] = tmp;

    int last = l;
    for (int i = l+1; i <= r; i++)
      if (items[i]->getAuxClauseData()->gain > 
          items[l]->getAuxClauseData()->gain)
      {
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

  void rquicksort(Array<Clause*>& ca) { rquicksort(ca, 0, ca.size()-1); }



  ////////////////// functions to handle existential formulas ///////////////
 private:

  void deleteExistFormulas(Array<ExistFormula*>& existFormulas)
  { existFormulas.deleteItemsAndClear(); }


  void getMLNClauses(Array<Clause*>& initialMLNClauses,
                     Array<ExistFormula*>& existFormulas)
  {
    for (int i = 0; i < mln0_->getNumClauses(); i++)
      if (isModifiableClause(i)) 
      {
        Clause* c = (Clause*) mln0_->getClause(i);
        initialMLNClauses.append(new Clause(*c));
      }

    const FormulaAndClausesArray* fnca = mln0_->getFormulaAndClausesArray();
    for (int i = 0; i < fnca->size(); i++)
      if (isNonModifiableFormula((*fnca)[i]))
        existFormulas.append(new ExistFormula((*fnca)[i]->formula));

    for (int i = 0; i < existFormulas.size(); i++)
    {
      string formula = existFormulas[i]->formula;      
      FormulaAndClauses tmp(formula, 0, false);
      const FormulaAndClausesArray* fnca
        = (*mlns_)[0]->getFormulaAndClausesArray();
      int a = fnca->find(&tmp);            
      existFormulas[i]->numPreds = (*fnca)[a]->numPreds;

      Array<Array<Clause*> >& cnfClausesForDomains 
        = existFormulas[i]->cnfClausesForDomains;
      cnfClausesForDomains.growToSize(mlns_->size());
      for (int j = 0; j < mlns_->size(); j++)
      {
        Array<Clause*>& cnfClauses = cnfClausesForDomains[j];
        fnca = (*mlns_)[j]->getFormulaAndClausesArray();
        a = fnca->find(&tmp);
        assert(a >= 0);
        IndexClauseHashArray* indexClauses = (*fnca)[a]->indexClauses;
        for (int k = 0; k < indexClauses->size(); k++)
        {
          Clause* c = new Clause(*((*indexClauses)[k]->clause));
          c->newAuxClauseData();
          cnfClauses.append(c);
        }
        cnfClauses.compress();        
      }
    }
  }


    //cnfClauses enter & exit function with its AuxClauseData's cache == NULL
  inline void pllCountsForExistFormula(Clause* cnfClause, 
                                       const int& domainIdx,
                                       int* clauseIdxInMln,
                                       Array<UndoInfo*>* const & undoInfos)
  {
    assert(cnfClause->getAuxClauseData()->cache == NULL);
    bool inCache = false; 
    bool hasDomainCounts = false; 
    double prevCNFClauseSize = 0;

    if (cacheClauses_)
    {
      int i; 
      if ((i = cachedClauses_->find(cnfClause)) >= 0) //if clause is in cache
      {
        inCache = true;
        Array<Array<Array<CacheCount*>*>*>* cache =
          (*cachedClauses_)[i]->getAuxClauseData()->cache;
        Array<Array<CacheCount*>*>*  domainCache = (*cache)[domainIdx];
        for (int j = 0; j < domainCache->size(); j++)
          if ((*domainCache)[j] != NULL) { hasDomainCounts = true; break; }
      }
     
      if (hasDomainCounts) // if clause and counts for domain are cached
      {
        pll_->insertCounts(clauseIdxInMln, undoInfos,
                           (*cachedClauses_)[i]->getAuxClauseData()->cache,
                           domainIdx);
        return;
      }
      else
      {
        if (cacheSizeMB_ <  maxCacheSizeMB_)
        {
          if (inCache) //if clause is in cache but not counts for domain
          {
            cnfClause->getAuxClauseData()->cache 
              = (*cachedClauses_)[i]->getAuxClauseData()->cache;
            prevCNFClauseSize = cnfClause->sizeMB();
          }
          else
            cnfClause->newCache(domains_->size(),
                                (*domains_)[0]->getNumPredicates());
        }
        else
        {
          static bool printCacheFull = true;
          if (printCacheFull)
          {
            cout << "Cache is full, approximate size = " << cacheSizeMB_ 
                 <<" MB" << endl;
            printCacheFull = false;
          }
        }
      }
    }
    
      //we do not have the counts
    pll_->computeCountsForNewAppendedClause(cnfClause,clauseIdxInMln,domainIdx,
                                            undoInfos, sampleClauses_, 
                                            cnfClause->getAuxClauseData()->cache);

    if (cnfClause->getAuxClauseData()->cache)
    {
      if (inCache)
      {
        cacheSizeMB_ += cnfClause->sizeMB() - prevCNFClauseSize;
        cnfClause->getAuxClauseData()->cache = NULL; 
      }
      else
      {
        if (cacheSizeMB_ < maxCacheSizeMB_)
        {
          cacheSizeMB_ += cnfClause->sizeMB();
          Array<Array<Array<CacheCount*>*>*>* cache 
            = cnfClause->getAuxClauseData()->cache;
          cnfClause->getAuxClauseData()->cache = NULL;
          Clause* copyClause = new Clause(*cnfClause);
          copyClause->getAuxClauseData()->cache = cache;
          copyClause->compress();
          cachedClauses_->append(copyClause);
        }
        else
        {
          cnfClause->getAuxClauseData()->deleteCache();
          cnfClause->getAuxClauseData()->cache = NULL; 
        }
      }
    }
  }


  inline void printNewScore(const string existFormStr,const int& lbfgsbIter,
                            const double& lbfgsbSec, const double& newScore,
                            const double& gain, const double& penalty, 
                            const double& wt)
  {
    if (structVerbose){
      cout << "*************************** " << candCnt_++ << ", iter " << iter_ 
	   << ", beam search iter " << bsiter_ << endl;
      
      cout << "exist formula : " << existFormStr << endl;
      cout << "op            : OP_ADD" << endl; 
      cout << "score    : " << newScore << endl;
      cout << "gain     : " << gain << endl;
      cout << "penalty  : " << penalty << endl;
      cout << "net gain : " << gain - penalty << endl;
      cout << "wt       : " << wt << endl;
      cout << "num LBFGSB iter      = " << lbfgsbIter << endl;
      cout << "time taken by LBFGSB = "; Timer::printTime(cout, lbfgsbSec);
      cout << endl;
      cout << "*************************** " << endl << endl;;
    }
  }

  inline void rquicksort(Array<ExistFormula*>& existFormulas, const int& l, 
                         const int& r)  
  {
    ExistFormula** items = (ExistFormula**) existFormulas.getItems();
    if (l >= r) return;
    ExistFormula* tmp = items[l];
    items[l] = items[(l+r)/2];
    items[(l+r)/2] = tmp;

    int last = l;
    for (int i = l+1; i <= r; i++)
      if (items[i]->gain > items[l]->gain)
      {
        ++last;
        ExistFormula* tmp = items[last];
        items[last] = items[i];
        items[i] = tmp;
      }
    
    tmp = items[l];
    items[l] = items[last];
    items[last] = tmp;
    rquicksort(existFormulas, l, last-1);
    rquicksort(existFormulas, last+1, r); 
  }


  inline void rquicksort(Array<ExistFormula*>& ef) 
  { rquicksort(ef, 0, ef.size()-1); }


    // Compute counts for formula, and learn optimal weights and gain.
    // If we are computing counts only, then the counts are added permanently into
    // PseudoLogLikelihood.
  inline void evaluateExistFormula(ExistFormula* const& ef, 
                                   const bool& computeCountsOnly,
              Array<Array<Array<IndexAndCount*> > >* const & iacArraysPerDomain,
                                   const double& prevScore)
  {
    bool undo = !computeCountsOnly;
    bool evalGainLearnWts = !computeCountsOnly;

    Array<int*> idxPtrs; //for clean up
    Array<UndoInfo*>* undoInfos = (undo) ? new Array<UndoInfo*> : NULL;

      //compute counts for the CNF clauses

    Array<Array<Clause*> >& cnfClausesForDomains = ef->cnfClausesForDomains;
    for (int d = 0; d < cnfClausesForDomains.size(); d++)
    {
      Array<Clause*>& cnfClauses = cnfClausesForDomains[d];    
      MLN* mln = (*mlns_)[d];
      for (int k = 0; k < cnfClauses.size(); k++)
      {
        Clause* c = cnfClauses[k];
          //if we adding counts permanently, then don't add counts that were added
          //previously; otherwise we can add the duplicate counts because they 
          //will be undone later.
        if (!undo && mln->containsClause(c)) continue;
        int* tmpClauseIdxInMLN = new int(mln->getNumClauses() + k);
        idxPtrs.append(tmpClauseIdxInMLN);
        if (iacArraysPerDomain)
        {
          Array<Array<IndexAndCount*> >& iacArrays = (*iacArraysPerDomain)[d];
          assert(iacArrays.size() == cnfClauses.size());
          Array<IndexAndCount*>& iacArray = iacArrays[k];

          Array<UndoInfo*> tmpUndoInfos;
          pllCountsForExistFormula(c, d, tmpClauseIdxInMLN, &tmpUndoInfos);
          for (int i = 0; i < tmpUndoInfos.size(); i++)
            iacArray.append(tmpUndoInfos[i]->affectedArr->lastItem());
          if (undoInfos) undoInfos->append(tmpUndoInfos);
          else           tmpUndoInfos.deleteItemsAndClear();
        }
        else
          pllCountsForExistFormula(c, d, tmpClauseIdxInMLN, undoInfos);
      }
    }

      //find optimal weights and gain of formula 

    if (evalGainLearnWts)
    {
      Array<double> priorMeans, priorStdDevs;    
          //either add one existentially quantified formula (when clauses do not
          //line up perfectly across databases) or add all CNF clauses (when
          //they do)
      int numAdded = (indexTrans_) ? 1 : cnfClausesForDomains[0].size();
      setPriorMeansStdDevs(priorMeans, priorStdDevs, numAdded, NULL);

      if (hasPrior_) 
        pll_->setMeansStdDevs(priorMeans.size(), priorMeans.getItems(), 
                              priorStdDevs.getItems());
      else           
        pll_->setMeansStdDevs(-1, NULL, NULL);

      if (indexTrans_)
      {
        for (int d = 0; d < cnfClausesForDomains.size(); d++)
        {
          int numCNFClauses = cnfClausesForDomains[d].size();
          indexTrans_->appendClauseIdxToClauseFormulaIdxs(1, numCNFClauses);
        }
      }
      
      int iter; bool error; double elapsedSec; 
      int numClausesFormulas = getNumClausesFormulas();
      Array<double>* wts = &(ef->wts);
      wts->growToSize(numClausesFormulas + numAdded + 1);

      double newScore = maximizeScore(numClausesFormulas, numAdded, wts, 
                                      NULL, NULL, iter, error, elapsedSec);

      if (indexTrans_)
      {
        for (int d = 0; d < cnfClausesForDomains.size(); d++)
        {
          int numCNFClauses = cnfClausesForDomains[d].size();
          indexTrans_->removeClauseIdxToClauseFormulaIdxs(1, numCNFClauses);
        }
      }

      double totalWt = 0;
      for (int i = 0; i < numAdded; i++)
        totalWt += (*wts)[numClausesFormulas+i+1];

      double penalty = ef->numPreds * penalty_;
      
      ef->gain = newScore-prevScore-penalty;
      ef->wt = totalWt;
      ef->newScore = newScore;

      if (error) {newScore=prevScore;cout<<"LBFGSB failed to find wts"<<endl;}
      printNewScore(ef->formula, iter, elapsedSec, newScore, 
                    newScore-prevScore, penalty, totalWt);
    }
    
    if (undo) { pll_->undoAppendRemoveCounts(undoInfos); delete undoInfos; }
    idxPtrs.deleteItemsAndClear();
  }//evaluateExistFormula()



  double evaluateExistFormulas(Array<ExistFormula*>& existFormulas, 
                               Array<ExistFormula*>& highGainWtFormulas,
                               const double& prevScore)
  {
    if (existFormulas.size() == 0) return 0;
    for (int i = 0; i < existFormulas.size(); i++)
      evaluateExistFormula(existFormulas[i], false, NULL, prevScore);
    
    Array<ExistFormula*> tmp(existFormulas);
    rquicksort(tmp);
    double minGain = DBL_MAX;
    cout << "evaluated existential formulas " << endl;
    cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;  
    for (int i = 0; i < tmp.size(); i++)
    {
      existFormulas[i] = tmp[i];
      double gain = existFormulas[i]->gain;
      double wt = existFormulas[i]->wt;
      cout << i << "\t" << existFormulas[i]->formula << endl
           << "\tgain = " << gain << ",  wt = " << wt << ",  op = OP_ADD" 
           << endl;
      if (gain > minGain_ && wt >= minWt_) 
      {
        highGainWtFormulas.append(tmp[i]);
        if (gain < minGain)  minGain = gain;
      }
    }
    cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl << endl;

    if (minGain == DBL_MAX) minGain = 0;
    return minGain;
  } 


  inline void appendExistFormulaToMLNs(ExistFormula* const & ef)
  {
    Array<Array<Array<IndexAndCount*> > > iacsPerDomain;
    Array<Array<Clause*> >& cnfClausesForDomains = ef->cnfClausesForDomains;
    iacsPerDomain.growToSize(cnfClausesForDomains.size());
    for (int d = 0; d < cnfClausesForDomains.size(); d++)
    {
      Array<Array<IndexAndCount*> >& iacArrays = iacsPerDomain[d];
      iacArrays.growToSize( cnfClausesForDomains[d].size() );
    }
    evaluateExistFormula(ef, true, &iacsPerDomain, 0);

    Array<double>& wts = ef->wts;
    int numClausesFormulas = getNumClausesFormulas();

      //update weights before CNF clauses are added as duplicate clause wts are
      //accumulated as they are added.
    if (indexTrans_ == NULL) updateWts(wts, NULL, NULL);

      //append CNF clauses to MLN
    for (int d = 0; d < cnfClausesForDomains.size(); d++)
    {
      MLN* mln = (*mlns_)[d];
      Array<Array<IndexAndCount*> >& iacArrays = iacsPerDomain[d];
      Array<Clause*>& cnfClauses = cnfClausesForDomains[d];

      for (int i = 0; i < cnfClauses.size(); i++)
      {
        int idx;
          //when we are learning the weight of a formula (i.e. indexsTrans !=NULL)
          //its CNF clause weight don't matter, so set them to 0.
        double wt = (indexTrans_) ?  0 : wts[numClausesFormulas+i+1];
          //if cnfClauses[i] is already in MLN, its weights will be correctly set
          //in updateWts() later
        mln->appendClause(ef->formula, true, new Clause(*cnfClauses[i]),
                          wt, false, idx);
        mln->setFormulaPriorMean(ef->formula, priorMean_);
        ((MLNClauseInfo*)mln->getMLNClauseInfo(idx))->priorMean 
          += priorMean_/cnfClauses.size();

        int* idxPtr = mln->getMLNClauseInfoIndexPtr(idx);
        Array<IndexAndCount*>& iacs = iacArrays[i]; 
        for (int j = 0; j < iacs.size(); j++)  iacs[j]->index = idxPtr;
      }
    }
    assert(pll_->checkNoRepeatedIndex());


    if (indexTrans_)
    {
         //update weights after the formula has been added to the MLN
      Array<string> appendedFormula; 
      appendedFormula.append(ef->formula);
      updateWts(wts, NULL, &appendedFormula);
        //MLNs has changed, the index translation must be recreated
      indexTrans_->createClauseIdxToClauseFormulaIdxsMap();
    }

    cout << "Modified MLN: Appended formula to MLN: " << ef->formula << endl;
  }


  inline bool effectExistFormulaOnMLNs(ExistFormula* ef, 
                                       Array<ExistFormula*>& existFormulas, 
                                       double& score)
  {
    cout << "effecting existentially quantified formula " << ef->formula 
         << " on MLN..." << endl;
    appendExistFormulaToMLNs(ef);
    score = ef->newScore;
    printMLNClausesWithWeightsAndScore(score, iter_); 
    printMLNToFile(NULL, iter_);

    int r = existFormulas.find(ef);
    assert(r >= 0);
    ExistFormula* rf = existFormulas.removeItemFastDisorder(r);
    assert(rf == ef);
    delete rf;
    return true;
  }


  //FLAG - exist - jesse
  bool effectBestCandidateOnMLNs(Array<Clause*>& bestCandidates, 
                                 Array<ExistFormula*>& existFormulas,
                                 Array<ExistFormula*>& highGainWtFormulas,
                                 double& score)
  {
    cout << "effecting best candidate among existential formulas and "
         << "best candidates on MLN..." << endl << endl;

    int a = 0, b = 0;
    bool ok = false;
    int numCands = bestCandidates.size() + highGainWtFormulas.size();
    for (int i = 0; i < numCands; i++)
    {
      if (a >= bestCandidates.size())
      {
        if ((ok=effectExistFormulaOnMLNs(highGainWtFormulas[b++],
					 existFormulas, score))) break;
      }
      else
      if (b >= highGainWtFormulas.size())
      {
        cout << "effecting best candidate " << a << " on MLN..." << endl;
        if ((ok=effectBestCandidateOnMLNs(bestCandidates[a++], score, NULL))) break;
        cout << "failed to effect candidate on MLN." << endl;
        delete bestCandidates[a-1];
      }
      else
      if (highGainWtFormulas[b]->gain > 
          bestCandidates[a]->getAuxClauseData()->gain)
      {
        if ((ok=effectExistFormulaOnMLNs(highGainWtFormulas[b++], 
					 existFormulas, score))) break;
      }
      else
      {
        cout << "effecting best candidate " << a << " on MLN..." << endl;
        if ((ok=effectBestCandidateOnMLNs(bestCandidates[a++], score, NULL))) break;
        cout << "failed to effect candidate on MLN." << endl;
        delete bestCandidates[a-1];
      }
    }
  
    for (int i = a; i < bestCandidates.size(); i++) delete bestCandidates[i];
    return ok;
  }

 /*
  void deleteExistFormulas(Array<ExistFormula*>& existFormulas);

  void getMLNClauses(Array<Clause*>& initialMLNClauses,
                     Array<ExistFormula*>& existFormulas);

    //cnfClauses enter & exit function with its AuxClauseData's cache == NULL
  inline void pllCountsForExistFormula(Clause* cnfClause, const int& domainIdx,
                                       int* clauseIdxInMLN,
                                       Array<UndoInfo*>* const & undoInfos);

  inline void printNewScore(const string existFormStr, const int& lbfgsbIter, 
                            const double& lbfgsbSec, const double& newScore, 
                            const double& gain, const double& penalty, 
                            const double& wt);

  inline void rquicksort(Array<ExistFormula*>& existFormulas, const int& l, 
                         const int& r);

  inline void rquicksort(Array<ExistFormula*>& ef);
 
  inline void evaluateExistFormula(ExistFormula* const& ef,
                                   const bool& computeCountsOnly,
              Array<Array<Array<IndexAndCount*> > >* const & iacArraysPerDomain,
                                   const double& prevScore);

  double evaluateExistFormulas(Array<ExistFormula*>& existFormulas, 
                               Array<ExistFormula*>& highGainWtFormulas,
                               const double& prevScore);

  inline void appendExistFormulaToMLNs(ExistFormula* const & ef);


  inline bool effectExistFormulaOnMLNs(ExistFormula* ef, 
                                       Array<ExistFormula*>& existFormulas, 
                                       double& score);

  bool effectBestCandidateOnMLNs(Array<Clause*>& bestCandidates, 
                                 Array<ExistFormula*>& existFormulas,
                                 Array<ExistFormula*>& highGainWtFormulas,
                                 double& score);

  void updateWeightsSGD(int step);
*/
  /////////////////////////////////////////////////////////////////////////
 private:
    // mln0_ and mlns_ are not owned by StructLearn; do not delete;
    // if there are more than one domains, mln0_ corresponds to domains_[0]
  MLN* mln0_; 
  Array<MLN*>* mlns_;
  bool startFromEmptyMLN_;
  string outMLNFileName_;
  Array<Clause*> addedClauses;
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

  int beamSize_;
  int bestGainUnchangedLimit_;
  int numEstBestClauses_;
  double minGain_;
  double minWt_;
  double penalty_;

  bool sampleGndPreds_;
  double fraction_;
  int minGndPredSamples_;
  int maxGndPredSamples_;

  bool reEvalBestCandsWithTightParams_;
  bool structVerbose;

  Timer timer_;
  int candCnt_;

  int iter_;
  int bsiter_;
  double startSec_;
  //double maxRunTime_ = 0;

    //Used to map clause indexes in MLNs to the weights indexes that are 
    //presented to LBFGSB. This is required if the clauses do not line up
    //across databases (as when an existentially quantified formula has a 
    //different number of formulas in its CNF for different databases).
  IndexTranslator* indexTrans_;

  bool structGradDescent_;
  bool withEM_;
  bool partialPrint_;
  bool timeBased_;
  double maxRunTime_;
  int plusSearch;
  int hrs_;
  int numClauses_;
  double minPllGain_;
  bool jdebug;// = false;
};


#endif
