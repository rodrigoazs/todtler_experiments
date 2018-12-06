#ifndef FEATUREARRAY_H
#define FEATUREARRAY_H

#include "feature.h"
#include <map>

typedef hash_map<Predicate*, int, HashPredicate, EqualPredicate>
		PredicateToIntMap;

class FeatureArray {
public:
	FeatureArray(int maxNumFreeVars) :
		centralFeature_(NULL), nextVariableValue_(-1),
				features_(new Array<NodeFeature*>),
				compFeatures_(new Array<CompNodeFeature*>),
				featureValues_(new Array<Array<bool>*>),
				compFeatureValues_(new Array<Array<bool>*>),
				maxNumFreeVars_(maxNumFreeVars)
	{
	}

	~FeatureArray();

	void constructFeaturesForGnding(Predicate* gndPred, Domain* currDomain,
			bool predTruthValue, map<int, PredicateHashArray*>* constIdToPred,
			PredicateToIntMap &relPathIdxForPred,
			Array<Array<RelPath*>*>* relPaths);

	void print(Domain* domain, bool printValues = false);

	int getNumSimpleFeatures()
	{
		return features_->size();
	}
	int getNumCompFeatures()
	{
		return compFeatures_->size();
	}
	int getTotalNumFeatures()
	{
		return getNumSimpleFeatures() + getNumCompFeatures();
	}

	const Array<NodeFeature*>* getFeatures() const
	{
		return features_;
	}
	const Array<CompNodeFeature*>* getCompFeatures() const
	{
		return compFeatures_;
	}
	const Array<Array<bool>*>* getFeatureValues() const
	{
		return featureValues_;
	}
	const Array<Array<bool>*>* getCompFeatureValues() const
	{
		return compFeatureValues_;
	}

private:
	bool noRepeatedConstants(Predicate* gndPred);

	void makeCentralFeature(Predicate* gndPred, bool predTruthValue,
			Array<bool>* featureValues);

	//return the index of the feature; -1 if not present
	int findFeature(NodeFeature* nf);
	int findCompFeature(CompNodeFeature* cnf);

	//append the given feature to the feature arrays, updating the
	//values arrays; assumes (but does not check) that the feature does
	//not exist
	void appendNewFeature(NodeFeature* nf);
	void appendNewCompFeature(CompNodeFeature* cnf);

	NodeFeature* centralFeature_; //this is the feature with index 0 in the feature array
	int nextVariableValue_;

	Array<NodeFeature*>* features_;
	Array<CompNodeFeature*>* compFeatures_;

	Array<Array<bool>*>* featureValues_;
	Array<Array<bool>*>* compFeatureValues_;
	
	int maxNumFreeVars_; 
};

#endif
