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
#ifndef CLAUSE_H_JUN_26_2005
#define CLAUSE_H_JUN_26_2005

#include <ext/hash_set>
using namespace __gnu_cxx;
#include <ostream>
#include <sstream>
using namespace std;
#include <climits>
#include "domain.h"
#include "hashstring.h"
#include "hashlist.h"
#include "arraysaccessor.h"
#include "powerset.h"
#include "multdarray.h"
#include "hashint.h"
#include "database.h"
#include "predicate.h"
#include "groundpredicate.h"
#include "clausesampler.h"
#include "clausehelper.h"

const bool useInverseIndex = true;
  // 0 = no ouput, 1 = some output, 2 = full output
const int clausedebug = 0;


/******************************************************************************/
/** Need to define this type in the beginning as it is used in this file */
class HashClause;
class EqualClause;
typedef HashArray<Clause*, HashClause, EqualClause> ClauseHashArray;
/******************************************************************************/

class AddGroundClauseStruct;


  // Ensure that the dirty_ bit is consistently updated.
class Clause
{
  friend class ClauseSampler;

 public: 
  Clause() 
    : wt_(0), predicates_(new Array<Predicate*>), intArrRep_(NULL),
      hashCode_(0), dirty_(true), isHardClause_(false),
    varIdToVarsGroundedType_(NULL), auxClauseData_(NULL),
    jdebug(false) {}

  Clause(const double& wt) 
    : wt_(wt), predicates_(new Array<Predicate*>), intArrRep_(NULL),
      hashCode_(0), dirty_(true), isHardClause_(false),
    varIdToVarsGroundedType_(NULL), auxClauseData_(NULL),
    jdebug(false) {}


  Clause(const Clause& c)
  {
    wt_ = c.wt_;
    predicates_ = new Array<Predicate*>;
    Array<Predicate*>* cpredicates = c.predicates_;
    for (int i = 0; i < cpredicates->size(); i++)
    {
      Predicate* p = (*cpredicates)[i];
      predicates_->append(new Predicate(*p, this));      
    }
    dirty_ = c.dirty_;

    if (!dirty_) { assert(noDirtyPredicates()); }

    if (c.intArrRep_)  intArrRep_ = new Array<int>(*(c.intArrRep_));
    else               intArrRep_ = NULL;

    hashCode_ = c.hashCode_;
    
    isHardClause_ = c.isHardClause_;

    if (c.varIdToVarsGroundedType_)
    {
      varIdToVarsGroundedType_ = new Array<VarsGroundedType*>;
      for (int i = 0; i < c.varIdToVarsGroundedType_->size(); i++)
      {
        VarsGroundedType* vgt = (*(c.varIdToVarsGroundedType_))[i];
        varIdToVarsGroundedType_->append(new VarsGroundedType(*vgt));
      }
    }
    else
      varIdToVarsGroundedType_ = NULL;

    if (c.auxClauseData_)
    {
      auxClauseData_ = new AuxClauseData(c.auxClauseData_->gain,
                                         c.auxClauseData_->op,
                                         c.auxClauseData_->removedClauseIdx,
                                         c.auxClauseData_->hasBeenExpanded,
                                         c.auxClauseData_->lastStepExpanded,
                                       c.auxClauseData_->lastStepOverMinWeight);
      if (c.auxClauseData_->constTermPtrs) trackConstants();
      if (c.auxClauseData_->cache)
      {
        Array<Array<Array<CacheCount*>*>*>* cache, * otherCache;
        cache  = new Array<Array<Array<CacheCount*>*>*>;
        otherCache = c.auxClauseData_->cache;

        cache->growToSize(otherCache->size(),NULL);

        for (int i = 0; i < otherCache->size(); i++)
        {
          (*cache)[i] = new Array<Array<CacheCount*>*>;
          (*cache)[i]->growToSize((*otherCache)[i]->size(), NULL);
          for (int j = 0; j < (*otherCache)[i]->size(); j++)
          {
            Array<CacheCount*>* ccArr = (*(*otherCache)[i])[j];
            if (ccArr == NULL) continue;
            (*(*cache)[i])[j] = new Array<CacheCount*>;
            (*(*cache)[i])[j]->growToSize(ccArr->size());
            for (int k = 0; k < ccArr->size(); k++)
            {
              (*(*(*cache)[i])[j])[k] = new CacheCount((*ccArr)[k]->g,
                                                       (*ccArr)[k]->c,
                                                       (*ccArr)[k]->cnt); 
            }
          }
        }
        auxClauseData_->cache = cache;
      }
      
      auxClauseData_->prevClauseStr = c.auxClauseData_->prevClauseStr;
      auxClauseData_->addedPredStr = c.auxClauseData_->addedPredStr;
      auxClauseData_->removedPredIdx = c.auxClauseData_->removedPredIdx;
    }
    else
      auxClauseData_ = NULL;

    jdebug = false;
  }


  ~Clause() 
  {
    for (int i = 0; i < predicates_->size(); i++)
      delete (*predicates_)[i];
    delete predicates_;
    if (intArrRep_) delete intArrRep_;
    if (varIdToVarsGroundedType_) deleteVarIdToVarsGroundedType();
    if (auxClauseData_) delete auxClauseData_;
  }

  void setJDebug(bool val){
    jdebug = val;
  }
  
    //returns approximate size in MB, mainly due to cache in auxClauseData_
  double sizeMB() const
  {
    double sizeMB = (fixedSizeB_ + intArrRep_->size()*sizeof(int) + 
                     predicates_->size()*sizeof(Predicate*)) /1000000.0;
    for (int i = 0; i < predicates_->size(); i++)
      sizeMB += (*predicates_)[i]->sizeMB();
    if (auxClauseData_ != NULL) sizeMB += auxClauseData_->sizeMB(); 
    return sizeMB;
  }

  static void substitute(Clause* clause, int varId, int constId){
    for(int jx = 0; jx < clause->getNumPredicates(); jx++){
      Predicate* pred = clause->getPredicate(jx);
      for(int kx = 0; kx < pred->getNumTerms(); kx++){
	if (pred->getTerm(kx)->getType() == Term::VARIABLE){
	  int termId = pred->getTerm(kx)->getId();
	  if (termId == varId)
	    pred->setTermToConstant(kx,constId);
	}
      }
    }
  }


  static void substitute(Clause* clause, hash_map<int,int>* term2const){
    for(int jx = 0; jx < clause->getNumPredicates(); jx++){
      Predicate* pred = clause->getPredicate(jx);
      for(int kx = 0; kx < pred->getNumTerms(); kx++){
	if (pred->getTerm(kx)->getType() == Term::VARIABLE){

	  int termId = pred->getTerm(kx)->getId();
	  hash_map<int,int>::iterator it = term2const->find(termId);
	  if (it == term2const->end()){
	    
	  }
	  else{
	    int constId = (*term2const)[termId]; 
	    pred->setTermToConstant(kx,constId);
	  }
	}
      }
    }
    
    //computeAndStoreIntArrRep();
  }

  static void buildMap(Predicate* evalPred, Predicate* gndPred, hash_map<int,int>* map){
    //    Predicate* evalPred = clause->getPredicate((*prevTerm)[jx]);
    for(int kx = 0; kx < gndPred->getNumTerms(); kx++){
      if (evalPred->getTerm(kx)->getType()  == Term::VARIABLE){
	int tId =  evalPred->getTerm(kx)->getId();
	int cId = gndPred->getTerm(kx)->getId();
	(*map)[tId] = cId;
      }
    }
  }

  static int factorial(int x){
    int val = 1;
    for(int ix = 1; ix <= x; ix++){
      val = val * ix;
    }
    return val;
  }

  static int choose(int n, int k){
    int n1 = factorial(n);
    int d1 = factorial(k);
    int d2 = factorial(n-k);
    return (n1/(d1*d2));
  }
  static Array< Array<int> *>* allSubSets(Array<int>* positions){
    Array< Array<int> *>* subsets = new Array< Array<int> *>();
    hash_map<int,int>* map = new hash_map<int,int>();
    for(int ix = 0; ix < positions->size(); ix++){
      Array<int> * tmp = new Array<int>();
      tmp->append((*positions)[ix]);		
      subsets->append(tmp);
      (*map)[(*positions)[ix]] = ix;
    }

    int start = 0;
    int finish = positions->size();
    for(int ix = 1; ix < positions->size(); ix++){
      cout << "Round " << ix << " Start " << start << " " << finish << endl;
      for(int jx =start; jx < finish; jx++){
	Array<int>* currArray = (*subsets)[jx];
	int lastElt = (*currArray)[currArray->size()-1];
	
	//hash_map<int,int>::iterator it = map->find(lastElt);
	int lastPos = (*map)[lastElt];
	for(int kx = (lastPos+1); kx < positions->size(); kx++){
	  Array<int> * tmp = new Array<int>(*currArray);
	  tmp->append((*positions)[kx]);		
	  subsets->append(tmp);
	}
      }
      start = finish;
      finish = finish + choose(positions->size(), ix+1);
    }
    delete map;
    return subsets;
  }

  static Array< Clause *>* getPartialGroundings(Clause* clause, Predicate* gndPred, Array<int> * inclusion){
    
    //Array<int>* checkIt = new Array<int>();
    
    Array<Clause*>* groundClauses = new Array< Clause *>();
    int gId = gndPred->getId();
    Array<int>* prevTerm = new Array<int>();
    const Array< Predicate * > * cPredicates = clause->getPredicates();
    for(int ix = 0; ix < cPredicates->size(); ix++){
      int curId = ((*cPredicates)[ix])->getId();
      //cout << "Here" << endl;
      if (curId == gId){
	//cout << "Here 1" << endl;
	Clause* cGnd = new Clause(*clause);
	hash_map<int,int>* term2const = new hash_map<int,int>;
	//cout << gndPred->getNumTerms() << " " << ((*cPredicates)
	for(int jx = 0; jx < gndPred->getNumTerms(); jx++){
	  if ((*cPredicates)[ix]->getTerm(jx)->getType()  == Term::VARIABLE){
	    int tId = ((*cPredicates)[ix])->getTerm(jx)->getId();
	    int cId = gndPred->getTerm(jx)->getId();
	    (*term2const)[tId] = cId;
	  }
	}
	//cout << "Here 2" << endl;
	substitute(cGnd, term2const);
	for(int jx = 0; jx < prevTerm->size(); jx++){
	  Clause* newClause = new Clause(*cGnd);
	  hash_map<int,int>* map = new hash_map<int,int>(*term2const);
	  //clause->getPredicate((*prevTerm)[jx])
	  buildMap(newClause->getPredicate((*prevTerm)[jx]),gndPred, map);
	  substitute(newClause,map); 
	  if (jx == 1){
	    Clause* nc1 = new Clause(*newClause);
	    buildMap(nc1->getPredicate((*prevTerm)[0]),gndPred,map);
	    substitute(nc1,map);
	    groundClauses->append(nc1);
	    inclusion->append(1);
	  }
	  newClause->canonicalize();
	  groundClauses->append(newClause);
	  inclusion->append(-1);
	  delete map;
	}
	prevTerm->append(ix);
	//cout << "Here 3" << endl;term2const
	delete term2const;
	cGnd->canonicalize();
	groundClauses->append(cGnd);
	inclusion->append(1);
      }
    }
    delete prevTerm;
    return groundClauses;
  }


  static void computeFixedSizeB()
  {
    fixedSizeB_ = sizeof(Clause) + sizeof(Array<Predicate*>) +
                  sizeof(Array<int>);
    //+ sizeof(Array<VarsGroundedType*>); // this is transient
  }


  void compress()
  {
    predicates_->compress();
    for (int i = 0; i < predicates_->size(); i++) (*predicates_)[i]->compress();
    if (intArrRep_) intArrRep_->compress();
    if (varIdToVarsGroundedType_) varIdToVarsGroundedType_->compress();
    if (auxClauseData_) auxClauseData_->compress();
  }


  bool same(Clause* const & c)
  {
    if (this == c)  return true;
    const Array<int>* cArr  = c->getIntArrRep();
    const Array<int>* myArr = getIntArrRep();
    if (myArr->size() != cArr->size()) return false;
    const int* cItems  = c->getIntArrRep()->getItems();
    const int* myItems = getIntArrRep()->getItems();
    return (memcmp(myItems, cItems, myArr->size()*sizeof(int))==0);
  }

  size_t hashCode() 
  {
    if (dirty_) computeAndStoreIntArrRep();
    return hashCode_;
  }


  int getNumPredicates() const { return predicates_->size(); }

  double getWt() const { return wt_; }

  const double* getWtPtr() const { return &wt_; }

    // not setting dirty bit because it does not affect the clause
  void setWt(const double& wt) { wt_ = wt; }  
  void addWt(const double& wt) { wt_ += wt;  }
 
  void setDirty() { dirty_ = true; }
  bool isDirty() const { return dirty_; }

    // Caller should not delete returned Predicate*.
  Predicate* getPredicate(const int& idx) const { return (*predicates_)[idx]; }
  
    // Caller should not delete returned array nor modify its contents.
  const Array<Predicate*>* getPredicates() const { return predicates_; }


  bool containsPredicate(const Predicate* const & pred) const
  {
    for (int i = 0; i < predicates_->size(); i++)
      if ((*predicates_)[i]->same((Predicate*)pred)) return true;
    return false;
  }

  
  int getNumVariables() const
  {
    hash_set<int> intset;
    for (int i = 0; i < predicates_->size(); i++)
      for (int j = 0; j < (*predicates_)[i]->getNumTerms(); j++)
      {
        if ((*predicates_)[i]->getTerm(j)->getType() == Term::VARIABLE)
          intset.insert((*predicates_)[i]->getTerm(j)->getId());
      }
    return intset.size();
  }


  int getNumVariablesAssumeCanonicalized() const
  {
    int minVarId = 0;
    for (int i = 0; i < predicates_->size(); i++)
      for (int j = 0; j < (*predicates_)[i]->getNumTerms(); j++)
      {
        if ((*predicates_)[i]->getTerm(j)->getType() == Term::VARIABLE &&
            (*predicates_)[i]->getTerm(j)->getId() < minVarId)
          minVarId = (*predicates_)[i]->getTerm(j)->getId();

      }
    return -minVarId;
  }


  bool isHardClause() const { return isHardClause_; }
  void setIsHardClause(const bool& b) { isHardClause_ = b; }


    // p is stored in MLN and the caller of this function should not delete it.
  void appendPredicate(Predicate* const& p) {predicates_->append(p);setDirty();}

 /**
  * Removes a predicate from this clause. After removing a predicate, the clause
  * is no longer canonicalized, so canonicalize() must be called.
  * 
  * @param i Index of predicate to be removed.
  */
  Predicate* removePredicate(const int& i) 
  { 
    if (0 <= i && i < predicates_->size()) 
    { setDirty(); return predicates_->removeItemFastDisorder(i); }
    return NULL;
  }


 /**
  * returns true if this clause contains redundant predicates.
  */
  bool hasRedundantPredicates()
  {
    for (int i = 0; i < predicates_->size(); i++)
    {
      Predicate* ip = (*predicates_)[i];
      for (int j = i+1; j < predicates_->size(); j++)
      {
        Predicate* jp = (*predicates_)[j];
        if (jp->same(ip) && jp->getSense() == ip->getSense()) return true;
      }
    }
    return false;
  }


    //returns true if redundant predicates were removed
  bool removeRedundantPredicates()
  {
    bool changed = false;
    for (int i = 0; i < predicates_->size(); i++)
    {
      Predicate* ip = (*predicates_)[i];
      for (int j = i+1; j < predicates_->size(); j++)
      {
        Predicate* jp = (*predicates_)[j];
        if (jp->same(ip) && jp->getSense() == ip->getSense())
        {
          predicates_->removeItemFastDisorder(j);
          changed = true;
          j--;
        }
      }
    }
    return changed;
  }

  
  void removeRedundantPredicatesAndCanonicalize()
  { if (removeRedundantPredicates()) canonicalize(); }

  
  void canonicalize(){
    canonicalize(true);
  }

  void canonicalize(bool sort){
    if (sort){
      canonicalize(1);
    }
    else{
      canonicalize(0);
    }
  }
  
  void canonicalize(int sort){
    // rename the variables in decreasing order (negative numbers) 
    Array<VarsGroundedType*>* vgtArr = new Array<VarsGroundedType*>;
    createVarIdToVarsGroundedType(vgtArr);
    
    if (sort == 1){
      sortPredicatesByIdAndSenseAndTerms(0, predicates_->size()-1, vgtArr);
    }
    else if (sort == 2){
      sortPredicatesByIdAndTerms(0, predicates_->size()-1, vgtArr);
    }
    //cerr << "Sorted preds\n";
    IntHashArray varAppearOrder;
    getVarOrder(varAppearOrder);
    
    int newVarId = 0;
    //cerr << "For Loop\n";
    for (int i = 0; i < varAppearOrder.size(); i++){
      int varId = varAppearOrder[i];
      ++newVarId;
      Array<Term*>& vars = (*vgtArr)[varId]->vars;
      for (int j = 0; j < vars.size(); j++)  vars[j]->setId(-newVarId);
    }
    assert(newVarId == varAppearOrder.size());
    
    int i = 0, j = 0;
    //cerr << "While Loop\n";
    while (i < predicates_->size()){
      while (++j < predicates_->size() 
             && (*predicates_)[i]->getId() == (*predicates_)[j]->getId() 
             && (*predicates_)[i]->getSense() == (*predicates_)[j]->getSense());
      if (i < j-1) sortPredicatesByTerms(i, j-1);
      i = j;
    }
    //cerr << "end while loop\n";
    deleteVarIdToVarsGroundedType(vgtArr);
    setDirty();
  }

  static bool moveTermsFromUnseenToSeen(Array<Term*>* const & terms,
                                 PredicateHashArray& unseenPreds,
                                 Array<Predicate*>& seenPreds)
  {
    for (int j = 0; j < terms->size(); j++)
    {
      bool parIsPred;
      Predicate* parent = (Predicate*) (*terms)[j]->getParent(parIsPred);      
      assert(parIsPred);
      int a;
      if ((a=unseenPreds.find(parent)) >= 0)
      {
        Predicate* par = unseenPreds.removeItemFastDisorder(a);
        if (unseenPreds.empty()) return true;
        assert(par == parent);
        seenPreds.append(par);
      }
    }
    return false;
  }
  
  
    //Returns true if there is a path of shared variables between any two preds
  bool checkPredsAreConnected()
  {
    if (predicates_->size() <= 1) return true;
    Array<Array<Term*>*>* varIdToTerms = new Array<Array<Term*>*>;
    hash_map<int,Array<Term*>*>* constIdToTerms=new hash_map<int,Array<Term*>*>;
    createVarConstIdToTerms(varIdToTerms, constIdToTerms);
    PredicateHashArray unseenPreds;
    Array<Predicate*> seenPreds;
    hash_set<int> seenIds;

    seenPreds.append((*predicates_)[0]);
    for (int i = 1; i < predicates_->size(); i++) 
      unseenPreds.append((*predicates_)[i]);

    while (!seenPreds.empty())
    {
      Predicate* curPred = seenPreds.removeLastItem();
      for (int i = 0; i < curPred->getNumTerms(); i++) 
      {
        const Term* t = curPred->getTerm(i);
        int id = t->getId();
        if (seenIds.find(id) != seenIds.end()) continue;
        seenIds.insert(id);

        if (t->getType() == Term::VARIABLE)
        {
          Array<Term*>* terms = (*varIdToTerms)[-id];
          for (int j = 0; j < terms->size(); j++)
          {
            bool parIsPred;
            Predicate* parent = (Predicate*) (*terms)[j]->getParent(parIsPred);
            assert(parIsPred);
            int a;
            if ((a=unseenPreds.find(parent)) >= 0)
            {
              Predicate* par = unseenPreds.removeItemFastDisorder(a);
              if (unseenPreds.empty()) 
              {
                deleteVarConstIdToTerms(varIdToTerms, constIdToTerms);
                return true;
              }
              seenPreds.append(par);

              //commented out: not true if there a duplicate preds in clause
              //               e.g. !isMale(p) v isMale(p)
              //assert(par == parent);
            }
          }
        }
        else
        if (t->getType() == Term::CONSTANT)
        {
          Array<Term*>* terms = (*constIdToTerms)[id];
          for (int j = 0; j < terms->size(); j++)
          {
            bool parIsPred;
            Predicate* parent = (Predicate*) (*terms)[j]->getParent(parIsPred);
            assert(parIsPred);
            int a;
            if ((a=unseenPreds.find(parent)) >= 0)
            {
              Predicate* par = unseenPreds.removeItemFastDisorder(a);
              if (unseenPreds.empty()) 
              {
                deleteVarConstIdToTerms(varIdToTerms, constIdToTerms);
                return true;
              }
              seenPreds.append(par);
                //commented out: not true if there a duplicate preds in clause
                //               e.g. !isMale(p) v isMale(p)
              //assert(par == parent);
            }
          }
        }
      } // for each term of curPred
    } // while (!seenPreds.empty())

    assert(!unseenPreds.empty());
    deleteVarConstIdToTerms(varIdToTerms, constIdToTerms);
    return false;
  }


  void canonicalizeWithoutVariables()
  {
    sortPredicatesByIdAndSenseAndTerms(0, predicates_->size()-1, NULL);
    int i = 0, j = 0;
    while (i < predicates_->size())
    {
      while (++j < predicates_->size() 
             && (*predicates_)[i]->getId() == (*predicates_)[j]->getId() 
             && (*predicates_)[i]->getSense() == (*predicates_)[j]->getSense());
      if (i < j-1) sortPredicatesByTerms(i, j-1);
      i = j;
    }
    setDirty();
  }

  
  AuxClauseData* getAuxClauseData() const { return auxClauseData_; }


  void setAuxClauseData(AuxClauseData* const& acd ) 
  { 
    if (auxClauseData_ == acd) return;
    if (auxClauseData_) delete auxClauseData_;
    auxClauseData_ = acd;
  }


