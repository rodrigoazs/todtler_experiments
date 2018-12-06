#include "markovBlanket.h"
#include <map>

//================Public methods==============================//
MarkovBlanketsFinder::MarkovBlanketsFinder(FeatureArray* featArray,
		double threshold, int countIsAtLeast) :
	featArray_(featArray), threshold_(threshold),
			countIsAtLeast_(countIsAtLeast)
{
	allFeatureValues_ = new Array<Array<bool>*>;
	const Array<Array<bool>*>* featValues = featArray_->getFeatureValues();
	const Array<Array<bool>*>* compFeatValues =
			featArray->getCompFeatureValues();

	assert(featValues->size() == compFeatValues->size());
	for (int i = 0; i < featValues->size(); i++) {
		Array<bool>* currArray = new Array<bool>;
		currArray->append((*featValues)[i]);
		currArray->append((*compFeatValues)[i]);

		assert(currArray->size() == featArray_->getTotalNumFeatures());
		allFeatureValues_->append(currArray);
	}

	//construct the markov blankets
	markovBlankets_ = new Array<set<int>*>;
	for (int i = 0; i < featArray_->getTotalNumFeatures(); i++)
		markovBlankets_->append(new set<int>);

	//construct the clause candidates
	clauseCandidates_ = new Array<Array<int>*>;

}

MarkovBlanketsFinder::~MarkovBlanketsFinder()
{
	for (int i = 0; i < allFeatureValues_->size(); i++)
		delete (*allFeatureValues_)[i];
	delete allFeatureValues_;

	for (int i = 0; i < markovBlankets_->size(); i++)
		delete (*markovBlankets_)[i];
	delete markovBlankets_;

	for (int i = 0; i < clauseCandidates_->size(); i++)
		delete (*clauseCandidates_)[i];
	delete clauseCandidates_;
}

void MarkovBlanketsFinder::growShrink()
{
	//Calclulate ordering
	assert(allFeatureValues_->size() > 0);

	int numFeatures = featArray_->getTotalNumFeatures();

	Array<Array<double> > uncondIndepMatrix;
	uncondIndepMatrix.growToSize(numFeatures, 0);
	for (int i = 0; i < numFeatures; i++)
		uncondIndepMatrix[i].growToSize(numFeatures, 0);

	set<int> *dummyMbX = new set<int>;

	for (int x = 0; x < numFeatures - 1; x++)
		for (int y = x+1; y < numFeatures; y++) {
			uncondIndepMatrix[x][y] = uncondIndepMatrix[y][x]
					= chiSquareAlphaValue(x, y, dummyMbX);
		}

	Array<int> featureOrdering;
	calculateHeuristicFeatureOrdering(uncondIndepMatrix, featureOrdering);

	Array<bool> doneOnes;
	doneOnes.growToSize(numFeatures, false);

	assert(featureOrdering.size() == numFeatures);

	for (int i = 0; i < featureOrdering.size(); i++) {
		int currFeatIndex = featureOrdering[i];

		Array<int> subFeatureOrdering;
		calculateFeatureOrderingFromArray(uncondIndepMatrix[currFeatIndex],
				subFeatureOrdering);

		assert(subFeatureOrdering.size() == numFeatures);

		//grow phase
		for (int j = 0; j < subFeatureOrdering.size(); j++) {
			int currSubFeatIndex = subFeatureOrdering[j];

			if (currSubFeatIndex == currFeatIndex)
				continue;

			if (doneOnes[currSubFeatIndex] && (*markovBlankets_)[currSubFeatIndex]->find(currFeatIndex)
					== (*markovBlankets_)[currSubFeatIndex]->end())
				continue;

			double alphaValue = chiSquareAlphaValue(currFeatIndex,
					currSubFeatIndex, (*markovBlankets_)[currFeatIndex]);

			if (alphaValue < threshold_)
				(*markovBlankets_)[currFeatIndex]->insert(currSubFeatIndex);
		}

		//shrink phase
		set<int>::iterator iter = (*markovBlankets_)[currFeatIndex]->begin();
		while (iter != (*markovBlankets_)[currFeatIndex]->end()) {
			int itemRemoved = *iter;
			//cout << "Shrink phase: the current subfeature is " << itemRemoved << endl;
			(*markovBlankets_)[currFeatIndex]->erase(iter);

			double alphaValue = chiSquareAlphaValue(currFeatIndex, itemRemoved,
					(*markovBlankets_)[currFeatIndex]);

			if (alphaValue < threshold_) {//put it back: it's needed
				pair<set<int>::iterator, bool> result = (*markovBlankets_)[currFeatIndex]->insert(itemRemoved);
				assert(result.second);
				iter = result.first;
				iter++;
			} else
				iter
						= (*markovBlankets_)[currFeatIndex]->lower_bound(itemRemoved);
		}
		doneOnes[currFeatIndex] = true;
	}

	//consolidation phase
	for (int i = 0; i < numFeatures; i++) {
		for (set<int>::iterator iter = (*markovBlankets_)[i]->begin(); iter != (*markovBlankets_)[i]->end(); iter++) {

			if ((*markovBlankets_)[*iter]->find(i) == (*markovBlankets_)[*iter]->end())
				(*markovBlankets_)[*iter]->insert(i);
		}
	}

	/*
	cout << "****Printing all Markov Blankets: ****\n";
	for (int i = 0; i < numFeatures; i++) {
		cout << "\t--Printing MB for feature " << i << endl;
		for (set<int>::iterator iter = (*markovBlankets_)[i]->begin(); iter != (*markovBlankets_)[i]->end(); iter++)
			cout << *iter << " ";
		cout << endl;
	}
	*/
	delete dummyMbX;
}

