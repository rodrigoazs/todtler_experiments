#include "featureArray.h"

//==========================Public Methods implementations=============//

FeatureArray::~FeatureArray()
{
	for (int i = 0; i < features_->size(); i++)
		delete (*features_)[i];
	delete features_;

	for (int i = 0; i < compFeatures_->size(); i++)
		delete (*compFeatures_)[i];
	delete compFeatures_;

	for (int i = 0; i < featureValues_->size(); i++)
		delete (*featureValues_)[i];
	delete featureValues_;

	for (int i = 0; i < compFeatureValues_->size(); i++)
		delete (*compFeatureValues_)[i];
	delete compFeatureValues_;
}

void FeatureArray::constructFeaturesForGnding(Predicate* gndPred,
		Domain* currDomain, bool predTruthValue,
		//Array<PredicateHashArray *>* constIdToPred,
		map<int, PredicateHashArray*>* constIdToPred,
		PredicateToIntMap &relPathIdxForPred, Array<Array<RelPath*>*>* relPaths)
{
	//first check if the gnding contains repeated constants. we are
	//going to skip those in order to avoid a variety of issues
	if (!noRepeatedConstants(gndPred))
		return;

	Array<bool>* newFeatureValues = new Array<bool>;
	if (features_->size() > 0)
		newFeatureValues->growToSize(features_->size(), false);
	featureValues_->append(newFeatureValues);

	makeCentralFeature(gndPred, predTruthValue, newFeatureValues);

	// cout << "Made central feature:\n";
	//centralFeature_->print(currDomain);
	//prepare all gndings with shared constants
	PredicateHashArray hashGroundings;
	for (int c = 0; c < gndPred->getNumTerms(); c++) {
		PredicateHashArray * gndingsWithConstant = (*constIdToPred)[gndPred->getTerm(c)->getId()];
		for (int g = 0; g < gndingsWithConstant->size(); g++)
			hashGroundings.append((*gndingsWithConstant)[g]);
	}

	//do the simple features
	for (int g = 0; g < hashGroundings.size(); g++) {
		Predicate* currentGnding = hashGroundings[g];

		int tmpNextVarValue = nextVariableValue_;
		NodeFeature* candidateFeature = new NodeFeature(currentGnding, gndPred,
				centralFeature_, tmpNextVarValue);
		//cout << "Made candidate feature:\n";
		//candidateFeature->print(currDomain);

		//do we have this feature?
		int featureIndex = findFeature(candidateFeature);
		if (featureIndex >= 0) {
			assert(featureIndex < newFeatureValues->size());
			(*newFeatureValues)[featureIndex] = true;
			delete candidateFeature;
		} else {
			appendNewFeature(candidateFeature);
			nextVariableValue_ = tmpNextVarValue;
		}
	}

	// cout << "Done with simple features\n";
	//do the composite features
	Array<bool>* newCompFeatureValues = new Array<bool>;
	if (compFeatures_->size() > 0)
		newCompFeatureValues->growToSize(compFeatures_->size(), false);
	compFeatureValues_->append(newCompFeatureValues);

	for (int g = 0; g < hashGroundings.size(); g++) {
		if (hashGroundings[g]->getNumTerms() < 2)
			continue;

		Predicate* currentGnding = hashGroundings[g];

		int rpIndex = relPathIdxForPred[currentGnding];
		Array<RelPath*>* thisRelPaths = (*relPaths)[rpIndex];

		for (int r = 0; r < thisRelPaths->size(); r++) {
			int tmpNextVarValue = nextVariableValue_;
			CompNodeFeature *candCNFeature = new CompNodeFeature(currentGnding, (*thisRelPaths)[r], gndPred,
					centralFeature_,tmpNextVarValue, maxNumFreeVars_);

			if (candCNFeature->valid()) {
				int featureIndex = findCompFeature(candCNFeature);
				if (featureIndex >= 0) {
					assert(featureIndex < newCompFeatureValues->size());
					(*newCompFeatureValues)[featureIndex] = true;
					delete candCNFeature;
				} else {
					appendNewCompFeature(candCNFeature);
					nextVariableValue_ = tmpNextVarValue;
				}
			} else
				delete candCNFeature;
		}
	}
}

