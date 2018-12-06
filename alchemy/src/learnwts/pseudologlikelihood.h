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
#ifndef LOGPSEUDOLIKELIHOOD_H_AUG_18_2005
#define LOGPSEUDOLIKELIHOOD_H_AUG_18_2005

#include <fstream>
#include <iostream>
#include <cmath>
#include "array.h"
#include "random.h"
#include "domain.h"
#include "clause.h"
#include "mln.h"
#include "indextranslator.h"

//NOTE: "domain index" and "database index" are used interchangeably

  // expl and logl aren't available in cygwin / windows, so use exp and log
#ifndef expl
# define expl exp
# define logl log
#endif

////////////////////////// helper data structures //////////////////////////////

  //LogPseudolikelihood is used in weight learning which makes the closed-world
  //assumption, so there is no ground predicate whose truth value is UNKNOWN
const bool DB_HAS_UNKNOWN_PREDS = false;

struct IndexAndCount 
{
  IndexAndCount() : index(NULL), count(0) {}
  IndexAndCount(int* const & i, const long double& c) : index(i), count(c) {}
  int* index;
  //Flag - long double??
  long double count;
};

  // used to info required to undo the addition/removal of IndexAndCount
struct UndoInfo
{
  UndoInfo(Array<IndexAndCount*>* const & affArr, 
           IndexAndCount* const & iac, const int& remIdx, const int& domIdx)
    : affectedArr(affArr), remIac(iac), remIacIdx(remIdx), domainIdx(domIdx) {}
  ~UndoInfo() {if (remIac) delete remIac; }
  Array<IndexAndCount*>* affectedArr;
  IndexAndCount* remIac; //appended or removed IndexAndCount
  int remIacIdx; //index of remIac before it was removed from affectedArr
  int domainIdx;
};


struct SampledGndings
{
  IntHashArray trueGndings;
  IntHashArray falseGndings;
  int totalTrue;
  int totalFalse;
};


/////////////////////////////////////////////////////////////////////////////

  //This class returns the negative of the weighted pseudo-log-likelihood.
class PseudoLogLikelihood
{
 public:
  PseudoLogLikelihood(const Array<bool>* const & areNonEvidPreds,
                      const Array<Domain*>* const & domains,
                      const bool& wtFOPred, const bool& sampleGndPreds, 
                      const double& fraction, const int& minGndPredSamples, 
                      const int& maxGndPredSamples) 
    : domains_(new Array<Domain*>) , numMeans_(-1), 
      priorMeans_(NULL), priorStdDevs_(NULL), wtFOPred_(wtFOPred),
    sampleGndPreds_(sampleGndPreds), idxTrans_(NULL), jdebug(false)
  {

    if (areNonEvidPreds)
    {
      areNonEvidPreds_ = new Array<bool>(*areNonEvidPreds);
      assert(areNonEvidPreds_->size() == (*domains)[0]->getNumPredicates());
    }
    else
    {
      areNonEvidPreds_ = new Array<bool>;
      areNonEvidPreds_->growToSize((*domains)[0]->getNumPredicates(), true);
    }

    int numDomains = domains->size();
    assert(numDomains > 0);
    domains_->growToSize(numDomains,NULL);
    for (int i = 0; i < numDomains; i++)
      (*domains_)[i] = (*domains)[i];

    gndPredClauseIndexesAndCountsArr_ 
      = new Array<Array<Array<Array<Array<IndexAndCount*>*>*>*>*>;
    gndPredClauseIndexesAndCountsArr_->growToSize(numDomains,NULL);
    for (int i = 0; i < numDomains; i++)
    {        
      (*gndPredClauseIndexesAndCountsArr_)[i] 
        = new Array<Array<Array<Array<IndexAndCount*>*>*>*>;
      (*gndPredClauseIndexesAndCountsArr_)[i]->growToSize(
                                       (*domains_)[i]->getNumPredicates(),NULL);
    }

    createNumGndings();

    if (sampleGndPreds_)
    {
      sampledGndingsMaps_ = new Array<Array<SampledGndings*>*>;
      sampledGndingsMaps_->growToSize(numDomains, NULL);
      for (int i = 0; i < numDomains; i++)
      {
        (*sampledGndingsMaps_)[i] = new Array<SampledGndings*>;
        (*sampledGndingsMaps_)[i]->growToSize((*domains_)[i]->getNumPredicates()
                                              , NULL);
      }
      random_ = new Random;
      random_->init(-3);
      samplePredGroundings(fraction, minGndPredSamples,  maxGndPredSamples);
    }
    else
    {
      sampledGndingsMaps_ = NULL;      
      random_ = NULL;
    }
  }

  
  ~PseudoLogLikelihood()
  {    
    delete areNonEvidPreds_;
    delete domains_;

    for (int i = 0; i < gndPredClauseIndexesAndCountsArr_->size(); i++)
    {
      Array<Array<Array<Array<IndexAndCount*>*>*>*>*
      gndPredClauseIndexesAndCounts
        = (*gndPredClauseIndexesAndCountsArr_)[i];

      int numPreds = gndPredClauseIndexesAndCounts->size();
      for (int p = 0; p < numPreds; p++) // for each predicate
      {
        if ( (*gndPredClauseIndexesAndCounts)[p] )
        {
          Array<Array<Array<IndexAndCount*>*>*>* gndingsToClauseIndexesAndCounts
            = (*gndPredClauseIndexesAndCounts)[p];
          int numGnds = gndingsToClauseIndexesAndCounts->size();
          for (int g = 0; g < numGnds; g++) // for each grounding
          {
            for (int h = 0; h < (*gndingsToClauseIndexesAndCounts)[g]->size(); 
                 h++)
            {
              for (int j = 0;
                   j < (*(*gndingsToClauseIndexesAndCounts)[g])[h]->size();
                   j++)
              {
                delete (*(*(*gndingsToClauseIndexesAndCounts)[g])[h])[j];
              }
              delete (*(*gndingsToClauseIndexesAndCounts)[g])[h];
            }
            delete (*gndingsToClauseIndexesAndCounts)[g];
          }
          delete gndingsToClauseIndexesAndCounts;
        }
      }
      delete gndPredClauseIndexesAndCounts;      
    }
    delete gndPredClauseIndexesAndCountsArr_;

    numGndings_->deleteItemsAndClear();      
    delete numGndings_;

    if (sampledGndingsMaps_)
    {
      for (int i = 0; i < sampledGndingsMaps_->size(); i++)
      {
        (*sampledGndingsMaps_)[i]->deleteItemsAndClear();
        delete (*sampledGndingsMaps_)[i];
      }
      delete sampledGndingsMaps_;
    }

    if (random_) delete random_;
  }


  void compress()
  {
    for (int i = 0; i < gndPredClauseIndexesAndCountsArr_->size(); i++)
    {
      Array<Array<Array<Array<IndexAndCount*>*>*>*>*
      gndPredClauseIndexesAndCounts 
        = (*gndPredClauseIndexesAndCountsArr_)[i];
      
      int numPreds = gndPredClauseIndexesAndCounts->size();
      for (int p = 0; p < numPreds; p++) // for each predicate
      {

        if ((*gndPredClauseIndexesAndCounts)[p])
        {
          Array<Array<Array<IndexAndCount*>*>*>* gndingsToClauseIndexesAndCounts
            = (*gndPredClauseIndexesAndCounts)[p];
          int numGnds = gndingsToClauseIndexesAndCounts->size();
          for (int g = 0; g < numGnds; g++) // for each grounding
          {
            Array<Array<IndexAndCount*>*>* combosToClauseIndexesAndCounts
              = (*gndingsToClauseIndexesAndCounts)[g];
            int numCombos = combosToClauseIndexesAndCounts->size();
            for (int c = 0; c < numCombos; c++) // for each combo
              (*combosToClauseIndexesAndCounts)[c]->compress();
          }
        }
      }
    }

    //numGndings_ not compress because set to exact size in constructor
  }

  
    //similar to computeAndSetCounts()
  void insertCounts(int* const & clauseIdxInMLN,
                    Array<UndoInfo*>* const & undoInfos,
                    Array<Array<Array<CacheCount*>*>*>* const & cache,
                    const int& d)
  {
    Array<Array<Array<Array<IndexAndCount*>*>*>*>*
    gndPredClauseIndexesAndCounts;

    Array<IndexAndCount*>* gArr;
    CacheCount* cc;
    assert(cache->size() == domains_->size());

    gndPredClauseIndexesAndCounts = (*gndPredClauseIndexesAndCountsArr_)[d];
    for (int p = 0; p < (*cache)[d]->size(); p++)
    {
      Array<CacheCount*>* ccArr = (*(*cache)[d])[p];
      if (ccArr == NULL) continue;
      for (int i = 0; i < ccArr->size(); i++)
      {
        assert((*gndPredClauseIndexesAndCounts)[p] != NULL);
        cc = (*ccArr)[i];
        gArr = (*(*(*gndPredClauseIndexesAndCounts)[p])[cc->g])[cc->c];
          //gth grounding of clause's pred should not have been looked at
        assert(gArr->size()==0 ||*(gArr->lastItem()->index)!=*clauseIdxInMLN);
        assert(cc->cnt != 0);
        gArr->append(new IndexAndCount(clauseIdxInMLN, cc->cnt));
        if (undoInfos) undoInfos->append(new UndoInfo(gArr, NULL, -1, d));
      }
    }
  } 


