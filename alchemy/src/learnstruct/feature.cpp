#include "feature.h"

//==============================================================
//                   NodeFeature
//==============================================================

//===============Public methods implementations================//
NodeFeature::NodeFeature(Predicate* currentPred, int& nextVariableValue) :
	Predicate(currentPred->getTemplate())
{

	for (int i = 0; i < currentPred->getNumTerms(); i++) {
		Term * newTerm = new Term(nextVariableValue--, this, true);
		sharingsMask_.append(newTerm->getId());
		appendTerm(newTerm);
	}
}

NodeFeature::NodeFeature(Predicate* currentPred, Predicate* centralPred,
		NodeFeature* centralFeature, int &nextVariableValue) :
	Predicate(currentPred->getTemplate())
{
	prepareSharingsMask(currentPred, centralPred, centralFeature);

	assert(sharingsMask_.size() == currentPred->getNumTerms());

	for (int i = 0; i < currentPred->getNumTerms(); i++) {
		int termValue = selectTermValue(currentPred, nextVariableValue, i,
				sharingsMask_);
		Term * newTerm = new Term(termValue, this, true);
		appendTerm(newTerm);
	}
}

NodeFeature::NodeFeature(RelPath* relPath, Predicate* centralPred,
		NodeFeature* firstPart, NodeFeature* centralFeature,
		int &nextVariableValue) :
	Predicate(relPath->repPred->getTemplate())
{
	prepareSharingsMask(relPath->repPred, centralPred, centralFeature);

	assert(sharingsMask_.size() == relPath->repPredSharedIndices->size());

	Array<int> tmpAllSharingsMask;
	//tmpAllSharingsMask.append(sharingsMask_); 
	tmpAllSharingsMask.growToSize(relPath->repPred->getNumTerms(), 0);

	for (int i = 0; i < relPath->repPredSharedIndices->size(); i++) {
		int currEntry = (*relPath->repPredSharedIndices)[i];
		if (currEntry != -1) {
			if (tmpAllSharingsMask[i] == 0)
				tmpAllSharingsMask[i] = firstPart->getTerm(currEntry)->getId();
			else
				assert(tmpAllSharingsMask[i] == firstPart->getTerm(currEntry)->getId());
		}
	}

	for (int i = 0; i < relPath->repPred->getNumTerms(); i++) {
		int termValue = selectTermValue(relPath->repPred, nextVariableValue, i,
				tmpAllSharingsMask);
		Term * newTerm = new Term(termValue, this, true);
		appendTerm(newTerm);
	}
}

bool NodeFeature::isSemSame(NodeFeature* nf)
{
	if (getId() != nf->getId())
		return false;

	//compare the masks

	for (int i = 0; i < sharingsMask_.size(); i++)
		if (sharingsMask_[i] != nf->sharingsMask_[i])
			return false;

	return matchesPredicate(nf);
}

bool NodeFeature::matchesPredicate(Predicate* gndPred)
{
	if (gndPred->getId() != getId())
		return false;

	for (int i = 0; i < gndPred->getNumTerms(); i++)
		for (int j = i; j < gndPred->getNumTerms(); j++)
			if (gndPred->getTerm(i)->getId() == gndPred->getTerm(j)->getId() && getTerm(i)->getId() != getTerm(j)->getId() || gndPred->getTerm(i)->getId() != gndPred->getTerm(j)->getId() && getTerm(i)->getId() == getTerm(j)->getId() )
				return false;
	return true;
}

bool NodeFeature::containsVariable(int varId)
{
	for (int i = 0; i < getNumTerms(); i++)
		if (getTerm(i)->getId() == varId)
			return true;

	return false;
}

void NodeFeature::print(Domain* domain)
{
	cout << "----------------------\n";
	Predicate::print(cout, domain);
	cout << endl;
	// for (int i = 0; i < sharingsMask_.size(); i++) 
	//  cout << sharingsMask_[i] << " ";

	//cout << endl;
}

//==============Utility methods implementations============//

void NodeFeature::prepareSharingsMask(Predicate* currentPred,
		Predicate* centralPred, NodeFeature* centralFeature)
{
	sharingsMask_.growToSize(currentPred->getNumTerms(), 0);
	for (int i = 0; i < currentPred->getNumTerms(); i++)
		for (int j = 0; j < centralPred->getNumTerms(); j++)
			if (currentPred->getTerm(i)->getId() == centralPred->getTerm(j)->getId())
				sharingsMask_[i] = centralFeature->getTerm(j)->getId();
}

