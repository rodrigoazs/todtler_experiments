    //Returns value. Gradients are set in gradient.
//domainIdx = 0 --- Flag
  void getProbs(const double* const & wt,
		const int& arrSize, const int& domainIdx)
  {
    long double wpll = 0; // weighted pseudo-log-likelihood


      //used if sampling ground predicates
    long double tmpPerPredPll;
    long double* tmpPerPredGrad = NULL;

    
    Array<Array<Array<Array<IndexAndCount*>*>*>*>* gndPredClauseIndexesAndCounts
      = (*gndPredClauseIndexesAndCountsArr_)[domainIdx];

    int numPreds = gndPredClauseIndexesAndCounts->size();
    for (int p = 0; p < numPreds; p++) // for each predicate
    {
      if (!(*areNonEvidPreds_)[p]) continue; 

      //commented out: even though pred does not appear in any clause, each
      //of its groundings contribute -ln(2) to the wpll
      //if ((*gndPredClauseIndexesAndCounts)[p] == NULL) continue;

      long double perPredPll = 0;
      memset(perPredGrad, 0, arrSize*sizeof(long double));

        //if pred p appears in one or more clauses
      if ((*gndPredClauseIndexesAndCounts)[p] != NULL)
      {
        Array<Array<Array<IndexAndCount*>*>*>* gndingsToClauseIndexesAndCounts 
          = (*gndPredClauseIndexesAndCounts)[p];

	//use all groundings of predicate
	int numGnds = gndingsToClauseIndexesAndCounts->size();
	assert(numGnds == (*((*numGndings_)[domainIdx]))[p]);
	for (int g = 0; g < numGnds; g++) // for each grounding
	  computePerPredLL(gndingsToClauseIndexesAndCounts, g, wt);
      
      }
  }