    //similar to computeAndSetCounts()
  void insertCounts(const Array<int*>& clauseIdxInMLNs,
                    Array<UndoInfo*>* const & undoInfos,
                    Array<Array<Array<CacheCount*>*>*>* const & cache)
  {
    assert(cache->size() == domains_->size());
    assert(clauseIdxInMLNs.size() == domains_->size());

    for (int d = 0; d < cache->size(); d++) 
      insertCounts(clauseIdxInMLNs[d], undoInfos, cache, d);
  }


    //If undoInfos is not NULL, it is used to store pointers to 
    //Array<IndexAndCount*> that have new entries appended so that the new
    //entries can be easily removed later.
  void computeCountsForNewAppendedClause(const Clause* const & c,
                                         int* const & clauseIdxInMLN,
                                         const int& domainIdx,
                                         Array<UndoInfo*>* const & undoInfos,
                                         const bool& sampleClauses,
                              Array<Array<Array<CacheCount*>*>*>* const & cache)
  {
    computeCountsRemoveCountsHelper(true, c, clauseIdxInMLN, domainIdx, 
                                    undoInfos, sampleClauses, cache);
  }


  void removeCountsForClause(const Clause* const & c, 
                             int* const & clauseIdxInMLN, const int& domainIdx,
                             Array<UndoInfo*>* const & undoInfos)
  {
    computeCountsRemoveCountsHelper(false, c, clauseIdxInMLN, domainIdx, 
                                    undoInfos, false, NULL);
  }


    //the contents of undoInfos will be deleted
  void undoAppendRemoveCounts(const Array<UndoInfo*>* const & undoInfos)
  {
    for (int i = undoInfos->size() - 1; i >= 0; i--)
    {
      if ((*undoInfos)[i]->remIacIdx >= 0) // if this was a removal
      {
        Array<IndexAndCount*>* affectedArr = (*undoInfos)[i]->affectedArr;
        IndexAndCount* remIac = (*undoInfos)[i]->remIac;
        (*undoInfos)[i]->remIac = NULL; //so that it won't get deleted later
        int remIacIdx = (*undoInfos)[i]->remIacIdx;

        if (affectedArr->size() == remIacIdx) //if removed item was the last one
          affectedArr->append(remIac);
        else
        {
          assert(remIacIdx < affectedArr->size());
          IndexAndCount* tmpRemIac = (*affectedArr)[remIacIdx];
          (*affectedArr)[remIacIdx] = remIac;
          affectedArr->append(tmpRemIac);
        }
      }
      else
      {    // this was an addition
        IndexAndCount* iac = (*undoInfos)[i]->affectedArr->removeLastItem();
	/*
	  cerr << "iac (add) " << iac << " " << iac->index << " " 
	  << (*iac->index) << endl;
	*/
        delete iac;
      }

      assert(noRepeatedIndex((*undoInfos)[i]->affectedArr));
      delete (*undoInfos)[i];
    }
  }


  long double getValueAndGradient(double* const & gradient, const double* const & wt,
                             const int& arrSize)
  {
    long double wpll = 0;
    memset(gradient, 0, arrSize*sizeof(double));
    /*
    for(int ix = 0; ix < arrSize; ix++){
      cout << "wt " << ix << " " << wt[ix] << endl;
    }
    */
        //if there is one database, or the clauses for all databases line up
    if (idxTrans_ == NULL)
    {
      for (int i = 0; i < domains_->size(); i++)
        wpll += getValueAndGradientForDomain(gradient, wt, arrSize, i);
    }
    else
    {   //the clauses for multiple databases do not line up
      Array<Array<double> >* wtsPerDomain = idxTrans_->getWtsPerDomain();
      Array<Array<double> >* gradsPerDomain = idxTrans_->getGradsPerDomain();
      const Array<Array<Array<IdxDiv>*> >* cIdxToCFIdxsPerDomain 
        = idxTrans_->getClauseIdxToClauseFormulaIdxsPerDomain();
      
      for (int i = 0; i < domains_->size(); i++)
      {
        Array<double>& wts = (*wtsPerDomain)[i];
	for(int ix = 0; ix < wts.size(); ix++){
	  // cout << "weight " << ix << " " << wts[ix] << endl;
	}
        Array<double>& grads = (*gradsPerDomain)[i];
        assert(grads.size() == wts.size()); //size is num of clauses in domain i
        memset((double*)wts.getItems(), 0, wts.size()*sizeof(double));
        memset((double*)grads.getItems(), 0, grads.size()*sizeof(double));

        //NOTE: wts, grads and *cIdxToCFIdxsPerDomain)[i] may be larger than
        //      the actual number of clauses in domain i. This occurs in order
        //      to avoid the cost of resizing the the arrays, and is why we need
        //      the two "if ((*idxDivs)[k].idx < arrSize)" checks below.

          //map clause/formula weights to clause weights
        for (int j = 0; j < wts.size(); j++)
        {
          Array<IdxDiv>* idxDivs =(*cIdxToCFIdxsPerDomain)[i][j];          
          for (int k = 0; k < idxDivs->size(); k++)
            if ((*idxDivs)[k].idx < arrSize)
              wts[j] += wt[ (*idxDivs)[k].idx ] / (*idxDivs)[k].div;
        }
                
        wpll += getValueAndGradientForDomain((double*)grads.getItems(), 
                                            (double*)wts.getItems(), 
                                            wts.size(), i);
        
          // map clause gradient to clause/formula gradients
        for (int j = 0; j < grads.size(); j++)
        {
          Array<IdxDiv>* idxDivs =(*cIdxToCFIdxsPerDomain)[i][j];          
          for (int k = 0; k < idxDivs->size(); k++)
            if ((*idxDivs)[k].idx < arrSize)            
              gradient[ (*idxDivs)[k].idx ] += grads[j] / (*idxDivs)[k].div;
        }
      } // for each domain
    }

    //printWtsGradsWPLL(wt, gradient, arrSize, wpll); //for testing

      // if there are prior penalties
    if (numMeans_ > 0)
    {
      //commented out: numMeans_ can be larger than arrSize,so just consider
      //               the first arrSize items in priorMeans & priorStdDevs
      //assert(numMeans_ == arrSize);
        
        // subtract the gaussian priors
      for (int i = 0; i < arrSize; i++)
      {
        //since at this point the value and gradient have been negated,
        //add the priors
        wpll += (wt[i]-priorMeans_[i])*(wt[i]-priorMeans_[i])/
          (2*priorStdDevs_[i]*priorStdDevs_[i]);         
        
        gradient[i] += (wt[i]-priorMeans_[i])/
          (priorStdDevs_[i]*priorStdDevs_[i]);
      }
    }
    
    //printWtsGradsWPLL(wt, gradient, arrSize, wpll); //for testing
    
    return wpll;
  } 



    //set numMeans to -1 if there is no prior
  void setMeansStdDevs(const int& arrSize, const double* const & priorMeans, 
                       const double* const & priorStdDevs)
  {
    numMeans_ = arrSize;
    priorMeans_ = priorMeans;
    priorStdDevs_ = priorStdDevs;
  }


