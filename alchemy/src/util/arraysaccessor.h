/*
 * All of the documentation and software included in the
 * Alchemy Software is copyrighted by Stanley Kok, Parag
 * Singla, Matthew Richardson, Pedro Domingos, Marc
 * Sumner, Hoifung Poon, and Daniel Lowd.
 * 
 * Copyright [2004-07] Stanley Kok, Parag Singla, Matthew
 * Richardson, Pedro Domingos, Marc Sumner, Hoifung
 * Poon, and Daniel Lowd. All rights reserved.
 * 
 * Contact: Pedro Domingos, University of Washington
 * (pedrod@cs.washington.edu).
 * 
 * Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that
 * the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above
 * copyright notice, this list of conditions and the
 * following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the
 * above copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use
 * of this software must display the following
 * acknowledgment: "This product includes software
 * developed by Stanley Kok, Parag Singla, Matthew
 * Richardson, Pedro Domingos, Marc Sumner, Hoifung
 * Poon, and Daniel Lowd in the Department of Computer Science and
 * Engineering at the University of Washington".
 * 
 * 4. Your publications acknowledge the use or
 * contribution made by the Software to your research
 * using the following citation(s): 
 * Stanley Kok, Parag Singla, Matthew Richardson and
 * Pedro Domingos (2005). "The Alchemy System for
 * Statistical Relational AI", Technical Report,
 * Department of Computer Science and Engineering,
 * University of Washington, Seattle, WA.
 * http://www.cs.washington.edu/ai/alchemy.
 * 
 * 5. Neither the name of the University of Washington nor
 * the names of its contributors may be used to endorse or
 * promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY OF WASHINGTON
 * AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
 * OF WASHINGTON OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#ifndef ARRAYSACCESSOR_H_OCT_17_2005
#define ARRAYSACCESSOR_H_OCT_17_2005

#include "array.h"


template <typename Type>
class ArraysAccessor
{
 public:
  ArraysAccessor() : arrays_(new Array<const Array<Type>*>), 
                     indexes_(new Array<int>), 
                     itemIdx_(0), freeze_(false), reset_(false), noComb_(true){}

  ~ArraysAccessor() { delete arrays_; delete indexes_; }


  void appendArray(const Array<Type>* const & arr) 
  {
    assert(!freeze_);
    if (arrays_->size() == 0)
    {
      if (arr->size() == 0) noComb_ = true;
      else                  noComb_ = false;
    }
    else
    {
      if (arr->size() == 0) noComb_ = true;
    }
    arrays_->append(arr);
  }

  
  const Array<Type>* getArray(const int& idx) const { return (*arrays_)[idx]; }

  int getNumArrays() const { return arrays_->size();}


  void clear() 
  { 
    arrays_->clear(); 
    indexes_->clear(); 
    itemIdx_ = 0;
    freeze_ = false; 
    noComb_ = true;
  }
  

  void deleteArraysAndClear()
  {
    for (int i = 0; i < arrays_->size(); i++)
      delete (*arrays_)[i];
    clear();
  }


    // Start from first combination again.
  void reset()  { reset_ = true; }

  
    // returns false if there are no more elements
    // a contains the next combination of elements in the arrays that were added
  bool getNextCombination(Array<Type>& itemArr, Array<int>* idxArr=NULL)
  {
    if (noComb_) return false;
    if (!freeze_) prepareAccess();
    if (reset_) { reset_ = false; prepareAccess(); }
    if ((*indexes_)[0] < 0) return false;
    itemArr.clear();
    if (idxArr)  idxArr->clear();
    for (int i = 0; i < arrays_->size(); i++)
    {
      itemArr.append( (*((*arrays_)[i]))[ (*indexes_)[i] ] );
      if (idxArr) idxArr->append((*indexes_)[i]);
    }

    for (int i = arrays_->size()-1; i >= 0; i--)
    {
      (*indexes_)[i]++;
      if ((*indexes_)[i] < (*arrays_)[i]->size()) return true;
      (*indexes_)[i] = 0;
    }
    (*indexes_)[0] = -1;
    return true;    
  }
  
 private:
  bool distinctComb(Array<Type> itemArr){
    for(int i = 0; i < (itemArr.size()-1); i++){
      for(int j = (i+1); j < itemArr.size(); j++){
	if (itemArr[i] == itemArr[j]){
	  return false;
	}
      }
    }
    return true;
  }
  bool distinctCombUnordered(Array<Type> itemArr){
    for(int i = 0; i < (itemArr.size()-1); i++){
      for(int j = (i+1); j < itemArr.size(); j++){
	if (itemArr[i] >= itemArr[j]){
	  return false;
	}
      }
    }
    return true;
  }

  bool nextCombDistinctUnordered(Array<Type>& itemArr, Array<int>* idxArr=NULL)
  {
    if (noComb_) return false;
    if (!freeze_) prepareAccess();
    if (reset_) { reset_ = false; prepareAccess(); }
    if ((*indexes_)[0] < 0) return false;
    itemArr.clear();
    if (idxArr)  idxArr->clear();
    for (int i = 0; i < arrays_->size(); i++)
    {
      itemArr.append( (*((*arrays_)[i]))[ (*indexes_)[i] ] );
      if (idxArr) idxArr->append((*indexes_)[i]);
    }
    return distinctCombUnordered(itemArr);

  }

  bool nextCombDistinct(Array<Type>& itemArr, Array<int>* idxArr=NULL)
  {
    if (noComb_) return false;
    if (!freeze_) prepareAccess();
    if (reset_) { reset_ = false; prepareAccess(); }
    if ((*indexes_)[0] < 0) return false;
    itemArr.clear();
    if (idxArr)  idxArr->clear();
    for (int i = 0; i < arrays_->size(); i++)
    {
      itemArr.append( (*((*arrays_)[i]))[ (*indexes_)[i] ] );
      if (idxArr) idxArr->append((*indexes_)[i]);
    }
    return distinctComb(itemArr);

  }
  
 public:

    // returns false if there are no more elements
    // a contains the next combination of elements in the arrays that were added
  bool getDistinctNextCombination(Array<Type>& itemArr, Array<int>* idxArr=NULL)
  {
    //cout << "here\n";
    bool res = getNextCombination(itemArr, idxArr);
    //done = false;
    while(res && !distinctComb(itemArr)){
      itemArr.clear();
      if (idxArr){
	idxArr->clear();
      }
      res = getNextCombination(itemArr,idxArr);
    }
    // cout << "loop 1\n";
    Array<Type> tmpArr;
    bool done = true;//false;
    Array<Type> garbage;
    while(done && !nextCombDistinct(tmpArr)){// && !done){
      garbage.clear();
      done = getNextCombination(garbage);
      tmpArr.clear();
    }
    if (!res){
      /*
      cout << "Badness ";
      for(int xx = 0; xx < itemArr.size(); xx++){
	cout << itemArr[xx] << " ";
      }
      cout << endl;
      */
    }
    //cout << "returning\n";
    return res;
  }
  bool getDistinctNextCombinationUnordered(Array<Type>& itemArr, Array<int>* idxArr=NULL)
  {
    //cout << "here\n";
    bool res = getNextCombination(itemArr, idxArr);
    //done = false;
    while(res && !distinctCombUnordered(itemArr)){
      itemArr.clear();
      if (idxArr){
	idxArr->clear();
      }
      res = getNextCombination(itemArr,idxArr);
    }
    // cout << "loop 1\n";
    Array<Type> tmpArr;
    bool done = true;//false;
    Array<Type> garbage;
    while(done && !nextCombDistinctUnordered(tmpArr)){// && !done){
      garbage.clear();
      done = getNextCombination(garbage);
      tmpArr.clear();
    }
 
    return res;
  }


  int numItemsInCombination() const { return (*arrays_)[0]->size(); }

  
  bool hasNextCombination()
  {
    if (noComb_) return false;
    if (!freeze_) prepareAccess();
    if (reset_) { reset_ = false; prepareAccess(); }
    if ((*indexes_)[0] < 0) return false;
    itemIdx_ = 0;
    return true;
  }

  bool hasDistinctNextCombination()
  {
    if (noComb_) return false;
    if (!freeze_) prepareDistinctAccess();
    if (reset_) { reset_ = false; prepareAccess(); }
    if ((*indexes_)[0] < 0) return false;
    itemIdx_ = 0;
    /*
    bool done = false;
    Array<Type> tmpArr;
    while(!nextCombDistinct(tmpArr) && !done){
      Array<Type> garbage;
      done = getNextCombination(garbage);
    }
    */
    return true; //done;
  }


  bool nextItemInCombination(Type& item, int&idx)
  {
    if (itemIdx_ >= indexes_->size()) 
    {
      for (int i = arrays_->size()-1; i >= 0; i--)
      {
        (*indexes_)[i]++;
        if ((*indexes_)[i] < (*arrays_)[i]->size()) return false;
        (*indexes_)[i] = 0;
      }
      (*indexes_)[0] = -1;
      return false;
    }

    
    idx = (*indexes_)[itemIdx_];
    item = (*((*arrays_)[itemIdx_]))[idx];
    itemIdx_++;
    return true;
  }


  bool nextDistinctItemInCombination(Type& item, int&idx)
  {
    if (itemIdx_ >= indexes_->size()) 
    {
      for (int i = arrays_->size()-1; i >= 0; i--)
      {
        (*indexes_)[i]++;
	//check for conflicts
	cout << "i = " << i << " " << (*indexes_)[i] << endl;
        if ((*indexes_)[i] < (*arrays_)[i]->size()) return false;
        (*indexes_)[i] = 0;
      }
      (*indexes_)[0] = -1;
      return false;
    }
    idx = (*indexes_)[itemIdx_];
    item = (*((*arrays_)[itemIdx_]))[idx];
    /*
    for(int ix = 0; ix < itemIdx_; ix++){
      int tmpIdx = (*indexes_)[ix];
      if ((*((*arrays_)[ix]))[tmpIdx] == item){
	(*indexes_)[itemIdx_]++;
	idx++;
	if (idx >= (*arrays_)[itemIdx_]->size()){
	  return false;
	}
	item = (*((*arrays_)[itemIdx_]))[idx];
      }
    }
    */
    cout << "idx = " << idx << " " << itemIdx_ << endl;
    itemIdx_++;

    return true;
  }

  bool nextDistinctItemInCombination(Type& item){
    int i; 
    return nextDistinctItemInCombination(item,i);
  }


  bool nextItemInCombination(Type& item) 
  {
    int i; 
    return nextItemInCombination(item,i);
  }


  int numCombinations() const
  {
    if (arrays_->size() == 0) return 0;
    int n = 1;
    //cerr << "arrays size " << arrays_->size() << " ";
    for (int i = 0; i < arrays_->size(); i++){
      n *= (*arrays_)[i]->size();
      //cerr << (*arrays_)[i]->size() << " "; 

    }
    //cerr << endl;
    return n;    
  }

  
 private:

  void prepareAccess()
  {
    freeze_ = true;
    if (noComb_ || arrays_->size() == 0) return;
    indexes_->growToSize(arrays_->size());
    for (int i = 0; i < arrays_->size(); i++) (*indexes_)[i] = 0;
    //arrays_->compress();
    //indexes_->compress();
  }
  
  void prepareDistinctAccess()
  {
    freeze_ = true;
    if (noComb_ || arrays_->size() == 0) return;
    indexes_->growToSize(arrays_->size());
    for (int i = 0; i < arrays_->size(); i++){
      (*indexes_)[i] = 0;
      Type item = (*((*arrays_)[i]))[0];
      for(int j = 0; j < i; j++){
	
	int idx = (*indexes_)[j];
	if ((*((*arrays_)[j]))[idx] == item){
	  (*indexes_)[i]++;
	  
	  item = (*((*arrays_)[i]))[(*indexes_)[i]];
	}
	
      }
    }
    //arrays_->compress();
    //indexes_->compress();
  }
  

 private:

    // the contents of arrays_ are not owned by ArraysAccessor;
  Array<const Array<Type>*>* arrays_;
  Array<int>* indexes_;
  int itemIdx_;
  bool freeze_;
  bool reset_;
  bool noComb_;
};


#endif