  void newAuxClauseData() 
  {if (auxClauseData_) delete auxClauseData_; auxClauseData_=new AuxClauseData;}


  static void setClauseSampler(ClauseSampler* const & cs) 
  { if (clauseSampler_) delete clauseSampler_; clauseSampler_ = cs; }


  static const ClauseSampler* getClauseSampler() { return clauseSampler_; }
  

  bool containsConstants() const
  {
    return (auxClauseData_ && auxClauseData_->constTermPtrs && 
            auxClauseData_->constTermPtrs->size() > 0);
  }


    //auxClauseData_ must have been set to a valid AuxClauseData object
  void trackConstants()
  {
    if (auxClauseData_ == NULL) auxClauseData_ = new AuxClauseData; 
    Array<Term*>*& constTermPtrs = auxClauseData_->constTermPtrs;
    if (constTermPtrs) constTermPtrs->clear();
    for (int i = 0; i < predicates_->size(); i++)
    {
      Predicate* p = (*predicates_)[i];
      for (int j = 0; j < p->getNumTerms(); j++)
      {
        const Term* t = p->getTerm(j);
        if (t->getType() == Term::CONSTANT)
        {
          if (constTermPtrs == NULL) constTermPtrs = new Array<Term*>;
          constTermPtrs->append((Term*)t);
        }
      }
    }
  }


    //auxClauseData_ must have been set to a valid AuxClauseData object  
  void newCache(const int& numDomains, const int& numPreds)
  {
    assert(auxClauseData_);
    auxClauseData_->deleteCache();
    auxClauseData_->cache = new Array<Array<Array<CacheCount*>*>*>;
    auxClauseData_->cache->growToSize(numDomains, NULL);
    for (int i = 0; i < numDomains; i++)
    {
      (*auxClauseData_->cache)[i] = new Array<Array<CacheCount*>*>;
      (*auxClauseData_->cache)[i]->growToSize(numPreds, NULL);
    }
  }

  
  void translateConstants(const Domain* const & orig, const Domain* const& nnew)
  {
    if (auxClauseData_ == NULL || auxClauseData_->constTermPtrs == NULL) return;

    Array<Term*>* constTermPtrs = auxClauseData_->constTermPtrs;
    for (int i = 0; i < constTermPtrs->size(); i++)
    {
      Term* t = (*constTermPtrs)[i];
      int newId = nnew->getConstantId(orig->getConstantName(t->getId()));
      t->setId(newId);
      if (newId < 0)
      {
        cout << "ERROR: in Clause::translateConstants(). Failed to translate "
             <<orig->getConstantName(t->getId())<<" from one domain to another."
             << "Check that the constant exists in all domains." << endl;
        exit(-1);                                      
      }
    }
  }

    //Returns a map from typeId to variable ids. ReturnedArray[typeId] is NULL
    //if there are no variables for the corresponding typeId.
    //Caller is responsible for deleting the return Array and its contents.
  Array<Array<int>*>* getTypeIdToVarIdsMapAndSmallestVarId(int& smallestVarId)
  {
    smallestVarId = 0;
    hash_set<int> intSet;
    Array<Array<int>*>* arr = new Array<Array<int>*>;
    for (int i = 0; i < predicates_->size(); i++)
    {
      Predicate* pred = (*predicates_)[i];
      for (int j = 0; j < pred->getNumTerms(); j++)
      {
        const Term* t = pred->getTerm(j);
        if (t->getType() == Term::VARIABLE)
        {
          int varId = t->getId();
          if (intSet.find(varId) != intSet.end()) continue;
          intSet.insert(varId);

          int typeId = pred->getTermTypeAsInt(j); 
          if (typeId >= arr->size()) arr->growToSize(typeId+1, NULL);
          if ((*arr)[typeId] == NULL) (*arr)[typeId] = new Array<int>;
          (*arr)[typeId]->append(varId); 
          
          if (varId < smallestVarId) smallestVarId = varId;
        }
      }
    }
    return arr;
  }


  static void sortByLen(Array<Clause*>& ca) { sortByLen(ca, 0, ca.size()-1); } 


  static string getOpAsString(const int& op)
  {
    if (op == OP_ADD)             return "OP_ADD";
    if (op == OP_REPLACE_ADDPRED) return "OP_REPLACE_ADDPRED";
    if (op == OP_REPLACE_REMPRED) return "OP_REPLACE_REMPRED";
    if (op == OP_REMOVE)          return "OP_REMOVE";
    if (op == OP_REPLACE)         return "OP_REPLACE";
    if (op == OP_NONE)            return "OP_NONE";
    return "";
  }

  
  ostream& print(ostream& out, const Domain* const& domain, 
                 const bool& withWt, const bool& asInt, 
                 const bool& withStrVar) const
    {
      
      return print(out,domain,withWt,asInt,withStrVar,false);
    }
  
  

  ostream& print(ostream& out, const Domain* const& domain, 
                 const bool& withWt, const bool& asInt, 
                 const bool& withStrVar, 
		 const bool& conjunction) const
  {
    if (withWt) out << wt_ << " ";

    Array<Predicate*> eqPreds;
    Array<Predicate*> internalPreds;
    for (int i = 0; i < predicates_->size(); i++)
    {
      if ((*predicates_)[i]->isEqualPred()) 
        eqPreds.append((*predicates_)[i]);
      else
      if ((*predicates_)[i]->isInternalPred()) 
        internalPreds.append((*predicates_)[i]);
      else
      {
        if (asInt)           (*predicates_)[i]->printAsInt(out);
        else if (withStrVar) (*predicates_)[i]->printWithStrVar(out, domain);
        else                 (*predicates_)[i]->print(out,domain);
	if (conjunction){
	  if (i < predicates_->size()-1 || !eqPreds.empty() ||
	      !internalPreds.empty()) out << " ^ ";
	}
	else{
	  if (i < predicates_->size()-1 || !eqPreds.empty() ||
	      !internalPreds.empty()) out << " v ";
	}
      }
    }

    for (int i = 0; i < eqPreds.size(); i++)
    {
      if (asInt)           eqPreds[i]->printAsInt(out);
      else if (withStrVar) eqPreds[i]->printWithStrVar(out,domain);
      else                 eqPreds[i]->print(out,domain);
      out << ((i != eqPreds.size()-1 || !internalPreds.empty())?" v ":"");      
    }

    for (int i = 0; i < internalPreds.size(); i++)
    {
      if (asInt)           internalPreds[i]->printAsInt(out);
      else if (withStrVar) internalPreds[i]->printWithStrVar(out,domain);
      else                 internalPreds[i]->print(out,domain);
      out << ((i!=internalPreds.size()-1)?" v ":"");      
    }

    return out;
  }

  ostream& printAsInt(ostream& out) const
  { return print(out, NULL, true, true, false); }


  ostream& printConjunction(ostream& out, const Domain* const & domain) const
    {
      return print(out, domain, false, false, false, true);
    }

  ostream& printWithoutWt(ostream& out, const Domain* const & domain) const
  { return print(out, domain, false, false, false); }

  ostream& 
  printWithoutWtWithStrVar(ostream& out, const Domain* const & domain) const
  { return print(out, domain, false, false, true); }

  ostream& printWithWtAndStrVar(ostream& out, const Domain* const& domain) const
  { return print(out, domain, true, false, true); }

  ostream& print(ostream& out, const Domain* const& domain) const
  { return print(out, domain, true, false, false); }
    
  ostream& printWithoutWtWithStrVarAndPeriod(ostream& out, 
                                              const Domain* const& domain) const
  {
    printWithoutWtWithStrVar(out, domain);
    if (isHardClause_) out << ".";
    return out;
  }


 private:

  static int comparePredicatesByIdAndTerms( const Predicate* const & l, 
					    const Predicate* const & r,
					    const Array<VarsGroundedType*>* const & vgtArr)
  {
    if (l->getId() < r->getId()) return -1;
    if (l->getId() > r->getId()) return 1;
    /*
    if (l->getSense() && !(r->getSense())) return -1;
    if (!(l->getSense()) && r->getSense()) return 1;
    */
    assert(l->getNumTerms() == r->getNumTerms());
    for (int i = 0; i < l->getNumTerms(); i++)
    {
      int lid = l->getTerm(i)->getId();
      int rid = r->getTerm(i)->getId();
      if (vgtArr && lid < 0 && rid < 0)
      {
        bool lbound = (*vgtArr)[-lid]->vars.size() > 1;
        bool rbound = (*vgtArr)[-rid]->vars.size() > 1;
        if (lbound && !rbound) return -1;
        if (!lbound && rbound) return 1;
      }
      if (lid > rid) return -1;
      if (lid < rid) return 1;
    }
    return 0;
  }


  void sortPredicatesByIdAndTerms(const int& l, const int& r,
				  const Array<VarsGroundedType*>* const & vgtArr)
  {
    if (l >= r) return;
    Predicate* tmp = (*predicates_)[l];
    (*predicates_)[l] = (*predicates_)[(l+r)/2];
    (*predicates_)[(l+r)/2] = tmp;

    int last = l;
    for (int i = l+1; i <= r; i++)
      if (comparePredicatesByIdAndTerms((*predicates_)[i],
                                                (*predicates_)[l], vgtArr) < 0)
      {
        ++last;
        tmp = (*predicates_)[last];
        (*predicates_)[last] = (*predicates_)[i];
        (*predicates_)[i] = tmp;
      }
    
    tmp = (*predicates_)[l];
    (*predicates_)[l] = (*predicates_)[last];
    (*predicates_)[last] = tmp;
    sortPredicatesByIdAndSenseAndTerms(l, last-1, vgtArr);
    sortPredicatesByIdAndSenseAndTerms(last+1, r, vgtArr);  
  }

  static int comparePredicatesByIdAndSenseAndTerms(const Predicate* const & l, 
						   const Predicate* const & r,
						   const Array<VarsGroundedType*>* const & vgtArr)
  {
    if (l->getId() < r->getId()) return -1;
    if (l->getId() > r->getId()) return 1;
    if (l->getSense() && !(r->getSense())) return -1;
    if (!(l->getSense()) && r->getSense()) return 1;
    assert(l->getNumTerms() == r->getNumTerms());
    for (int i = 0; i < l->getNumTerms(); i++)
    {
      int lid = l->getTerm(i)->getId();
      int rid = r->getTerm(i)->getId();
      if (vgtArr && lid < 0 && rid < 0)
      {
        bool lbound = (*vgtArr)[-lid]->vars.size() > 1;
        bool rbound = (*vgtArr)[-rid]->vars.size() > 1;
        if (lbound && !rbound) return -1;
        if (!lbound && rbound) return 1;
      }
      if (lid > rid) return -1;
      if (lid < rid) return 1;
    }
    return 0;
  }

  
  void sortPredicatesByIdAndSenseAndTerms(const int& l, const int& r,
                                 const Array<VarsGroundedType*>* const & vgtArr)
  {
    if (l >= r) return;
    Predicate* tmp = (*predicates_)[l];
    (*predicates_)[l] = (*predicates_)[(l+r)/2];
    (*predicates_)[(l+r)/2] = tmp;

    int last = l;
    for (int i = l+1; i <= r; i++)
      if (comparePredicatesByIdAndSenseAndTerms((*predicates_)[i],
                                                (*predicates_)[l], vgtArr) < 0)
      {
        ++last;
        tmp = (*predicates_)[last];
        (*predicates_)[last] = (*predicates_)[i];
        (*predicates_)[i] = tmp;
      }
    
    tmp = (*predicates_)[l];
    (*predicates_)[l] = (*predicates_)[last];
    (*predicates_)[last] = tmp;
    sortPredicatesByIdAndSenseAndTerms(l, last-1, vgtArr);
    sortPredicatesByIdAndSenseAndTerms(last+1, r, vgtArr);  
  }


    //l and r must have the same id and sense
  static int comparePredicatesByTerms(const Predicate* const & l,
                                      const Predicate* const & r)
  {
    assert(l->getId() == r->getId());
    assert(l->getSense() == r->getSense());
    for (int i = 0; i < l->getNumTerms(); i++)
    {
      const Term* lTerm =l->getTerm(i);
      const Term* rTerm =r->getTerm(i);
      int lTermType = lTerm->getType();
      int rTermType = rTerm->getType();
      if (lTermType == Term::VARIABLE && rTermType == Term::VARIABLE)
      {
        if (lTerm->getId() > rTerm->getId()) return -1;
        if (lTerm->getId() < rTerm->getId()) return 1;
      }
      else
      if (lTermType == Term::CONSTANT && rTermType == Term::CONSTANT)
      {
        if (lTerm->getId() < rTerm->getId()) return -1;
        if (lTerm->getId() > rTerm->getId()) return 1;
      }
      else
      if (lTermType == Term::VARIABLE && rTermType == Term::CONSTANT)
        return -1;
      else
      if (lTermType == Term::CONSTANT && rTermType == Term::VARIABLE)
        return 1;
      else
      {
        assert(false);
      }
    }
    return 0;
  }


    //The lth to rth predicates must have the same id and sense 
  void sortPredicatesByTerms(const int& l, const int& r)
  {
    if (l >= r) return;
    Predicate* tmp = (*predicates_)[l];
    (*predicates_)[l] = (*predicates_)[(l+r)/2];
    (*predicates_)[(l+r)/2] = tmp;
    
    int last = l;
    for (int i = l+1; i <= r; i++)
      if (comparePredicatesByTerms((*predicates_)[i],(*predicates_)[l]) < 0)
      {
        ++last;
        tmp = (*predicates_)[last];
        (*predicates_)[last] = (*predicates_)[i];
        (*predicates_)[i] = tmp;
      }
    
    tmp = (*predicates_)[l];
    (*predicates_)[l] = (*predicates_)[last];
    (*predicates_)[last] = tmp;
    sortPredicatesByTerms(l, last-1);
    sortPredicatesByTerms(last+1, r);  
  }


  bool noDirtyPredicates() const
  {
    for (int i = 0; i < predicates_->size(); i++)
      if ((*predicates_)[i]->isDirty()) return false;
    return true;
  }

  const Array<int>* getIntArrRep() 
  { if (dirty_) computeAndStoreIntArrRep(); return intArrRep_; }


  void computeAndStoreIntArrRep()
  {
    dirty_ = false;
    if (intArrRep_ == NULL) intArrRep_ = new Array<int>;
    else                    intArrRep_->clear();

    int numPred = predicates_->size();
    for (int i = 0; i < numPred; i++)
    {
      if ((*predicates_)[i]->getSense()) intArrRep_->append(1);
      else                               intArrRep_->append(0);
      (*predicates_)[i]->appendIntArrRep(*intArrRep_);      
    }
    hashCode_ = Hash::hash(*intArrRep_);
  }

  
  ///////////// functions for counting number of true groundings ////////////
 public:
  void addUnknownClauses(const Domain* const & domain, 
                         const Database* const& db, const int& gndPredIdx, 
                         const GroundPredicate* const & groundPred,
                         const AddGroundClauseStruct* const & agcs)
  {
    if (clausedebug >= 2)
    {
      cout << "In Clause::addUnknownClauses()" << endl;
    }
    createVarIdToVarsGroundedType(domain);
    if (gndPredIdx >= 0) groundPredVars(gndPredIdx, groundPred);
    countNumTrueGroundings(domain, db, true, false, gndPredIdx, groundPred, 
                           NULL, NULL, NULL, NULL, agcs, NULL, NULL, NULL,
                           true, false);
    restoreVars();
    deleteVarIdToVarsGroundedType();
  }


  void getUnknownClauses(const Domain* const & domain, 
                         const Database* const& db, const int& gndPredIdx, 
                         const GroundPredicate* const & groundPred,
                         const Predicate* const & gndPred,
                         Array<GroundClause*>* const& unknownGndClauses,
                         Array<Clause*>* const& unknownClauses)
  {
    createVarIdToVarsGroundedType(domain);
    if (gndPredIdx >= 0) 
    {
      if (groundPred)          groundPredVars(gndPredIdx, groundPred);
      else  { assert(gndPred); groundPredVars(gndPredIdx, (Predicate*)gndPred);}
    }
    countNumTrueGroundings(domain, db, true, false, gndPredIdx, groundPred, 
                           gndPred, unknownGndClauses, unknownClauses, NULL,
                           NULL, NULL, NULL, NULL, true, false);
    restoreVars();
    deleteVarIdToVarsGroundedType();
  }


  bool isSatisfiable(const Domain* const & domain, const Database* const& db,
                     const bool& hasUnknownPreds)
  {
    createVarIdToVarsGroundedType(domain);

    double i = countNumTrueGroundings(domain, db, hasUnknownPreds ,true,
                                      -1, NULL, NULL, NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, true, false);
    restoreVars();
    deleteVarIdToVarsGroundedType();
    return (i > 0);
  }

    //count the number of groundings of clause variables
  double getNumDistinctGroundings(const Domain* const & domain)
  {
    //cout << "In groundings" << endl;
    createVarIdToVarsGroundedType(domain);
    //cout << "Let's count" << endl;
    //double n = countNumGroundings(true);
    double n = countNumGroundings();
    //cout << "NOw for the delete" << endl;
    deleteVarIdToVarsGroundedType();
    return n;
  }

    //count the number of groundings of clause variables
  double getNumGroundings(const Domain* const & domain)
  {
    //cout << "In groundings" << endl;
    createVarIdToVarsGroundedType(domain);
    //cout << "Let's count" << endl;
    double n = countNumGroundings();
    //cout << "NOw for the delete" << endl;
    deleteVarIdToVarsGroundedType();
    return n;
  }

  static int pow(int base, int exp){
	  int result = 1;
	  for(int ix = 0; ix < exp; ix++){
		  result = result * base;
	  }
	  return result;
  }
  Array<double>* templateGroundings(const Domain* const & domain, const Database* const & db){
    int templateDebug = 0;
    ArraysAccessor<int> acc;
    IntHashArray seen;
    Array<Array<int>*> vars;
    Array<double>* counts = new Array<double>;
    counts->growToSize(pow(2,predicates_->size()));
    for(int ix = 0; ix < counts->size(); ix++){
      (*counts)[ix] = 1;
    }
    vars.growToSize(predicates_->size());
    for(int ix = 0; ix < predicates_->size(); ix++){
      Array<int>* tmp = new Array<int>;
      
      for(int jx = 0; jx < (*predicates_)[ix]->getNumTerms(); jx++){
	const Term* term = (*predicates_)[ix]->getTerm(jx);
	if (templateDebug > 2){
	  cout << "ix = " << ix << " jx = " << jx << " term id = " << term->getId() << endl;
	}
	tmp->append(term->getId());
	if (!seen.contains(term->getId())){
	  seen.append(term->getId());
	  int termType = (*predicates_)[ix]->getTermTypeAsInt(jx);
	  const Array<int>* types = domain->getConstantsByType(termType);
	  if (templateDebug > 3){
	    for(int kx = 0; kx < types->size(); kx++){
	      cout << (*types)[kx] << " ";
	    }
	    cout << endl;
	  }
	  acc.appendArray(types);
	}
      }
      vars[ix] = tmp;
    }
    //cout << "Vars size " << vars.size() << endl;
    Array<int> grounding;
    //Array<bool> truth;
    //truth.growToSize(predicates_->size());
    Array<Predicate*> newPreds;
    newPreds.growToSize(predicates_->size());
    for(int ix = 0; ix < predicates_->size(); ix++){
      newPreds[ix] = new Predicate(*(*predicates_)[ix]);
    }
    int loop = 0;
    while (acc.hasDistinctNextCombination()){
      acc.getDistinctNextCombination(grounding);
      int index = 0;
      loop++;
      for(int ix = 0; ix < newPreds.size(); ix++){
	for(int jx = 0; jx < newPreds[ix]->getNumTerms(); jx++){
	  Array<int>* tmpVar = vars[ix];
	  int idx = -1 - (*tmpVar)[jx];
	  if (templateDebug > 1){
	    cerr << "ix = " << ix << " jx = " << jx << " idx " << idx << " value " << grounding[idx] << " tmpVar " << (*tmpVar)[jx] << endl;
	  }
	  newPreds[ix]->setTermToConstant(jx, grounding[idx]);
	}
	TruthValue tv = db->getValue(newPreds[ix]);
	if (templateDebug > 1){
	  newPreds[ix]->print(cout, domain);
	  cout << " Truth Value " << tv << endl;
	}
	if (tv == TRUE){
	  index = index + pow(2,(predicates_->size() - ix - 1)); 
	  //truth[ix] = true;	
	}
	/*
	  else{
	  truth[ix] = false;
	  }
	*/
      }
      (*counts)[index]++;
      if (templateDebug > 1){
	cout << "Loop " << loop << " Index " << index;
	cout << " Count Size " << counts->size();
	cout << " Value " << (*counts)[index] <<  endl;
      }
    }
    //cerr << "Loop = " << loop << endl;
    //newPreds.deleteItemsAndClear();
    if (templateDebug > 0){
      cout << "Final Counts: ";
      for(int ix = 0; ix < counts->size(); ix++){
	cout << (*counts)[ix] << " ";
      }
      cout << endl;
    }
    return counts;
  }
  
  bool arraysEqual(Array<int>* a1, Array<int>* a2){
    if (a1->size() != a2->size()){
      return false;
    }
    for(int ix = 0; ix < a1->size(); ix++){
      if ((*a1)[ix] != ((*a2)[ix])){
	return false;
      }
    } 
    return true;
  }

  bool arraysEqualProj(Array<int>* a1, Array<int>* grounding,
		       Array<int>* pos){
    if (a1->size() != pos->size()){
      return false;
    }
    for(int ix = 0; ix < a1->size(); ix++){
      int indx = (*pos)[ix];
      if ((*a1)[ix] != ((*grounding)[indx])){
	return false;
      }
    } 
    return true;
  }
  
  

