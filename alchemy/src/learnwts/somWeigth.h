  void computeCountsRemoveCountsHelper(bool computeCounts,
                                       const Array<Clause*>* const & c,
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

    for(int ix = 0; ix < c->size(); ix++){
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
	      createMappingOfVarIdToMultipliersAndVarGroundingsAndOffset(gndPred, domain, multipliers, offsetDueToConstants, varIdToMults, 
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
      
    }
    //flag -
    for (int i = 0; i < seenPreds.size(); i++)  delete seenPreds[i];

  }

//computeCountsRemoveCountsHelper()
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

      double cnt =
        ((Clause*)clause)->countDiffNumTrueGroundings(&gndPred, domain, db,
                                                      DB_HAS_UNKNOWN_PREDS,
						      sampleClauses, c);
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
