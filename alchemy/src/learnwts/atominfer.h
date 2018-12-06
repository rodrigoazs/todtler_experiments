#ifndef ATOMINFER_H_JAN_07_2008
#define ATOMINFER_H_JAN_07_2008



  static  Array< Clause *>* getPartialGroundings(Clause* clause, Predicate* gndPred){
    
    Array<Clause*>* groundClauses = new Array< Clause *>();
    int gId = gndPred->getId();
    
    const Array< Predicate * > * cPredicates = clause->getPredicates();
    for(int ix = 0; ix < cPredicates->size(); ix++){
      int curId = ((*cPredicates)[ix])->getId();
      if (curId == gId){
	Clause* cGnd = new Clause();
	for(int jx = 0; jx < cPredicates->size(); jx++){
	cGnd->appendPredicate((*cPredicates)[jx]);
	}
	//  hash_map<int,Array<Term*>*>* constIdToTerms=new hash_map<int,Array<Term*>*>;
	hash_map<int,int>* term2const = new hash_map<int,int>;
	for(int jx = 0; jx < gndPred->getNumTerms(); jx++){
	  int tId = ((*cPredicates)[jx])->getTerm(jx)->getId();
	  int cId = gndPred->getTerm(jx)->getId();
	  (*term2const)[tId] = cId;
	}
	for(int jx = 0; jx < cGnd->getNumPredicates(); jx++){
	  Predicate* pred = cGnd->getPredicate(jx);
	  for(int kx = 0; kx < pred->getNumTerms(); kx++){
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
	cGnd->canonicalize();
	groundClauses->append(cGnd);
      }
    }
    
    return groundClauses;
  }
#endif