  //  Array<double>* allTemplateGroundings(const Domain* const & domain, const Database* const & db, Array<Array<double>*>* d1Counts, Array<Array<double>*>* d2Counts, Array<bool>* d1Sub, Array<bool>* d2Sub, int templateSize){
  Array<double>* allTemplateGroundings(const Domain* const & domain, const Database* const & db, Array<Array<double>*>* d1Counts, Array<Array<double>*>* d2Counts, Array<Array<int>*>* d1Idx, Array<Array<int>*>* d2Idx, int templateSize){
    int templateDebug = 0;
    ArraysAccessor<int> acc;
    IntHashArray seen;
    Array<Array<int>*> vars;
    Array<double>* counts = new Array<double>;
    counts->growToSize(pow(2,predicates_->size()));
    double initValue = 0;//1.0 /  getNumDistinctGroundings(domain);
    for(int ix = 0; ix < counts->size(); ix++){
      (*counts)[ix] = initValue;
    }
    Array<Array<int>*> d1FirstGround;
    Array<Array<int>*> d2FirstGround;// = new Array<Array<double>*>;
    d1FirstGround.growToSize(d1Counts->size());
    d2FirstGround.growToSize(d2Counts->size());
    Array<Array<int>*> d1Vars;
    Array<Array<int>*> d2Vars;// = new Array<Array<double>*>;
    d1Vars.growToSize(d1Counts->size());
    d2Vars.growToSize(d2Counts->size());
    Array<double> d1FirstGroundCnt;
    Array<double> d2FirstGroundCnt;
    d1FirstGroundCnt.growToSize(d1Counts->size(),0);
    d2FirstGroundCnt.growToSize(d2Counts->size(),0);
    for(int ix = 0; ix < d1Idx->size(); ix++){
      Array<int>* part = (*d1Idx)[ix];
      Array<int>* tmpPos = new Array<int>;
      IntHashArray seenVars; 
      //cout << "ix: " << ix << " ";
      for(int jx = 0; jx< part->size(); jx++){
	int predIdx = (*part)[jx];
	//cout << "PredIdx = " << predIdx << " ";
	for(int kx = 0; kx < (*predicates_)[predIdx]->getNumTerms(); kx++){
	  const Term* term = (*predicates_)[predIdx]->getTerm(kx);
	  
	  if (!seenVars.contains(term->getId())){
	    seenVars.append(term->getId());
	    tmpPos->append(-1 - term->getId());
	  }
	}
      }

      
      //cout << endl;
      /*
      cout << "D1 Vars: " << ix << ": ";
      for(int vx = 0; vx < tmpPos->size(); vx++){
	cout << (*tmpPos)[vx] << " ";
      }
      cout << endl; 
      */
      d1FirstGround[ix] = new Array<int>;
      d1FirstGround[ix]->growToSize(tmpPos->size(),-1);	  
      d1Vars[ix] = tmpPos;
      
	//}
	//for(int ix = 0; ix < d2Idx->size(); ix++){
      part = (*d2Idx)[ix];
      tmpPos = new Array<int>;
      seenVars.clear(); //IntHashArray seenVars; 
      //cout << "ix: " << ix << " ";
      for(int jx = 0; jx< part->size(); jx++){
	int predIdx = (*part)[jx];
	//cout << "PredIdx = " << predIdx << " ";
	for(int kx = 0; kx < (*predicates_)[predIdx]->getNumTerms(); kx++){
	  const Term* term = (*predicates_)[predIdx]->getTerm(kx);
	  
	  if (!seenVars.contains(term->getId())){
	    seenVars.append(term->getId());
	    tmpPos->append(-1 - term->getId());
	  }
	}
      }
      /*
      cout << endl;
      cout << "D2 Vars: " << ix << ": ";
      for(int vx = 0; vx < tmpPos->size(); vx++){
	cout << (*tmpPos)[vx] << " ";
      }
      cout << endl << endl;
      */
      d2FirstGround[ix] = new Array<int>;
      d2FirstGround[ix]->growToSize(tmpPos->size(),-1);	  
      d2Vars[ix] = tmpPos;
      
    }
    
    vars.growToSize(predicates_->size());
    for(int ix = 0; ix < predicates_->size(); ix++){
      Array<int>* tmp = new Array<int>;
      
      for(int jx = 0; jx < (*predicates_)[ix]->getNumTerms(); jx++){
	const Term* term = (*predicates_)[ix]->getTerm(jx);
	if (templateDebug > 2){
	  //cout << "ix = " << ix << " jx = " << jx << " term id = " << term->getId() << endl;
	}
	//if (term->getId() <= 0){
	  tmp->append(term->getId());
	  //}
	if (!seen.contains(term->getId())){
	  cout << "Term id: " << term->getId() << " ix " << ix << " jx " << jx << "\n";
	  seen.append(term->getId());
	  if (term->getId() > 0){
	    
	  }
	  else{
	    int termType = (*predicates_)[ix]->getTermTypeAsInt(jx);
	    const Array<int>* types = domain->getConstantsByType(termType);
	    if (templateDebug > 3){
	      for(int kx = 0; kx < types->size(); kx++){
		//cout << (*types)[kx] << " ";
	      }
	      //	    cout << endl;
	    }
	    acc.appendArray(types);
	  }
	}
      }
      vars[ix] = tmp;
    }
    //cout << "Vars size " << vars.size() << endl;
    Array<int> grounding;
    Array<bool> truth;
    truth.growToSize(predicates_->size(), false);
    Array<Predicate*> newPreds;
    newPreds.growToSize(predicates_->size());
    for(int ix = 0; ix < predicates_->size(); ix++){
      newPreds[ix] = new Predicate(*((*predicates_)[ix]));
    }
    int loop = 0;
    Array<int> prev;
    prev.growToSize(acc.getNumArrays(),-1);
    Array<int> offset;
    offset.growToSize(newPreds.size(), 0);
    Array<bool> hasChanged;
    hasChanged.growToSize(newPreds.size(), true);
    while (acc.hasDistinctNextCombination()){
      acc.getDistinctNextCombination(grounding);
      /*
	cerr << "Grounding: ";
	for(int xx = 0; xx < grounding.size(); xx++){
	cerr << grounding[xx] << " ";
	}
	cerr << endl;
      */
      if (loop == 0){
	loop++;
	for(int ix = 0; ix < d1Vars.size(); ix++){
	  Array<int>* curVars = d1Vars[ix];
	  //cout << "ix " << ix << " ground: ";
	  for(int jx = 0; jx < curVars->size(); jx++){
	    //(*(d1FirstGround[ix]))[jx] = grounding[-1 - (*curVars)[jx]];
	    (*(d1FirstGround[ix]))[jx] = grounding[(*curVars)[jx]];
	    //cout << (*(d1FirstGround[ix]))[jx] << " ";
	  }
	  //cout << ": ";
	  curVars = d2Vars[ix];
	  if (curVars == NULL){
	    cerr << "Cur Vars is null\n";
	  }
	  for(int jx = 0; jx < curVars->size(); jx++){
	    //(*(d2FirstGround[ix]))[jx] = grounding[-1 -(*curVars)[jx]];
	    (*(d2FirstGround[ix]))[jx] = grounding[(*curVars)[jx]];
	    //cout << (*(d2FirstGround[ix]))[jx] << " ";
	  }
	  //cout << endl;
	}
      }
 
      for(int ix = 0; ix < d1Vars.size(); ix++){
	if (arraysEqualProj(d1FirstGround[ix], &grounding, d1Vars[ix])){
	  d1FirstGroundCnt[ix]++;
	  /*
	    cout << "D1 ix = " << ix << " loop = " << loop << " ground: ";
	    for(int gx = 0; gx < grounding.size(); gx++){
	    cout << grounding[gx] << " ";
	    }
	  cout << " count " << d1FirstGroundCnt[ix] << endl;
	  */
	}
	if (arraysEqualProj(d2FirstGround[ix], &grounding, d2Vars[ix])){

	  d2FirstGroundCnt[ix]++;
	  /*
	  cout << "D2 ix = " << ix << " loop = " << loop;
	  for(int gx = 0; gx < grounding.size(); gx++){
	    cout << grounding[gx] << " ";
	  }
	  cout << " Count " << d2FirstGroundCnt[ix] << endl;
	  */
	}
      }
	    /*
	      bool arraysEqualProj(Array<int>* a1, Array<int>*& grounding,
	      Array<int>* pos){
	    */
      
      int index = 0;

      for(int ix = 0; ix < newPreds.size(); ix++){
	bool changed = false;
	for(int jx = 0; jx < newPreds[ix]->getNumTerms(); jx++){
	  //flag - here
	  Array<int>* tmpVar = vars[ix];
	  if ((*tmpVar)[jx] < 0){
	    int idx = -1 - (*tmpVar)[jx];
	    
	    if (templateDebug > 1){ //|| true){//false){
	      cerr << "ix = " << ix << " jx = " << jx << " idx " << idx << " value " << grounding[idx] << " tmpVar " << (*tmpVar)[jx] << endl;
	    }
	    if (prev[idx] != grounding[idx]){
	      //cerr << "Setting " << jx << " to " << grounding[idx] << endl;
	      newPreds[ix]->setTermToConstant(jx, grounding[idx]);
	      changed = true;
	      //prev[idx] = grounding[idx];
	    }
	  }
	}
	if (changed == true){
	  TruthValue tv = db->getValue(newPreds[ix]);
	  hasChanged[ix] = true;
	  if (tv == TRUE){
	    offset[ix] = pow(2,(predicates_->size() - ix - 1));
	    index = index + pow(2,(predicates_->size() - ix - 1)); 
	    truth[ix] = true;
	    //int d2Pos = newPreds.size() - ix - 1;
	    //cout << "true: ix = " << ix << " d2Pos " << d2Pos << endl; 
	    //(*((*d2Counts)[d2Pos]))[1]++;
	    //update singles
	  }
	  else{
	    //int d2Pos = newPreds.size() - ix - 1;
	    //cout << "false: ix = " << ix << " d2Pos " << d2Pos << endl; 
	    //
	    offset[ix] = 0;
	    //update singles 
	    truth[ix] = false;
	  }
	  if (templateDebug > 1){
	    newPreds[ix]->print(cout, domain);
	    cout << " Truth Value " << tv << endl;
	   
	  }
	}
	else{
	  hasChanged[ix] = false;
	  index = index + offset[ix];
      	}
	//newPreds[ix]->print(cout, domain);
	//cout << " " ;
      }
      //cout << endl;
      for(int ix = 0; ix < grounding.size(); ix++){
	prev[ix] = grounding[ix];
      }
      (*counts)[index]++;
      //int subidx = 0;
      /*
      if (templateSize == 2){
	(*((*d1Counts)[0]))[truth[0]]++;
	(*((*d2Counts)[0]))[truth[1]]++;
      }
      else{
	for(int ix = 0; ix < newPreds.size(); ix++){
	  int d2Pos = newPreds.size() - ix - 1;
	  (*((*d2Counts)[d2Pos]))[truth[ix]]++;
	}
      }
      */
      for(int ix =0 ; ix < d1Counts->size(); ix++){
	int index = 0;
	Array<int>* pidx = (*d1Idx)[ix];
	for(int jx = 0; jx < pidx->size(); jx++){
	  index += truth[(*pidx)[jx]] * pow(2, jx);
	}
	(*((*d1Counts)[ix]))[index]++;
	index = 0;
	pidx = (*d2Idx)[ix];
	for(int jx = 0; jx < pidx->size(); jx++){
	  index += truth[(*pidx)[jx]] * pow(2, jx);
	}
	(*((*d2Counts)[ix]))[index]++;
      }

      //}
      if (templateDebug > 1){
	cout << "Loop " << loop << " Index " << index;
	cout << " Count Size " << counts->size();
	cout << " Value " << (*counts)[index] <<  endl;
      }
      //loop++;
    }
    //cerr << "Loop = " << loop << endl;
    //newPreds.deleteItemsAndClear();
    if (templateDebug > 0){
      cout << "Final Counts: ";
      for(int ix = 0; ix < counts->size(); ix++){
	cout << (*counts)[ix] << " ";
      }
      cout << endl;
    }
    for(int ix = 0; ix < d1Counts->size(); ix++){
      Array<double>* tmpArr = (*d1Counts)[ix];
      for(int jx = 0; jx < tmpArr->size(); jx++){
	
	//cout << "D1 " << jx << " " << (*tmpArr)[jx] << " / ";	
	(*tmpArr)[jx] = (*tmpArr)[jx] / d1FirstGroundCnt[ix];// + 1;

	//cout << d1FirstGroundCnt[ix] << " = " << (*tmpArr)[jx] << endl;	

      }
      tmpArr = (*d2Counts)[ix];
      for(int jx = 0; jx < tmpArr->size(); jx++){
	/*
	cout << "D2 " << jx << " " << (*tmpArr)[jx] << " / " 
	     << d2FirstGroundCnt[ix] << endl;
	*/
	(*tmpArr)[jx] = (*tmpArr)[jx] / d2FirstGroundCnt[ix];// + 1;
	//cout << " = " << (*tmpArr)[jx] << endl;
      }
      //cout << endl;
    }
    d1FirstGround.deleteItemsAndClear();
    d2FirstGround.deleteItemsAndClear();
    d1Vars.deleteItemsAndClear();
    d2Vars.deleteItemsAndClear();
    //d1FirstGroundCnt.deleteItemsAndClear();
    //d2FirstGroundCnt.deleteItemsAndClear();
    newPreds.deleteItemsAndClear();
    vars.deleteItemsAndClear();
    return counts;
  }


  double getNumTrueGroundingsAnd(const Domain* const & domain,
                              const Database* const & db,
                              const bool& hasUnknownPreds)
  {
    createVarIdToVarsGroundedType(domain);
    double n = countAndNumTrueGroundings(domain, db, hasUnknownPreds, false, 
                                      -1, NULL, NULL, NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, true, false);
    restoreVars();
    deleteVarIdToVarsGroundedType();
    return n;
  }


  double getNumTrueGroundings(const Domain* const & domain,
                              const Database* const & db,
                              const bool& hasUnknownPreds)
  {
    createVarIdToVarsGroundedType(domain);
    double n = countNumTrueGroundings(domain, db, hasUnknownPreds, false, 
                                      -1, NULL, NULL, NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, true, false);
    restoreVars();
    deleteVarIdToVarsGroundedType();
    return n;
  }


  void getNumTrueUnknownGroundings(const Domain* const & domain,
                                   const Database* const & db,
                                   const bool& hasUnknownPreds,
                                   double& numTrue, double& numUnknown)
  {
    createVarIdToVarsGroundedType(domain);
    numTrue = countNumTrueGroundings(domain, db, hasUnknownPreds, false, 
                                     -1, NULL, NULL, NULL, NULL, &numUnknown,
                                     NULL, NULL, NULL, NULL, true, false);
    restoreVars();
    deleteVarIdToVarsGroundedType();
  }


  double getNumUnknownGroundings(const Domain* const & domain,
                                 const Database* const & db,
                                 const bool& hasUnknownPreds)
  {
    double numTrue, numUnknown;
    getNumTrueUnknownGroundings(domain, db, hasUnknownPreds,numTrue,numUnknown);
    return numUnknown;
  }
  
  void getNumTrueFalseUnknownGroundings(const Domain* const & domain,
                                        const Database* const & db,
                                        const bool& hasUnknownPreds,
                                        double& numTrue, double& numFalse,
                                        double& numUnknown)
  {
    getNumTrueUnknownGroundings(domain, db, hasUnknownPreds,numTrue,numUnknown);
    numFalse = getNumGroundings(domain) - numTrue - numUnknown;
    assert(numTrue >= 0);
    assert(numUnknown >= 0);
    assert(numFalse >= 0);
  }


    //Count the difference betwe.en the number of true groundings of the clause
    //when gndPred is held to the opposite of its actual value and when held to
    //its actual value. 
  Array<double>* countJesse(Predicate* const & gndPred, 
                                    const Domain* const & domain,
                                    Database* const & db,
                                    const bool& hasUnknownPreds,
                                    const bool& sampleClauses,
                                    const int& combo)
  {
    assert(gndPred->isGrounded());

      //store the indexes of the predicates that can be grounded as gndPred
      //create mapping of variable ids (e.g. -1) to variable addresses,
      //note whether they have been grounded, and store their types
    createVarIdToVarsGroundedType(domain);
    //cout << endl;
    if (false){
      printWithWtAndStrVar(cout, domain);
      cout << " : ";
      gndPred->print(cout, domain);
      cout << endl;
    }
    TruthValue actual = db->getValue(gndPred);
    assert(actual == TRUE || actual == FALSE);
    TruthValue opp = (actual == TRUE) ? FALSE : TRUE;
    bool flipped = false;

      //count # true groundings when gndPred is held to its actual value
    double numTrueGndActual = countNumTrueGroundings(domain, db, false, false,
						     -1, NULL, NULL, NULL, NULL, NULL, NULL,
						     NULL, NULL, NULL, true, false);
      
      /*
          cnt = countNumTrueGroundings(domain, db, hasUnknownPreds, false,
                                       -1, NULL, NULL, NULL, NULL, NULL, NULL,
                                       NULL, NULL, NULL, true, false);
 

      countNumTrueGroundingsForAllComb(gndPredIndexes, gndPred, actual, flipped,
                                       domain, hasUnknownPreds, sampleClauses);
      */
    //cout << "numTrueGndActual = " << numTrueGndActual << endl;
    
      //count # true groundings when gndPred is held to opposite value
    double numTrueGndOpp = 0.0;
    
    // Pred not in block: Count gndings with pred flipped
    flipped = true;
    
    //set gndPred to have the opposite of its actual value
    db->setValue(gndPred, opp);
    
    numTrueGndOpp += countNumTrueGroundings(domain, db, false, false,
					    -1, NULL, NULL, NULL, NULL, NULL, NULL,
					    NULL, NULL, NULL, true, false);
    /*
      countNumTrueGroundingsForAllComb(gndPredIndexes, gndPred, opp, flipped,
      domain, hasUnknownPreds, sampleClauses);
    */
    db->setValue(gndPred, actual);
   
    //cout << "numTrueGndOpp    = " << numTrueGndOpp << endl;

    Array<double>* jesse = new Array<double>(2);
    (*jesse)[0] =  numTrueGndOpp;
    (*jesse)[1] =  numTrueGndActual;
    deleteVarIdToVarsGroundedType();
    return jesse; //numTrueGndOpp - numTrueGndActual;
  }



    //Count the difference between the number of true groundings of the clause
    //when gndPred is held to the opposite of its actual value and when held to
    //its actual value. 
  double countDiffNumTrueGroundings(Predicate* const & gndPred, 
                                    const Domain* const & domain,
                                    Database* const & db,
                                    const bool& hasUnknownPreds,
                                    const bool& sampleClauses,
                                    const int& combo)
  {
    assert(gndPred->isGrounded());

      //store the indexes of the predicates that can be grounded as gndPred
    Array<int> gndPredIndexes;

    for (int i = 0; i < predicates_->size(); i++)
    {
      if ((*predicates_)[i]->canBeGroundedAs(gndPred)) gndPredIndexes.append(i);
    }
      //create mapping of variable ids (e.g. -1) to variable addresses,
      //note whether they have been grounded, and store their types
    createVarIdToVarsGroundedType(domain); 

    TruthValue actual = db->getValue(gndPred);
    assert(actual == TRUE || actual == FALSE);
    TruthValue opp = (actual == TRUE) ? FALSE : TRUE;
    bool flipped = false;
    //cerr << "CH 1 \n";
      //count # true groundings when gndPred is held to its actual value
    double numTrueGndActual = 
      countNumTrueGroundingsForAllComb(gndPredIndexes, gndPred, actual, flipped,
                                       domain, hasUnknownPreds, sampleClauses);
    //cout << "numTrueGndActual = " << numTrueGndActual << endl;
    
      //count # true groundings when gndPred is held to opposite value
    double numTrueGndOpp = 0.0;
    //cerr << "CH 2 \n";
    int blockIdx = domain->getBlock(gndPred);
    if (blockIdx >= 0)
    {
        // Pred in block: We have to look at combination c
        // of other preds in block
      assert(combo < domain->getBlockSize(blockIdx));
      
      const Predicate* oldTruePred = domain->getTruePredInBlock(blockIdx);

      int oldTrueOne = 0;
      if (oldTruePred)
        oldTrueOne = domain->getIndexOfPredInBlock(oldTruePred, blockIdx);
//cout << "oldTrueOne " << oldTrueOne << endl;
      assert(oldTrueOne > -1);

      int newTrueOne = (oldTrueOne <= combo) ? combo + 1 : combo;
//cout << "newTrueOne " << newTrueOne << endl;
      const Predicate* newTruePred =
        domain->getPredInBlock(newTrueOne, blockIdx);

      bool isOldTrue =
        oldTruePred ? gndPred->same((Predicate*)oldTruePred) : false;
      bool isNewTrue = gndPred->same((Predicate*)newTruePred);      
      TruthValue newTV =
        ((!isOldTrue && isNewTrue) || (isOldTrue && !isNewTrue)) ? opp : actual;
      
      if (oldTruePred) assert(db->getValue(oldTruePred) == TRUE);
      assert(db->getValue(newTruePred) == FALSE);
      
      if (oldTruePred) db->setValue(oldTruePred, FALSE);
      db->setValue(newTruePred, TRUE);
      //cerr << "CH 3 \n";
      numTrueGndOpp +=
        countNumTrueGroundingsForAllComb(gndPredIndexes, gndPred, newTV, 
                                         newTV != actual, domain,
                                         hasUnknownPreds, sampleClauses);

      //numTrueGndOpp +=
      //  countNumTrueGroundingsForAllComb(gndPredIndexes, (*block)[oldTrueOne],
      //                                   opp, flipped, domain, hasUnknownPreds,
      //                                   sampleClauses);
      //numTrueGndOpp +=
      //  countNumTrueGroundingsForAllComb(gndPredIndexes, (*block)[newTrueOne],
      //                                   opp, flipped, domain, hasUnknownPreds,
      //                                   sampleClauses);
      
      if (oldTruePred) db->setValue(oldTruePred, TRUE);
      db->setValue(newTruePred, FALSE);
      
      delete newTruePred;
    }
    else
    {
        // Pred not in block: Count gndings with pred flipped
      flipped = true;

        //set gndPred to have the opposite of its actual value
      db->setValue(gndPred, opp);

      numTrueGndOpp +=
        countNumTrueGroundingsForAllComb(gndPredIndexes, gndPred, opp, flipped,
                                        domain, hasUnknownPreds, sampleClauses);

      db->setValue(gndPred, actual);
    }
    //cout << "numTrueGndOpp    = " << numTrueGndOpp << endl;

    deleteVarIdToVarsGroundedType();
    return numTrueGndOpp - numTrueGndActual;
  }