void MarkovBlanketsFinder::constructClausesFromMB(
						  ClauseOpHashArray& constructedClauses, int maxVars, int maxPreds)
{
	makeIntArrayClauses();
	makeActualClauses(constructedClauses, maxVars, maxPreds);
}

void MarkovBlanketsFinder::growBlanketAndMakeClauses(
						     ClauseOpHashArray& constructedClauses, int maxVars, int maxPreds)
{
	growShrink();
	constructClausesFromMB(constructedClauses, maxVars, maxPreds);
}

//======================Grow-shrink helpers==========================//
//NOTE: the ranks array will be corrupted after calling this method
//orders the features in increasing magnitude of the ranks (i.e. pValues or avg pValues)
void MarkovBlanketsFinder::calculateFeatureOrderingFromArray(
		Array<double> & ranks, Array<int>& featureOrdering)
{
	featureOrdering.clear();
	for (int i = 0; i < ranks.size(); i++) {
		int minIndex = 0;
		for (int j = 1; j < ranks.size(); j++)
			if (ranks[j] < ranks[minIndex])
				minIndex = j;

		assert(ranks[minIndex] < 100);

		featureOrdering.append(minIndex);
		ranks[minIndex] = 100;
	}
}

//if the test is unreliable, it will return 1, which is essentially making the features independent
double MarkovBlanketsFinder::chiSquareAlphaValue(int xIndex, int yIndex,
		set<int>* markovBlanketX)
{

	Array<IndependenceCounts *> *theCounts = new Array<IndependenceCounts *>;
	calculateCounts(xIndex, yIndex, markovBlanketX, theCounts);
	//the smaller the alpha, the less likely the null hypo(that the two features are indep)
	double alphaValue = 1;

	for (int i = 0; i < theCounts->size(); i++) {
		IndependenceCounts *ic = (*theCounts)[i];

		if (!ic->allAreGreaterThan(countIsAtLeast_)) {
		  //cout << "Counts are not reliable. Not considering these ones\n";
			continue;
		}

		int m;
		double p;
		if (countIsAtLeast_ == 0) {
		  m = 3; //equivalent sample size: see p. 179 of Mitchell textbook
		  //m = 10; //equivalent sample size: see p. 179 of Mitchell textbook
		  p = 0.5;
		} else {
			p = m = 0;
		}
		double pm = p * m;

		double thisAlpha = 0;

		double theorX1Y1 = (ic->totalX1() * ic->totalY1() + pm) / (ic->total()
				+ m);
		double theorX1Y0 = (ic->totalX1() * ic->totalY0() + pm) / (ic->total()
				+ m);
		double theorX0Y1 = (ic->totalX0() * ic->totalY1() + pm) / (ic->total()
				+ m);
		double theorX0Y0 = (ic->totalX0() * ic->totalY0() + pm) / (ic->total()
				+ m);

		double chi = (theorX1Y1 - ic->x1y1) * (theorX1Y1 - ic->x1y1)
				/ theorX1Y1 + (theorX1Y0 - ic->x1y0) * (theorX1Y0 - ic->x1y0)
				/ theorX1Y0 + (theorX0Y1 - ic->x0y1) * (theorX0Y1 - ic->x0y1)
				/ theorX0Y1 + (theorX0Y0 - ic->x0y0) * (theorX0Y0 - ic->x0y0)
				/ theorX0Y0;

		//num of degrees of freedom is 1
		if (chi >= 7.882)
			thisAlpha = 0.005;
		else if (chi >= 6.637)
			thisAlpha = 0.01;
		else if (chi >= 5.025)
			thisAlpha = 0.025;
		else if (chi >= 3.843)
			thisAlpha = 0.05;
		else if (chi >= 2.706)
			thisAlpha = .1;
		else if (chi >= 1.323)
			thisAlpha = .25;
		else if (chi >= 0.455)
			thisAlpha = .5;
		else if (chi >= 0.102)
			thisAlpha = .7;
		else if (chi >= 0.016)
			thisAlpha = .9;
		else if (chi >= 0.004)
			thisAlpha = .95;
		else
			thisAlpha = .975;

		//above values were taken from http://www.statsoft.com/textbook/sttable.html

		if (thisAlpha < alphaValue)
			alphaValue = thisAlpha;

	}
	//clean up
	for (int i = 0; i < theCounts->size(); i++)
		delete (*theCounts)[i];
	delete theCounts;

	return alphaValue;
}