  void setSampleGndPreds(const bool& sgp) 
  { 
    if (sgp) { assert(sampledGndingsMaps_); assert(random_); }
    sampleGndPreds_ = sgp; 
  }


  bool checkNoRepeatedIndex(const MLN* const & mln=NULL)
  {
    bool ret = true;
    for (int d = 0; d < domains_->size(); d++)
    {
      Array<Array<Array<Array<IndexAndCount*>*>*>*>*
      gndPredClauseIndexesAndCounts
        = (*gndPredClauseIndexesAndCountsArr_)[d];

      int numPreds = gndPredClauseIndexesAndCounts->size();
      for (int p = 0; p < numPreds; p++) // for each predicate
      {
        Array<Array<Array<IndexAndCount*>*>*>* gndingsToClauseIndexesAndCounts 
          = (*gndPredClauseIndexesAndCounts)[p];
        
        if (gndingsToClauseIndexesAndCounts == NULL) continue;
        
        int numGnds = gndingsToClauseIndexesAndCounts->size();
        for (int g = 0; g < numGnds; g++) // for each grounding
        {
          Array<Array<IndexAndCount*>*>* gndings 
            = (*gndingsToClauseIndexesAndCounts)[g];

          for (int c = 0; c < gndings->size(); c++)
          {
            bool ok = noRepeatedIndex((*gndings)[c], mln);
            if (!ok) 
            {
              cout << "ERROR: repeated index in domain " << d << " for pred " 
                   << (*domains_)[0]->getPredicateName(p) << " gnding " << g 
                   << " combination " << c << endl;
              ret = false;
            }
          }
        }
      } // for each predicate
    } // for each domain
    return ret;
  }


  void printGndPredClauseIndexesAndCounts(const MLN* const & mln=NULL)
  {
    for (int d = 0; d < domains_->size(); d++)
    {
      cout << "domainIdx: " << d << endl;
      cout << "gndPredClauseIndexesAndCounts[predIdx][gndingIdx][combIdx][i]"
           << endl;

      Array<Array<Array<Array<IndexAndCount*>*>*>*>*
      gndPredClauseIndexesAndCounts
        = (*gndPredClauseIndexesAndCountsArr_)[d];

      int numPreds = gndPredClauseIndexesAndCounts->size();
      for (int p = 0; p < numPreds; p++) // for each predicate
      {
        Array<Array<Array<IndexAndCount*>*>*>* gndingsToClauseIndexesAndCounts 
          = (*gndPredClauseIndexesAndCounts)[p];
        
        if (gndingsToClauseIndexesAndCounts == NULL) 
        {
          cout << "gndPredClauseIndexesAndCounts[" << p << "] = NULL" << endl;
          continue;
        }
        
        int numGnds = gndingsToClauseIndexesAndCounts->size();
        for (int g = 0; g < numGnds; g++) // for each grounding
        {
          Array<Array<IndexAndCount*>*>* gndings 
            = (*gndingsToClauseIndexesAndCounts)[g];

          for (int c = 0; c < gndings->size(); c++)
          {
            Array<IndexAndCount*>* combos 
              = (*gndings)[c];
            int numClauseIdx = combos->size();
          
            if (numClauseIdx == 0)
            {
              cout << "gndPredClauseIndexesAndCounts[" << p << "][" << g 
                   << "][" << c << "] = empty" << endl;
              continue;
            }
          
            for (int i = 0; i < numClauseIdx; i++)
            {
              cout << "gndPredClauseIndexesAndCounts[" << p << "][" << g << "]["
                   << c << "][" << i << "] (clauseIndex,count) = "
                   << *((*combos)[i]->index)
                   << ", " << (*combos)[i]->count 
                   << ",  " << (*combos)[i] << endl;
            
              if (mln)
              {
                cout << "                                      \t";
                mln->getClause(*((*combos)[i]->index)) ->print(cout, 
                                                               (*domains_)[0]);
                cout << endl;
              }
            }
          }

        }
      } // for each predicate
    } // for each domain
  }

  
  void setIndexTranslator(IndexTranslator* const & it) { idxTrans_ = it; }


  IndexTranslator* getIndexTranslator() const { return idxTrans_; }


 private:
  void createNumGndings()
  {
    numGndings_ = new Array<Array<long double>*>; 
    numGndings_->growToSize(domains_->size());
    for (int i = 0; i < domains_->size(); i++)
    {
      (*numGndings_)[i] = new Array<long double>;
      (*numGndings_)[i]->growToSize((*domains_)[i]->getNumPredicates(), -1);
    }

    for (int i = 0; i < domains_->size(); i++)
    {
      const Domain* domain = (*domains_)[i];
      for (int j = 0; j < domain->getNumPredicates(); j++)
      {
        if (!(*areNonEvidPreds_)[j]) continue;
        const PredicateTemplate* pt = domain->getPredicateTemplate(j);
          //compute num groundings of pred
        long double numGndings = 1;
        for (int t = 0; t < pt->getNumTerms(); t++)
        {
          int typeId = pt->getTermTypeAsInt(t);
          numGndings *= domain->getNumConstantsByType(typeId);
        }
        (*((*numGndings_)[i]))[j] = numGndings;
	//cout << "num ground " << i << " " << j << " " << numGndings << endl;
      }
    }
  }


  void createAllPossiblePredsGroundings(const Predicate* const & pred,
                                        const Domain* const & domain,
                                        ArraysAccessor<int>& acc)
  {
    for (int i = 0; i < pred->getNumTerms(); i++)
    {
      int typeId = pred->getTermTypeAsInt(i);
      const Array<int>* constArr = domain->getConstantsByType(typeId);
      acc.appendArray(constArr);
    }
  }

  
  bool isSampledGndPred(const int& g, const SampledGndings* const & sg)
  {
    if (sg->falseGndings.contains(g)) return true;
    if (sg->trueGndings.contains(g)) return true;
    return false;
  }