 private:
  static void addVarId(Term* const & t, const int& typeId, 
                const Domain* const & domain,                
                Array<VarsGroundedType*>*& vgtArr)
  {
    int id = -(t->getId());
    assert(id > 0);
    if (id >= vgtArr->size()) vgtArr->growToSize(id+1,NULL);
    VarsGroundedType*& vgt = (*vgtArr)[id];
    if (vgt == NULL) 
    {
      vgt = new VarsGroundedType; 
      // vgt->isGrounded init to false
      vgt->typeId = typeId;
      assert(vgt->typeId >= 0);
      vgt->numGndings = domain->getNumConstantsByType(vgt->typeId);
      assert(vgt->numGndings > 0);
    }
    assert(typeId == vgt->typeId);
    vgt->vars.append(t);
  }
  
  void createVarIdToVarsGroundedType(const Domain* const & domain,
                                     Array<VarsGroundedType*>*& vgtArr) const
  {    
      //for each predicate 
    for (int i = 0; i < predicates_->size(); i++)
    {
      Predicate* p = (*predicates_)[i];
        //for each variable of the predicate
      for (int j = 0; j < p->getNumTerms(); j++)
      {
        Term* t = (Term*) p->getTerm(j);
        if (t->getType() == Term::VARIABLE)
          addVarId(t, p->getTermTypeAsInt(j), domain, vgtArr);
        else
        if (t->getType() == Term::FUNCTION)
        {
          cout << "Clause::createVarIdToVarsGroundedType() not expecting a "
               << "FUNCTION term" << endl;
          exit(-1);
        }
      }// for each variable of the predicate
    } // for each predicate
  }


  void createVarIdToVarsGroundedType(const Domain* const & domain)
  {    
    //cout << "Call delete create" << endl;
    deleteVarIdToVarsGroundedType();
    //cout << "Deleted" << endl;
    varIdToVarsGroundedType_ = new Array<VarsGroundedType*>;
    //cout << "new array" << endl;
    createVarIdToVarsGroundedType(domain, varIdToVarsGroundedType_);
    //cout << "returning" << endl;
  }


  static void deleteVarIdToVarsGroundedType(Array<VarsGroundedType*>*& vgtArr)
  {
    for (int i = 0; i < vgtArr->size(); i++)
      if ((*vgtArr)[i]) delete (*vgtArr)[i];
    delete vgtArr;
    vgtArr = NULL;
  }


  void deleteVarIdToVarsGroundedType()
  {
    if (varIdToVarsGroundedType_)
      deleteVarIdToVarsGroundedType(varIdToVarsGroundedType_);
  }


  void getVarOrder(IntHashArray& varAppearOrder) const
  {
    //cerr << "Get Var Order\n";
    varAppearOrder.clear();
    //cerr << "Get Var Order Preds.size = ";
    //cerr << predicates_->size() << endl;
    for (int i = 0; i < predicates_->size(); i++)
    {
      //cerr << "Pred loop " << i << endl;
      const Predicate* p = (*predicates_)[i];
      //p->printAsInt(cerr);
      //cerr << endl;
      for (int j = 0; j < p->getNumTerms(); j++)
      {
        const Term* t = p->getTerm(j);
        if (t->getType() == Term::VARIABLE)
        {
          int id = -(t->getId());
          assert(id > 0);
          varAppearOrder.append(id);
        }
      }
    }
  }


  void createVarIdToVarsGroundedType(Array<VarsGroundedType*>*& vgtArr)
  {    
      //for each predicate 
    for (int i = 0; i < predicates_->size(); i++)
    {
      Predicate* p = (*predicates_)[i];
        //for each variable of the predicate
      for (int j = 0; j < p->getNumTerms(); j++)
      {
        Term* t = (Term*) p->getTerm(j);
        if (t->getType() == Term::VARIABLE)
        {
          assert(t->getId()<0);
          int id = -(t->getId());
          if (id >= vgtArr->size()) vgtArr->growToSize(id+1,NULL);
          VarsGroundedType*& vgt = (*vgtArr)[id];
          if (vgt == NULL)  vgt = new VarsGroundedType; 
          vgt->vars.append(t);
        }
        assert(t->getType() != Term::FUNCTION);
      }// for each variable of the predicate
    } // for each predicate    
  }
  

  void createVarConstIdToTerms(Array<Array<Term*>*>*& varIdToTerms,
                               hash_map<int,Array<Term*>*>*& constIdToTerms)
  {    
    for (int i = 0; i < predicates_->size(); i++) //for each predicate 
    {
      Predicate* p = (*predicates_)[i];
      for (int j = 0; j < p->getNumTerms(); j++) //for each term of predicate
      {
        Term* t = (Term*) p->getTerm(j);
        if (t->getType() == Term::VARIABLE)
        {
          if (varIdToTerms == NULL) continue;
          int id = -(t->getId());
          if (id >= varIdToTerms->size()) varIdToTerms->growToSize(id+1,NULL);
          Array<Term*>*& termArr = (*varIdToTerms)[id];
          if (termArr == NULL) termArr = new Array<Term*>;             
          termArr->append(t);
        }
        else
        if (t->getType() == Term::CONSTANT)
        {
          if (constIdToTerms == NULL) continue;
          int id = t->getId();
          Array<Term*>* termArr;
          hash_map<int,Array<Term*>*>::iterator it = constIdToTerms->find(id);
          if (it == constIdToTerms->end()) 
          {
            termArr = new Array<Term*>;
            (*constIdToTerms)[id] = termArr;
          }
          else
            termArr = (*it).second;
          termArr->append(t);
        }
        else
        if (t->getType() == Term::FUNCTION)
        {
          cout << "Clause::createVarIdToTerms() not expecting a "
               << "FUNCTION term" << endl;
          exit(-1);
        }
      }// for each variable of the predicate
    } // for each predicate
  }


  void deleteVarConstIdToTerms(Array<Array<Term*>*>*& varIdToTerms,
                               hash_map<int,Array<Term*>*>*&  constIdToTerms)
  {
    if (varIdToTerms)
    {
      for (int i = 0; i < varIdToTerms->size(); i++)
        if ((*varIdToTerms)[i]) delete (*varIdToTerms)[i];
      delete varIdToTerms;
      varIdToTerms = NULL;
    }
    if (constIdToTerms)
    {
      hash_map<int,Array<Term*>*>::iterator it = constIdToTerms->begin();
      for (; it != constIdToTerms->end(); it++) delete (*it).second;
      delete constIdToTerms;
      constIdToTerms = NULL;
    }
  }


    //Predicate at position predIdx must be able to be grounded as gndPred.
    //Call Predicate::canBeGroundedAs() to check this.
    //After one or more invocation of this function restoreVars() should
    //be called to restore the variables in the clause to their original 
    //values. Since the variables will be restored later, the dirty_ bit is 
    //not set.
  void groundPredVars(const int& predIdx, Predicate* const& gndPred,
                      Array<VarsGroundedType*>*& vgtArr) const
  {
    assert((*predicates_)[predIdx]->canBeGroundedAs(gndPred));
    assert(predIdx < predicates_->size());
    assert(gndPred->isGrounded());

    const Predicate* pred = (*predicates_)[predIdx];
    for (int i = 0; i < pred->getNumTerms(); i++)
    {
      const Term* t = pred->getTerm(i);
      if (t->getType() == Term::VARIABLE)
      {
        int constId = gndPred->getTerm(i)->getId();
        VarsGroundedType* vgt = (*vgtArr)[-t->getId()];
        Array<Term*>& vars = vgt->vars;
        for (int j = 0; j < vars.size(); j++)  vars[j]->setId(constId);
        vgt->isGrounded = true;
      }
      assert(t->getType() != Term::FUNCTION);
    }
  }


    //Predicate at position predIdx must be able to be grounded as gndPred.
    //Call Predicate::canBeGroundedAs() to check this.
    //After one or more invocation of this function restoreVars() should
    //be called to restore the variables in the clause to their original 
    //values. Since the variables will be restored later, the dirty_ bit is 
    //not set.
  void groundPredVars(const int& predIdx, 
                      const GroundPredicate* const& gndPred) const
  {
    assert(varIdToVarsGroundedType_);
    assert((*predicates_)[predIdx]->canBeGroundedAs(gndPred));
    assert(predIdx < predicates_->size());

    const Predicate* pred = (*predicates_)[predIdx];
    for (int i = 0; i < pred->getNumTerms(); i++)
    {
      const Term* t = pred->getTerm(i);
      if (t->getType() == Term::VARIABLE)
      {
        int constId = gndPred->getTermId(i);
        VarsGroundedType* vgt = (*varIdToVarsGroundedType_)[-t->getId()];
        Array<Term*>& vars = vgt->vars;
        for (int j = 0; j < vars.size(); j++)  vars[j]->setId(constId);
        vgt->isGrounded = true;
      }
      assert(t->getType() != Term::FUNCTION);
    }
  }


    //Predicate at position predIdx must be able to be grounded as gndPred.
    //Call Predicate::canBeGroundedAs() to check this.
    //After one or more invocation of this function restoreVars() should
    //be called to restore the variables in the clause to their original 
    //values. Since the variables will be restored later, the dirty_ bit is 
    //not set.
  void groundPredVars(const int& predIdx, Predicate* const& gndPred)
  {
    assert(varIdToVarsGroundedType_);
    groundPredVars(predIdx, gndPred, varIdToVarsGroundedType_); 
  }


    // restore variables to original values
  static void restoreVars(Array<VarsGroundedType*>* const & vgtArr)
  {    
    for (int i = 1; i < vgtArr->size(); i++)
    {
      if ((*vgtArr)[i] == NULL) continue;
      Array<Term*>& vars = (*vgtArr)[i]->vars;
      for (int j = 0; j < vars.size(); j++)  vars[j]->setId(-i);
      (*vgtArr)[i]->isGrounded = false;
    }
  }
  

    // restore variables to original values
  void restoreVars()
  {    
    assert(varIdToVarsGroundedType_);
    restoreVars(varIdToVarsGroundedType_);
  }


  /**
   * Adds variable ids and groundings to a LitIdxVarIdsGndings structure.
   * 
   * @param varId Id of variable being added.
   * @param varType Type of variable being added.
   * @param domain Domain in which this all takes place.
   * @param ivg LitIdxVarIdsGndings to which the ids and groundings are
   * being added.
   */
  static void addVarIdAndGndings(const int& varId, const int& varType,
                                 const Domain* const & domain,
                                 LitIdxVarIdsGndings* const & ivg)
  {
    ivg->varIds.append(varId);
    const Array<int>* constants = domain->getConstantsByType(varType);
    ivg->varGndings.appendArray(constants);
    ivg->varTypes.append(varType);
  }

  /**
   * Constructs a LitIdxVarIdsGndings structure for one literal.
   * Caller should delete the returned LitIdxVarsGndings*.
   * Returns NULL if the literal at litIdx is grounded.
   * 
   * @param lit Literal whose groundings are being constructed.
   * @param litIdx Index of literal in the clause
   * @param domain Domain in which the clause occurs
   * 
   * @return Structure containing the index, variable ids and groundings
   * for the given literal.
   */
  static LitIdxVarIdsGndings* createLitIdxVarIdsGndings(Predicate* const & lit,
                                                     const unsigned int& litIdx,
                                                   const Domain* const & domain)
  {
    LitIdxVarIdsGndings* ivg = new LitIdxVarIdsGndings;
    ivg->litIdx = litIdx;
    for (int i = 0; i < lit->getNumTerms(); i++)
    {
      const Term* t = lit->getTerm(i);
      int termType = t->getType();
      if (termType == Term::VARIABLE)
      {
        int varId = t->getId();
        if (!ivg->varIds.contains(varId))
          addVarIdAndGndings(varId, lit->getTermTypeAsInt(i), domain, ivg);
      }
      assert(t->getType() != Term::FUNCTION);
      assert(ivg->varIds.size() == ivg->varGndings.getNumArrays());
    }
    ivg->litUnseen = true;
    return ivg;
  }

  /**
   * Constructs a LitIdxVarIdsGndings structure for each literal in the
   * clause.
   * 
   * @param clauseLits Array of literals for which the groundings are being
   * constructed.
   * @param ivgArr Array where the structures are put.
   * @param domain Domain in which the clause occurs.
   * @param setGroundedClausesToNull If true, grounded clauses are set to
   * NULL; otherwise, they are not.
   */
  void createAllLitIdxVarsGndings(Array<Predicate*>& clauseLits, 
                                  Array<LitIdxVarIdsGndings*>& ivgArr,
                                  const Domain* const & domain,
                                  const bool& setGroundedClausesToNull) const
  {
    assert(varIdToVarsGroundedType_); // this must already be created
    
      // for each literal
    for (unsigned int i = 0; i < (unsigned int) clauseLits.size(); i++)
    {
        //the literal was grounded when a previous literal was grounded
      if (clauseLits[i] == NULL) continue;

      if (clausedebug >= 2)
      {
        cout << "createAllLitIdxVarsGndings lit before: ";
        clauseLits[i]->printWithStrVar(cout, domain);
        cout << endl;
      }
            
      ivgArr.append(createLitIdxVarIdsGndings(clauseLits[i], i, domain));
      
        //ground variables of the last literal we looked at throughout clause
      ArraysAccessor<int>& varGndings = ivgArr.lastItem()->varGndings;
      Array<int>& varIds = ivgArr.lastItem()->varIds;
      for (int j = 0; j < varIds.size(); j++)
      {
          // get the first constant that can be used to ground the var
        int constId = varGndings.getArray(j)->item(0);
        
          // ground all occurrences of var
        Array<Term*>& vars = (*varIdToVarsGroundedType_)[-varIds[j]]->vars;
        for (int k = 0; k < vars.size(); k++) vars[k]->setId(constId);
      }
    
        //store subsequent literals that are grounded when literal i is grounded
      Array<Predicate*>& subseqGndLits = ivgArr.lastItem()->subseqGndLits;

      for (int j = i + 1; j < clauseLits.size(); j++)
      {
        Predicate* subseqLit = clauseLits[j];
        if (subseqLit == NULL) continue;
        if (subseqLit->isGrounded()) 
        {
          subseqGndLits.append(subseqLit);
          if (setGroundedClausesToNull) clauseLits[j] = NULL;
        }
      } //for each subsequent literal

      if (clausedebug >= 2)
      {
        cout << "createAllLitIdxVarsGndings lit after: ";
        clauseLits[i]->printWithStrVar(cout, domain);
        cout << endl;
      }
    } //for each literal
  }


    // Also sets to -1 the ids of the parent terms of functions in ivgArr[i]. 
  static void deleteAllLitIdxVarsGndings(Array<LitIdxVarIdsGndings*>& ivgArr)
  { 
    for (int i = 0; i < ivgArr.size(); i++)
    {
      //ivgArr[i]->varGndings.deleteArraysAndClear();
      delete ivgArr[i];
    }
  }


  static void quicksortLiterals(pair<double,Predicate*> items[], 
                         const int& l, const int& r)
  {
    if (l >= r) return;
    pair<double,Predicate*> tmp = items[l];
    items[l] = items[(l+r)/2];
    items[(l+r)/2] = tmp;

    int last = l;
    for (int i = l+1; i <= r; i++)
      if (items[i].first > items[l].first)
      {
        ++last;
        tmp = items[last];
        items[last] = items[i];
        items[i] = tmp;
      }
    
    tmp = items[l];
    items[l] = items[last];
    items[last] = tmp;
    quicksortLiterals(items, l, last-1);
    quicksortLiterals(items, last+1, r);  
  }


    // sort literals in decreasing order of (# true groundings/# groundings)
  void sortLiteralsByTrueDivTotalGroundings(Array<Predicate*>& clauseLits,
                                            const Domain* const & domain,
                                            const Database* const & db) const
  {
    assert(predicates_->size() == clauseLits.size());

    Array<pair<double, Predicate*> > arr;
    for (int i = 0; i < clauseLits.size(); i++)
    {
      Predicate* lit = clauseLits[i];
    
        // put all the grounded literal in the front
      if (lit->isGrounded()) 
      {
        arr.append(pair<double,Predicate*>(DBL_MAX, lit));
        continue;
      }

        //estimate how likely the literal is true
      double numTrue = (lit->getSense())? db->getNumTrueGndPreds(lit->getId())
                                         :db->getNumFalseGndPreds(lit->getId());
      double numTotal = lit->getNumGroundingsIfAllVarDiff(domain);

        //get number of groundings of the literal
      double numGnd = 1;

      Array<int> varIds; //used to check unique var ids. A hash_set is slower.
      varIds.growToSize(lit->getNumTerms(),1);
      for (int i = 0; i < lit->getNumTerms(); i++)
      {
        const Term* t = lit->getTerm(i);
        if (t->getType() == Term::VARIABLE)
        {
          int tid = t->getId();
          if (!varIds.contains(tid))
          {
            varIds.append(tid);
            numGnd *= domain->getNumConstantsByType(lit->getTermTypeAsInt(i));
          }
        }
        assert(t->getType() != Term::FUNCTION);
      }

      arr.append(pair<double,Predicate*>(numTrue/numTotal/numGnd, lit));
    }
  
    quicksortLiterals((pair<double,Predicate*>*) arr.getItems(),0,arr.size()-1);
    assert(arr.size() == clauseLits.size());
    for (int i = 0; i < arr.size(); i++) clauseLits[i] = arr[i].second;
  }

  static bool literalAndSubLitsTrue(Predicate* const & lit,
				    const Array<Predicate*>& subseqLits,
				    const Database* const & db){
    TruthValue tv = db->getValue(lit);
    lit->setTruthValue(tv);
    
    //cout << tv << " " << lit->getSense() << endl;
    bool result = (db->sameTruthValueAndSense(tv, lit->getSense()));
    if (!result){
      return result;
    }
    for (int i = 0; i < subseqLits.size(); i++)
    {
      tv = db->getValue(subseqLits[i]);
      subseqLits[i]->setTruthValue(tv);
      if (!(db->sameTruthValueAndSense(tv,subseqLits[i]->getSense()))) 
	return false;
    }
 
    return true;
  }

  static bool literalOrSubsequentLiteralsAreTrue(Predicate* const & lit,
                                          const Array<Predicate*>& subseqLits,
                                          const Database* const & db)
  {
    TruthValue tv = db->getValue(lit);
    lit->setTruthValue(tv);
    if (db->sameTruthValueAndSense(tv, lit->getSense())) return true;
    for (int i = 0; i < subseqLits.size(); i++)
    {
      tv = db->getValue(subseqLits[i]);
      subseqLits[i]->setTruthValue(tv);
      if (db->sameTruthValueAndSense(tv,subseqLits[i]->getSense())) return true;
    }
    return false;
  }


  bool hasTwoLiteralsWithOppSense(const Database* const & db) const
  {    
    PredicateSet predSet; // used to detect duplicates
    PredicateSet::iterator iter;
    
    for (int i = 0; i < predicates_->size(); i++)
    {
      Predicate* predicate = (*predicates_)[i];
      assert(predicate->isGrounded());
      if (db->getValue(predicate) == UNKNOWN)
      {
        if ( (iter = predSet.find(predicate)) != predSet.end() )
        {
            // the two gnd preds are of opp sense, so clause must be satisfied
          if ((*iter)->getSense() !=  predicate->getSense()) return true;
        }
        else
          predSet.insert(predicate);        
      }
    }
      
    return false;
  }


    //returns true if the (ground) clause has two literals with opposite sense
    //i.e. the clause is satisfied; otherwise returns false
  bool createAndAddUnknownClause(Array<GroundClause*>* const& unknownGndClauses,
                                 Array<Clause*>* const& unknownClauses,
                                 double* const & numUnknownClauses,
                                 const AddGroundClauseStruct* const & agcs,
                                 const Database* const & db);

  bool groundPredicates(const Array<int>* const & set,
                        const Array<int>& gndPredIndexes,
                        Predicate* const & gndPred,
                        const TruthValue& gndPredTV,
                        const Database* const & db,
                        bool& sameTruthValueAndSense,
                        bool& gndPredPosHaveSameSense)
  {
    sameTruthValueAndSense = false;
    gndPredPosHaveSameSense = true;
    bool prevSense = false;
    for (int i = 0; i < set->size(); i++)
    {
      //cerr << "i = " << i << " of " << set->size() << endl;
      int gndPredIdx = gndPredIndexes[(*set)[i]];
      Predicate* pred = (*predicates_)[gndPredIdx];
        // if inconsistent grounding of variables, proceed to next combination
      if (!pred->canBeGroundedAs(gndPred)) return false;
      //cerr << "gp 1\n";
      groundPredVars(gndPredIdx, gndPred);
      //cerr << "gp 2\n";
      if (db->sameTruthValueAndSense(gndPredTV, pred->getSense()))
        sameTruthValueAndSense = true;
      if (i == 0) 
        prevSense = pred->getSense();
      else
	if (prevSense != pred->getSense())
	  gndPredPosHaveSameSense = false;
    }
    return true;
  }