//The mb should not contain either x or y 
//Caller is responsible for deleting the contents of theCounts array
void MarkovBlanketsFinder::calculateCounts(int xIndex, int yIndex,
		set<int> *markovBlanketX, Array<IndependenceCounts *> *theCounts)
{
	Array<int> * mbX = new Array<int>;
	for (set<int>::iterator iter = markovBlanketX->begin(); iter
			!= markovBlanketX->end(); iter++)
		mbX->append(*iter);

	map<int, IndependenceCounts *> counts;
	for (int r = 0; r < allFeatureValues_->size(); r++) {
		int key = 0;
		int powerOf2 = 1;

		bool valueOfX = (*(*allFeatureValues_)[r])[xIndex];
		bool valueOfY = (*(*allFeatureValues_)[r])[yIndex];

		for (int mb = 0; mb < mbX->size(); mb++) {
			assert((*mbX)[mb] != xIndex && (*mbX)[mb] != yIndex);

			key += (*(*allFeatureValues_)[r])[(*mbX)[mb]] * powerOf2;
			powerOf2 *= 2;
		}

		//see if we already have a record of this
		IndependenceCounts *ic= NULL;
		if (counts.find(key) == counts.end()) {
			ic = new IndependenceCounts;
			ic->setAllToZero();
			counts.insert(make_pair(key, ic));
		} else
			ic = counts[key];

		if (valueOfX) {
			if (valueOfY)
				ic->x1y1++;
			else
				ic->x1y0++;
		} else {
			if (valueOfY)
				ic->x0y1++;
			else
				ic->x0y0++;
		}
	}

	for (map<int, IndependenceCounts*>::iterator iter = counts.begin(); iter
			!= counts.end(); iter++)
		theCounts->append(iter->second);

	delete mbX;
}

void MarkovBlanketsFinder::calculateHeuristicFeatureOrdering(
		Array<Array<double> > & uncondIndepMatrix, Array<int>& featureOrdering)
{
	int numFeatures = uncondIndepMatrix.size();
	Array<double> pRanks;
	pRanks.growToSize(numFeatures, 0);

	for (int x = 0; x < numFeatures; x++) {
		for (int y = 0; y < numFeatures; y++)
			if (y != x)
				pRanks[x] += uncondIndepMatrix[x][y];

		pRanks[x] /= ((double)numFeatures) - 1;
	}

	calculateFeatureOrderingFromArray(pRanks, featureOrdering);
}

