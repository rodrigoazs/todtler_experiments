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
#ifndef RANDOM_H_JUN_21_2005
#define RANDOM_H_JUN_21_2005

#include <iostream>
using namespace std;

  //if you are generating more than 100,000,000 random number, use randomB 
class Random
{
 public:
  Random()  : initNum_(0) {}
  ~Random() {}

    // Initialize with a negative integer. Call this only once.
  void init(const long& initNum)
  {    
    if (initNum_ < 0)
    {
      cout << "WARNING: you can only call Random::init() once. Ignoring call."
           << endl;
      return;
    }

    if (initNum >= 0) 
    { 
      cout << "Random::init() expects a negative number but is given " 
           << initNum << endl;
      exit(-1);
    }

    initNum_ = initNum;
    

    num2_ = 123456789;
    a_ = 0;

    randomA(&initNum_);
    //randomB(&initNum_);
  }
  

    //0 < number returned < 1
  float random() 
  {
    return randomA(&initNum_);
    //return randomB(&initNum_);
  }


  int randomOneOf(const int& a) { return (int) (a * random()); }


 private:
  float randomA(long* initNum)
  {
    int c;
    long d;
    float tmp;
    
    if (*initNum <= 0 || !a_) 
    {
      if (-(*initNum) < 1) 
        *initNum = 1;
      else 
        *initNum = -(*initNum);
      
      for (c = 32+7; c >= 0; c--) 
      {
        d = (*initNum)/127773;
        *initNum = 16807 * (*initNum - d*127773) - 2836*d;
        
        if (*initNum < 0) 
          *initNum += 2147483647;
        
        if (c < 32)
          b_[c] = *initNum;
      }
      a_ = b_[0];
    }
    d = (*initNum)/127773;
    *initNum = 16807 * (*initNum - d*127773) - 2836*d;
    if (*initNum < 0) 
      *initNum += 2147483647;
    c = a_ / (1 + (2147483647-1)/ 32);
    a_ = b_[c];
    b_[c] = *initNum;
    if ((tmp = (1.0/2147483647)*a_) > (1.0 - 1.2e-7)) 
      return (1.0 - 1.2e-7);
    else 
      return tmp;
  }


  float randomB(long* initNum)
  {
    int c;
    long d;
    float tmp;
    
    if (*initNum <= 0) 
    {
      if (-(*initNum) < 1) 
        *initNum = 1;
      else 
        *initNum = -(*initNum);
      
      num2_ = (*initNum);
      
      for (c = 32+7; c >= 0; c--) 
      {
        d = (*initNum)/53668;
        *initNum = 40014 * (*initNum-d*53668) - d*12211;
        
        if (*initNum < 0) 
          *initNum += 2147483563;
        
        if (c < 32) 
          b_[c] = *initNum;
      }
      a_ = b_[0];
    }
    
    d = (*initNum)/53668;
    *initNum = 40014 * (*initNum-d*53668) - d*12211;
    if (*initNum < 0) *initNum += 2147483563;
    d = num2_/52774;
    num2_ = 40692 * (num2_-d*52774) - d*3791;
    if (num2_ < 0) 
      num2_ += 2147483399;
    c = a_ / (1 + (2147483563-1)/32);
   a_ = b_[c] - num2_;
   b_[c] = *initNum;
   if (a_ < 1) a_ += (2147483563-1);
   if ((tmp = (1.0/2147483563)*a_) > (1.0 - 1.2e-7)) 
     return (1.0 - 1.2e-7);
   else 
     return tmp;
  }

  
 private:
  long initNum_;
  long num2_;
  long a_;
  long b_[32];

};


#endif