  double countNumGroundings(){
    return countNumGroundings(false);
  }
    //count the number of groundings of clause variables
  double countNumGroundings(bool distinct)
  {
    double n = 1;
    //cerr << "Hi ";
    //cerr << varIdToVarsGroundedType_->size() << endl;
    if (distinct){
      Array<int> off;
      off.growToSize(varIdToVarsGroundedType_->size());
      for(int i = 0; i < off.size(); i++){
	off[i] = 0;
      }
      for (int i = 1; i < varIdToVarsGroundedType_->size(); i++){
	int type = (*varIdToVarsGroundedType_)[i]->typeId;
	for(int jx = (i+1); jx < off.size(); jx++){
	  if ((*varIdToVarsGroundedType_)[jx]->typeId == type){
	    off[jx]++;
	  }
	}
      }
      for (int i = 1; i < varIdToVarsGroundedType_->size(); i++)
	{
	  //cerr << "I = " << i << " " << varIdToVarsGroundedType_->size() << endl;
	  if ((*varIdToVarsGroundedType_)[i] == NULL){
	    cerr << "This is very bad !! \n";
	  }
	  if (!((*varIdToVarsGroundedType_)[i]->isGrounded))
	    n *= ((*varIdToVarsGroundedType_)[i]->numGndings - off[i]);
	}
    }
    else{
      for (int i = 1; i < varIdToVarsGroundedType_->size(); i++)
	{
	  //cerr << "I = " << i << " " << varIdToVarsGroundedType_->size() << endl;
	  if ((*varIdToVarsGroundedType_)[i] == NULL){
	    cerr << "This is very bad !! \n";
	  }
	  if (!((*varIdToVarsGroundedType_)[i]->isGrounded))
	    n *= (*varIdToVarsGroundedType_)[i]->numGndings;
	}
    }
    
    return n;
  }
  

  static double addCountToCombination(bool inComb[], const int& inCombSize,
                               const Array<int>* const & set,
                               MultDArray<double>& gndedPredPosArr,
                               const double& count)
  {
    memset(inComb, false, inCombSize*sizeof(bool));
    for (int i = 0; i < set->size(); i++)  inComb[(*set)[i]] = true;
    Array<int> multDArrIndexes(inCombSize); //e.g. [0][1][0][1]
    for (int i = 0; i < inCombSize; i++)
    {
      if (inComb[i]) multDArrIndexes.append(1);
      else           multDArrIndexes.append(0);    
    }
    gndedPredPosArr.addItem(&multDArrIndexes, count);   
    return gndedPredPosArr.getItem(&multDArrIndexes);
  }

  
  static void minusRepeatedCounts(bool inComb[], const int& inCombSize,
                           const Array<int>& inCombIndexes,
                           const Array<int>* const & set,
                           const Array<int>* const & falseSet,
                           MultDArray<double>& gndedPredPosArr,
                           const double& count)
  {
    memset(inComb, false, inCombSize*sizeof(bool));
    for (int i = 0; i < set->size(); i++)  inComb[(*set)[i]] = true;
    
    for (int i = 0; i < falseSet->size(); i++)
    {
      int idx = inCombIndexes[(*falseSet)[i]];
      assert(inComb[idx] == true);
      inComb[idx] = false;
    }
    
    Array<int> multDArrIndexes(inCombSize); //e.g. [0][1][0][1]
    for (int i = 0; i < inCombSize; i++)
    {
      if (inComb[i]) multDArrIndexes.append(1);
      else           multDArrIndexes.append(0);    
    }      
    
      //subtract count from that of a smaller combination that includes it
    gndedPredPosArr.addItem(&multDArrIndexes, -count);
  }  


  ////////////////////// for getting unknown clauses ////////////////////////
  void getBannedPreds(Array<Predicate*>& bannedPreds,
                      const int& gndPredIdx,
                      const GroundPredicate* const & groundPred,
                      const Predicate* const & gndPred) const
  {
    assert(gndPredIdx < predicates_->size());
    for (int i = 0; i < gndPredIdx; i++)
    {
      if ( (groundPred && (*predicates_)[i]->canBeGroundedAs(groundPred)) ||
           (gndPred && (*predicates_)[i]->canBeGroundedAs((Predicate*)gndPred)))
        bannedPreds.append((*predicates_)[i]);
    }
  }



  static void createBannedPreds(Array<Predicate*>& clauseLits,
                                Array<LitIdxVarIdsGndings*>& ivgArr,
                                Array<Predicate*>& bannedPreds)
  {
    if (bannedPreds.size() == 0) return;

    int a;
    for (int i = 0; i < ivgArr.size(); i++)
    {
      LitIdxVarIdsGndings* ivg = ivgArr[i];
      a = bannedPreds.find(clauseLits[ivg->litIdx]);
      if (a >= 0) ivg->bannedPreds.append(bannedPreds[a]);
      for (int j = 0; j < ivg->subseqGndLits.size(); j++)
      {
        a = bannedPreds.find(ivg->subseqGndLits[j]);
        if (a >= 0) ivg->bannedPreds.append(bannedPreds[a]);        
      }
    }   
  }


    //the array parameter should be LitIdxVarIdsGndings.bannedGndPreds
  static bool bannedPredsAreGndedAsGndPred(
                                const Array<Predicate*>& bannedPreds,
                                    const GroundPredicate* const & groundPred,
                                    const Predicate* const & gndPred)
  {
    for (int i = 0; i < bannedPreds.size(); i++)
    {
      if ( (groundPred && bannedPreds[i]->same(groundPred)) ||
           (gndPred    && bannedPreds[i]->same((Predicate*)gndPred)) ) 
        return true;
    }
    return false;
  }


  bool containsGndPredBeforeIdx(const int& gndPredIdx, 
                                const GroundPredicate* const & groundPred,
                                const Predicate* const & gndPred)
  {
    assert(gndPredIdx < predicates_->size());
 
    if (gndPredIdx < 0) return false;

    if (groundPred)
    {
      for (int i = 0; i < gndPredIdx; i++)
        if ((*predicates_)[i]->same(groundPred)) return true;
    }
    else
    {
      assert(gndPred);
      for (int i = 0; i < gndPredIdx; i++)
        if ((*predicates_)[i]->same((Predicate*)gndPred)) return true; 
    }
    return false;
  }
  ////////////////////////////////////////////////////////////////////////////
   //Even though it is more intuitive to use a recursive function to count
    //the number of true groundings, we are not doing so in order to allow the 
    //compiler to inline it.
    //Returns the number of true groundings, unknown clauses, number of unknown
    //clauses, and satisfiability
    //If gndPredId >= 0, the returned clauses do not contain groundPred/gndPred 
    //before position gndPredIdx
    //No more than one of the array parameters can be non-NULL.
    //No more than one of the groundPred/gndPred parameters can be non-NULL  
 
 
  double countNumTrueGroundings(const Domain* const & domain,
                                const Database* const & db,
                                const bool& hasUnknownPreds,
                                const bool& checkSatOnly,
                                  // params below: find unknown clauses
                                const int& gndPredIdx,
                                const GroundPredicate* const & groundPred,
                                const Predicate* const & gndPred,
                                Array<GroundClause*>* const & unknownGndClauses,
                                Array<Clause*>* const & unknownClauses,
                                double* const & numUnknownClauses,
                                  // params below: add unknown clauses to MRF
                                const AddGroundClauseStruct* const & agcs,
                                  // params below: get active clauses and count
                            Array<GroundClause *> * const & activeGroundClauses,
                                int* const & activeClauseCnt,
                                GroundPredicateHashArray* const& seenGndPreds,
                                bool const & ignoreActivePreds,
                                bool const & getSatisfied)
  //const bool& checkAllTrueAllFalse)                                
  {
    assert(unknownGndClauses == NULL || unknownClauses == NULL);
    assert(groundPred == NULL || gndPred == NULL);
      // Assert if activeGroundClauses isn't NULL, then others are and
      // vice versa
    if (activeGroundClauses)
    {
      assert(unknownGndClauses == NULL && unknownClauses == NULL &&
             agcs == NULL);
    }
    if (unknownGndClauses || unknownClauses || agcs)
    {
      assert(activeGroundClauses == NULL);
    }
    bool getActiveClauses = (activeGroundClauses || activeClauseCnt);

    if (activeClauseCnt) *activeClauseCnt = 0;
    if (numUnknownClauses) *numUnknownClauses = 0;

    bool findUnknownClauses = (unknownGndClauses || unknownClauses || 
                               numUnknownClauses || agcs);
      // these predicates must not be grounded as groundPred/gndPred
    Array<Predicate*> bannedPreds;
    if (findUnknownClauses)
      getBannedPreds(bannedPreds, gndPredIdx, groundPred, gndPred);

    double numTrueGndings = 0;
    
      //Copy the literals so that their original order in the clause is
      //not affected by the subsequent sorting
    Array<Predicate*>* origClauseLits = new Array<Predicate*>(*predicates_);

      // Array of partially grounded clauses achieved by using the inverted
      // index
    Array<Array<Predicate*>* > partGroundedClauses;

    if ((findUnknownClauses || getActiveClauses) && useInverseIndex)
    {
      bool newIgnoreActivePreds = ignoreActivePreds;
        // If in a neg. clause, then we can only index on evidence
      if (wt_ < 0) newIgnoreActivePreds = true;
      
        // Put the indexable literals first and ground them
      sortLiteralsByNegationAndArity(*origClauseLits, newIgnoreActivePreds, db);
      groundIndexableLiterals(domain, db, *origClauseLits, partGroundedClauses,
                              ignoreActivePreds);
    }
    else
    {
        //Sort preds in decreasing order of #TrueGndOfLiteral/#numOfGroundings.
        //The larger the number of true groundings of a literal, the more likely
        //it is to be true, so put it in front so that we can decide whether the
        //clause is true early.The larger the number of groundings of the
        //literal, the larger the savings when we decide that preceding literals
        //are true.
      sortLiteralsByTrueDivTotalGroundings(*origClauseLits, domain, db);
        // Put the original clause as the only clause into partGroundedClauses
      Array<Predicate*>* clauseLitsCopy = new Array<Predicate*>;
      clauseLitsCopy->growToSize(origClauseLits->size());
      for (int i = 0; i < origClauseLits->size(); i++)
        (*clauseLitsCopy)[i] = new Predicate(*(*origClauseLits)[i]);
      partGroundedClauses.append(clauseLitsCopy);
    }

      // At this point partGroundedClauses holds the nodes of the branch and
      // bound algorithm. This means nothing more is indexed and we must ground
      // out the rest of the predicates
    if (clausedebug)
    {
      cout << "Partially grounded clauses to be completed: " << endl;
      for (int pgcIdx = 0; pgcIdx < partGroundedClauses.size(); pgcIdx++)
      {
        cout << "\t";
        for (int i = 0; i < partGroundedClauses[pgcIdx]->size(); i++)
        {
          (*partGroundedClauses[pgcIdx])[i]->printWithStrVar(cout, domain);
          cout << " ";
        }
        cout << endl;
      }
    }

    bool stillActivating = true;
      // Go through each clause in partGroundedClauses (nodes of the branch and
      // bound algorithm if using inverted index; otherwise, the original
      // clause), ground them out and check truth values
    for (int pgcIdx = 0; pgcIdx < partGroundedClauses.size(); pgcIdx++)
    {
        // clauseLits is a sorted copy of predicates_
      Array<Predicate*> clauseLits = *(partGroundedClauses[pgcIdx]);
      assert(clauseLits.size() == origClauseLits->size());
        // Set the var to groundings in this clause to be those in clauseLits
      Array<int>* origVarIds = new Array<int>;
      
      for (int i = 0; i < clauseLits.size(); i++)
      {
        assert(clauseLits[i]->getNumTerms() ==
               (*origClauseLits)[i]->getNumTerms());
          // Ground variables throughout clause
        for (int j = 0; j < (*origClauseLits)[i]->getNumTerms(); j++)
        {
          const Term* oldTerm = (*origClauseLits)[i]->getTerm(j);
          const Term* newTerm = clauseLits[i]->getTerm(j);
          if (oldTerm->getType() == Term::VARIABLE)
          {
            int varId = oldTerm->getId();
            origVarIds->append(varId);
            if (newTerm->getType() == Term::CONSTANT)
            {
              int constId = newTerm->getId();
              assert(constId >= 0);
              Array<Term*>& vars = (*varIdToVarsGroundedType_)[-varId]->vars;
              for (int k = 0; k < vars.size(); k++) vars[k]->setId(constId);
            }
          }
        }
          // Set the preds in clauseLits to point to the original predicates_
        delete clauseLits[i];
        clauseLits[i] = (*origClauseLits)[i];
      }
      
        //simulate a stack, back/front corresponds to top/bottom
        //ivg stands for index, varIds, groundings
      Array<LitIdxVarIdsGndings*> ivgArr;
      createAllLitIdxVarsGndings(clauseLits, ivgArr, domain, true);
      if (findUnknownClauses)
        createBannedPreds(clauseLits, ivgArr, bannedPreds);
      int ivgArrIdx = 0; //store current position in ivgArr
      bool lookAtNextLit = false;
    
        // while stack is not empty
      while (ivgArrIdx >= 0 && stillActivating)
      {
          // get variable groundings at top of stack
        LitIdxVarIdsGndings* ivg = ivgArr[ivgArrIdx];
        //Predicate* lit = clauseLits[ivg->litIdx];
        Predicate* lit = (*origClauseLits)[ivg->litIdx];
        Array<int>& varIds = ivg->varIds;
        ArraysAccessor<int>& varGndings = ivg->varGndings;
        bool& litUnseen = ivg->litUnseen;
        bool hasComb;

        if (clausedebug)
        {
          cout << "Looking at lit: ";
          lit->printWithStrVar(cout, domain);
          cout << endl;
        }

          // while there are groundings of literal's variables
        while ((hasComb = varGndings.hasNextCombination()) || litUnseen)
        {
            // there may be no combinations if the literal is fully grounded
          if (litUnseen) litUnseen = false;

          if (hasComb)
          {
              //ground the literal's variables throughout the clause
            int constId;
            int v = 0; // index of varIds
              //for each variable in literal
            while (varGndings.nextItemInCombination(constId))
            {
              Array<Term*>& vars =
                (*varIdToVarsGroundedType_)[-varIds[v++]]->vars;
              for (int i = 0; i < vars.size(); i++) vars[i]->setId(constId);
            }
          }

          if (clausedebug)
          {
            cout << "Clause is now: ";
            printWithWtAndStrVar(cout, domain);
            cout << endl;
          }

            // Getting active clauses / counts
          if (getActiveClauses)
          {
              // Check if memory is available: Linux specific
              // TODO: Check available memory in Windows and Mac
#ifdef _SC_AVPHYS_PAGES
            if (sysconf(_SC_AVPHYS_PAGES) < 100)
            {
              stillActivating = false;
              cout << "Stopped activating" << endl;
            }
#endif
            if (!stillActivating) break;

              // proceed further only if:
              // 1. positive weight and partially gnded clause is unsatisfied or
              // 2. negative weight and partially gnded clause is satisfied
            bool proceed = true;
            if (wt_ >= 0 && !getSatisfied)
              proceed = isUnsatisfiedGivenActivePreds(lit, ivg->subseqGndLits,
                                                      db, ignoreActivePreds);

            if (clausedebug)
            {
              cout << " proceed " << proceed << endl;
            }

            if (proceed)
            {
                // if there are more literals
              if (ivgArrIdx + 1 < ivgArr.size())
              {
                lookAtNextLit = true;
                ivgArrIdx++; // move up stack
                break;
              }

                // Now we can check neg. clauses: if not satisfied (no true
                // literals) or satisfied with evidence atom, then do not
                // activate
              if (wt_ < 0 && !getSatisfied &&
                  !isSatisfiedGivenActivePreds(db, ignoreActivePreds))
              {
                if (clausedebug) cout << "continuing..." << endl;
                continue;
              }
          
                // At this point all the literals are grounded
                // and does not have any true literal. To make sure that
                // it is active, need to check the following two conditions:
                // 1. It may have the same literal appearing in opposite senses
                // => satisfied (and hence not active)
                // 2. It may be empty when evidence is pruned away => not active
              bool active;
              bool accumulateClauses = activeGroundClauses;
              if (!accumulateClauses)
                active = isActive(db);
              else
                active = createAndAddActiveClause(activeGroundClauses,
                                                  seenGndPreds, db,
                                                  getSatisfied);
            
              if (clausedebug) cout << "Active " << active << endl;
              if (active) (*activeClauseCnt)++;
            }
          }
            // Getting unknown clauses / counts
          else
          {
              // if literal or subsequent grounded literals are true,
            if (literalOrSubsequentLiteralsAreTrue(lit, ivg->subseqGndLits, db))
            {
              if (clausedebug)
                cout << "Clause satisfied" << endl;
              
              if (checkSatOnly) return 1;
                //count the number of combinations of remaining variables
              double numComb = 1;
              for (int i = ivgArrIdx + 1; i < ivgArr.size(); i++)
              {
                int numVar = ivgArr[i]->varGndings.getNumArrays();
                for (int j = 0; j < numVar; j++)
                  numComb *= ivgArr[i]->varGndings.getArray(j)->size();
              }
              numTrueGndings += numComb;
            }
            else
            if (findUnknownClauses && 
                bannedPredsAreGndedAsGndPred(ivg->bannedPreds, groundPred,
                                             gndPred))
            {
              if (clausedebug)
                cout << "Banned preds are grounded as ground pred" << endl;
              //do nothing, will move down stack later
            }
            else
            {
                // if there are more literals
              if (ivgArrIdx + 1 < ivgArr.size())
              {
                if (clausedebug) cout << "Moving to next literal" << endl;
                lookAtNextLit = true;
                ivgArrIdx++; // move up stack
                break;
              }
                //At this point all the literals are grounded, and they are
                //either unknown or false (have truth values opposite of their
                //senses).

              
              //if (checkAllTrueAllFalse && numTrueGndings > 0.0) return numTrueGndings; //TODO: CHANGED
              

              bool twoLitWithOppSense = false;
              if (hasUnknownPreds)
              {
                if (hasTwoLiteralsWithOppSense(db)) 
                {
                  twoLitWithOppSense = true;
                  ++numTrueGndings;
                  if (checkSatOnly) return 1;
                }
              }

              if (!twoLitWithOppSense && findUnknownClauses)
              {
                assert(!containsGndPredBeforeIdx(gndPredIdx, groundPred,
                                                 gndPred));

                  //Create a new clause by appending unknown predicates to it.
                createAndAddUnknownClause(unknownGndClauses, unknownClauses, 
                                          numUnknownClauses, agcs, db);
              }
            }
          }
        } //while there are groundings of literal's variables

          // If we have stopped due to no memory, then exit the while loop
        if (!stillActivating) break;
        
          //if we exit the while loop in order to look at next literal 
          //(i.e. without considering all groundings of current literal)
        if (lookAtNextLit) { lookAtNextLit = false; }
          //mv down stack
        else { varGndings.reset(); litUnseen = true; ivgArrIdx--; }

      } // while stack is not empty

        // Restore variables
      for (int i = 0; i < origVarIds->size(); i++)
      {
        int varId = (*origVarIds)[i];
        assert(varId < 0);
        Array<Term*>& vars = (*varIdToVarsGroundedType_)[-varId]->vars;
        for (int j = 0; j < vars.size(); j++) vars[j]->setId(varId);
        (*varIdToVarsGroundedType_)[-varId]->isGrounded = false;
      }
      delete origVarIds;

      deleteAllLitIdxVarsGndings(ivgArr);
      delete partGroundedClauses[pgcIdx];
    }
    delete origClauseLits;

    if (getActiveClauses)
    {
      if (stillActivating) return 1;
      else return 0;
    }
    return numTrueGndings;
  }

  ////////////////////////////////////////////////////////////////////////////