//=======================Clause constructing methods======
//Caller is responsible for deleting contents of clauseCandidates
void MarkovBlanketsFinder::makeIntArrayClauses()
{
  const Array<NodeFeature*>* features = featArray_->getFeatures();
  //const Array<CompNodeFeature*>* compFeatures = featArray_->getCompFeatures();
  int targetFeatIndex = 0;
  
  Array<bool> isUnary;
  isUnary.growToSize(featArray_->getTotalNumFeatures(), false);
  for (int i = 0; i < features->size(); i++)
    isUnary[i] = ((*features)[i]->getNumTerms() == 1);
  
  map<int, int> doWeHaveIt;
  set<int>* targetFeatBlt= (*markovBlankets_)[targetFeatIndex];
  
  Array<Array<int>*>* workingArray1 = new Array<Array<int>*>; //clean up
  Array<Array<int>*>* workingArray2 = new Array<Array<int>*>;
  Array<int>* workingClauseIds1 = new Array<int>; //clean up
  Array<int>* workingClauseIds2 = new Array<int>;
  
  Array<int>* firstClause = new Array<int>;
  firstClause->append(targetFeatIndex);
  workingArray1->append(firstClause);
  workingClauseIds1->append((int) pow(2.0, targetFeatIndex));
  
  while (workingArray1->size() > 0) {
    assert(workingArray1->size() == workingClauseIds1->size());
    for (int i = 0; i < workingArray1->size(); i++) {
      Array<int> *currClause = (*workingArray1)[i];
      int currClauseId = (*workingClauseIds1)[i];
      
      for (set<int>::iterator iter = targetFeatBlt->begin(); iter
	     != targetFeatBlt->end(); iter++) {
	
	bool found = false;
	for (int j = 0; j < currClause->size() && !found; j++)
	  found = (*currClause)[j] == *iter;
	
	if (!found) {
	  int newClauseId = currClauseId + (int) pow(2.0, *iter);
	  if (doWeHaveIt.find(newClauseId) == doWeHaveIt.end()) {
	    Array<int>* newClause = new Array<int>(*currClause);
	    newClause->append(*iter);
	    doWeHaveIt.insert(make_pair(newClauseId, 1));
	    
	    if (isUnary[*iter]) {
	      Array<int>* forFuture = new Array<int>(*newClause);
	      workingArray2->append(forFuture);
	      workingClauseIds2->append(newClauseId);
	    }
	    //newClause->printWithoutWt(cout, );
	    //cout << endl;
	    if (clauseCandidates_->append(newClause) < 0)
	      delete newClause;
	  }
	}
	
      }
      delete currClause;
    }
    delete workingArray1;
    delete workingClauseIds1;
    workingArray1 = workingArray2;
    workingClauseIds1 = workingClauseIds2;
    
    workingArray2 = new Array<Array<int>*>;
    workingClauseIds2 = new Array<int>;
  }
}

//caller is responsible for deleting constructedClauses
void MarkovBlanketsFinder::makeActualClauses(
					     ClauseOpHashArray & constructedClauses, int maxVars, int maxPreds)
{
  const Array<NodeFeature*>* features = featArray_->getFeatures();
  const Array<CompNodeFeature*>* compFeatures = featArray_->getCompFeatures();
  
  for (int i = 0; i < clauseCandidates_->size(); i++) {
    Array<int>* currCandidate = (*clauseCandidates_)[i];
    Clause* newClause = new Clause();
    
    for (int j = 0; j < currCandidate->size(); j++) {
      if ((*currCandidate)[j] < features->size())
	copyFeatureAndAddToClause(newClause, (*features)[(*currCandidate)[j]], true);
      else {
	int index = (*currCandidate)[j] - features->size();
	assert(index < compFeatures->size());
	copyCompFeatureAndAddToClause(newClause, (*compFeatures)[index], true);
      }
    }
    
    //construct aux data   
    newClause->canonicalize();
    AuxClauseData * acd = new AuxClauseData();
    acd->op = OP_ADD;
    newClause->setAuxClauseData(acd);
    int ok = 0;
    if (newClause->getNumVariables() > maxVars){
      ok = -1;
    }
    else if (newClause->getNumPredicates() > maxPreds){
      ok = -1;
    }
    else{
      ok = constructedClauses.append(newClause);
    }
    if (ok >= 0) {
      Array<Clause *> flipClauses;
      flipClauses.append(newClause);
      
      for (int p = 0; p < newClause->getNumPredicates(); p++) {
	int currSize = flipClauses.size();
	for (int c = 0; c < currSize; c++) {
	  Clause *newnewClause = new Clause(*flipClauses[c]);
	  newnewClause->setDirty();
	  newnewClause->getPredicate(p)->setSense(false);
	  newnewClause->canonicalize();
	  flipClauses.append(newnewClause);
	}
      }
      for (int c = 1; c < flipClauses.size(); c++) {
	if (constructedClauses.append(flipClauses[c]) < 0)
	  delete flipClauses[c];
      }
    } else
      delete newClause;
  }
}//make actual clauses


void MarkovBlanketsFinder::copyFeatureAndAddToClause(Clause* clause,
		NodeFeature * feature, bool predSense)
{
	Predicate* copiedPred = new Predicate(*feature);
	copiedPred->setSense(predSense);
	clause->appendPredicate(copiedPred);
}

void MarkovBlanketsFinder::copyCompFeatureAndAddToClause(Clause* clause,
		CompNodeFeature * compFeature, bool predSense)
{
	Predicate* copiedPred1 = compFeature->getCopyOfPart1();
	Predicate* copiedPred2 = compFeature->getCopyOfPart2();

	copiedPred1->setSense(predSense);
	copiedPred2->setSense(predSense);

	clause->appendPredicate(copiedPred1);
	clause->appendPredicate(copiedPred2);
}