int NodeFeature::selectTermValue(Predicate* currentPred,
		int& nextVariableValue, int termIdx, Array<int>& setValues)
{
	int chosenValue = 0;

	if (setValues[termIdx] < 0)
		chosenValue = setValues[termIdx];
	else {
		//first see if this is a repeated constant in this grounding
		for (int j = termIdx-1; j >= 0; j--)
			if (currentPred->getTerm(termIdx)->getId() == currentPred->getTerm(j)->getId()) {
				chosenValue = getTerm(j)->getId();
				break;
			}

		if (chosenValue == 0)
			chosenValue = nextVariableValue--;
	}
	assert(chosenValue < 0);
	return chosenValue;
}

//==============================================================
//                   CompNodeFeature
//==============================================================
CompNodeFeature::CompNodeFeature(Predicate* currentPred, RelPath* relPath,
		Predicate* centralPred, NodeFeature* centralFeature,
		int& nextVariableValue, int maxNumFreeVars)
{
	//check if this is a valid path wrt centralPred
	isValid_ = false;
	for (int i = 0; i < relPath->repPred->getNumTerms() && !isValid_; i++) {
		int currConst = relPath->repPred->getTerm(i)->getId();
		isValid_ = (currentPred->containsConstant(currConst)
				&& !centralPred->containsConstant(currConst));
	}
	if (isValid_)
		isValid_ = calculateNumFreeVars(centralPred, currentPred,
				relPath->repPred) <= maxNumFreeVars;//1;

	if (isValid_) {
		part1_ = new NodeFeature(currentPred, centralPred,
				centralFeature, nextVariableValue);

		part2_ = new NodeFeature(relPath, centralPred, part1_,
				centralFeature, nextVariableValue);

		sharedOfPart2_.append(relPath->repPredSharedIndices);
		sharedOfPart1_.append(relPath->sharedIndices);
	} else {
		part1_ = NULL;
		part2_ = NULL;
	}
}

bool CompNodeFeature::isSemSame(CompNodeFeature* cnf)
{
	//checks for symmetric sameness

	//first direction
	bool isSame = part1_->isSemSame(cnf->part1_)
			&& part2_->isSemSame(cnf->part2_);

	//is the same stuff shared between the two parts
	for (int i = 0; i < sharedOfPart2_.size() && isSame; i++) {
		if (sharedOfPart2_[i] != cnf->sharedOfPart2_[i])
			isSame = false;
	}

	if (isSame)
		return true;

	//second direction: compare part 1 to part 2
	isSame = part1_->isSemSame(cnf->part2_) && part2_->isSemSame(cnf->part1_);

	for (int i = 0; i < sharedOfPart1_.size() && isSame; i++) {
		if (sharedOfPart1_[i] != cnf->sharedOfPart2_[i])
			isSame = false;
	}
	return isSame;
}

void CompNodeFeature::print(Domain* domain)
{
	if (!isValid_) {
		cout << "Feature not valid\n";
		return;
	}

	cout << "============================\n";
	part1_->print(domain);
	part2_->print(domain);
	cout << "============================\n";
}

Predicate* CompNodeFeature::getCopyOfPart1()
{
	Predicate* castPred = part1_;
	return new Predicate(*castPred);
}
Predicate* CompNodeFeature::getCopyOfPart2()
{
	Predicate* castPred = part2_;
	return new Predicate(*castPred);
}

int CompNodeFeature::calculateNumFreeVars(Predicate* centralPred,
		Predicate* currentPred, Predicate* repPred)
{
	Array<bool> currPredFreeVars;
	Array<bool> repPredFreeVars;

	currPredFreeVars.growToSize(currentPred->getNumTerms(), false);
	repPredFreeVars.growToSize(repPred->getNumTerms(), false);

	for (int i = 0; i < currentPred->getNumTerms(); i++) {
		int currTermId = currentPred->getTerm(i)->getId();
		currPredFreeVars[i] = (centralPred->containsConstant(currTermId)
				|| repPred->containsConstant(currTermId));
	}
	for (int i = 0; i < repPred->getNumTerms(); i++) {
		int currTermId = repPred->getTerm(i)->getId();
		repPredFreeVars[i] = (currentPred->containsConstant(currTermId)
				|| centralPred->containsConstant(currTermId));
	}

	int numFreeVars = 0;
	for (int i = 0; i < currPredFreeVars.size(); i++)
		if (!currPredFreeVars[i])
			numFreeVars++;
	for (int i = 0; i < repPredFreeVars.size(); i++)
		if (!repPredFreeVars[i])
			numFreeVars++;

	return numFreeVars;
}