void FeatureArray::print(Domain* domain, bool printValues)
{
	if (getTotalNumFeatures() == 0) {
		cout << "No Features were constructed\n";
		return;
	}

	/*
	  cout << "===================================\n";
	cout << "Central feature: \n";
	centralFeature_->print(domain);
	cout << "SimpleFeatures: \n";
	for (int i = 0; i < features_->size(); i++) {
		cout << i << ": ";
		(*features_)[i]->print(domain);
	}
	*/
	if (printValues) {
		for (int i = 0; i < featureValues_->size(); i++) {
			Array<bool>* currValues = (*featureValues_)[i];
			for (int j = 0; j < currValues->size(); j++)
				cout << (*currValues)[j] << " ";
			cout << endl;
		}
	}

	cout << "CompositeFeatures: \n";
	for (int i = 0; i < compFeatures_->size(); i++) {
		cout << i << ": ";
		(*compFeatures_)[i]->print(domain);
	}

	if (printValues) {
		for (int i = 0; i < compFeatureValues_->size(); i++) {
			Array<bool>* currValues = (*compFeatureValues_)[i];
			for (int j = 0; j < currValues->size(); j++)
				cout << (*currValues)[j] << " ";
			cout << endl;
		}
	}

	cout << "===================================\n";
}

//======================Utility methods====================================//
bool FeatureArray::noRepeatedConstants(Predicate* gndPred)
{
	bool good = true;
	for (int i = 1; i < gndPred->getNumTerms() && good; i++)
		for (int j = i-1; j >= 0 && good; j--)
			good = gndPred->getTerm(i)->getId() != gndPred->getTerm(j)->getId();
	return good;
}

void FeatureArray::makeCentralFeature(Predicate* gndPred, bool predTruthValue,
		Array<bool>* featureValues)
{
	bool ok = true;

	if (centralFeature_ == NULL) {
		centralFeature_ = new NodeFeature(gndPred, nextVariableValue_);
		assert(features_->size() == 0 && featureValues->size() == 0);
		features_->append(centralFeature_);
		featureValues->append(predTruthValue);
	} else {
		ok = centralFeature_->matchesPredicate(gndPred);
		(*featureValues)[0] = predTruthValue;
	}

	assert(ok);
}

int FeatureArray::findFeature(NodeFeature* nf)
{
	for (int i = 0; i < features_->size(); i++)
		if (nf->isSemSame((*features_)[i]))
			return i;
	return -1;
}

int FeatureArray::findCompFeature(CompNodeFeature* cnf)
{
	for (int i = 0; i < compFeatures_->size(); i++)
		if (cnf->isSemSame((*compFeatures_)[i]))
			return i;
	return -1;
}

void FeatureArray::appendNewFeature(NodeFeature* nf)
{
	features_->append(nf);
	for (int i = 0; i < featureValues_->size() - 1; i++) {
		Array<bool>* currValueArray = (*featureValues_)[i];
		currValueArray->append(false);
	}

	Array<bool>* thisValueArray = (*featureValues_)[featureValues_->size() - 1];
	thisValueArray->append(true);
}

void FeatureArray::appendNewCompFeature(CompNodeFeature* cnf)
{
	compFeatures_->append(cnf);
	for (int i = 0; i < compFeatureValues_->size() - 1; i++) {
		Array<bool>* currValueArray = (*compFeatureValues_)[i];
		currValueArray->append(false);
	}

	Array<bool>* thisValueArray = (*compFeatureValues_)[compFeatureValues_->size() - 1];
	thisValueArray->append(true);
}