    //Even though it is more intuitive to use a recursive function to count
    //the number of true groundings, we are not doing so in order to allow the 
    //compiler to inline it.
    //Returns the number of true groundings, unknown clauses, number of unknown
    //clauses, and satisfiability
    //If gndPredId >= 0, the returned clauses do not contain groundPred/gndPred 
    //before position gndPredIdx
    //No more than one of the array parameters can be non-NULL.
    //No more than one of the groundPred/gndPred parameters can be non-NULL  
  double countNumTrueGroundingsJesse(const Domain* const & domain,
                                const Database* const & db,
                                const bool& hasUnknownPreds,
                                const bool& checkSatOnly,
                                  // params below: find unknown clauses
                                const int& gndPredIdx,
                                const GroundPredicate* const & groundPred,
                                const Predicate* const & gndPred,
                                Array<GroundClause*>* const & unknownGndClauses,
                                Array<Clause*>* const & unknownClauses,
                                double* const & numUnknownClauses,
                                  // params below: add unknown clauses to MRF
                                const AddGroundClauseStruct* const & agcs,
                                  // params below: get active clauses and count
                            Array<GroundClause *> * const & activeGroundClauses,
                                int* const & activeClauseCnt,
                                GroundPredicateHashArray* const& seenGndPreds,
                                bool const & ignoreActivePreds,
                                bool const & getSatisfied)
  {
    bool jDebug = false;//true;
    /*
    cout << "Under Eval: ";
    print(cout, domain);
    cout << endl;
    */
    assert(unknownGndClauses == NULL || unknownClauses == NULL);
    assert(groundPred == NULL || gndPred == NULL);
      // Assert if activeGroundClauses isn't NULL, then others are and
      // vice versa
    if (activeGroundClauses)
    {
      assert(unknownGndClauses == NULL && unknownClauses == NULL &&
             agcs == NULL);
    }
    if (unknownGndClauses || unknownClauses || agcs)
    {
      assert(activeGroundClauses == NULL);
    }
    bool getActiveClauses = (activeGroundClauses || activeClauseCnt);

    if (activeClauseCnt) *activeClauseCnt = 0;
    if (numUnknownClauses) *numUnknownClauses = 0;

    bool findUnknownClauses = (unknownGndClauses || unknownClauses || 
                               numUnknownClauses || agcs);
      // these predicates must not be grounded as groundPred/gndPred
    Array<Predicate*> bannedPreds;
    if (findUnknownClauses)
      getBannedPreds(bannedPreds, gndPredIdx, groundPred, gndPred);

    double numTrueGndings = 0;
    
      //Copy the literals so that their original order in the clause is
      //not affected by the subsequent sorting
    Array<Predicate*>* origClauseLits = new Array<Predicate*>(*predicates_);

      // Array of partially grounded clauses achieved by using the inverted
      // index
    Array<Array<Predicate*>* > partGroundedClauses;

    if ((findUnknownClauses || getActiveClauses) && useInverseIndex)
    {
      bool newIgnoreActivePreds = ignoreActivePreds;
        // If in a neg. clause, then we can only index on evidence
      if (wt_ < 0) newIgnoreActivePreds = true;
      
        // Put the indexable literals first and ground them
      sortLiteralsByNegationAndArity(*origClauseLits, newIgnoreActivePreds, db);
      groundIndexableLiterals(domain, db, *origClauseLits, partGroundedClauses,
                              ignoreActivePreds);
    }
    else
    {
        //Sort preds in decreasing order of #TrueGndOfLiteral/#numOfGroundings.
        //The larger the number of true groundings of a literal, the more likely
        //it is to be true, so put it in front so that we can decide whether the
        //clause is true early.The larger the number of groundings of the
        //literal, the larger the savings when we decide that preceding literals
        //are true.
      sortLiteralsByTrueDivTotalGroundings(*origClauseLits, domain, db);
        // Put the original clause as the only clause into partGroundedClauses
      Array<Predicate*>* clauseLitsCopy = new Array<Predicate*>;
      clauseLitsCopy->growToSize(origClauseLits->size());
      for (int i = 0; i < origClauseLits->size(); i++)
        (*clauseLitsCopy)[i] = new Predicate(*(*origClauseLits)[i]);
      partGroundedClauses.append(clauseLitsCopy);
    }

      // At this point partGroundedClauses holds the nodes of the branch and
      // bound algorithm. This means nothing more is indexed and we must ground
      // out the rest of the predicates
    if (clausedebug >= 2)
    {
      cout << "Partially grounded clauses to be completed: " << endl;
      for (int pgcIdx = 0; pgcIdx < partGroundedClauses.size(); pgcIdx++)
      {
        cout << "\t";
        for (int i = 0; i < partGroundedClauses[pgcIdx]->size(); i++)
        {
          (*partGroundedClauses[pgcIdx])[i]->printWithStrVar(cout, domain);
          cout << " ";
        }
        cout << endl;
      }
    }
    //cout << partGroundedClauses.size() << endl;
    //FLAG - Look at changing!!
    bool countDistinct = true;//false;//true;//false; 
    //int numDups = 0;
    //int fallThrough = 0;
    Array<Array<int>*> termIdx;
    termIdx.growToSize(origClauseLits->size());
    for(int ix = 0; ix < termIdx.size(); ix++){
      Array<int>* tmpTrmArr = new Array<int>;
      tmpTrmArr->growToSize((*origClauseLits)[ix]->getNumTerms(),-1);
      termIdx[ix] = tmpTrmArr;
    }
    /*
      Array<Array<Term*>*>* termIdx = new Array<Array<Term*>*>;//[
      termIdx->growToSize(origClauseLits->size());
      for(int ix = 0; ix < termIdx->size(); ix++){
      Array<Term*>* tmpTrmArr = new Array<Term*>;
      tmpTrmArr->growToSize((*origClauseLits)[ix]->getNumTerms(),NULL);
      (*termIdx)[ix] = tmpTrmArr;
      }
    */
    //cout << "Size: " << getNumPredicates() << " " << varIdToVarsGroundedType_->size() << endl;    
    bool stillActivating = true;
      // Go through each clause in partGroundedClauses (nodes of the branch and
      // bound algorithm if using inverted index; otherwise, the original
      // clause), ground them out and check truth values
    for (int pgcIdx = 0; pgcIdx < partGroundedClauses.size(); pgcIdx++)
    {
        // clauseLits is a sorted copy of predicates_
      Array<Predicate*> clauseLits = *(partGroundedClauses[pgcIdx]);
      assert(clauseLits.size() == origClauseLits->size());
        // Set the var to groundings in this clause to be those in clauseLits
      Array<int>* origVarIds = new Array<int>;
      //flag - could set vars to be equal

      for (int i = 0; i < clauseLits.size(); i++)
      {
        assert(clauseLits[i]->getNumTerms() ==
               (*origClauseLits)[i]->getNumTerms());
          // Ground variables throughout clause
        for (int j = 0; j < (*origClauseLits)[i]->getNumTerms(); j++)
        {
          const Term* oldTerm = (*origClauseLits)[i]->getTerm(j);
          const Term* newTerm = clauseLits[i]->getTerm(j);
          if (oldTerm->getType() == Term::VARIABLE)
          {
            int varId = oldTerm->getId();
            origVarIds->append(varId);
            if (newTerm->getType() == Term::CONSTANT)
            {
              int constId = newTerm->getId();
              assert(constId >= 0);
              Array<Term*>& vars = (*varIdToVarsGroundedType_)[-varId]->vars;
              for (int k = 0; k < vars.size(); k++) vars[k]->setId(constId);
            }
          }
        }
          // Set the preds in clauseLits to point to the original predicates_
        delete clauseLits[i];
        clauseLits[i] = (*origClauseLits)[i];
      }

      if (jDebug){
	cout << "Start counting: ";
	printWithoutWt(cout,domain);
	cout << endl;
      }
      //Flag distinct
      IntHashArray seenConsts;// = new IntHashArray;
      //HashArray<Term*, HashTerm, EqualTerm> seenConsts;// = new HashArray<Term*, HashTerm, EqualTerm>;
        //simulate a stack, back/front corresponds to top/bottom
        //ivg stands for index, varIds, groundings
      Array<LitIdxVarIdsGndings*> ivgArr;
      createAllLitIdxVarsGndings(clauseLits, ivgArr, domain, true);

      Array<Array<int>*> prevNumVars;
      //flag
      prevNumVars.growToSize(ivgArr.size());
      //cout << "Here " << prevNumVars.size() << " " << ivgArr.size() << endl;
      for(int ix = 0; ix < prevNumVars.size(); ix++){
	Array<int>* tmpInt = new Array<int>;
	//cout << "ix " << ix << " ";
	int numVar = ivgArr[ix]->varGndings.getNumArrays();
	//cout << numVar << endl;
	tmpInt->growToSize(numVar);
	for(int jx = 0; jx < tmpInt->size(); jx++){
	  (*tmpInt)[jx] = 0;
	}
	prevNumVars[ix] = tmpInt;
      }

      //cout << "Here " << prevNumVars.size() << " " << ivgArr.size() << endl;
      for(int ix = 0; ix < prevNumVars.size(); ix++){
	Array<int> typeArray = ivgArr[ix]->varTypes;
	Array<int> varArray = ivgArr[ix]->varIds;
	//int numVars = ivgArr[ix]->
	Array<int>* tmpInt = prevNumVars[ix];
	for(int vx = 0; vx < typeArray.size(); vx++){
	  int initType = typeArray[vx];
	  int initVar = varArray[vx];
	  for(int jx = (vx+1); jx < typeArray.size(); jx++){
	      if (typeArray[jx] == initType &&
		  varArray[jx] != initVar){
		(*tmpInt)[jx]++;
	      }
	  }
	  //cout << "Cur var id = " << curType << endl;
	  for(int jx = (ix+1); jx < prevNumVars.size(); jx++){
	    Array<int> currTypes = ivgArr[jx]->varTypes;
	    Array<int> currIds = ivgArr[jx]->varIds;
	    tmpInt = prevNumVars[jx];
	    //cout << "ix = " << ix << " jx " << jx << " " << currTypes.size() << " " << tmpInt->size() << endl;
	    for(int kx = 0; kx < currTypes.size(); kx++){
	      //cout << "Under Eval: " << currTypes[kx] << " ";
	      if (currTypes[kx] == initType &&
		  currIds[kx] != initVar){
		(*tmpInt)[kx]++;
	      }
	    }
	   
	  }
	  //cout << endl;
	}
      }

      /*
	for(int ix = 0; ix < prevNumVars.size(); ix++){
	Array<int>* tmpInt = prevNumVars[ix];
	
	cout << "Lit " << ix << ": ";
	for(int jx = 0; jx < tmpInt->size(); jx++){
	cout << (*tmpInt)[jx] << " ";
	}
	  cout << endl;
	  
	  }
      */
      if (findUnknownClauses)
        createBannedPreds(clauseLits, ivgArr, bannedPreds);
      int ivgArrIdx = 0; //store current position in ivgArr
      bool lookAtNextLit = false;
      //Term* oldTerm;
        // while stack is not empty
      while (ivgArrIdx >= 0 && stillActivating)
      {
          // get variable groundings at top of stack
        LitIdxVarIdsGndings* ivg = ivgArr[ivgArrIdx];
        //Predicate* lit = clauseLits[ivg->litIdx];
        Predicate* lit = (*origClauseLits)[ivg->litIdx];
        Array<int>& varIds = ivg->varIds;
        ArraysAccessor<int>& varGndings = ivg->varGndings;
        bool& litUnseen = ivg->litUnseen;
        bool hasComb;
	//<<<<<<< clause.h
	if (jDebug){
	  cout << "Looking at lit: ";
	  lit->printWithStrVar(cout, domain);
	  cout << " " << ivg->litIdx << endl;
 	}
        //if (clausedebug)
	//=======

        if (clausedebug >= 2)
	  //>>>>>>> 1.28
        {
          cout << "Looking at lit: ";
          lit->printWithStrVar(cout, domain);
          cout << endl;
        }
	

          // while there are groundings of literal's variables
        while ((hasComb = varGndings.hasDistinctNextCombination()) || litUnseen)
        {
            // there may be no combinations if the literal is fully grounded
          if (litUnseen) litUnseen = false;
	  bool repeatedConst = false;
	  //cout << "Reseting\n";
          if (hasComb)
          {
              //ground the literal's variables throughout the clause
            Array<int> constId;
            int v = 0; // index of varIds
	    if (ivg->litIdx == 0){
	      seenConsts.clear();
	      for(int ix = 0; ix < termIdx.size(); ix++){
		for(int jx = 0; jx < termIdx[ix]->size(); jx++){
		  (*termIdx[ix])[jx] = -1;
		}
	      }

	    }
	    else{
	      seenConsts.clear();
	      //cerr << "here 0\n";
	      for(unsigned int ix = 0; ix < ivg->litIdx; ix++){
		for(int jx = 0; jx < termIdx[ix]->size(); jx++){
		  seenConsts.append((*termIdx[ix])[jx]);
		}
	      }
	      for(int ix = ivg->litIdx; ix < termIdx.size(); ix++){
		for(int jx = 0; jx < termIdx[ix]->size(); jx++){
		  (*termIdx[ix])[jx] = -1;
		}
	      }
	      //cerr << "here 1\n";
	    }
	    //flag - partially ground;
	    varGndings.getDistinctNextCombination(constId);
	    /*
	      cout << ivg->litIdx << ": ";
	      for(int ix  = 0; ix < constId.size(); ix++){
	      cout << constId[ix] << " ";
	      }
	      cout << endl;
	    */
	    //for each variable in literal
            //while (varGndings.nextItemInCombination(constId))
	    for(int ix = 0; ix < constId.size(); ix++){
            
              Array<Term*>& vars =
                (*varIdToVarsGroundedType_)[-varIds[v++]]->vars;
              for (int i = 0; i < vars.size(); i++){
		if (jDebug){
		  vars[i]->print(cout, domain);
		  cout << " -> ";
		}
		
		vars[i]->setId(constId[ix]);
		if (countDistinct && i == 0){
		  if (seenConsts.contains(constId[ix])){
		    repeatedConst = true;
		  }  
		  else{
		    //cerr << "Here 2 " << (v-1) << "\n";
		    Array<int>* tmpIntArr = termIdx[ivg->litIdx];
		    /*
		      cerr << "Got array " << termIdx.size() << " " << 
		      ivg->litIdx << "\n";
		    */
		    if ((*tmpIntArr)[v-1] > -1){
		      //cerr << (*tmpIntArr)[v-1] << " " << seenConsts.size();
		      seenConsts.removeItem((*tmpIntArr)[v-1]);
		      //cerr << "removed\n";
		      (*tmpIntArr)[v-1] = -1;
		      //cerr << "reset\n";
		    }
		    //cerr << "Here 2a\n";
		    seenConsts.append(constId[ix]);
		    (*tmpIntArr)[v-1] = constId[ix];
		    //cerr << "Here 3\n";
		  }
		}
		if (jDebug){
		  vars[i]->print(cout, domain);
		  cout << " v = " << v << " varIds.size " << varIds.size() << " i = " << i << " " << vars.size() << " lit idx " << ivg->litIdx << " ntg " << numTrueGndings << " ";
		  printWithoutWt(cout, domain);
		  cout << endl;
		}
	      }
	    
	    
	    }
	  }
	  if (repeatedConst){
	    //numDups++;
	    continue;
	  }
	  //<<<<<<< clause.h
	  /*
          if (clausedebug)
	  =======
	  */

          if (clausedebug >= 2)
	    //>>>>>>> 1.28
          {
            cout << "Clause is now: ";
            printWithWtAndStrVar(cout, domain);
            cout << endl;
          }

            // Getting active clauses / counts
          if (getActiveClauses)
          {
              // Check if memory is available: Linux specific
              // TODO: Check available memory in Windows and Mac
#ifdef _SC_AVPHYS_PAGES
            if (sysconf(_SC_AVPHYS_PAGES) < 100)
            {
              stillActivating = false;
              cout << "Stopped activating" << endl;
            }
#endif
            if (!stillActivating) break;

              // proceed further only if:
              // 1. positive weight and partially gnded clause is unsatisfied or
              // 2. negative weight and partially gnded clause is satisfied
            bool proceed = true;
            if (wt_ >= 0 && !getSatisfied)
              proceed = isUnsatisfiedGivenActivePreds(lit, ivg->subseqGndLits,
                                                      db, ignoreActivePreds);

            if (clausedebug >= 2)
            {
              cout << " proceed " << proceed << endl;
            }

            if (proceed)
            {
                // if there are more literals
              if (ivgArrIdx + 1 < ivgArr.size())
              {
                lookAtNextLit = true;
                ivgArrIdx++; // move up stack
                break;
              }

                // Now we can check neg. clauses: if not satisfied (no true
                // literals) or satisfied with evidence atom, then do not
                // activate
              if (wt_ < 0 && !getSatisfied &&
                  !isSatisfiedGivenActivePreds(db, ignoreActivePreds))
              {
                if (clausedebug >= 2) cout << "continuing..." << endl;
                continue;
              }
          
                // At this point all the literals are grounded
                // and does not have any true literal. To make sure that
                // it is active, need to check the following two conditions:
                // 1. It may have the same literal appearing in opposite senses
                // => satisfied (and hence not active)
                // 2. It may be empty when evidence is pruned away => not active
              bool active;
              bool accumulateClauses = activeGroundClauses;
              if (!accumulateClauses)
                active = isActive(db);
              else
                active = createAndAddActiveClause(activeGroundClauses,
                                                  seenGndPreds, db,
                                                  getSatisfied);
            
              if (clausedebug >= 2) cout << "Active " << active << endl;
              if (active) (*activeClauseCnt)++;
            }
          }
            // Getting unknown clauses / counts
          else
          {
	    /*
	      if (termIdx->size() > 1){
	      Array<Term*>* tmpArr = (*termIdx)[1];
	      cout << "Check sat 0: " << (*tmpArr)[0] << endl;
	    }
	    */
              // if literal or subsequent grounded literals are true,
	    /*
	    if (!literalOrSubsequentLiteralsAreTrue(lit, ivg->subseqGndLits, db)){
	      cout << "Not true: ";
	      print(cout, domain);
	      cout << endl;
	    }
	    */

            if (literalOrSubsequentLiteralsAreTrue(lit, ivg->subseqGndLits, db))
            {
              if (clausedebug >= 2)
                cout << "Clause satisfied" << endl;
              
	      /*
		if (termIdx->size() > 1){
		Array<Term*>* tmpArr = (*termIdx)[1];
		cout << "Check sat 1: " << (*tmpArr)[0] << endl;
		}
	      */

              if (checkSatOnly) return 1;
                //count the number of combinations of remaining variables
              double numComb = 1;
              for (int i = ivgArrIdx + 1; i < ivgArr.size(); i++)
              {
                int numVar = ivgArr[i]->varGndings.getNumArrays();
		Array<int>* tmpIntArr = prevNumVars[i];
		/*
		  if (termIdx->size() > 1){
		  Array<Term*>* tmpArr = (*termIdx)[1];
		  cout << "Check counting: " << (*tmpArr)[0] << endl;
		}
		*/
                for (int j = 0; j < numVar; j++){
		  //flag
		  if (countDistinct){
		    int off = (*tmpIntArr)[j];
		    numComb *= (ivgArr[i]->varGndings.getArray(j)->size()-off);
		    //fallThrough++;
		  }
		  else{
		    
		    numComb *= ivgArr[i]->varGndings.getArray(j)->size();
		    //cout << "numComb: " << numComb << endl;
		  }
		}
              }
	      /*
		if (termIdx->size() > 1){
		Array<Term*>* tmpArr = (*termIdx)[1];
		cout << "Check sat 2: " << (*tmpArr)[0] << endl;
		}
	      */
              numTrueGndings += numComb;
	      /*
		cout << "Fall Through: " << numComb << " " << numTrueGndings << " ";
		print(cout, domain);
		cout << endl;
	      */
            }
            else
            if (findUnknownClauses && 
                bannedPredsAreGndedAsGndPred(ivg->bannedPreds, groundPred,
                                             gndPred))
            {
              if (clausedebug >= 2)
                cout << "Banned preds are grounded as ground pred" << endl;
              //do nothing, will move down stack later
            }
            else
            {
                // if there are more literals
              if (ivgArrIdx + 1 < ivgArr.size())
              {
                if (clausedebug >= 2) cout << "Moving to next literal" << endl;
                lookAtNextLit = true;
                ivgArrIdx++; // move up stack
                break;
              }
                //At this point all the literals are grounded, and they are
                //either unknown or false (have truth values opposite of their
                //senses).
              bool twoLitWithOppSense = false;
              if (hasUnknownPreds)
              {
                if (hasTwoLiteralsWithOppSense(db)) 
                {
                  twoLitWithOppSense = true;
                  ++numTrueGndings;
                  if (checkSatOnly) return 1;
                }
              }

              if (!twoLitWithOppSense && findUnknownClauses)
              {
                assert(!containsGndPredBeforeIdx(gndPredIdx, groundPred,
                                                 gndPred));

                  //Create a new clause by appending unknown predicates to it.
                createAndAddUnknownClause(unknownGndClauses, unknownClauses, 
                                          numUnknownClauses, agcs, db);
              }
            }
	    /*
	      if (termIdx->size() > 1){
	      Array<Term*>* tmpArr = (*termIdx)[1];
	      cout << "Check sat 3: " << (*tmpArr)[0] << endl;
	      }
	    */
	  }//else counting
	  /*
	    if (termIdx->size() > 1){
	    Array<Term*>* tmpArr = (*termIdx)[1];
	    cout << "Check sat 4: " << (*tmpArr)[0] << endl;
	    }
	  */
        } //while there are groundings of literal's variables

          // If we have stopped due to no memory, then exit the while loop
        if (!stillActivating) break;
        
          //if we exit the while loop in order to look at next literal 
          //(i.e. without considering all groundings of current literal)
        if (lookAtNextLit) { lookAtNextLit = false; }
          //mv down stack
        else { varGndings.reset(); litUnseen = true; ivgArrIdx--; }

      } // while stack is not empty
      
      /*
      seenConsts.deleteItemsAndClear();
      */
      //ERROR!!!

      
      //delete seenConsts;
      /*
	if (termIdx->size() > 1){
	Array<Term*>* tmpArr = (*termIdx)[1];
	cout << "Check stack empty 2: " << (*tmpArr)[0] << endl;
      }
      */
        // Restore variables
      for (int i = 0; i < origVarIds->size(); i++)
      {
        int varId = (*origVarIds)[i];
        assert(varId < 0);
        Array<Term*>& vars = (*varIdToVarsGroundedType_)[-varId]->vars;
        for (int j = 0; j < vars.size(); j++) vars[j]->setId(varId);
        (*varIdToVarsGroundedType_)[-varId]->isGrounded = false;
      }
      delete origVarIds;

      deleteAllLitIdxVarsGndings(ivgArr);
      delete partGroundedClauses[pgcIdx];
      for(int ix = 0; ix < prevNumVars.size(); ix++){
	Array<int>* tmpIA = prevNumVars[ix];
	//tmpIA->deleteItemsAndClear();
	delete tmpIA;
	
      }
      //prevNumVars.deleteItemsAndClear();
      /*
	if (termIdx->size() > 1){
	Array<Term*>* tmpArr = (*termIdx)[1];
	cout << "Check stack empty 3: " << (*tmpArr)[0] << endl;
	}
      */
    }
    delete origClauseLits;
    for(int ix = 0; ix < termIdx.size(); ix++){
      Array<int>* tmpTrmArr = termIdx[ix];
      if (tmpTrmArr){
	delete tmpTrmArr;
	termIdx[ix] = NULL;
      }
    }
    /*
    for(int ix = 0; ix < termIdx->size(); ix++){      
      Array<Term*>* tmpArr = (*termIdx)[ix];
      //tmpArr->deleteItemsAndClear();
      delete tmpArr;
    }
    delete termIdx;
    */
    if (getActiveClauses)
    {
      if (stillActivating) return 1;
      else return 0;
    }
    //cout << "Num dups " << numDups << " fall " << fallThrough << endl;
    return numTrueGndings;
  }


 ////////////////////////////////////////////////////////////////////////////

