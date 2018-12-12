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
        
    def balance_neg(target, data, size, seed=None):
        '''Receives [facts, pos, neg] and balance neg according to pos'''
        ret = copy.deepcopy(data)
        neg = []
        random.seed(seed)
        random.shuffle(ret)
        ret = ret[:size]
        random.seed(None)
        for entities in ret:
            example = [x.capitalize() for x in entities]
            neg.append(target.capitalize() + '(' + ','.join(example) + ')')
        return neg
        
    def get_neg(target, data):
        '''Receives [facts, pos, neg] and return neg'''
        ret = copy.deepcopy(data)
        neg = []
        for entities in ret:
            example = [x.capitalize() for x in entities]
            neg.append(target.capitalize() + '(' + ','.join(example) + ')')
        return neg
        
    def generate_neg(target, data, amount=1, seed=None):
        '''Receives [facts, pos, neg] and generates balanced neg examples in neg according to pos'''
        pos = copy.deepcopy(data)
        neg = []
        objects = set()
        subjects = {}
        for entities in pos:
            if entities[0] not in subjects:
                subjects[entities[0]] = set()
            subjects[entities[0]].add(entities[1])
            objects.add(entities[1])
        random.seed(seed)
        target_objects = list(objects)
        for entities in pos:
            key = entities[0]
            for j in range(amount):
                for tr in range(10):
                    r = random.randint(0, len(target_objects)-1)
                    if target_objects[r] not in subjects[key]:
                        neg.append(target.capitalize() + '(' + ','.join([key.capitalize(), target_objects[r].capitalize()]) + ')')
                        break
        random.seed(None)
        return neg
    
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
    
    def load(dataset, bk=None, target=None, seed=None, balanced=False):
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
        pos = []
        neg = []
        for i in range(len(data[0])): #positives
            facts.append([])
            pos.append([])
            neg.append([])
            for relation, value in data[0][i].items():
                if not bk or relation in accepted:
                    if not target or relation.lower() != target.lower():
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
                            if dataset == 'twitter':
                                if relation == 'tweets':
                                    example = [example[0], '"' + example[1].lower() + '"']
                                    facts[i].append(relation.capitalize() + '(' + ','.join(example) + ')')  
                                    continue
                                if relation == 'typeaccount':
                                    continue
                            if dataset == 'yeast':
                                if relation == 'classprotein':
                                    continue
                            if dataset == 'nell_finances':
                                if relation in ['ceoof', 'ceoeconomicsector']:
                                    continue
                            facts[i].append(relation.capitalize() + '(' + ','.join(example)+ ')')
                    else:
                        for ex in value:
                            example = [x.capitalize() for x in ex]
                            if dataset == 'imdb':
                                if relation in ['actor', 'director']:
                                    pos[i].append('Isa' + '(' + ','.join(example + [relation.capitalize()]) + ')')  
                                    continue
                                if relation == 'female':
                                    pos[i].append('Gender' + '(' + ','.join(example + [relation.capitalize()]) + ')')  
                                    continue
                            if dataset == 'uwcse':
                                if relation in ['student', 'professor']:
                                    pos[i].append('Isa' + '(' + ','.join(example + [relation.capitalize()]) + ')')  
                                    continue
                                if len(example) > 2:
                                    continue
                            if dataset == 'twitter':
                                if relation == 'tweets':
                                    example = [example[0], '"' + example[1].lower() + '"']
                                    pos[i].append(relation.capitalize() + '(' + ','.join(example) + ')')  
                                    continue
                                if relation == 'typeaccount':
                                    continue
                            if dataset == 'yeast':
                                if relation == 'classprotein':
                                    continue
                            if dataset == 'nell_finances':
                                if relation in ['ceoof', 'ceoeconomicsector']:
                                    continue
                            pos[i].append(relation.capitalize() + '(' + ','.join(example)+ ')')
        if target:
            for i in range(len(data[1])): #negatives
                if target.lower() in data[1][i]:
                    value = data[1][i][target.lower()]
                    if balanced:
                        print('az')
                        neg[i] = datasets.balance_neg(target.lower(), value, int(balanced * len(data[0][i][target.lower()])), seed=seed)
                        #if len(neg[i]) > len(data[0][i][target]):
                        #    # NEW
                        #    amnt = math.ceil((2 if not balanced else balanced))
                        #    temp = datasets.generate_neg(target, data[0][i][target], amount=amnt, seed=seed)
                        #    temp = neg[i] + temp
                        #    temp = temp[:int(balanced * len(data[0][i][target]))]
                        #    neg[i] = temp
                    else:
                        neg[i] = datasets.get_neg(target.lower(), value)
                else:
                    value = data[0][i][target.lower()]
                    if balanced:
                        neg[i] = datasets.generate_neg(target.lower(), value, amount=(1 if not balanced else balanced), seed=seed)
        return [facts, pos, neg]