  void computeCountsRemoveCountsHelper(bool computeCounts,
                                       const Clause* const & c,
                                       int* const & clauseIdxInMLN,
                                       const int& domainIdx,
                                       Array<UndoInfo*>* const & undoInfos,
                                       const bool& sampleClauses,
                              Array<Array<Array<CacheCount*>*>*>* const & cache)
  {
    const Domain* domain = (*domains_)[domainIdx];
    //cout << "before: c = ";
    //c->printWithWtAndStrVar(cout, domain);
    //cout << endl;
    Database* db = domain->getDB();

      //to store index of 1st pred in c that has terms which are all diff vars
    Array<int> predIdxWithAllTermsDiffVars(domain->getNumPredicates());
    predIdxWithAllTermsDiffVars.growToSize(domain->getNumPredicates());
    int* parr = (int*) predIdxWithAllTermsDiffVars.getItems();
    memset(parr, -1, domain->getNumPredicates()*sizeof(int));

      //find out which preds have terms that are all different variables
    Array<bool> predAllTermsAreDiffVars(c->getNumPredicates());
    createPredAllTermsAreDiffVars(c, predAllTermsAreDiffVars, 
                                  predIdxWithAllTermsDiffVars);

      //used to store canonicalized predicates (before they are grounded)
    PredicateHashArray seenPreds;

      //for each pred that clause contains
    for (int p = 0; p < c->getNumPredicates(); p++)
    {

      Predicate* pred = c->getPredicate(p);
      int predId = pred->getId();
      if (!(*areNonEvidPreds_)[predId]) continue;

      Predicate gndPred(*pred);
      gndPred.canonicalize();
      bool predIsInitiallyGnded = gndPred.isGrounded();

      SampledGndings* sg = NULL;
      if (sampleGndPreds_) sg = (*(*sampledGndingsMaps_)[domainIdx])[predId];

      Predicate* seenPred = new Predicate(gndPred);
        //if the predicate has been seen before
      if (seenPreds.append(seenPred) < 0) { delete seenPred; continue; }

      if (predAllTermsAreDiffVars[p])
      {
        //cout << "all terms DIFF vars, predIdx = " << p << endl;        
        
          //create all possible groundings of pred
        ArraysAccessor<int> acc;
        createAllPossiblePredsGroundings(&gndPred, domain, acc);
	Timer timer_;
	double bsec = timer_.time();
	if (jdebug){
	  cout << "Diff Vars ";
	  pred->print(cout, domain);
	  cout << " " << acc.numCombinations() << endl;
	}
          // for each grounding of pred
        int g = -1; //gth grounding
        while (acc.hasNextCombination())
        {
          ++g;
	  double itersec = timer_.time();
          int t = 0; int constId; 
          while (acc.nextItemInCombination(constId))
            ((Term*) gndPred.getTerm(t++))->setId(constId);

          if (sampleGndPreds_ && !isSampledGndPred(g,sg)) continue;

          if (computeCounts)
          {
            computeAndSetCounts(c, clauseIdxInMLN, predId, gndPred, g, db,
                                domainIdx, undoInfos, sampleClauses, cache);
          }
          else
            removeCounts(clauseIdxInMLN, predId, g, domainIdx, undoInfos);
	  
	  if (jdebug && g % 1000 == 0){
	    cout << "iter " << g << " took ";
	    timer_.printTime(cout, timer_.time()-itersec); cout << endl << endl;
	  }
        } //for each grounding of pred 
	if (jdebug){
	  cout << "took ";
	  timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
	}
      }
      else
      {  //there are constant terms or repeated variables

          //if there is a pred with this id that has terms that are all vars
        if (predIdxWithAllTermsDiffVars[predId] >= 0) continue;

        //cout << "all terms NOT diff vars, predIdx = " << p << endl;

          //create multipliers that are used to determine the index of a gnding
        Array<int> multipliers(gndPred.getNumTerms());
        createMultipliers(multipliers, gndPred, domain);

          //fix offset due to constants in gndPred that is used to determine
          //the index of a grounding  
        int offsetDueToConstants = 0; 

          //compute mapping of varId to array of multipliers, create all 
          //groundings of variables, and compute fix offset due to constants
          //pair.first is the multiplier, pair.second is the Term that the 
          //multiplier that the corresponds to
        Array<Array<pair<int,Term*> >* > varIdToMults;
        Array<int> negVarIdsArr;
        ArraysAccessor<int> groundings;
        createMappingOfVarIdToMultipliersAndVarGroundingsAndOffset(
          gndPred, domain, multipliers, offsetDueToConstants, varIdToMults, 
          negVarIdsArr, groundings);
        
          //if the predicate has some variables
        if (!predIsInitiallyGnded)
        {
            // ground gndPred
          int constId, constIdx;
	  Timer timer_;
	  double bsec = timer_.time();
	  if (jdebug){
	    cout << "Has some vars and consts Vars ";
	    pred->print(cout, domain);
	    cout << " " << groundings.numCombinations() << endl;
	  }

          while (groundings.hasNextCombination())
          {

            int g = offsetDueToConstants; //index of grounding
            int j = -1;
            while (groundings.nextItemInCombination(constId, constIdx))
            {
              ++j;
              int negVarId = negVarIdsArr[j];
              Array<pair<int,Term*> >* multsAndTerms = varIdToMults[negVarId];
              for (int m = 0; m < multsAndTerms->size(); m++)
              {
                g += constIdx * (*multsAndTerms)[m].first;
                (*multsAndTerms)[m].second->setId(constId); //ground gndPred
              }
            }

            if (sampleGndPreds_ && !isSampledGndPred(g,sg)) continue;

            if (computeCounts)
              computeAndSetCounts(c, clauseIdxInMLN, predId, gndPred, g, db, 
                                  domainIdx, undoInfos, sampleClauses, cache);
            else
              removeCounts(clauseIdxInMLN, predId, g, domainIdx, undoInfos);

          }
	  if (jdebug){
	    cout << "took ";
	    timer_.printTime(cout, timer_.time()-bsec); cout << endl << endl;
	    
	  }
        }
        else
        {  // the predicate is initially grounded
          int g = offsetDueToConstants;

          bool ok = true;
          if (sampleGndPreds_) ok = isSampledGndPred(g,sg);

          if (ok)
          {
            if (computeCounts)
              computeAndSetCounts(c, clauseIdxInMLN, predId, gndPred, g, db, 
                                  domainIdx, undoInfos, sampleClauses, cache);
            else
              removeCounts(clauseIdxInMLN, predId, g, domainIdx, undoInfos);
          }
        }

        for (int j = 0; j < varIdToMults.size(); j++) delete varIdToMults[j];
      } //there are constant terms or repeated variables
    } //for each pred that clause contains

    for (int i = 0; i < seenPreds.size(); i++)  delete seenPreds[i];

    //cout << "after: c = ";
    //c->printWithWtAndStrVar(cout, domain);
    //cout << endl;
  } //computeCountsRemoveCountsHelper()

// Commented out: This way of computing pll causes overflows when exponentiating
// a few thousand counts
/*    
  void computePerPredPllAndGrad(const Array<Array<Array<IndexAndCount*>*>*>*
                                const& gndingsToClauseIndexesAndCounts,
                                const int& g, const double* const & wt, 
                                long double& perPredPll, 
                                long double * const & perPredGrad)
  {
      // compute W.(N^bar-N)

	  // Changed to logl, expl because otherwise changing
	  // order of .mln files changes weight results
    long double wdotn = 0;
      // pmb = 1 + sum(exp(wdotn))
    long double pmb = 1;
    
    Array<Array<IndexAndCount*>*>* gndings =
      (*gndingsToClauseIndexesAndCounts)[g];

      // For each valid assignment of vars in block
    for (int c = 0; c < gndings->size(); c++)
    {
      Array<IndexAndCount*>* clauseIndexesAndCounts = (*gndings)[c];
      assert(noRepeatedIndex(clauseIndexesAndCounts));
    
      int numClausesUnifyWith = clauseIndexesAndCounts->size();

      for (int i = 0; i < numClausesUnifyWith; i++)
      {
          // grounding g unifies with clause (*clauseIndexesAndCounts)[i].idx
        wdotn += wt[ *( (*clauseIndexesAndCounts)[i]->index ) ] * 
          (*clauseIndexesAndCounts)[i]->count;
      }
      
      pmb += expl(wdotn);
    }

    perPredPll -= logl(pmb);
      // Is this still right with blocking?
      // update gradient
    for (int c = 0; c < gndings->size(); c++)
    {
      Array<IndexAndCount*>* clauseIndexesAndCounts 
        = (*gndings)[c];
      
      for (int i = 0; i < clauseIndexesAndCounts->size(); i++)
      {
        perPredGrad[ *( (*clauseIndexesAndCounts)[i]->index ) ]
          += ( (1.0/pmb-1) * (*clauseIndexesAndCounts)[i]->count );
      }
    }
  }
*/
  //Jesse
 public:
  void getProbs(const double* const & wt,
		const int& arrSize, const int& domainIdx, ofstream& out)
  {
    //long double wpll = 0; // weighted pseudo-log-likelihood


    //used if sampling ground predicates
    //long double tmpPerPredPll;
    //long double* tmpPerPredGrad = NULL;

    
    Array<Array<Array<Array<IndexAndCount*>*>*>*>* gndPredClauseIndexesAndCounts
      = (*gndPredClauseIndexesAndCountsArr_)[domainIdx];

    int numPreds = gndPredClauseIndexesAndCounts->size();
    for (int p = 0; p < numPreds; p++) // for each predicate
    {
      if (!(*areNonEvidPreds_)[p]) continue; 

      //commented out: even though pred does not appear in any clause, each
      //of its groundings contribute -ln(2) to the wpll
      //if ((*gndPredClauseIndexesAndCounts)[p] == NULL) continue;

      //long double perPredPll = 0;
      //memset(perPredGrad, 0, arrSize*sizeof(long double));

        //if pred p appears in one or more clauses
      if ((*gndPredClauseIndexesAndCounts)[p] != NULL)
      {
        Array<Array<Array<IndexAndCount*>*>*>* gndingsToClauseIndexesAndCounts 
          = (*gndPredClauseIndexesAndCounts)[p];

	//use all groundings of predicate
	int numGnds = gndingsToClauseIndexesAndCounts->size();
	assert(numGnds == (*((*numGndings_)[domainIdx]))[p]);
	Array<Predicate*> allGroundPred;
	Predicate::createAllGroundings(p, (*domains_)[0], allGroundPred);
	// for each grounding
	for (int g = 0; g < numGnds; g++){ 

	  Predicate* underEval = (Predicate*)allGroundPred[g];
	  underEval->print(out, (*domains_)[0]);
	  out << " ";
	  long double lprob = computePerPredLL(gndingsToClauseIndexesAndCounts, g, wt, out);
	  int truthValue = (*domains_)[0]->getDB()->getValue(underEval);
	  long double prob = exp(lprob);//1/(1+exp(-lprob));
	  prob = (10000 * prob + .5)/(10000+1);
	  if (truthValue == 0){
	    prob = 1 -prob;
	  }
	  
	  out << lprob << " " << prob << " " << truthValue << endl; //" " << lprob << " " << truthValue << endl;
	}
      }
    }
  }
 private:

  long double computePerPredLL(const Array<Array<Array<IndexAndCount*>*>*>*
			       const& gndingsToClauseIndexesAndCounts,
			       const int& g, const double* const & wt, ofstream& out) 
  {
      // compute W.(N^bar-N)

      // Changed to logl, expl because otherwise changing
      // order of .mln files changes weight results
    long double wdotn = 0;
    long double lpmb = 0;
    
    Array<Array<IndexAndCount*>*>* gndings =
      (*gndingsToClauseIndexesAndCounts)[g];

      // For each valid assignment of vars in block
    for (int c = 0; c < gndings->size(); c++)
    {
      Array<IndexAndCount*>* clauseIndexesAndCounts = (*gndings)[c];
      /*
      cerr << g << " " << c << " ";
      for(int ix = 0; ix < clauseIndexesAndCounts->size(); ix++){
	cerr << (*(*clauseIndexesAndCounts)[ix]->index) << " ";
      }
      cerr << endl;
      */
      assert(noRepeatedIndex(clauseIndexesAndCounts));
    
      int numClausesUnifyWith = clauseIndexesAndCounts->size();

      for (int i = 0; i < numClausesUnifyWith; i++)
      {
	out << "(" << wt[ *( (*clauseIndexesAndCounts)[i]->index ) ] << " * "
	     << (*clauseIndexesAndCounts)[i]->count << ") "; //endl;
          // grounding g unifies with clause (*clauseIndexesAndCounts)[i].idx
        wdotn += wt[ *( (*clauseIndexesAndCounts)[i]->index ) ] * 
          (*clauseIndexesAndCounts)[i]->count;
      }
      
      if (wdotn > 0) lpmb += -logl(1+expl(-wdotn))-wdotn;
      else lpmb += -logl(1+expl(wdotn));
    }
    out << " wdotn " << wdotn << " ";
    return lpmb;
    //perPredPll += lpmb;
  }


  void computePerPredPllAndGrad(const Array<Array<Array<IndexAndCount*>*>*>*
                                const& gndingsToClauseIndexesAndCounts,
                                const int& g, const double* const & wt, 
                                long double& perPredPll, 
                                double * const & perPredGrad)
  {
      // compute W.(N^bar-N)

      // Changed to logl, expl because otherwise changing
      // order of .mln files changes weight results
    long double wdotn = 0;
    long double lpmb = 0;
    
    Array<Array<IndexAndCount*>*>* gndings =
      (*gndingsToClauseIndexesAndCounts)[g];

      // For each valid assignment of vars in block
    for (int c = 0; c < gndings->size(); c++)
    {
      Array<IndexAndCount*>* clauseIndexesAndCounts = (*gndings)[c];
      /*
	cerr << g << " " << c << " ";
	for(int ix = 0; ix < clauseIndexesAndCounts->size(); ix++){
	cerr << (*(*clauseIndexesAndCounts)[ix]->index) << " ";
	}
	cerr << endl;
      */
      assert(noRepeatedIndex(clauseIndexesAndCounts));
    
      int numClausesUnifyWith = clauseIndexesAndCounts->size();

      for (int i = 0; i < numClausesUnifyWith; i++)
      {

	if (g == -1){
	  cout << "wt = " << wt[ *( (*clauseIndexesAndCounts)[i]->index ) ] << " * " << (*clauseIndexesAndCounts)[i]->count << endl;
	}
          // grounding g unifies with clause (*clauseIndexesAndCounts)[i].idx
        wdotn += wt[ *( (*clauseIndexesAndCounts)[i]->index ) ] * 
          (*clauseIndexesAndCounts)[i]->count;
      }
      
      if (wdotn > 0) lpmb += -logl(1+expl(-wdotn))-wdotn;
      else lpmb += -logl(1+expl(wdotn));
    }
    perPredPll += lpmb;
    if (g == -1){
      cout << "g " << g << " lpmb " << lpmb << " perPredPll " << perPredPll << " wdotn " << wdotn << endl;
    }
      // update gradient
    for (int c = 0; c < gndings->size(); c++)
    {
      Array<IndexAndCount*>* clauseIndexesAndCounts 
        = (*gndings)[c];
      
      for (int i = 0; i < clauseIndexesAndCounts->size(); i++)
      {
        perPredGrad[ *( (*clauseIndexesAndCounts)[i]->index ) ] 
          += (expl(lpmb)-1) * (*clauseIndexesAndCounts)[i]->count;
	if (g == -1){
	  cout << "gradient update = " << (expl(lpmb)-1) << " * " << (*clauseIndexesAndCounts)[i]->count << " " << ((expl(lpmb)-1) * (*clauseIndexesAndCounts)[i]->count) << " perPredGrad " << perPredGrad[ *( (*clauseIndexesAndCounts)[i]->index ) ] << endl;
	}
      }
    }

  }

  void computeSampledPerPredPllAndGrad(IntHashArray& gndings, 
                                       const int& totalGndings,
                                       long double& tmpPerPredPll,
                                       double* const & tmpPerPredGrad,
                                       long double& perPredPll,
                                       double* const & perPredGrad,
                                       const int& arrSize,
                                       const double* const & wt,
                                    const Array<Array<Array<IndexAndCount*>*>*>*
                                       const& gndingsToClauseIndexesAndCounts)
  {
    tmpPerPredPll = 0;
    memset(tmpPerPredGrad, 0, arrSize*sizeof(double));
    for (int i = 0; i < gndings.size(); i++){
      
      computePerPredPllAndGrad(gndingsToClauseIndexesAndCounts, 
                               gndings[i], wt, tmpPerPredPll, 
                               tmpPerPredGrad);
    }
    if (gndings.size() > 0)
    {
      perPredPll += totalGndings * tmpPerPredPll/gndings.size();

      for (int i = 0; i < arrSize; i++)
        perPredGrad[i] += totalGndings * tmpPerPredGrad[i]/gndings.size();
    }
    
  }