    //Even though it is more intuitive to use a recursive function to count
    //the number of true groundings, we are not doing so in order to allow the 
    //compiler to inline it.
    //Returns the number of true groundings, unknown clauses, number of unknown
    //clauses, and satisfiability
    //If gndPredId >= 0, the returned clauses do not contain groundPred/gndPred 
    //before position gndPredIdx
    //No more than one of the array parameters can be non-NULL.
    //No more than one of the groundPred/gndPred parameters can be non-NULL  
  double countAndNumTrueGroundings(const Domain* const & domain,
                                const Database* const & db,
                                const bool& hasUnknownPreds,
                                const bool& checkSatOnly,
                                  // params below: find unknown clauses
                                const int& gndPredIdx,
                                const GroundPredicate* const & groundPred,
                                const Predicate* const & gndPred,
                                Array<GroundClause*>* const & unknownGndClauses,
                                Array<Clause*>* const & unknownClauses,
                                double* const & numUnknownClauses,
                                  // params below: add unknown clauses to MRF
                                const AddGroundClauseStruct* const & agcs,
                                  // params below: get active clauses and count
                            Array<GroundClause *> * const & activeGroundClauses,
                                int* const & activeClauseCnt,
                                GroundPredicateHashArray* const& seenGndPreds,
                                bool const & ignoreActivePreds,
				   bool const & getSatisfied)

  {
    bool jDebug = false;//true;
    /*
    cout << "Score: ";
    printWithoutWt(cout, domain);
    cout << endl;
    */
    assert(unknownGndClauses == NULL || unknownClauses == NULL);
    assert(groundPred == NULL || gndPred == NULL);
      // Assert if activeGroundClauses isn't NULL, then others are and
      // vice versa
    if (activeGroundClauses)
    {
      assert(unknownGndClauses == NULL && unknownClauses == NULL &&
             agcs == NULL);
    }
    if (unknownGndClauses || unknownClauses || agcs)
    {
      assert(activeGroundClauses == NULL);
    }
    //bool getActiveClauses = (activeGroundClauses || activeClauseCnt);

    if (activeClauseCnt) *activeClauseCnt = 0;
    if (numUnknownClauses) *numUnknownClauses = 0;

    bool findUnknownClauses = (unknownGndClauses || unknownClauses || 
                               numUnknownClauses || agcs);
      // these predicates must not be grounded as groundPred/gndPred
    Array<Predicate*> bannedPreds;
    if (findUnknownClauses)
      getBannedPreds(bannedPreds, gndPredIdx, groundPred, gndPred);

    double numTrueGndings = 0;
    
      //Copy the literals so that their original order in the clause is
      //not affected by the subsequent sorting
    Array<Predicate*>* origClauseLits = new Array<Predicate*>(*predicates_);

      // Array of partially grounded clauses achieved by using the inverted
      // index
    Array<Array<Predicate*>* > partGroundedClauses;

    //Sort preds in decreasing order of #TrueGndOfLiteral/#numOfGroundings.
    //The larger the number of true groundings of a literal, the more likely
    //it is to be true, so put it in front so that we can decide whether the
    //clause is true early.The larger the number of groundings of the
    //literal, the larger the savings when we decide that preceding literals
    //are true.
    sortLiteralsByTrueDivTotalGroundings(*origClauseLits, domain, db);
    Array<Predicate*>* tmp = new Array<Predicate*>;
    tmp->growToSize(origClauseLits->size());
    int pos = 0;
    for(int ix = (origClauseLits->size()-1); ix >= 0; ix--){
      (*tmp)[pos] = (*origClauseLits)[ix];
      pos++;
    }
    delete origClauseLits;
    origClauseLits = tmp;
    
    // Put the original clause as the only clause into partGroundedClauses
    Array<Predicate*>* clauseLitsCopy = new Array<Predicate*>;
    clauseLitsCopy->growToSize(origClauseLits->size());
    for (int i = 0; i < origClauseLits->size(); i++)
      (*clauseLitsCopy)[i] = new Predicate(*(*origClauseLits)[i]);
    partGroundedClauses.append(clauseLitsCopy);
  

    // At this point partGroundedClauses holds the nodes of the branch and
    // bound algorithm. This means nothing more is indexed and we must ground
    // out the rest of the predicates
    if (clausedebug)
      {
	cout << "Partially grounded clauses to be completed: " << endl;
	for (int pgcIdx = 0; pgcIdx < partGroundedClauses.size(); pgcIdx++)
	  {
	    cout << "\t";
	    for (int i = 0; i < partGroundedClauses[pgcIdx]->size(); i++)
	      {
		(*partGroundedClauses[pgcIdx])[i]->printWithStrVar(cout, domain);
		cout << " ";
	      }
	    cout << endl;
	  }
      }
    //cout << partGroundedClauses.size() << endl;
    bool countDistinct = true;//false; 
    int numDups = 0;
    //int fallThrough = 0;

    Array<Array<int>*> termIdx;// = new Array<Array<Term*>*>;//[
    termIdx.growToSize(origClauseLits->size());
    for(int ix = 0; ix < termIdx.size(); ix++){
      Array<int>* tmpTrmArr = new Array<int>;
      tmpTrmArr->growToSize((*origClauseLits)[ix]->getNumTerms(),-1);
      termIdx[ix] = tmpTrmArr;
    }
    //cout << "Start the real work\n";
    bool stillActivating = true;
      // Go through each clause in partGroundedClauses (nodes of the branch and
      // bound algorithm if using inverted index; otherwise, the original
      // clause), ground them out and check truth values
    for (int pgcIdx = 0; pgcIdx < partGroundedClauses.size(); pgcIdx++)
    {
        // clauseLits is a sorted copy of predicates_
      Array<Predicate*> clauseLits = *(partGroundedClauses[pgcIdx]);
      assert(clauseLits.size() == origClauseLits->size());
        // Set the var to groundings in this clause to be those in clauseLits
      Array<int>* origVarIds = new Array<int>;
      //flag - could set vars to be equal

      for (int i = 0; i < clauseLits.size(); i++)
      {
        assert(clauseLits[i]->getNumTerms() ==
               (*origClauseLits)[i]->getNumTerms());
          // Ground variables throughout clause
        for (int j = 0; j < (*origClauseLits)[i]->getNumTerms(); j++)
        {
          const Term* oldTerm = (*origClauseLits)[i]->getTerm(j);
          const Term* newTerm = clauseLits[i]->getTerm(j);
          if (oldTerm->getType() == Term::VARIABLE)
          {
            int varId = oldTerm->getId();
            origVarIds->append(varId);
            if (newTerm->getType() == Term::CONSTANT)
            {
              int constId = newTerm->getId();
              assert(constId >= 0);
              Array<Term*>& vars = (*varIdToVarsGroundedType_)[-varId]->vars;
              for (int k = 0; k < vars.size(); k++) vars[k]->setId(constId);
            }
          }
        }
          // Set the preds in clauseLits to point to the original predicates_
        delete clauseLits[i];
        clauseLits[i] = (*origClauseLits)[i];
      }

      if (jDebug){
	cout << "Start counting: ";
	printWithoutWt(cout,domain);
	cout << endl;
      }
      //Flag distinct
      IntHashArray seenConsts;
      
      //simulate a stack, back/front corresponds to top/bottom
      //ivg stands for index, varIds, groundings
      Array<LitIdxVarIdsGndings*> ivgArr;
      createAllLitIdxVarsGndings(clauseLits, ivgArr, domain, true);
      //cout << "Dudes: " << clauseLits.size() << " " << ivgArr.size() << endl;
      int ivgArrIdx = 0; //store current position in ivgArr
      bool lookAtNextLit = false;

        // while stack is not empty
      while (ivgArrIdx >= 0 && stillActivating)
      {
          // get variable groundings at top of stack
        LitIdxVarIdsGndings* ivg = ivgArr[ivgArrIdx];
        //Predicate* lit = clauseLits[ivg->litIdx];
        Predicate* lit = (*origClauseLits)[ivg->litIdx];
        Array<int>& varIds = ivg->varIds;
        ArraysAccessor<int>& varGndings = ivg->varGndings;
        bool& litUnseen = ivg->litUnseen;
        bool hasComb;
	if (jDebug){
	  cout << "Looking at lit: ";
	  lit->printWithStrVar(cout, domain);
	  cout << " " << ivg->litIdx << endl;
 	}
        if (clausedebug)
        {
          cout << "Looking at lit: ";
          lit->printWithStrVar(cout, domain);
          cout << endl;
        }
	

          // while there are groundings of literal's variables
        while ((hasComb = varGndings.hasNextCombination()) || litUnseen)
        {
            // there may be no combinations if the literal is fully grounded
          if (litUnseen) litUnseen = false;
	  bool repeatedConst = false;
          if (hasComb)
          {
            //ground the literal's variables throughout the clause
            Array<int> constId;
            int v = 0; // index of varIds
	    if (ivg->litIdx == 0){
	      seenConsts.clear();
	      for(int ix = 0; ix < termIdx.size(); ix++){
		for(int jx = 0; jx < termIdx[ix]->size(); jx++){
		  (*termIdx[ix])[jx] = -1;
		}
	      }

	    }
	    else{
	      seenConsts.clear();
	      //cerr << "here 0\n";
	      for(unsigned int ix = 0; ix < ivg->litIdx; ix++){
		for(int jx = 0; jx < termIdx[ix]->size(); jx++){
		  seenConsts.append((*termIdx[ix])[jx]);
		}
	      }
	      for(int ix = ivg->litIdx; ix < termIdx.size(); ix++){
		for(int jx = 0; jx < termIdx[ix]->size(); jx++){
		  (*termIdx[ix])[jx] = -1;
		}
	      }
	      //cerr << "here 1\n";
	    }
	    //flag - partially ground;
	    varGndings.getDistinctNextCombination(constId);
	    //for each variable in literal
            //while (varGndings.nextItemInCombination(constId))
	    for(int ix = 0; ix < constId.size(); ix++){
            
              Array<Term*>& vars =
                (*varIdToVarsGroundedType_)[-varIds[v++]]->vars;
              for (int i = 0; i < vars.size(); i++){
		if (jDebug){
		  vars[i]->print(cout, domain);
		  cout << " -> ";
		}
		
		vars[i]->setId(constId[ix]);
		if (countDistinct && i == 0){
		  if (seenConsts.contains(constId[ix])){
		    repeatedConst = true;
		  }  
		  else{
		    Array<int>* tmpIntArr = termIdx[ivg->litIdx];
		    if ((*tmpIntArr)[v-1] > -1){
		      seenConsts.removeItem((*tmpIntArr)[v-1]);
		      (*tmpIntArr)[v-1] = -1;
		    }
		    seenConsts.append(constId[ix]);
		    (*tmpIntArr)[v-1] = constId[ix];
		  }
		}
	      }
	    }
	  }


	  
	  if (countDistinct && repeatedConst){
	    if (jDebug){
	      cout << "Dup: ";
	      printWithoutWt(cout, domain);
	      cout << " " << numTrueGndings << endl;
	    }
	    numDups++;
	    continue;
	  }
	  
	  
          if (clausedebug)
          {
            cout << "Clause is now: ";
            printWithWtAndStrVar(cout, domain);
            cout << endl;
          }
	  //lit->print(cout, domain);
	  //cout << " ";
	  // if literal or subsequent grounded literals are true,
	  //if (literalIsTrue(lit,db)){
	  if (literalAndSubLitsTrue(lit, ivg->subseqGndLits, db)){
	    //numTrueGndings += numComb;
	    //}
	    //else
	    //{
	      // if there are more literals
	    if (ivgArrIdx + 1 < ivgArr.size())
	      {
		if (clausedebug) cout << "Moving to next literal" << endl;
		lookAtNextLit = true;
		ivgArrIdx++; // move up stack
		break;
	      }
	    else{
	      /*
	      cout << "idx = " << ivgArrIdx << " size " << ivgArr.size() << " ";
	      printWithoutWt(cout, domain);
	      cout << endl;
	      */
	      numTrueGndings++;
	    }
	  }
	  else{
	    continue;
	  }
	  
        } //while there are groundings of literal's variables
	
          //if we exit the while loop in order to look at next literal 
          //(i.e. without considering all groundings of current literal)
        if (lookAtNextLit) { lookAtNextLit = false; }
          //mv down stack
        else { varGndings.reset(); litUnseen = true; ivgArrIdx--; }

      } // while stack is not empty

      //seenConsts.deleteItemsAndClear();
      for(int ix = 0; ix < termIdx.size(); ix++){
	Array<int>* tmpTrmArr = termIdx[ix];
	for(int jx = 0; jx < tmpTrmArr->size(); jx++){
	  (*tmpTrmArr)[jx] = -1; //NULL;
	}
      }
      //delete seenConsts;
      // Restore variables
      for (int i = 0; i < origVarIds->size(); i++)
      {
        int varId = (*origVarIds)[i];
        assert(varId < 0);
        Array<Term*>& vars = (*varIdToVarsGroundedType_)[-varId]->vars;
        for (int j = 0; j < vars.size(); j++) vars[j]->setId(varId);
        (*varIdToVarsGroundedType_)[-varId]->isGrounded = false;
      }
      delete origVarIds;

      deleteAllLitIdxVarsGndings(ivgArr);
      delete partGroundedClauses[pgcIdx];
      /*
	for(int ix = 0; ix < prevNumVars.size(); ix++){
	Array<int>* tmpIA = prevNumVars[ix];
	//tmpIA->deleteItemsAndClear();
	delete tmpIA;
	
	}
      */
 
    }
    delete origClauseLits;
    
    for(int ix = 0; ix < termIdx.size(); ix++){      
      Array<int>* tmpArr = termIdx[ix];
      //tmpArr->deleteItemsAndClear();
      delete tmpArr;
    }
    //delete termIdx;
    
    return numTrueGndings;
  }


  double countNumTrueGroundingsForAllComb(const Array<int>& gndPredIndexes,
                                          Predicate* const & gndPred,
                                          const TruthValue& gndPredTV,
                                          const bool& gndPredFlipped,
                                          const Domain* const & domain,
                                          const bool& hasUnknownPreds,
                                          const bool& sampleClauses)
  {

    //Look Here
    assert(varIdToVarsGroundedType_);
    const Database* db = domain->getDB();
    double count = 0;
    int inCombSize = gndPredIndexes.size();
    bool* inComb = new bool[inCombSize];

      //initialize the number of true groundings for each way grounding the
      //predicates represented by gndPredIndexes.
      //e.g. gndedPredPosArr[0][0][1][1] is the number of true groundings when
      //the 1st and 2nd predicates in gndPredIndexes are not grounded and
      //the 3rd and 4th are grounded
    Array<int> dim;
    for (int i = 0; i < gndPredIndexes.size(); i++) dim.append(2);
      //WARNING: this may take up a lot of memory when gndPred can be grounded
      //at many positions in the clause
    MultDArray<double> gndedPredPosArr(&dim);
    const Array<double>* oneDArray = gndedPredPosArr.get1DArray();
    double* oneDArrayItems = (double*) oneDArray->getItems();
    memset(oneDArrayItems, 0, oneDArray->size()*sizeof(double));
    //cerr << "JD 1\n";
      //for each possible combination of grounding the predicates
      //(represented by gndPredIndexes) as gndPred
    PowerSet* ps = PowerSet::getPowerSet();
      //the combinations are accessed in order of decreasing size
    ps->prepareAccess(gndPredIndexes.size(), false);
    //jdebug = true;
    const Array<int>* set;
    while (ps->getNextSet(set))
    {
      //cerr << "JD while\n";
      
        //ground the predicates in current combination
      bool sameTruthValueAndSense; //a ground pred has the same tv and sense
      bool gndPredPosSameSense;
      bool valid = groundPredicates(set, gndPredIndexes, gndPred, gndPredTV, db,
                                    sameTruthValueAndSense,gndPredPosSameSense);

      //cerr << "get valid\n";
        //If it is not possible to ground the predicates according to current
        //combination or the grounded predicates are not all of the same sense,
        //then skip the combination.
        //We can ignore the combination when the grounded predicates are not all
        //of the same sense because the counts when gndPred is held to true and
        //false cancel one another out exactly
      if (!valid || !gndPredPosSameSense) { restoreVars(); continue; }
      
      //cerr << "call count\n";
        //count number of true groundings
      double cnt, numGndings = countNumGroundings();
      //cerr << "JD while 1\n";
        //if any of the grounded predicates has the same truth value and sense
      if (sameTruthValueAndSense)
        cnt = numGndings;
      else
      {
        double samp; int np;
        bool toSampleClauses = (sampleClauses && (np=predicates_->size()) > 1 &&
                                (samp = clauseSampler_->computeNumSamples(np))
                                < numGndings);
        //commented out: for testing sampling only
        //toSampleClauses = (sampleClauses && np > 1);
	if (sampleClauses && jdebug){
	  //cout << "Here " << samp << " numGnding "  << numGndings << " " << np << endl;
	}
        if (toSampleClauses)
        {

          Predicate* flippedGndPred = (gndPredFlipped) ? gndPred : NULL;
          cnt = clauseSampler_->estimateNumTrueGroundings(this, flippedGndPred,
                                                          domain, samp);
          if (cnt > numGndings) cnt = numGndings;
          else if (cnt < 0)     cnt = 0;

	  if (jdebug){
	    cout << "Sampled: " << samp << " " << cnt << " numGnding " << numGndings << endl;
	  }
        }
        else{
	  //cerr << "lets count\n";
          cnt = countNumTrueGroundings(domain, db, hasUnknownPreds, false,
                                       -1, NULL, NULL, NULL, NULL, NULL, NULL,
                                       NULL, NULL, NULL, true, false);
	  //cerr << "counted\n";
	}
      }

        // add cnt to that of current combination
      double cntDueToThisComb
        = addCountToCombination(inComb,inCombSize,set,gndedPredPosArr,cnt);
      count += cntDueToThisComb;

      //for (int i = 0; i < inCombSize; i++)  cout << ((int) inComb[i]) << " ";
      //cout << " = " << cntDueToThisComb << "/" << cnt << endl;
    
        //find the indexes that are in this combination
      Array<int> inCombIndexes;
      for (int i = 0; i < set->size(); i++)  inCombIndexes.append((*set)[i]);

        // subtract all the repeated counts of cntDueToThisComb
      PowerSetInstanceVars psInstVars;
      ps->prepareAccess(inCombIndexes.size(), psInstVars);
      const Array<int>* falseSet;
      while (ps->getNextSet(falseSet, psInstVars))
      {
          // at least one of the predicates must be gnded as gndPred
        if (falseSet->size() == set->size()) continue;
        minusRepeatedCounts(inComb, inCombSize, inCombIndexes, set, falseSet,
                            gndedPredPosArr, cntDueToThisComb);
      }
      restoreVars();
    } //for each possible combination of grounding the predicates as gndPred
    delete [] inComb;
    return count;
  }


    // Returns true if the ground clause was active 
  bool createAndAddActiveClause(
                            Array<GroundClause *> * const & activeGroundClauses,
                                GroundPredicateHashArray* const& seenGndPreds,
                                const Database* const & db,
                                bool const & getSatisfied);
  
  bool isActive(const Database* const & db)
  {
    PredicateSet predSet; // used to detect duplicates
    PredicateSet::iterator iter;
    bool isEmpty = true;
 
    for (int i = 0; i < predicates_->size(); i++)
    {
      Predicate* predicate = (*predicates_)[i];
      assert(predicate); 
	  assert(predicate->isGrounded());
      if ( (iter=predSet.find(predicate)) != predSet.end() )
      {
          // the two gnd preds are of opp sense, so clause must be satisfied
	    if (wt_ >= 0 && (*iter)->getSense() !=  predicate->getSense())
		  return false;
        continue;
	  }
      else
        predSet.insert(predicate);
	  
      bool isEvidence = db->getEvidenceStatus(predicate);
	  if (!isEvidence)
        isEmpty = false;
    }
    return !isEmpty;
  }

    // Assumption: clause has pos. weight
  bool isUnsatisfiedGivenActivePreds(Predicate* const & lit,
                                     const Array<Predicate*>& subseqLits,
                                     const Database* const & db,
                                     bool const & ignoreActivePreds)
  {
//cout << "ignoreActivePreds " << ignoreActivePreds << endl;
//cout << "lit "; lit->printWithStrVar(cout, db->getDomain()); cout << endl;
      // If atom has been deactivated, then we don't want any clauses that
      // it's in
    if (db->getDeactivatedStatus(lit)) return false;
    bool active = false;
    if (!ignoreActivePreds)
      active = db->getActiveStatus(lit);
//cout << "active " << active << endl;
    TruthValue tv = db->getValue(lit);
    lit->setTruthValue(tv);
//cout << "tv  " << tv << endl;
    if (!active && db->sameTruthValueAndSense(tv, lit->getSense()))
      return false;

    for (int i = 0; i < subseqLits.size(); i++)
    {
//cout << "subseqLit " << i << " ";
//subseqLits[i]->printWithStrVar(cout, db->getDomain());
//cout << endl;
    if (!ignoreActivePreds)
      active = db->getActiveStatus(subseqLits[i]);
//cout << "active " << active << endl;
    tv = db->getValue(subseqLits[i]);
    subseqLits[i]->setTruthValue(tv);
//cout << "tv  " << tv << endl;
    if (!active && db->sameTruthValueAndSense(tv,subseqLits[i]->getSense()))
      return false;
    }
    return true;
  }


    // Assumption: clause has neg. weight
  bool isSatisfiedGivenActivePreds(const Database* const & db,
                                   bool const & ignoreActivePreds)
  {
    bool isSatisfied = false;
    for (int i = 0; i < predicates_->size(); i++)
    {
      Predicate* lit = getPredicate(i);
      assert(lit->isGrounded());
        // If atom has been deactivated, then we don't want any clauses
        // that it's in
      if (db->getDeactivatedStatus(lit)) return false;
      bool active = false;
      bool evidence = false;
      if (!ignoreActivePreds)
        active = db->getActiveStatus(lit);
    
      TruthValue tv = db->getValue(lit);
      lit->setTruthValue(tv);
        // If true evidence atom is in clause, then clause is always satisfied
        // and we want to ignore it.
      evidence = db->getEvidenceStatus(lit);

//cout << "Lit: ";
//lit->printWithStrVar(cout, db->getDomain());
//cout << " ev. " << evidence << " act. " << active << " stvas "
//<< db->sameTruthValueAndSense(tv, lit->getSense()) << endl;
      if (evidence && db->sameTruthValueAndSense(tv, lit->getSense()))
        return false;
        // Any active atom in clause or any true non-active atom
        // means it is candidate for active clause
      if (active || db->sameTruthValueAndSense(tv, lit->getSense()))
        isSatisfied = true;
    }
    return isSatisfied;
  }


