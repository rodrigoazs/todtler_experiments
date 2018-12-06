#include "ut-structlearn.h"

/////////////////////// code to handle existential formulas //////////////////

struct ExistFormula {
	ExistFormula(const string& fformula) :
		formula(fformula), gain(0), wt(0), newScore(0), numPreds(0)
	{
	}
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

void UtStructLearn::deleteExistFormulas(Array<ExistFormula*>& existFormulas)
{
	existFormulas.deleteItemsAndClear();
}

void UtStructLearn::getMLNClauses(Array<Clause*>& initialMLNClauses,
		Array<ExistFormula*>& existFormulas)
{
	for (int i = 0; i < mln0_->getNumClauses(); i++)
		if (isModifiableClause(i)) {
			Clause* c = (Clause*) mln0_->getClause(i);
			initialMLNClauses.append(new Clause(*c));
		}

	const FormulaAndClausesArray* fnca = mln0_->getFormulaAndClausesArray();
	for (int i = 0; i < fnca->size(); i++)
		if (isNonModifiableFormula((*fnca)[i]))
			existFormulas.append(new ExistFormula((*fnca)[i]->formula));

	for (int i = 0; i < existFormulas.size(); i++) {
		string formula = existFormulas[i]->formula;
		FormulaAndClauses tmp(formula, 0, false);
		const FormulaAndClausesArray* fnca = (*mlns_)[0]->getFormulaAndClausesArray();
		int a = fnca->find(&tmp);
		existFormulas[i]->numPreds = (*fnca)[a]->numPreds;

		Array<Array<Clause*> >& cnfClausesForDomains =
				existFormulas[i]->cnfClausesForDomains;
		cnfClausesForDomains.growToSize(mlns_->size());
		for (int j = 0; j < mlns_->size(); j++) {
			Array<Clause*>& cnfClauses = cnfClausesForDomains[j];
			fnca = (*mlns_)[j]->getFormulaAndClausesArray();
			a = fnca->find(&tmp);
			assert(a >= 0);
			IndexClauseHashArray* indexClauses = (*fnca)[a]->indexClauses;
			for (int k = 0; k < indexClauses->size(); k++) {
				Clause* c = new Clause(*((*indexClauses)[k]->clause));
				c->newAuxClauseData();
				cnfClauses.append(c);
			}
			cnfClauses.compress();
		}
	}
}

//cnfClauses enter & exit function with its AuxClauseData's cache == NULL
inline
void UtStructLearn::pllCountsForExistFormula(Clause* cnfClause,
		const int& domainIdx, int* clauseIdxInMln,
		Array<UndoInfo*>* const & undoInfos)
{
	assert(cnfClause->getAuxClauseData()->cache == NULL);
	bool inCache = false;
	bool hasDomainCounts = false;
	double prevCNFClauseSize = 0;

	if (cacheClauses_) {
		int i;
		if ((i = cachedClauses_->find(cnfClause)) >= 0) //if clause is in cache
		{
			inCache = true;
			Array<Array<Array<CacheCount*>*>*>* cache = (*cachedClauses_)[i]->getAuxClauseData()->cache;
			Array<Array<CacheCount*>*>* domainCache = (*cache)[domainIdx];
			for (int j = 0; j < domainCache->size(); j++)
				if ((*domainCache)[j] != NULL) {
					hasDomainCounts = true;
					break;
				}
		}

		if (hasDomainCounts) // if clause and counts for domain are cached
		{
			pll_->insertCounts(clauseIdxInMln, undoInfos, (*cachedClauses_)[i]->getAuxClauseData()->cache, domainIdx);
			return;
		} else {
			if (cacheSizeMB_ < maxCacheSizeMB_) {
				if (inCache) //if clause is in cache but not counts for domain
				{
					cnfClause->getAuxClauseData()->cache = (*cachedClauses_)[i]->getAuxClauseData()->cache;
					prevCNFClauseSize = cnfClause->sizeMB();
				} else
					cnfClause->newCache(domains_->size(), (*domains_)[0]->getNumPredicates());
			} else {
				static bool printCacheFull = true;
				if (printCacheFull) {
					cout << "Cache is full, approximate size = "
							<< cacheSizeMB_ <<" MB" << endl;
					printCacheFull = false;
				}
			}
		}
	}

	//we do not have the counts

	pll_->computeCountsForNewAppendedClause(cnfClause, clauseIdxInMln,
			domainIdx, undoInfos, sampleClauses_, cnfClause->getAuxClauseData()->cache);

	if (cnfClause->getAuxClauseData()->cache) {
		if (inCache) {
			cacheSizeMB_ += cnfClause->sizeMB() - prevCNFClauseSize;
			cnfClause->getAuxClauseData()->cache = NULL;
		} else {
			if (cacheSizeMB_ < maxCacheSizeMB_) {
				cacheSizeMB_ += cnfClause->sizeMB();
				Array<Array<Array<CacheCount*>*>*>* cache =
						cnfClause->getAuxClauseData()->cache;
				cnfClause->getAuxClauseData()->cache = NULL;
				Clause* copyClause = new Clause(*cnfClause);
				copyClause->getAuxClauseData()->cache = cache;
				copyClause->compress();
				cachedClauses_->append(copyClause);
			} else {
				cnfClause->getAuxClauseData()->deleteCache();
				cnfClause->getAuxClauseData()->cache = NULL;
			}
		}
	}
}

inline
void UtStructLearn::printNewScore(const string existFormStr,
		const int& lbfgsbIter, const double& lbfgsbSec, const double& newScore,
		const double& gain, const double& penalty, const double& wt)
{
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
	cout << "time taken by LBFGSB = ";
	Timer::printTime(cout, lbfgsbSec);
	cout << endl;
	cout << "*************************** " << endl << endl;;
}

inline
void UtStructLearn::rquicksort(Array<ExistFormula*>& existFormulas,
		const int& l, const int& r)
{
	ExistFormula** items = (ExistFormula**) existFormulas.getItems();
	if (l >= r)
		return;
	ExistFormula* tmp = items[l];
	items[l] = items[(l+r)/2];
	items[(l+r)/2] = tmp;

	int last = l;
	for (int i = l+1; i <= r; i++)
		if (items[i]->gain > items[l]->gain) {
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

inline
void UtStructLearn::rquicksort(Array<ExistFormula*>& ef)
{
	rquicksort(ef, 0, ef.size()-1);
}

// Compute counts for formula, and learn optimal weights and gain.
// If we are computing counts only, then the counts are added permanently into
// PseudoLogLikelihood.
inline
void UtStructLearn::evaluateExistFormula(ExistFormula* const& ef,
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
	for (int d = 0; d < cnfClausesForDomains.size(); d++) {
		Array<Clause*>& cnfClauses = cnfClausesForDomains[d];
		MLN* mln = (*mlns_)[d];
		for (int k = 0; k < cnfClauses.size(); k++) {
			Clause* c = cnfClauses[k];
			//if we adding counts permanently, then don't add counts that were added
			//previously; otherwise we can add the duplicate counts because they 
			//will be undone later.
			if (!undo && mln->containsClause(c))
				continue;
			int* tmpClauseIdxInMLN = new int(mln->getNumClauses() + k);
			idxPtrs.append(tmpClauseIdxInMLN);
			if (iacArraysPerDomain) {
				Array<Array<IndexAndCount*> >& iacArrays = (*iacArraysPerDomain)[d];
				assert(iacArrays.size() == cnfClauses.size());
				Array<IndexAndCount*>& iacArray = iacArrays[k];

				Array<UndoInfo*> tmpUndoInfos;
				pllCountsForExistFormula(c, d, tmpClauseIdxInMLN, &tmpUndoInfos);
				for (int i = 0; i < tmpUndoInfos.size(); i++)
					iacArray.append(tmpUndoInfos[i]->affectedArr->lastItem());
				if (undoInfos)
					undoInfos->append(tmpUndoInfos);
				else
					tmpUndoInfos.deleteItemsAndClear();
			} else
				pllCountsForExistFormula(c, d, tmpClauseIdxInMLN, undoInfos);
		}
	}

	//find optimal weights and gain of formula 

	if (evalGainLearnWts) {
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

		if (indexTrans_) {
			for (int d = 0; d < cnfClausesForDomains.size(); d++) {
				int numCNFClauses = cnfClausesForDomains[d].size();
				indexTrans_->appendClauseIdxToClauseFormulaIdxs(1,
						numCNFClauses);
			}
		}

		int iter;
		bool error;
		double elapsedSec;
		int numClausesFormulas = getNumClausesFormulas();
		Array<double>* wts = &(ef->wts);
		wts->growToSize(numClausesFormulas + numAdded + 1);

		double newScore = maximizeScore(numClausesFormulas, numAdded, wts, 
		NULL, NULL, iter, error, elapsedSec);

		if (indexTrans_) {
			for (int d = 0; d < cnfClausesForDomains.size(); d++) {
				int numCNFClauses = cnfClausesForDomains[d].size();
				indexTrans_->removeClauseIdxToClauseFormulaIdxs(1,
						numCNFClauses);
			}
		}

		double totalWt = 0;
		for (int i = 0; i < numAdded; i++)
			totalWt += (*wts)[numClausesFormulas+i+1];

		double penalty = ef->numPreds * penalty_;

		ef->gain = newScore-prevScore-penalty;
		ef->wt = totalWt;
		ef->newScore = newScore;

		if (error) {
			newScore=prevScore;
			cout<<"LBFGSB failed to find wts"<<endl;
		}
		printNewScore(ef->formula, iter, elapsedSec, newScore, newScore
				-prevScore, penalty, totalWt);
	}

	if (undo) {
		pll_->undoAppendRemoveCounts(undoInfos);
		delete undoInfos;
	}
	idxPtrs.deleteItemsAndClear();
}//evaluateExistFormula()


double UtStructLearn::evaluateExistFormulas(
		Array<ExistFormula*>& existFormulas,
		Array<ExistFormula*>& highGainWtFormulas, const double& prevScore)
{
	if (existFormulas.size() == 0)
		return 0;
	for (int i = 0; i < existFormulas.size(); i++)
		evaluateExistFormula(existFormulas[i], false, NULL, prevScore);

	Array<ExistFormula*> tmp(existFormulas);
	rquicksort(tmp);
	double minGain = DBL_MAX;
	cout << "evaluated existential formulas " << endl;
	cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
	for (int i = 0; i < tmp.size(); i++) {
		existFormulas[i] = tmp[i];
		double gain = existFormulas[i]->gain;
		double wt = existFormulas[i]->wt;
		cout << i << "\t" << existFormulas[i]->formula << endl << "\tgain = "
				<< gain << ",  wt = " << wt << ",  op = OP_ADD" << endl;
		if (gain > minGain_ && wt >= minWt_) {
			highGainWtFormulas.append(tmp[i]);
			if (gain < minGain)
				minGain = gain;
		}
	}
	cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl << endl;

	if (minGain == DBL_MAX)
		minGain = 0;
	return minGain;
}

inline
void UtStructLearn::appendExistFormulaToMLNs(ExistFormula* const & ef)
{
	Array<Array<Array<IndexAndCount*> > > iacsPerDomain;
	Array<Array<Clause*> >& cnfClausesForDomains = ef->cnfClausesForDomains;
	iacsPerDomain.growToSize(cnfClausesForDomains.size());
	for (int d = 0; d < cnfClausesForDomains.size(); d++) {
		Array<Array<IndexAndCount*> >& iacArrays = iacsPerDomain[d];
		iacArrays.growToSize(cnfClausesForDomains[d].size() );
	}
	evaluateExistFormula(ef, true, &iacsPerDomain, 0);

	Array<double>& wts = ef->wts;
	int numClausesFormulas = getNumClausesFormulas();

	//update weights before CNF clauses are added as duplicate clause wts are
	//accumulated as they are added.
	if (indexTrans_ == NULL)
		updateWts(wts, NULL, NULL);

	//append CNF clauses to MLN
	for (int d = 0; d < cnfClausesForDomains.size(); d++) {
		MLN* mln = (*mlns_)[d];
		Array<Array<IndexAndCount*> >& iacArrays = iacsPerDomain[d];
		Array<Clause*>& cnfClauses = cnfClausesForDomains[d];

		for (int i = 0; i < cnfClauses.size(); i++) {
			int idx;
			//when we are learning the weight of a formula (i.e. indexsTrans !=NULL)
			//its CNF clause weight don't matter, so set them to 0.
			double wt = (indexTrans_) ? 0 : wts[numClausesFormulas+i+1];
			//if cnfClauses[i] is already in MLN, its weights will be correctly set
			//in updateWts() later
			mln->appendClause(ef->formula, true, new Clause(*cnfClauses[i]), wt, false, idx);
			mln->setFormulaPriorMean(ef->formula, priorMean_);
			((MLNClauseInfo*)mln->getMLNClauseInfo(idx))->priorMean += priorMean_/cnfClauses.size();

			int* idxPtr = mln->getMLNClauseInfoIndexPtr(idx);
			Array<IndexAndCount*>& iacs = iacArrays[i];
			for (int j = 0; j < iacs.size(); j++)
				iacs[j]->index = idxPtr;
		}
	}
	assert(pll_->checkNoRepeatedIndex());

	if (indexTrans_) {
		//update weights after the formula has been added to the MLN
		Array<string> appendedFormula;
		appendedFormula.append(ef->formula);
		updateWts(wts, NULL, &appendedFormula);
		//MLNs has changed, the index translation must be recreated
		indexTrans_->createClauseIdxToClauseFormulaIdxsMap();
	}

	cout << "Modified MLN: Appended formula to MLN: " << ef->formula << endl;
}

inline
bool UtStructLearn::effectExistFormulaOnMLNs(ExistFormula* ef,
		Array<ExistFormula*>& existFormulas, double& score)
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

bool UtStructLearn::effectBestCandidateOnMLNs(Array<Clause*>& bestCandidates,
		Array<ExistFormula*>& existFormulas,
		Array<ExistFormula*>& highGainWtFormulas, double& score)
{
	cout << "effecting best candidate among existential formulas and "
			<< "best candidates on MLN..." << endl << endl;

	int a = 0, b = 0;
	bool ok = false;
	int numCands = bestCandidates.size() + highGainWtFormulas.size();
	for (int i = 0; i < numCands; i++) {
		if (a >= bestCandidates.size()) {
			if (ok=effectExistFormulaOnMLNs(highGainWtFormulas[b++],
					existFormulas, score))
				break;
		} else if (b >= highGainWtFormulas.size()) {
			cout << "effecting best candidate " << a << " on MLN..." << endl;
			if (ok=effectBestCandidateOnMLNs(bestCandidates[a++], score))
				break;
			cout << "failed to effect candidate on MLN." << endl;
			delete bestCandidates[a-1];
		} else if (highGainWtFormulas[b]->gain
				> bestCandidates[a]->getAuxClauseData()->gain) {
			if (ok=effectExistFormulaOnMLNs(highGainWtFormulas[b++],
					existFormulas, score))
				break;
		} else {
			cout << "effecting best candidate " << a << " on MLN..." << endl;
			if (ok=effectBestCandidateOnMLNs(bestCandidates[a++], score))
				break;
			cout << "failed to effect candidate on MLN." << endl;
			delete bestCandidates[a-1];
		}
	}

	for (int i = a; i < bestCandidates.size(); i++)
		delete bestCandidates[i];
	return ok;
}
