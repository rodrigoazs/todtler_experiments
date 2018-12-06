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
#ifndef JESSECLAUSE_H_JUN_26_2005
#define JESSECLAUSE_H_JUN_26_2005

#include <ext/hash_set>
#include <list>
#include <ext/hash_map>
using namespace __gnu_cxx;
#include <ostream>
#include <sstream>
using namespace std;

class JesseClause{
  list<string> clause;
  hash_set<string> varOver;
  hash_map<int,list<string>* > vars;

  JesseClause(){
    //clause = new list<string>();
    //varOver = new hash_set<string>();
    //vars = new hash_map<int,list<string>>();
  }

  JesseClause(string pred){
    //clause = new list<string>();
    //varOver = new hash_set<string>();
    //vars = new hash_map<int,list<string>>();
    clause.push_front(pred);
  }

  JesseClause(string pred, int type, string var){
    //clause = new list<string>();
    //varOver = new hash_set<string>();
    //vars = new hash_map<int,list<string>>();
    clause.push_front(pred);
    list<string>* vList = new list<string>;
    vList->push_front(var);
    vars[type] = vList;
      
  }


  ~JesseClause(){
    //delete clause;
    //delete varOver;
    //iterator through each thing in the hash_map;
    //delete vars;
  }


  void addPred(string pred){
	clause.push_back(pred);
  }

  void addPred(string pred, int type, string newVar, string sharedVar){
    clause.push_back(pred);
    //varOver.insert(sharedVar);
    if (false){//vars.containsKey(type)){
      list<string> *vList = vars[type];
      vList->push_front(newVar);
      vars[type] = vList;
    }
    else{
      list<string> *vList = new list<string>;
      vList->push_front(newVar);
      vars[type] = vList;
    }
  }

};


#endif