  /**
   * Sort literals for use by the inverted index.
   * For pos. clauses: negative literals first, then of increasing arity.
   * For neg. clauses: positive literals first, then of increasing arity.
   * 
   * @param clauseLits Array of Predicate* to be sorted.
   * @param ignoreActivePreds If true, active preds are not indexed, which
   * means only evidence preds are indexed (sorted to the front). This is used
   * in eager inference.
   */
  void sortLiteralsByNegationAndArity(Array<Predicate*>& clauseLits,
                                      const bool& ignoreActivePreds,
                                      const Database* const & db) const
  {
    assert(predicates_->size() == clauseLits.size());

    Array<pair<double, Predicate*> > arr;
    for (int i = 0; i < clauseLits.size(); i++)
    {
      Predicate* lit = clauseLits[i];
      bool isIndexable = lit->isIndexable(wt_ >= 0);
      
        // If lit is indexable, put it at the beginning
      if (isIndexable)
      {
          // Evidence first
        if (db->isClosedWorld(lit->getId()))
        {
            // Put neg. literals first because we assume sparseness
          if (lit->getSense())
            arr.append(pair<double, Predicate*>(5 + 
                                      (1.0 / (double)lit->getNumTerms()), lit));
          else
            arr.append(pair<double, Predicate*>(7 +
                                      (1.0 / (double)lit->getNumTerms()), lit));
        }
          // Then non-evidence if performing lazy
        else if (!ignoreActivePreds)
        {
            // Put neg. literals first because we assume sparseness
          if (lit->getSense())
            arr.append(pair<double, Predicate*>(1 + 
                                      (1.0 / (double)lit->getNumTerms()), lit));
          else
            arr.append(pair<double, Predicate*>(3 +
                                      (1.0 / (double)lit->getNumTerms()), lit));
        }
        else
          arr.append(pair<double, Predicate*>(-(double)lit->getNumTerms(), lit));        
      }
      else
        arr.append(pair<double, Predicate*>(-(double)lit->getNumTerms(), lit));
    }
  
    quicksortLiterals((pair<double,Predicate*>*) arr.getItems(),0,arr.size()-1);
    assert(arr.size() == clauseLits.size());
    for (int i = 0; i < arr.size(); i++)
    {
  	  clauseLits[i] = arr[i].second;
  	  //cout << clauseLits[i]->getSense() << " " << clauseLits[i]->getName()
      //     << endl;
    }
  }


  /**
   * Uses the inverted index to ground literals starting from the first literal
   * up to the last indexable literal.
   * Assumes clauseLits is ordered so that indexable lits are in the front.
   * 
   * @param domain Domain in which this takes place.
   * @param db Database containing the truth values / activity of the preds
   * @param clauseLits Array of Predicate*s representing the first-order
   * clause.
   * @param resultingClauses Partially grounded clauses resulting from grounding
   * indexable literals.
   */
  void groundIndexableLiterals(const Domain* const & domain,
                               const Database* const & db,
                               Array<Predicate*>& clauseLits,
                               Array<Array<Predicate*>* >& resultingClauses,
                               bool const & ignoreActivePreds)
  {
    if (clausedebug >= 1)
    {
      cout << "Grounding indexable literals for ";
      for (int i = 0; i < clauseLits.size(); i++)
      {
        clauseLits[i]->printWithStrVar(cout, domain);
        cout << " ";
      }
      cout << endl;
    }
    assert(clauseLits.size() > 0);
    bool posClause = (wt_ >= 0) ? true : false;

      // Initially, resultingClauses holds only copies of the original clause
    Array<Predicate*>* clauseLitsCopy = new Array<Predicate*>;
    clauseLitsCopy->growToSize(clauseLits.size());
    for (int i = 0; i < clauseLits.size(); i++)
      (*clauseLitsCopy)[i] = new Predicate(*clauseLits[i]);
    resultingClauses.append(clauseLitsCopy);
      // Go through each literal and, if indexable, branch and bound on it
    for (int litIdx = 0; litIdx < clauseLits.size(); litIdx++)
    {
        // Indexable lit
      if (clauseLits[litIdx]->isIndexable(posClause) &&
          (!ignoreActivePreds||db->isClosedWorld(clauseLits[litIdx]->getId())))
      {
        if (clausedebug >= 1)
          cout << "Looking at literal " << litIdx << endl;
          
          // tmpClauses holds the partially grounded clauses in one level of the
          // tree
        Array<Array<Predicate*>* > tmpClauses;
        if (clausedebug >= 1)
        {
          cout << "Resulting clauses size before: " << resultingClauses.size()
               << endl;
        }
          // pgc = partially grounded clause
        for (int pgcIdx = 0; pgcIdx < resultingClauses.size(); pgcIdx++)
        {
          if (clausedebug >= 2)
          {
            cout << "Resulting clause before " << pgcIdx << ": ";
            for (int i = 0; i < resultingClauses[pgcIdx]->size(); i++)
            {
              (*resultingClauses[pgcIdx])[i]->printWithStrVar(cout, domain);
              cout << " ";
            }
            cout << endl;
          }
	        // Branch and bound on indexable literals
	      Array<Predicate *>* indexedGndings = new Array<Predicate *>;
	        // Bound: Get true indexed gndings if negated literal,
            // otherwise false groundings
          bool trueGndings = !(clauseLits[litIdx]->getSense());
          
          // Fix: otherwise fail to retrieve neg-wt clauses if activating
          // positive query atom
          if (wt_ < 0 && !((Database *)db)->
                isClosedWorld((*resultingClauses[pgcIdx])[litIdx]->getId())) 
            trueGndings = clauseLits[litIdx]->getSense();
              
	      ((Database *)db)->getIndexedGndings(indexedGndings,
                                            (*resultingClauses[pgcIdx])[litIdx],
                                              ignoreActivePreds, trueGndings);
          if (clausedebug >= 2)
          {
            cout << "indexedGndings: " << endl;
            for (int i = 0; i < indexedGndings->size(); i ++)
            {
              cout << "\t";
              (*indexedGndings)[i]->printWithStrVar(cout, domain);
              cout << endl;
            }
          }
            // Branch on the indexed groundings: No. of gndings is the no.
            // of new branches
          int oldTmpSize = tmpClauses.size();
          tmpClauses.growToSize(oldTmpSize + indexedGndings->size());
          if (clausedebug >= 2)
          {
            cout << "tmpClause size old: " << oldTmpSize << " new: "
                 << tmpClauses.size() << endl;
          }
          
          for (int i = 0; i < indexedGndings->size(); i++)
          {
              // Reference clause is what was branched on. First, copy this
              // into tmpClauses. Later, variables are replaced with the
              // constants from the indexed grounding
            tmpClauses[oldTmpSize + i] =
              new Array<Predicate*>(resultingClauses[pgcIdx]->size());
            tmpClauses[oldTmpSize + i]->
              growToSize(resultingClauses[pgcIdx]->size());
            if (clausedebug >= 2)
              cout << "Tmp clause before " << oldTmpSize + i << ": ";
            for (int predNo = 0; predNo < tmpClauses[oldTmpSize + i]->size();
                 predNo++)
            {
              (*tmpClauses[oldTmpSize + i])[predNo] =
                new Predicate(*((*resultingClauses[pgcIdx])[predNo]));
              if (clausedebug >= 2)
              {
                (*tmpClauses[oldTmpSize + i])[predNo]->
                  printWithStrVar(cout, domain);
                cout << " ";
              }
            }
            if (clausedebug >= 2) cout << endl;

              // Ground variables throughout clause
            for (int j = 0; 
                 j < (*resultingClauses[pgcIdx])[litIdx]->getNumTerms(); j++)
            {
              const Term* term =
                (*resultingClauses[pgcIdx])[litIdx]->getTerm(j);
                // Was a variable that has now been grounded
              if (term->getType() == Term::VARIABLE)
              {
                int varId = term->getId();
                int constId = (*indexedGndings)[i]->getTerm(j)->getId();
                assert(constId >= 0);
                  // Ground this var in lit and subsequent lits
                for (int k = litIdx; k < tmpClauses[oldTmpSize + i]->size();
                     k++)
                {
                  Predicate* pred = (*tmpClauses[oldTmpSize + i])[k];
                  for (int l = 0; l < pred->getNumTerms(); l++)
                  {
                      // Matching variable
                    if (pred->getTerm(l)->getId() == varId)
                      pred->setTermToConstant(l, constId);
                  }
                }
              }
            }
              // Indexed grounding can be deleted
            delete (*indexedGndings)[i];
            if (clausedebug >= 2)
            {
              cout << "Tmp clause after " << oldTmpSize + i << ": ";
              for (int d = 0; d < tmpClauses[oldTmpSize + i]->size(); d++)
              {
                (*tmpClauses[oldTmpSize + i])[d]->printWithStrVar(cout, domain);
                cout << " ";
              }
              cout << endl;
            }
          }
          delete indexedGndings;
        }
          // Replace the clauses in previous level with the new level
          // First, get rid of the old ones
        for (int pgcIdx = 0; pgcIdx < resultingClauses.size(); pgcIdx++)
        {
          for (int i = 0; i < resultingClauses[pgcIdx]->size(); i++)
            delete (*resultingClauses[pgcIdx])[i];
          delete resultingClauses[pgcIdx];
        }
        
        if (clausedebug >= 1)
        {
          cout << "tmpClauses size: " << tmpClauses.size() << endl;
        }
        
          // Expand or shrink resultingClauses depending on the no. of new ones
        if (tmpClauses.size() > resultingClauses.size())
        {
          resultingClauses.growToSize(tmpClauses.size());
        }
        else if (resultingClauses.size() > tmpClauses.size())
        {
          resultingClauses.shrinkToSize(tmpClauses.size());
        }
          // Copy the new ones into resultingClauses
        for (int i = 0; i < tmpClauses.size(); i++)
        {
          resultingClauses[i] = tmpClauses[i];
        }

        if (clausedebug >= 1)
        {
          cout << "Resulting clauses size after: " << resultingClauses.size()
               << endl;
        }
      }
        // Reached first non-indexable literal
      else
      {
        break;
      }
    }  // For all literals

    if (clausedebug >= 2)
    {
      cout << "Resulting clauses returned from grounding indexables: " << endl;
      for (int pgcIdx = 0; pgcIdx < resultingClauses.size(); pgcIdx++)
      {
        cout << "\t";
        for (int i = 0; i < resultingClauses[pgcIdx]->size(); i++)
        {
          (*resultingClauses[pgcIdx])[i]->printWithStrVar(cout, domain);
          cout << " ";
        }
        cout << endl;
      }
    }

    return;
  }

/**
 * get Active Clauses unifying with the given predicate - if ignoreActivePreds 
 * is true, this is equivalent to getting all the unsatisfied clauses
 * 
 * @return true if memory is still available to keep activating. If no more
 * memory is available to activate further, false is returned.
 */
bool getActiveClausesAndCnt(Predicate*  const & gndPred,
                            const Domain* const & domain,
                            Array<GroundClause *>* const & activeGroundClauses,
                            int & activeClauseCnt,
                            GroundPredicateHashArray* const& seenGndPreds,
                            bool const & ignoreActivePreds,
                            bool const & getSatisfied)
{  
    //create mapping of variable ids (e.g. -1) to variable addresses,
    //note whether they have been grounded, and store their types   
  createVarIdToVarsGroundedType(domain); 
  double stillActivating = 1;
  if (gndPred == NULL)
  {
    stillActivating = countNumTrueGroundings(domain, domain->getDB(), false,
                                             false, -1, NULL, NULL, NULL, NULL,
                                             NULL, NULL, activeGroundClauses,
                                             &activeClauseCnt, seenGndPreds,
                                             ignoreActivePreds, getSatisfied);
    restoreVars();
  }
  else
  {
	assert(gndPred->isGrounded());

      	//store the indexes of the predicates that can be grounded as gndPred
    Array<int> gndPredIndexes;
    for (int i = 0; i < predicates_->size(); i++)
	  if ((*predicates_)[i]->canBeGroundedAs(gndPred)) gndPredIndexes.append(i);
    
    const Database* db = domain->getDB();
    Array<int> unarySet;
	unarySet.append(-1);

	activeClauseCnt = 0;
	for (int i = 0; i < gndPredIndexes.size(); i++)
	{
      //ground the predicates in current combination
      bool sameTruthValueAndSense; //a ground pred has the same tv and sense
      bool gndPredPosSameSense;
      unarySet[0] = i; //gndPredIndexes[i];
	  	//cout<<"size of unary predicate set "<<unarySet.size()<<endl;
	  	//cout<<"Element[0] = "<<unarySet[0]<<endl;
	  groundPredicates(&unarySet, gndPredIndexes, gndPred,
                       db->getValue(gndPred), db, sameTruthValueAndSense,
                       gndPredPosSameSense);
      int cnt;
      stillActivating = countNumTrueGroundings(domain, domain->getDB(), false,
                                               false, -1, NULL, NULL, NULL,
                                               NULL, NULL, NULL,
                                               activeGroundClauses, & cnt,
                                               seenGndPreds, ignoreActivePreds,
                                               getSatisfied);
	  activeClauseCnt += cnt;
	  restoreVars();
	}
  }

  assert(!activeGroundClauses ||
         activeGroundClauses->size() == activeClauseCnt);
  return (stillActivating == 1);
} 


public:

/**
 * Retrieves active clauses unifying with the given predicate - if
 * ignoreActivePreds is true, this is equivalent to getting all the
 * unsatisfied clauses. Returns the groundClauses in activeGroundClauses
 * 
 * @param gndPred Predicate with which clauses must unify.
 * @param domain Domain in which the clauses occur
 * @param activeGroundClauses Array to hold the retrieved clauses.
 * @param seenGndPreds HashArray of seen ground predicates.
 * @param ignoreActivePreds If true active predicates are ignored and only
 * unsatisfied clauses are retrieved.
 * 
 * @return true if memory is still available to keep activating. If no more
 * memory is available to activate further, false is returned.
 */
bool getActiveClauses(Predicate* const & gndPred, 
                      const Domain* const & domain,
                      Array<GroundClause *>* const & activeGroundClauses,
                      GroundPredicateHashArray * const & seenGndPreds,
					  bool const & ignoreActivePreds)
{
  int cnt = 0;
  bool getSatisfied = false;
  return getActiveClausesAndCnt(gndPred, domain, activeGroundClauses, cnt,
                                seenGndPreds, ignoreActivePreds, getSatisfied);
                         
} 


//get the count of Active Clauses unifying with the given predicate
//- if ignoreActivePreds is true, this is equivalent to getting the
//count of all the unsatisfied clauses
int getActiveClauseCnt(Predicate*  const & gndPred, 
                       const Domain* const & domain,
                       bool const & ignoreActivePreds)
{
  int cnt = 0;
   
  Array<GroundClause *> *activeGroundClauses = NULL;
  GroundPredicateHashArray* const & seenGndPreds = NULL;
  bool getSatisfied = false;
  getActiveClausesAndCnt(gndPred, domain, activeGroundClauses, cnt,
                         seenGndPreds, ignoreActivePreds, getSatisfied);

  return cnt;
}

//hack for length three

  /*
void reorder(){
  if (predicates_->size() == 3){
    IntHashArray tmp;
    int dup = -1;
    for(int jx = 0; jx < predicates_->size(); jx++){
      if (tmp.contains((*predicates_)[jx]->getId())){
	dup = (*predicates_)[jx]->getId();
      }
      tmp.append((*predicates_)[jx]->getId());
      
    }
    int last = -1;
    for(int jx = 0; jx < predicates_->size(); jx++){
      if ((*predicates_)[jx] != dup){
	last = jx;
      }
    }
    if (tmp.size() == 2){// && (last != (predicates_->size()-1))){
      Predicate* tmp = (*predicates_)[last]; 
      if (last == 0){
	predicates[0] = predicates[1];
	predicates[1] = predicates[2];
	predicates[2] = tmp;  
      }
      else if (last == 1){

      }
      canonicalize(false);
    }
  }
}
  */

 Array<Array<int>*>* rep(Domain* domain){
   Array<int> map;
   Array<Array<int>*>* rep = new Array<Array<int>*>;
   //map.growToSize(domain->getNumPredicates()+1);
   for(int ix = 0; ix <= domain->getNumPredicates(); ix++){
     map.append(-1);
   }
   //Array<int> code;
   int predId = 1;
   for(int ix =0 ; ix < predicates_->size(); ix++){
     Predicate* currPred = (*predicates_)[ix];
     Array<int>* predInt = new Array<int>;
     int currId = map[(*predicates_)[ix]->getId()]; //
     if (currId == -1){
       map[(*predicates_)[ix]->getId()] = predId;
       if (currPred->getSense()){
	 predInt->append(predId+1);
	 //rep->append(predId+1);
       }
       else{
	 predInt->append(predId);
	 //rep->append(predId);
       }
       predId +=2;
     }
     else{
       if (currPred->getSense()){
	 predInt->append(currId+1);
	 //rep->append(currId+1);
       }
       else{
	 predInt->append(currId);
	 //rep->append(currId);
       }

     }
     for(int jx = 0; jx < currPred->getNumTerms(); jx++){
       int termId = currPred->getTerm(jx)->getId();
       predInt->append(termId);
       //code.append(-termId);
     }
     rep->append(predInt);
   }
   
   return rep;
 }
 int secondOrder(Domain* domain){
   Array<int> map;
   //map.growToSize(domain->getNumPredicates()+1);
   for(int ix = 0; ix <= domain->getNumPredicates(); ix++){
     map.append(-1);
   }
   Array<int> code;
   int predId = 1;
   for(int ix =0 ; ix < predicates_->size(); ix++){
     Predicate* currPred = (*predicates_)[ix];
     int currId = map[(*predicates_)[ix]->getId()]; //
     if (currId == -1){
       map[(*predicates_)[ix]->getId()] = predId;
       if (currPred->getSense()){
	 code.append(predId+1);
       }
       else{
	 code.append(predId);
       }
       predId +=2;
     }
     else{
       if (currPred->getSense()){
	 code.append(currId+1);
       }
       else{
	 code.append(currId);
       }

     }
     for(int jx = 0; jx < currPred->getNumTerms(); jx++){
       int termId = currPred->getTerm(jx)->getId();
       code.append(-termId);
     }
   }
   int mult = 10;
   int score = code[code.size()-1];
   for(int ix = code.size()-2; ix >= 0; ix--){
     score = score + mult * code[ix];
     mult = mult * 10;
   } 
   return score;
 }

private:
  
  static void sortByLen(Array<Clause*>& clauses, const int& l, const int& r)
  {
    Clause** items = (Clause**) clauses.getItems();
    if (l >= r) return;
    Clause* tmp = items[l];
    items[l] = items[(l+r)/2];
    items[(l+r)/2] = tmp;

    int last = l;
    for (int i = l+1; i <= r; i++)
      if (items[i]->getNumPredicates() < items[l]->getNumPredicates())
      {
        ++last;
        Clause* tmp = items[last];
        items[last] = items[i];
        items[i] = tmp;
      }
    
    tmp = items[l];
    items[l] = items[last];
    items[last] = tmp;
    sortByLen(clauses, l, last-1);
    sortByLen(clauses, last+1, r); 
  }


private:
  double wt_;
  Array<Predicate*>* predicates_;
  Array<int>* intArrRep_;
  size_t hashCode_;
  bool dirty_;
  bool isHardClause_;

    // (*varIdToVarsGroundedType_)[v] is the VarsGroundType of variable -v
    // start accessing this array from index 1
  Array<VarsGroundedType*>* varIdToVarsGroundedType_ ;

  AuxClauseData* auxClauseData_;
  static ClauseSampler* clauseSampler_;
  static double fixedSizeB_;
  bool jdebug;
};


////////////////////////////////// hash /////////////////////////////////

class HashClause
{
 public:
  size_t operator()(Clause* const & c) const  { return c->hashCode(); }
};


class EqualClause
{
 public:
  bool operator()(Clause* const & c1, Clause* const & c2) const
  { return c1->same(c2); }
};


class EqualClauseOp
{
 public:
    //the auxClauseData_ of c1 and c2 must be NON-NULL
  bool operator()(Clause* const & c1, Clause* const & c2) const
  { 
    AuxClauseData* acd1 = c1->getAuxClauseData();
    AuxClauseData* acd2 = c2->getAuxClauseData();
    if (acd1->op != acd2->op) return false;
    if (acd1->op == OP_ADD) return c1->same(c2);
    if (acd1->op == OP_REMOVE) 
      return acd1->removedClauseIdx == acd2->removedClauseIdx;
      //acd1->op is OP_REPLACE || OP_REPLACE_ADDPRED || OP_REPLACE_REMPRED
    return acd1->removedClauseIdx == acd2->removedClauseIdx && c1->same(c2);
  }
};


/////////////////////////// containers /////////////////////////////////

typedef hash_set<Clause*, HashClause, EqualClause> ClauseSet;
typedef hash_set<Clause*, HashClause, EqualClauseOp> ClauseOpSet;
typedef hash_map<Clause*, Array<Clause*>*, HashClause, EqualClause> 
  ClauseToClausesMap;

typedef HashList<Clause*, HashClause, EqualClause> ClauseHashList;
//typedef HashArray<Clause*, HashClause, EqualClause> ClauseHashArray;
typedef HashArray<Clause*, HashClause, EqualClauseOp> ClauseOpHashArray;

#endif

