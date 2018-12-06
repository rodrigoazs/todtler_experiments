#ifndef MARKOV_BLANKET
#define MARKOV_BLANKET

#include <set>
#include "clause.h"
#include "feature.h"
#include "featureArray.h"

struct IndependenceCounts;

class MarkovBlanketsFinder {
public:
	MarkovBlanketsFinder(FeatureArray* featArray, double threshold,
			int countIsAtLeast);
	~MarkovBlanketsFinder();

	void growShrink();

	//caller responsible for any clauses added to given array
	void constructClausesFromMB(ClauseOpHashArray& constructedClauses, int maxVars, int maxPreds);

	void growBlanketAndMakeClauses(ClauseOpHashArray& constructedClauses, int maxVars, int maxPreds);

private:
	//growShrink helper methods
	void calculateFeatureOrderingFromArray(Array<double> & ranks,
			Array<int>& featureOrdering);
	double
			chiSquareAlphaValue(int xIndex, int yIndex, set<int>* markovBlanketX);
	void calculateCounts(int xIndex, int yIndex, set<int> *markovBlanketX,
			Array<IndependenceCounts *> *theCounts);
	void calculateHeuristicFeatureOrdering(
			Array<Array<double> > & uncondIndepMatrix,
			Array<int>& featureOrdering);

	//clause constructing helper methods
	void makeIntArrayClauses();
	void makeActualClauses(ClauseOpHashArray & constructedClauses, int maxVars, int maxPreds);

	void copyFeatureAndAddToClause(Clause* clause, NodeFeature * feature,
			bool predSense);
	void copyCompFeatureAndAddToClause(Clause* clause,
			CompNodeFeature * compFeature, bool predSense);

	FeatureArray* featArray_; //do not delete; not owned by this class
	Array<set<int>*>* markovBlankets_;
	Array<Array<bool>*>* allFeatureValues_;
	Array<Array<int>*>* clauseCandidates_;
	double threshold_;
	int countIsAtLeast_;
};

//used in fromMarkovNetworkRun when determining whether a pair of
//features are independent
struct IndependenceCounts {
	int x1y1;
	int x1y0;
	int x0y1;
	int x0y0;

	double totalX1()
	{
		return ((double)x1y1 + x1y0);
	}
	double totalY1()
	{
		return ((double)x1y1 + x0y1);
	}
	double totalX0()
	{
		return ((double)x0y1 + x0y0);
	}
	double totalY0()
	{
		return ((double)x1y0 + x0y0);
	}
	double total()
	{
		return ((double)x1y1 + x1y0 + x0y1 + x0y0);
	}

	void setAllToZero()
	{
		x1y1 = x1y0 = x0y1 = x0y0 = 0;
	}

	bool allAreGreaterThan(int c)
	{
		return (x1y1 >= c && x1y0 >= c && x0y1 >= c && x0y0 >= c);
	}

	void print()
	{
		cout << "** Printing Independence Counts **\n";
		cout << "x1y1 = " << x1y1 << endl;
		cout << "x1y0 = " << x1y0 << endl;
		cout << "x0y1 = " << x0y1 << endl;
		cout << "x0y0 = " << x0y0 << endl;
		cout << "totalX1 = " << totalX1() << endl;
		cout << "totalY1 = " << totalY1() << endl;
		cout << "totalX0 = " << totalX0() << endl;
		cout << "totalY0 = " << totalY0() << endl;
		cout << "total = " << total() << endl;
	}
};

#endif
