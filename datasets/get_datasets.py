'''
   Functions to return datasets in file folder
   Name:         get_datasets.py
   Author:       Rodrigo Azevedo
   Updated:      July 22, 2018
   License:      GPLv3
'''

import re
import os
import unidecode
import csv
import math
import random
import pandas as pd
import json
import copy

__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))

class datasets:
    def get_kfold(test_number, folds):
        '''Separate examples into train and test set.
        It uses k-1 folds for training and 1 single fold for testing'''
        train = []
        test = []
        for i in range(len(folds)):
            if i == test_number:
                test += folds[i]
            else:
                train += folds[i]
        return (train, test)
    
    def get_kfold_separated(test_number, folds):
        train = []
        test = []
        for i in range(len(folds)):
            if i == test_number:
                test = folds[i]
            else:
                train.append(folds[i])
        return (train, test)
    
    def get_kfold_small(train_number, folds):
        '''Separate examples into train and test set.
        It uses 1 single fold for training and k-1 folds for testing'''
        train = []
        test = []
        for i in range(len(folds)):
            if i == train_number:
                train += folds[i]
            else:
                test += folds[i]
        return (train, test)
    
    def group_folds(folds):
        '''Group folds in a single one'''
        train = []
        for i in range(len(folds)):
            train += folds[i]
        return train
    
    def split_into_folds(examples, n_folds=5, seed=None):
        '''For datasets as nell and yago that have only 1 mega-example'''
        temp = list(examples)
        random.seed(seed)
        random.shuffle(temp)
        s = math.ceil(len(examples)/n_folds)
        ret = []
        for i in range(n_folds-1):
            ret.append(temp[:s])
            temp = temp[s:]
        ret.append(temp)
        random.seed(None)
        return ret
    
    def get_json_dataset(dataset):
        '''Load dataset from json'''
        with open(os.path.join(__location__, 'files/json/' + dataset + '.json')) as data_file:
            data_loaded = json.load(data_file)
        return data_loaded
        
    def write_to_file(content, path):
        '''Takes a list (content) and a path/file (path) and writes each line of the list to the file location.'''
        with open(path, 'w') as f:
            for line in content:
                f.write(line + '\n')
        f.close()
    
    def load(dataset, bk=None):
        '''Load dataset from json and accept only predicates presented in bk'''
        pattern = '^(\w+)\(.*\).$'
        accepted = set()
        if bk:
            for line in bk:
                m = re.search(pattern, line)
                if m:
                    relation = re.sub('[ _]', '', m.group(1))
                    accepted.add(relation)
        data = datasets.get_json_dataset(dataset)
        facts = []
        for i in range(len(data[0])): #positives
            facts.append([])
            for relation, value in data[0][i].items():
                if not bk or relation in accepted:
                    for ex in value:
                        example = [x.capitalize() for x in ex]
                        if dataset == 'imdb':
                            if relation in ['actor', 'director']:
                                facts[i].append('Isa' + '(' + ','.join(example + [relation.capitalize()]) + ')')  
                                continue
                            if relation == 'female':
                                facts[i].append('Gender' + '(' + ','.join(example + [relation.capitalize()]) + ')')  
                                continue
                        if dataset == 'uwcse':
                            if relation in ['student', 'professor']:
                                facts[i].append('Isa' + '(' + ','.join(example + [relation.capitalize()]) + ')')  
                                continue
                            if len(example) > 2:
                                continue
                        facts[i].append(relation.capitalize() + '(' + ','.join(example)+ ')')                           
        return facts