  void newTransfer(Array<Clause*> initClauses){
    double maxScore = 0;
    string fname = outMLNFileName_;
    ofstream out(fname.c_str());
    if (!out.good()) { cout << "ERROR: failed to open " <<fname<<endl;exit(-1);}
 


    for(int ix = 0; ix < initClauses.size(); ix++){
      out << "Evaluate: ";
      initClauses[ix]->printWithoutWt(out, (*domains_)[0]);
      out << endl;
      const Array<Predicate*>* preds = initClauses[ix]->getPredicates();
      int numNonEqual = 0;
      ArraysAccessor<int> acc;
      Array<int> pos;
      for(int jx = 0; jx < preds->size(); jx++){
	if (!(*preds)[jx]->isEqualPred()){
	  numNonEqual++;
	  pos.append(jx);
	  Array<int>* vals = new Array<int>;
	  vals->append(0);
	  vals->append(1);
	  acc.appendArray(vals);
	}
      }
 
      Array<double> trueCnts;
      Array<double> possCnts;
      Array<double> probs;
      Array<int> sense;
      double sum = 0;
      double total = 0;
      ClauseHashArray cliques;
      Array<Clause*> forDecomp;
      Array<ClauseHashArray*> decomp;
      Array<int> mult;
      while(acc.getNextCombination(sense)){
	Clause* c1 = new Clause(*initClauses[ix]);
	for(int jx = 0; jx < pos.size(); jx++){
	  Predicate* pred = c1->getPredicate(pos[jx]);
	  if (sense[jx] == 0){
	    pred->setSense(false);
	  }
	  else{
	    pred->setSense(true);
	  }
	}
	Clause* r1 = new Clause(*c1);
	c1->setDirty();
	c1->canonicalize();
	if (cliques.contains(c1)){
	  int dupIndex = cliques.find(c1);
	  mult[dupIndex]++;
	  delete c1;
	  continue;
	}
	cliques.append(c1);
	forDecomp.append(r1);
	mult.append(1);

	double ntg_c1 = 1;//c1->getNumTrueGroundings((*domains_)[0], (*domains_)[0]->getDB(), false);
	double ng_c1 = 0;// c1->getNumDistinctGroundings((*domains_)[0]);      
	for(int dx = 0; dx < domains_->size(); dx++){
	  ntg_c1 += c1->getNumTrueGroundingsAnd((*domains_)[dx], (*domains_)[dx]->getDB(), false);
	  ng_c1 = c1->getNumDistinctGroundings((*domains_)[dx]);      
	}
	c1->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << ntg_c1 << endl;//" " << ng_c1 << endl;
	//cout << " Hash Code " << c1->hashCode() << endl;
	sum += (ng_c1 - ntg_c1);
	total = ng_c1;
	trueCnts.append(ntg_c1);
	possCnts.append(ng_c1);
	probs.append(ntg_c1);
      }
      double norm = 0;
      for(int jx = 0; jx < probs.size(); jx++){
	norm += probs[jx];
      }
      for(int jx = 0; jx < probs.size(); jx++){
	probs[jx] = probs[jx] / norm;
      }
      if (preds->size() == 2){
      

	for(int jx = 0; jx < cliques.size(); jx++){
	  double posPred1 = 0;
	  double posPred2 = 0;	  
	  for(int kx = 0; kx < cliques.size(); kx++){
	    Array<Predicate*>* currPreds = cliques[ix].getPredicates();
	  }
	}
	double negPred1 = 0;
	double negPred2 = 0;
	
      }
      else{
	for(int jx = 0; jx < probs.size(); jx++){

	}
      }

    }
  }


      //Do calculations
      ClauseHashArray full1;
      ClauseHashArray full2;
      Array<double> newProbs1;
      Array<double> newProbs2;
      Array<int> mult1;
      Array<int> mult2;

      /*
	cout << clause1.size() << " " << clause2.size() << " "
	<< dProbs1.size() << " " << dProbs2.size() << endl;
      */
      for(int jx = 0; jx < clause1.size(); jx++){
	
	ClauseHashArray* c1Array = clause1[jx];
	ClauseHashArray* c2Array = clause2[jx];
	Array<double>* probs1 = dProbs1[jx];
	Array<double>* probs2 = dProbs2[jx];
	Array<int>* m1 = idx1[jx];
	Array<int>* m2 = idx2[jx];
	/*
	  cout << c1Array->size() << " " << probs1->size()
	  << " " << c2Array->size() << " " << probs2->size() << endl;
	*/
	for(int kx = 0; kx < c1Array->size(); kx++){
	  if (!full1.contains((*c1Array)[kx])){
	    full1.append((*c1Array)[kx]);
	    newProbs1.append((*probs1)[kx]);
	    if (kx >= m1->size()){
	      cout << "Size mismatch\n";
	    }
	    if ((*m1)[kx] == 0){
	      cout << "bad " << jx << " " << kx << " ";
	      (*c1Array)[kx]->printWithoutWt(cout, (*domains_)[0]);
	      cout << endl;
	    }
	    mult1.append((*m1)[kx]);
	  }
	  else{
	    //cout << "Here\n";
	    int index = full1.find((*c1Array)[kx]);
	    if (newProbs1[index] != (*probs1)[kx]){
	      cout << "Probability mismatch\n";
	    }
	    if (mult1[index] != (*m1)[kx]){
	      cout << "Multiplier mismatch\n";
	    }
	  }
	}
	for(int kx = 0; kx < c2Array->size(); kx++){
	  if (!full2.contains((*c2Array)[kx])){
	    full2.append((*c2Array)[kx]);
	    newProbs2.append((*probs2)[kx]);
	    mult2.append((*m2)[kx]);
	  }
	}
      }
      Array<double> klScores;
      for(int jx = 0; jx < dProbs1.size(); jx++){
	klScores.append(0.0);
      }
      Array<Array<double>*> probNorm;
      for(int jx = 0; jx < dProbs1.size(); jx++){
	Array<double>* tmpD = new Array<double>;
	probNorm.append(tmpD);
      }

      for(int jx = 0; jx < probs.size(); jx++){

	probs[jx] = probs[jx] / norm;
	//cliques[jx]->printWithoutWt(cout, (*domains_)[0]);
	//cout << " " << probs[jx] << " | ";
	//cout << dProbs1.size() << " " << dProbs2.size() << " -> ";
	
	for(int kx = 0; kx < dProbs1.size(); kx++){
	  Clause* d1 = new Clause(*forDecomp[jx]);
	  Clause* d2 = new Clause(*forDecomp[jx]);
	  if (dProbs1.size() == 1){
	    Predicate* rmed = d1->removePredicate(1);
	    delete rmed;
	    rmed = d2->removePredicate(0);
	    delete rmed;
	  }
	  else{
	    if (kx == 0){
	      Predicate* rmed = d1->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(1);
	      delete rmed;
	      rmed = d2->removePredicate(0);
	      delete rmed;
	    }
	    else if (kx == 1){
	      Predicate* rmed = d1->removePredicate(1);
	      delete rmed;
	      rmed =d2->removePredicate(2);
	      delete rmed;
	      rmed =d2->removePredicate(0);
	      delete rmed;
	    }
	    else{
	      Predicate* rmed = d1->removePredicate(0);
	      delete rmed;
	      rmed = d2->removePredicate(2);
	      delete rmed;
	      rmed = d2->removePredicate(1);
	      delete rmed;
	    }
	  }
	  d1->canonicalize();
	  d2->canonicalize();
	  int index1 = full1.find(d1);
	  int index2 = full2.find(d2);
	  //Array<int>* c1Mult = idx1[kx];
	  //Array<int>* c2Mult = idx2[kx];
	  //cout << idx1.size() << " " << c1Mult->size() << " " << idx2.size() << " " << c2Mult->size() << endl;
	  if (mult1[index1] == 0){
	    cout << "This is bad!" << endl;
	  }
	  //cout << "mults: " << kx << " mult1 " << (1.0/mult1[index1]) << " mult2 " << (1.0/mult2[index2]) << " index1 " << index1 << " index2 " << index2 << " " << mult1.size() << " " << mult2.size() << endl;
	  cout << "kx: " << kx << " ";
	  d1->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";
	  d2->printWithoutWt(cout, (*domains_)[0]);
	  cout << " " ; //<< index1 << " " << index2 << " ";

	  double predProb = newProbs1[index1] * newProbs2[index2] * (1.0/mult1[index1]) * (1.0/mult2[index2]);// * (1/(*c1Mult)[index1]) * (1/(*c2Mult)[index2]);//1 - ((1-newProbs1[index1]) * (1-newProbs2[index2]));
	  Array<double>* dp = probNorm[kx];
	  dp->append(predProb);
	  cout << " " << newProbs1[index1] << " " << newProbs2[index2] << " " << predProb << endl;
	  //cout << "kx: " << kx << " " << predProb << " ";
	  //double kl1 = thisProb * log(thisProb/prob1) + (1-thisProb) * log((1-thisProb)/(1-prob1));
	  //double kl1 = probs[jx] * log(probs[jx]/predProb) + (1-probs[jx]) * log((1-probs[jx])/(1-predProb));
	  //cout << newProbs1[index1] << " " << newProbs2[index2] << " (" << kl1 <<  "): "; //newProbs1[index1] << " " << newProbs2[index2] << ": ";
	  delete d1;
	  delete d2;
	  //klScores[kx] += kl1;
	} 
	cout << endl;
      }
      for(int jx = 0; jx < probNorm.size(); jx++){
	Array<double>* td = probNorm[jx];
	double z = 0;
	//cout << td->size() << " " << mult.size() << endl;
	
	for(int kx = 0; kx < td->size(); kx++){
	  z += (*td)[kx] * mult[kx];
	}
	cout << "check z = " << z;
	out << "check z = " << z << endl;
	if (z > 1.0 || z < 1.0){
	  cout << " z badness ";
	}
	cout << endl;
	/*
	  for(int kx = 0; kx < td->size(); kx++){
	  (*td)[kx] = (*td)[kx]  / z;
	  }
	*/
      }
      for(int jx = 0; jx < probs.size(); jx++){
	out << "feature ";
      	forDecomp[jx]->printWithoutWt(out, (*domains_)[0]);
	out << " " << probs[jx] << " | ";
	
	//cout << "feature ";
      	forDecomp[jx]->printWithoutWt(cout, (*domains_)[0]);
	cout << " " << probs[jx] << " | ";
	
	for(int kx = 0; kx < probNorm.size(); kx++){
	  Array<double>* predProb = probNorm[kx];
	  double predFinal = mult[jx] * (*predProb)[jx];
	  //flag
	  double kl1 = probs[jx] * log(probs[jx]/predFinal);// + (1-probs[jx]) * log((1-probs[jx])/(1-predFinal));
	  //double kl1 = probs[jx] * log(probs[jx]/(*predProb)[jx]) + (1-probs[jx]) * log((1-probs[jx])/(1-(*predProb)[jx]));
	  //cout << newProbs1[index1] << " " << newProbs2[index2] << " (" << kl1 <<  "): "; //newProbs1[index1] << " " << newProbs2[index2] << ": ";
	  /*
	  out << (*predProb)[jx] << " (" << kl1 << "): ";
	  cout << (*predProb)[jx] << " (" << kl1 << "): ";
	  */

	  out << predFinal << " (" << kl1 << "): ";
	  cout << predFinal << " (" << kl1 << "): ";
	  klScores[kx] += kl1;
	}
	cout << endl;
	out << endl;
      }
      double minScore = klScores[0];
      for(int jx = 0; jx < klScores.size(); jx++){
	out << klScores[jx] << " ";
	cout << klScores[jx] << " ";
	if (klScores[jx] < minScore){
	  minScore = klScores[jx];
	}
      }
      if (maxScore < minScore){
	maxScore = minScore;
      }
      out << endl;
      cout << endl;
      out << "Min " << minScore << endl;
      for(int jx = 0; jx < probNorm.size(); jx++){
	Array<double>* tmp = probNorm[jx];
	delete tmp;
      } 
      for(int jx = 0; jx < clause1.size(); jx++){
	
	ClauseHashArray* ca1 = clause1[jx];
	ClauseHashArray* ca2 = clause2[jx];
	ca1->deleteItemsAndClear();
	ca2->deleteItemsAndClear();
	Array<double>* pr1 = dProbs1[jx];
	Array<double>* pr2 = dProbs2[jx];
	Array<int>* id1 = idx1[jx];
	Array<int>* id2 = idx2[jx];
	delete id1;
	delete id2;
	delete pr1;
	delete pr2;
	delete ca1;
	delete ca2;
      }
      /*

      */
      forDecomp.deleteItemsAndClear();
      cliques.deleteItemsAndClear();
      acc.deleteArraysAndClear();
      out << endl;
    }
    out << "Max: " << maxScore << endl;
    out.close();
  }