    //Returns value. Gradients are set in gradient.
  long double getValueAndGradientForDomain(double* const & gradient, 
                                      const double* const & wt,
                                      const int& arrSize, const int& domainIdx)
  {
    long double wpll = 0; // weighted pseudo-log-likelihood
    double* perPredGrad = new double[arrSize];

      //used if sampling ground predicates
    long double tmpPerPredPll;
    double* tmpPerPredGrad = NULL;
    //Timer jtime;
    //double totalTime;
    if (sampleGndPreds_) tmpPerPredGrad = new double[arrSize];
    
    Array<Array<Array<Array<IndexAndCount*>*>*>*>* gndPredClauseIndexesAndCounts
      = (*gndPredClauseIndexesAndCountsArr_)[domainIdx];
    /*
      double entries = 0;
      for(int ix = 0; ix < gndPredClauseIndexesAndCounts->size(); ix++){
      Array<Array<Array<IndexAndCount*>*>*>* t1 =  (*gndPredClauseIndexesAndCounts)[ix];
      if (t1){
      for(int jx = 0; jx < t1->size(); jx++){
      
      Array<Array<IndexAndCount*>*>* t2 = (*t1)[jx];
      if (t2){
      for(int kx = 0; kx < t2->size(); kx++){
      if ((*t2)[kx])
      entries += (*t2)[kx]->size();
      }
      }
      }
      }
      }

     cout << "Entries: " << entries << endl;
    */
    int numPreds = gndPredClauseIndexesAndCounts->size();
    //double calls = 0;
    for (int p = 0; p < numPreds; p++) // for each predicate
    {
      if (!(*areNonEvidPreds_)[p]) continue; 

      //commented out: even though pred does not appear in any clause, each
      //of its groundings contribute -ln(2) to the wpll
      //if ((*gndPredClauseIndexesAndCounts)[p] == NULL) continue;

      long double perPredPll = 0;
      memset(perPredGrad, 0, arrSize*sizeof(double));

        //if pred p appears in one or more clauses
      if ((*gndPredClauseIndexesAndCounts)[p] != NULL)
      {
        Array<Array<Array<IndexAndCount*>*>*>* gndingsToClauseIndexesAndCounts 
          = (*gndPredClauseIndexesAndCounts)[p];

        if (sampleGndPreds_)
        {
          SampledGndings* sg = (*(*sampledGndingsMaps_)[domainIdx])[p];
          computeSampledPerPredPllAndGrad(sg->trueGndings, sg->totalTrue, 
                                          tmpPerPredPll, tmpPerPredGrad,
                                          perPredPll, perPredGrad, arrSize, 
                                          wt, gndingsToClauseIndexesAndCounts);
          computeSampledPerPredPllAndGrad(sg->falseGndings, sg->totalFalse, 
                                          tmpPerPredPll, tmpPerPredGrad,
                                          perPredPll, perPredGrad, arrSize, 
                                          wt, gndingsToClauseIndexesAndCounts);
        }
        else
        {   //use all groundings of predicate
          int numGnds = gndingsToClauseIndexesAndCounts->size();
          assert(numGnds == (*((*numGndings_)[domainIdx]))[p]);
	  //calls += numGnds;
          for (int g = 0; g < numGnds; g++) // for each grounding
	    {
	      //double js = jtime.time();
	      computePerPredPllAndGrad(gndingsToClauseIndexesAndCounts, g, wt, 
                                     perPredPll, perPredGrad);
	      //double je = jtime.time();
	      //totalTime = totalTime + (je-js);
	    }
	}
      }
      else
      {   //pred p does not appear in any clauses
        perPredPll = (*((*numGndings_)[domainIdx]))[p] * -log(2);
        //perPredGrad entries are all zeroes
      }
      
      if (wtFOPred_) 
      {

	//cout << "per Pred " << perPredPll << " " << (*((*numGndings_)[domainIdx]))[p] << endl;
          //negate the value and gradient here
        wpll -= perPredPll / (*((*numGndings_)[domainIdx]))[p];
        for (int i = 0; i < arrSize; i++)
        {
          gradient[i] -= perPredGrad[i]/(*((*numGndings_)[domainIdx]))[p];
        }
      }
      else
      {
          //negate the value and gradient here
        wpll -= perPredPll;
        for (int i = 0; i < arrSize; i++) 
        {
          gradient[i] -= perPredGrad[i]; 
        }
      }
    } // for each predicate
    /*
      cout << "Calls: " << calls << " Avg " << (totalTime/calls) << endl;
    cout << "Total Time: ";
    jtime.printTime(cout, totalTime);
    cout << endl << endl;
    */
    delete [] perPredGrad;
    if (sampleGndPreds_) delete [] tmpPerPredGrad;

    return wpll;
  }

 public:
  //working
  void collapse(int numDomains, Array<Array<int>*>* clauses){
    /*
Array<Array<Array<Array<Array<IndexAndCount*>*>*>*>*>* 
    gndPredClauseIndexesAndCountsArr_;
    */
    Array<Array<Array<Array<Array<IndexAndCount*>*>*>*>*>* jesse =
      new Array<Array<Array<Array<Array<IndexAndCount*>*>*>*>*>;
    jesse->growToSize(numDomains,NULL);
    for(int ix = 0; ix < numDomains; ix++){
      //predicates
      (*jesse)[ix] = new Array<Array<Array<Array<IndexAndCount*>*>*>*>;
      (*jesse)[ix]->growToSize((*gndPredClauseIndexesAndCountsArr_)[ix]->size());
      //groundings//
      for(int jx = 0; jx < (*jesse)[ix]->size(); jx++){
	(*((*jesse)[ix]))[jx] = new Array<Array<Array<IndexAndCount*>*>*>;
	Array<Array<Array<IndexAndCount*>*>*>* arr = (*((*jesse)[ix]))[jx];
	long double numGndings = (*((*numGndings_)[ix]))[jx];
	for (int g = 0; g < numGndings; g++){ 
	  Array<Array<IndexAndCount*>*>* tmpArr =new Array<Array<IndexAndCount*>*>;
	  Array<IndexAndCount*>* counts = new Array<IndexAndCount*>;
	  IntHashArray okClause;

	  //cerr << "Here 1";
	  Array<IndexAndCount*>* oldCounts = 
	    (*((*((*((*gndPredClauseIndexesAndCountsArr_)[ix]))[jx]))[g]))[0];
	  for(int ok = 0; ok < oldCounts->size(); ok++){
	    okClause.append(*((*oldCounts)[ok]->index));
	  }
	  //cerr << "Here 2";
	  for(int sndIx = 0; sndIx < clauses->size(); sndIx++){
	    Array<int>* pattern = (*clauses)[sndIx];
	    long double newCnt = 0;
	    //cerr << oldCounts->size() << " " << patterns->size() << endl;
	    //HashInt
	    for(int featIx = 0; featIx < pattern->size(); featIx++){
	      int position = (*pattern)[featIx];
	      int index = okClause.find(position);
	      if (index >= 0){
		//cerr << position << " " << oldCounts->size() << endl;
		//}
		//if ((*oldCounts)[position])
		newCnt += (*oldCounts)[index]->count;
	      }
	    }
	    if (newCnt > 0){
	      int* clauseIdx = new int;
	      *clauseIdx = sndIx;
	      //cout << ix << " " << jx << " " << g << " " << (*clauseIdx) << " " << clauseIdx << endl;
	      counts->append(new IndexAndCount(clauseIdx, newCnt));
	    }
	  }
	  tmpArr->append(counts);
	  arr->append(tmpArr);
	}
	//arr->compress();
	//(*gndPredClauseIndexesAndCounts)[predId] = arr;
	
      }
      //Array<Array<Array<IndexAndCount*>*>*>*
      //Array<Array<IndexAndCount*>*>*>*
    }
    //gndPredClauseIndexesAndCountsArr_[d][p][g][i]->count is the difference
    //between the number of true groundings of the ith clause when g takes the  
    //opposite of its true value and its true value.
    //JD FLAG
    
    for (int i = 0; i < gndPredClauseIndexesAndCountsArr_->size(); i++)
    {
      Array<Array<Array<Array<IndexAndCount*>*>*>*>*
      gndPredClauseIndexesAndCounts
        = (*gndPredClauseIndexesAndCountsArr_)[i];

      int numPreds = gndPredClauseIndexesAndCounts->size();
      for (int p = 0; p < numPreds; p++) // for each predicate
      {
        if ( (*gndPredClauseIndexesAndCounts)[p] )
        {
          Array<Array<Array<IndexAndCount*>*>*>* gndingsToClauseIndexesAndCounts
            = (*gndPredClauseIndexesAndCounts)[p];
          int numGnds = gndingsToClauseIndexesAndCounts->size();
          for (int g = 0; g < numGnds; g++) // for each grounding
          {
            for (int h = 0; h < (*gndingsToClauseIndexesAndCounts)[g]->size(); 
                 h++)
            {
              for (int j = 0;
                   j < (*(*gndingsToClauseIndexesAndCounts)[g])[h]->size();
                   j++)
              {
                delete (*(*(*gndingsToClauseIndexesAndCounts)[g])[h])[j];
              }
              delete (*(*gndingsToClauseIndexesAndCounts)[g])[h];
            }
            delete (*gndingsToClauseIndexesAndCounts)[g];
          }
          delete gndingsToClauseIndexesAndCounts;
        }
      }
      delete gndPredClauseIndexesAndCounts;      
    }
    delete gndPredClauseIndexesAndCountsArr_;
    gndPredClauseIndexesAndCountsArr_ = jesse;
  }

