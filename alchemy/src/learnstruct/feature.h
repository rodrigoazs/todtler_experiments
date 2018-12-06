#ifndef FEATURE_H
#define FEATURE_H

#include "predicate.h"
#include "array.h"

struct RelPath;

//Represents a simple feature/node in Markov Blanket
class NodeFeature : public Predicate {
public:

	//Construct a central feature
	NodeFeature(Predicate* currentPred, int &nextVariableValue);

	//Construct a secondary feature wrt given centralFeature
	NodeFeature(Predicate* currentPred, Predicate* centralPred,
			NodeFeature* centralFeature, int &nextVariableValue);

	//Construct second part of a composite feature
	NodeFeature(RelPath* relPath, Predicate* centralPred,
			NodeFeature* firstPart, NodeFeature* centralFeature,
			int &nextVariableValue);

	//Is the given feature semantically the same wrt the central feature?
	bool isSemSame(NodeFeature* nf);

	//Could the given gndPred be transformed into this feature?
	bool matchesPredicate(Predicate* gndPred);

	bool containsVariable(int varId);

	void print(Domain* domain);

private:
	void prepareSharingsMask(Predicate* currentPred, Predicate* centralPred,
			NodeFeature* centralFeature);
	int selectTermValue(Predicate* currentPred, int& nextVariableValue,
			int termIdx, Array<int>& setValues);

	Array<int> sharingsMask_;
};

//Represents a composite feature
class CompNodeFeature {
public:
	CompNodeFeature(Predicate* currentPred, RelPath* relPath,
			Predicate* centralPred, NodeFeature* centralFeature,
			int& nextVariableValue, int maxNumFreeVars);
	~CompNodeFeature()
	{
		if (part1_ != NULL)
			delete part1_;
		if (part2_ != NULL)
			delete part2_;
	}

	bool isSemSame(CompNodeFeature* cnf);
	void print(Domain* domain);
	bool valid()
	{
		return isValid_;
	}

	//caller is responsible for returned pointers
	Predicate* getCopyOfPart1();
	Predicate* getCopyOfPart2();

private:
	int calculateNumFreeVars(Predicate* centralPred, Predicate* currentPred,
			Predicate* repPred);

	NodeFeature* part1_;
	NodeFeature* part2_;

	//position i, if >=0, contains the position in part1_ that has
	//a sharing with part2_. Listed wrt part2_
	Array<int> sharedOfPart2_;
	Array<int> sharedOfPart1_; //analogous to above; to preserve symmetry
	bool isValid_;
};

//Represents a relational path between two predicates. One of
//the predicates is assumed (i.e. this struct will be stored with
//respect to it), and the other one is listed.
struct RelPath {
	RelPath(Predicate *repPred_, Array<int>* sharedIndices_, int numShared_) :
		repPred(repPred_), sharedIndices(sharedIndices_), numShared(numShared_)
	{
		repPredSharedIndices = new Array<int>;
		repPredSharedIndices->growToSize(repPred->getNumTerms(), -1);

		for (int i = 0; i < sharedIndices->size(); i++)
			if ((*sharedIndices)[i] != -1)
				(*repPredSharedIndices)[(*sharedIndices)[i]] = i;
	}

	~RelPath()
	{
		delete sharedIndices;
		delete repPredSharedIndices;
	}

	//Sameness is established with respect to the first (omitted) part
	//of the path
	bool same(RelPath *anotherRP)
	{
		if (this == anotherRP)
			return true;
		if (sharedIndices->size() != anotherRP->sharedIndices->size())
			return false;
		if (numShared != anotherRP->numShared)
			return false;
		if (repPred->getId() != anotherRP->repPred->getId())
			return false;

		bool sameIdx = true;
		for (int i = 0; i < sharedIndices->size() && sameIdx; i++) {
			Array<int> *otherSI = anotherRP->sharedIndices;
			sameIdx = ((*sharedIndices)[i] == (*otherSI)[i]);
		}

		return sameIdx;
	}

	void print(Domain *domain)
	{
		cout << "__________________\n";
		cout << "RepPred: ";
		repPred->print(cout, domain);
		cout << endl;
		cout << "SharedIndices: ";
		for (int i = 0; i < sharedIndices->size(); i++)
			cout << (*sharedIndices)[i] << "  ";
		cout << endl;
		cout << "NumShared: " << numShared << endl;
		cout << "__________________\n";
	}

	Predicate *repPred;
	Array<int> *sharedIndices; //based on first part of path
	Array<int> *repPredSharedIndices; //based on repPred part of path
	int numShared;
};

#endif