 private:
  void createClauseIndexesAndCountsArrays(const int& predId, 
                                          const int& domainIdx)
  {
    Array<Array<Array<Array<IndexAndCount*>*>*>*>* gndPredClauseIndexesAndCounts
      = (*gndPredClauseIndexesAndCountsArr_)[domainIdx];
    if ((*gndPredClauseIndexesAndCounts)[predId] != NULL) return;

    Array<Array<Array<IndexAndCount*>*>*>* arr =
      new Array<Array<Array<IndexAndCount*>*>*>;
    long double numGndings = (*((*numGndings_)[domainIdx]))[predId];

      //for each grounding, create a record of the indexes and counts of the
      //clauses in which the grounding appears
    for (int g = 0; g < numGndings; g++) 
      arr->append(new Array<Array<IndexAndCount*>*>);
    arr->compress();
    (*gndPredClauseIndexesAndCounts)[predId] = arr;
  }

  void createComboClauseIndexesAndCountsArrays(const int& predId, 
                                               const int& domainIdx,
                                               Predicate* const & gndPred,
                                               const int& g)
  {
    Array<Array<Array<Array<IndexAndCount*>*>*>*>* gndPredClauseIndexesAndCounts
      = (*gndPredClauseIndexesAndCountsArr_)[domainIdx];

    Array<Array<IndexAndCount*>*>* comboClauseIndexesAndCounts
      = (*(*gndPredClauseIndexesAndCounts)[predId])[g];
    if (comboClauseIndexesAndCounts->size() > 0) return;

        // Check if this grounding is in a block
    int numCombInBlock = 1;
      
    int blockIdx = (*domains_)[domainIdx]->getBlock(gndPred);
    if (blockIdx >= 0)
      numCombInBlock = (*domains_)[domainIdx]->getBlockSize(blockIdx) - 1;
      
    comboClauseIndexesAndCounts->growToSize(numCombInBlock, NULL);
    for (int c = 0; c < numCombInBlock; c++)
    {
      (*comboClauseIndexesAndCounts)[c] = new Array<IndexAndCount*>;
    }
    comboClauseIndexesAndCounts->compress();
  }


    //Returns false if grounding g of gndPred with predId has been looked at;
    //otherwise returns true. The difference between the number of true 
    //groundings of clause c when gndPred is held to the opposite of its truth 
    //value and to its actual value is computed, and appended to an 
    //Array<IndexAndCount*>. If undoInfos is not NULL, a pointer to that 
    //Array<IndexAndCount*> is added in undoInfos.
    //Similar to insertCounts().
  bool computeAndSetCounts(const Clause* const clause, 
                           int* const & clauseIdxInMLN, 
                           const int& predId, Predicate& gndPred, 
                           const int& g, Database* const & db, 
                           const int& domainIdx,
                           Array<UndoInfo*>* const & undoInfos,
                           const bool& sampleClauses,
                           Array<Array<Array<CacheCount*>*>*>* const & cache)
  {
    Array<Array<Array<Array<IndexAndCount*>*>*>*>* gndPredClauseIndexesAndCounts
      = (*gndPredClauseIndexesAndCountsArr_)[domainIdx];
    const Domain* domain = (*domains_)[domainIdx];


    if ((*gndPredClauseIndexesAndCounts)[predId] == NULL)
      createClauseIndexesAndCountsArrays(predId, domainIdx);

    createComboClauseIndexesAndCountsArrays(predId, domainIdx, &gndPred, g);

    Array<Array<IndexAndCount*>*>* comboClauseIndexesAndCounts
      = (*(*gndPredClauseIndexesAndCounts)[predId])[g];
/*
cout << "Clause: ";
clause->printWithoutWtWithStrVar(cout, domain);
cout << endl;
cout << "Pred: ";
gndPred.printWithStrVar(cout, domain);
cout << endl;
cout << "PredId: " << predId << " Gnding: " << g << " # of combos: "
     << comboClauseIndexesAndCounts->size() << endl;
*/     
      // For each combination
    for (int c = 0; c < comboClauseIndexesAndCounts->size(); c++)
    {
      //cout << "Combo " << c << " ";
      //cout << comboClauseIndexesAndCounts->size() << endl;
      
        //if gth grounding of pred with predId has been looked at, ignore it
      Array<IndexAndCount*>* gArr = (*comboClauseIndexesAndCounts)[c];
      if (gArr->size() > 0 && *( gArr->lastItem()->index ) == *clauseIdxInMLN)
      {
//cout << "Already looked at" << endl;
        //return false;
        continue;
      }

      long double cnt =
        ((Clause*)clause)->countDiffNumTrueGroundings(&gndPred, domain, db,
                                                      DB_HAS_UNKNOWN_PREDS,
						      sampleClauses, c);
      //cout << "Count is " << cnt << endl;
        //ignore clauses if the difference in counts is zero
      if (cnt != 0)
      {

        gArr->append(new IndexAndCount(clauseIdxInMLN, cnt));
        if (undoInfos)
          undoInfos->append(new UndoInfo(gArr, NULL, -1, domainIdx));

        if (cache) 
        {
          Array<CacheCount*>*& ccArr = (*(*cache)[domainIdx])[predId];
          if (ccArr == NULL) ccArr = new Array<CacheCount*>;
          ccArr->append(new CacheCount(g, c, cnt));
        }
      }

      assert(noRepeatedIndex(gArr));
    }
    
    return true;
  }


    //returns true if an IndexCount is removed; return false otherwise
  bool removeCounts(int* const & clauseIdxInMLN, const int& predId, 
                    const int& g,  const int& domainIdx,
                    Array<UndoInfo*>* const & undoInfos)
  {
    bool removed = false;
    Array<Array<Array<Array<IndexAndCount*>*>*>*>* gndPredClauseIndexesAndCounts
      = (*gndPredClauseIndexesAndCountsArr_)[domainIdx];

    if ((*gndPredClauseIndexesAndCounts)[predId] == NULL) return false;

    Array<Array<IndexAndCount*>*>* comboClauseIndexesAndCounts
      = (*(*gndPredClauseIndexesAndCounts)[predId])[g];
      // For each combination
    for (int c = 0; c < comboClauseIndexesAndCounts->size(); c++)
    {
      Array<IndexAndCount*>* gArr =(*comboClauseIndexesAndCounts)[c];
      for (int i = 0; i < gArr->size(); i++)
      {
        if ((*gArr)[i]->index == clauseIdxInMLN)
        {
          IndexAndCount* ic = gArr->removeItemFastDisorder(i);

          if (undoInfos) // this is a temporary removal
          {
            undoInfos->append(new UndoInfo(gArr, ic, i, domainIdx));
            assert(noRepeatedIndex(gArr));
            //return true;
            removed = true;
          }
          else
          { // removing this IndexCount for good
            delete ic;

            assert(noRepeatedIndex(gArr));
            //return true;
            removed = true;
          }
        }
      }
    assert(noRepeatedIndex(gArr));
    }

    //return false;
    return removed;
  }

 public:
  void printJesse(){
    /*

    */
    Array<Array<Array<Array<IndexAndCount*>*>*>*>* t1 = (*gndPredClauseIndexesAndCountsArr_)[0];
    if (t1){
      Array<Array<Array<IndexAndCount*>*>*>* t2 = (*t1)[2];
      if (t2){
	Array<Array<IndexAndCount*>*>* t3 = (*t2)[5];
	if (t3){
	  Array<IndexAndCount*>* t4 = (*t3)[0];
	  if (t4){
	    IndexAndCount* iac1 = (*t4)[0];

	    cerr << "check: " << iac1->index << " " << iac1->count;
	    if (t4->size() > 1){
	      IndexAndCount* iac2 = (*t4)[1];
	      cerr << " " << iac2->index << " " << iac2->count << endl;
	    }
	  }
	}
      }
    }
  }

 private:
  void createPredAllTermsAreDiffVars(const Clause* const & c,
                                      Array<bool>& predAllTermsAreDiffVars,
                                      Array<int>& predIdxWithAllTermsDiffVars)
  {
    for (int p = 0; p < c->getNumPredicates(); p++)
    {
      Predicate* pred = c->getPredicate(p);
      bool allDiffVars;
      
      if (c->isDirty())
      {
          //force a complete check whether all terms are different variables
        allDiffVars = pred->checkAllTermsAreDiffVars();
      }
      else
      {
        assert(!pred->isDirty());
        allDiffVars = pred->allTermsAreDiffVars();
      }
      
      predAllTermsAreDiffVars.append(allDiffVars);
      if (allDiffVars)
      {
        int predId = pred->getId();
        if (predIdxWithAllTermsDiffVars[predId] < 0)
          predIdxWithAllTermsDiffVars[predId] = p;
      }
    }
    predAllTermsAreDiffVars.compress();
  }


  void createMultipliers(Array<int>& multipliers, 
                         const Predicate& gndPred,
                         const Domain* const & domain)
  {
    int mult = 1;
    int numTerms = gndPred.getNumTerms();
    multipliers.growToSize(numTerms);
    for (int j = numTerms-1; j >= 0; j--)
    {
      multipliers[j] = mult;
      int typeId = gndPred.getTermTypeAsInt(j);
      mult *= domain->getNumConstantsByType(typeId);
    }
  }


  void createMappingOfVarIdToMultipliersAndVarGroundingsAndOffset(
                                 const Predicate& gndPred,
                                 const Domain* const & domain,
                                 Array<int>& multipliers,
                                 int& offsetDueToConstants,
                                 Array<Array<pair<int,Term*> >* >& varIdToMults,
                                 Array<int>& negVarIdsArr,
                                 ArraysAccessor<int>& groundings)
  {
    for (int j = 0; j < gndPred.getNumTerms(); j++)
    {
      const Term* t =  gndPred.getTerm(j);
      if (t->getType() == Term::VARIABLE)
      {
        assert(t->getId()<0);
        int id = -(t->getId());
        if (id >= varIdToMults.size()) varIdToMults.growToSize(id+1,NULL);
        if (varIdToMults[id] == NULL) 
        {
          negVarIdsArr.append(id);
          varIdToMults[id] = new Array<pair<int,Term*> >; 
          int typeId = gndPred.getTermTypeAsInt(j);
          const Array<int>* constants = domain->getConstantsByType(typeId);
          groundings.appendArray(constants);
        }
        varIdToMults[id]->append(pair<int,Term*>(multipliers[j], (Term*)t));
      }
      else
      if (t->getType() == Term::CONSTANT)
      {
        int id = t->getId();
        assert(id >= 0);
        int typeId = gndPred.getTermTypeAsInt(j);
        const Array<int>* constants = domain->getConstantsByType(typeId);
        assert(constants->size() > 0);
          //ASSUMPTION: constants of the same type are consecutively numbered in
          //the constants array
        for (int i = 0; i < constants->size(); i++)
        {
          if ((*constants)[i] == id)
          {
            offsetDueToConstants += i*multipliers[j];
            break;
          }
        }
        //delete constants;
      }
      else
      {
        assert(false);
      }
    }
  }


  void randomlySelect(IntHashArray& gndings, const double& fraction,
                      const int& min, const int& max)
  {
    int size = int(fraction * gndings.size() + 0.5);
    if (min >= 0 && size < min)      size = min;
    else if (max >= 0 && size > max) size = max;
    while (gndings.size() > size)
      gndings.removeItemFastDisorder(random_->randomOneOf(gndings.size()));
  }


  void samplePredGroundingsForDomain(const Predicate* const& foPred, 
                                     const Domain* const & domain,
                                     SampledGndings* sampledGndings,
                                     const double& fraction, 
                                     const int& min, const int& max)
  {
    cout << "sampling predicate "; foPred->printWithStrVar(cout, domain); 
    cout << endl;

    assert(((Predicate*)foPred)->allTermsAreDiffVars());
    ArraysAccessor<int> acc;
    createAllPossiblePredsGroundings(foPred, domain, acc);
    Predicate* gndPred = (Predicate*) foPred;
    const Database* db = domain->getDB();

    IntHashArray& trueGndings = sampledGndings->trueGndings;
    IntHashArray& falseGndings = sampledGndings->falseGndings;

    int g = -1; //gth grounding
    while (acc.hasNextCombination()) //for each grounding of pred 
    {
      ++g;
      int t = 0;
      int constId; 
      while (acc.nextItemInCombination(constId))
        ((Term*) gndPred->getTerm(t++))->setId(constId);

      TruthValue tv = db->getValue(gndPred);
      if (tv == TRUE) trueGndings.append(g);
      else if (tv == FALSE) falseGndings.append(g);      
    }
    //acc.deleteArraysAndClear();

    sampledGndings->totalTrue = trueGndings.size();
    sampledGndings->totalFalse = falseGndings.size();
    randomlySelect(trueGndings, fraction, min, max);
    randomlySelect(falseGndings, fraction, min, max);
    trueGndings.compress();
    falseGndings.compress();

    cout << "\tsampled/total (true ground atoms) = " 
         << trueGndings.size() << "/" << sampledGndings->totalTrue << endl;
    cout << "\tsampled/total (false ground atoms) = " 
         << falseGndings.size() << "/" << sampledGndings->totalFalse << endl;
  }


  void samplePredGroundings(const double& fraction, 
                            const int& min, const int& max)
  {
    for (int d = 0; d < domains_->size(); d++)
    {
      cout << "domain " << d << endl;
      const Domain* domain = (*domains_)[d];
      Array<SampledGndings*>* sgm = (*sampledGndingsMaps_)[d];
      for (int p = 0; p < domain->getNumPredicates(); p++)
      {
        if (!(*areNonEvidPreds_)[p]) continue;
        SampledGndings* sg = new SampledGndings;
        assert((*sgm)[p] == NULL);
        (*sgm)[p] = sg;
        Predicate* foPred = domain->createPredicate(p, true);
        assert(foPred);
        samplePredGroundingsForDomain(foPred, domain, sg, fraction, min, max);
        delete foPred;
      }
    }
  }
  

  bool noRepeatedIndex(const Array<IndexAndCount*>* const & gArr,
                       const MLN* const & mln0=NULL)
  {
    hash_set<int> set;
    for (int i = 0; i < gArr->size(); i++)
    {
      int ii = *((*gArr)[i]->index);
      if (set.find(ii) != set.end()) 
      {
        cout << "ERROR: in PseudoLogLikelihood::noRepeatedIndex. "
             << "Repeated index " << ii << " found. ";
        if (mln0) 
          mln0->getClause(ii)->printWithoutWtWithStrVar(cout, (*domains_)[0]);
        cout << endl;
        return false;
      }
      set.insert(ii);
    }
    return true;
  }


  void printWtsGradsWPLL(const double* const & wts, const double* const & grads,
                         const int& arrSize, const long double& wpll)
  {
    cout.precision(10);
   
    cout << "wts = " << endl;
    for (int i = 0; i < arrSize; i++) cout << " " << i << ":" << wts[i];
    cout << endl;
    cout << "grads = " << endl;
    for (int i = 0; i < arrSize; i++) cout << " " << i << ":" << grads[i];
    cout << endl;
    cout << "wpll = " << wpll << endl;
    cout << endl;
    cout.precision(6);
  }

 public:
  void setJDebug(bool val){
    jdebug = val;
  }

 private:
  Array<bool>* areNonEvidPreds_;

    //contents of array are not owned by PseudoLogLikelihood; do not delete them
  Array<Domain*>* domains_;

    //gndPredClauseIndexesAndCountsArr_[d][p][g][i]->index is the index into an 
    //MLN of the ith clause that the grounding g of pred p unifies with in 
    //domain d. 
    //The index indexes into the wt and gradient parameter of 
    //getValueAndGradient().
    //gndPredClauseIndexesAndCountsArr_[d][p][g][i]->count is the difference
    //between the number of true groundings of the ith clause when g takes the  
    //opposite of its true value and its true value.
  Array<Array<Array<Array<Array<IndexAndCount*>*>*>*>*>* 
    gndPredClauseIndexesAndCountsArr_;

  int numMeans_; // size priorMeans_ and priorStdDevs_ arrays
  const double* priorMeans_; //not owned by PseudoLogLikelihood, so don't delete
  const double* priorStdDevs_;//not owned by PseudoLogLikelihood,so don't delete

  bool wtFOPred_;  
  Array<Array<long double>*>* numGndings_;

  bool sampleGndPreds_;
    // sampledGndingsMaps_[d][p] is the SampledGndings of the predicate with 
    // id p in domain d
  Array<Array<SampledGndings*>*>* sampledGndingsMaps_;
  Random* random_;

  IndexTranslator* idxTrans_; //not owned by PseudoLogLikelihood; do not delete
  bool jdebug;
};


#endif
