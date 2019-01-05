#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Dec  1 20:54:57 2018

@author: rodrigoazs
"""

from datasets.get_datasets import *
import shutil
import time
import math
import numpy as np
import os
import re
import sys
import re

if os.name == 'posix' and sys.version_info[0] < 3:
    import subprocess32 as subprocess
else:
    import subprocess
    
def score(source, n_folds):
    folds = ['domains/' + source + '-fold' + str(i+1) + '.db' for i in range(n_folds)]
    text = ['#!/bin/bash',
            "databases='" + ','.join(folds) + "';",
            'for inputfile in `find $1 -type f -name "*.mln" ! -name "*output*" ! -empty`; do',
            " logfile=`echo ${inputfile} | sed 's/.mln/-output.log/g'`;",
            " outputfile=`echo ${inputfile} | sed 's/.mln/-output.mln/g'`;",
            ' echo "learnstruct -i ${inputfile} -o ${outputfile} -t ${databases} -search 999 -penalty 0.001 -minWt 0.05 -fractAtoms 1 -maxAtomSamples 10000 -maxClauseSamples 10000 -tightConvThresh 1e-3 -looseConvThresh 1e-2 -tightMaxIter 10 -plusType 2 -multipleDatabases -startFromEmptyMLN > ${logfile}";',
            ' ./alchemy/bin/learnstruct -i ${inputfile} -o ${outputfile} -t ${databases} -search 999 -penalty 0.001 -minWt 0.05 -fractAtoms 1 -maxAtomSamples 10000 -maxClauseSamples 10000 -tightConvThresh 1e-3 -looseConvThresh 1e-2 -tightMaxIter 10 -plusType 2 -multipleDatabases -startFromEmptyMLN > ${logfile}',
            'done']
    return text

def call_process(call):
    '''Create a subprocess and wait for it to finish. Error out if errors occur.'''
    try:
        p = subprocess.Popen(call, shell=True)
        os.waitpid(p.pid, 0)
    except:
        raise(Exception('Encountered problems while running process: ', call))
        
def write_to_file(content, path):
    '''Takes a list (content) and a path/file (path) and writes each line of the list to the file location.'''
    with open(path, 'w') as f:
        for line in content:
            f.write(line + '\n')
    f.close()
    
def read_from_file(path):
    with open(path) as f:
        lines = f.read().splitlines()
        return lines
    
def save(data):
    with open('experiments/transfer_experiment.json', 'w') as fp:
        json.dump(data, fp)
        
def delete_train_files():
    '''Remove files from train folder'''
    try:
        delete_create_dir('clauses')
        delete_create_dir('domains')
        delete_create_dir('models')
        delete_create_dir('test')
        delete_files('mln')
        delete_files('csv')
    except:
        pass
    
def delete_models():
    try:
        delete_create_dir('models')
    except:
        pass
    
def delete_create_dir(path):
    try:
        shutil.rmtree(path)
    except:
        pass
    os.makedirs(path)
    
def save_experiment(data, experiment_title, exp_number):
    if not os.path.exists('experiments/' + experiment_title + '/experiment' + str(exp_number)):
        os.makedirs('experiments/' + experiment_title + '/experiment' + str(exp_number))
    results = []
    if os.path.isfile('experiments/' + experiment_title + '/experiment' + str(exp_number) + '/' + experiment_title + '.json'):
        with open('experiments/' + experiment_title + '/experiment' + str(exp_number) + '/' + experiment_title + '.json', 'r') as fp:
            results = json.load(fp)
    results.append(data)
    with open('experiments/' + experiment_title + '/experiment' + str(exp_number) + '/' + experiment_title + '.json', 'w') as fp:
        json.dump(results, fp)
    
def count_directories(d):
    return len([os.path.join(d, o) for o in os.listdir(d) if os.path.isdir(os.path.join(d,o))])
    
def create_dir(path):
    try:
        os.makedirs(path)
    except:
        pass
    
def delete_files(ext):
    test = os.listdir()
    for item in test:
        if item.endswith('.' + ext):
            os.remove(item)
            
def fix_symbols_model(model):
    new_model = []
    for line in model:
        m = re.search('^(\w+)\(([\w, +]+)*\)$', line)
        if m:
            t = re.sub('\+', '', line)
            new_model.append(t)
        else:
            new_model.append(line)
    return new_model
            
def get_last_model(path):
    test = os.listdir(path)
    last = 0
    for item in test:
        if item.startswith('model-') and item.endswith('.mln'):
            #print(item)
            n = re.sub('[^\d]', '', item)
            if n:
                n = int(n)
                if n > last:
                    last = n
        #else:
            #os.remove(path + '/' + item)
    #print('last is '+ str(last))
    return last
    
def remove_all_but(path, last):
    test = os.listdir(path)
    for item in test:
        if item != 'model-' + str(last) + '.mln':
            #print(path + '/' + item)
            os.remove(path + '/' + item)
    
def copy_all_models(path, experiment_path):
    test = os.listdir(path)
    for item in test:
        if item.startswith('model-') and item.endswith('.mln'):
            #print('copying '+ item)
            shutil.copyfile(path + '/' + item, experiment_path + '/' + item)            
       
def gen_test_files(source, sdata):
    try:
        os.makedirs('test')
    except:
        pass
    count = 1
    for data in sdata:
        write_to_file(data, 'test/' + source + '-fold' + str(count) + '.db')
        count += 1
        
def gen_test_file(data):
    try:
        os.makedirs('test')
    except:
        pass
    write_to_file(data, 'test/test.db')
        
def gen_source_bk(source):
    global bk
    write_to_file(bk[source], 'domains/' + source + '.mln')
    
def get_test_infer(infer, pos, neg):
    retPos = []
    retNeg = []
    for line in infer:
        m = re.search('^(\w+\([\w, +"]*\)) (.*)$', line)
        if m:
            if m.group(1) in pos:
                retPos.append(m.group(2) + ' 1')
            elif m.group(1) in neg or len(neg) == 0:
                retNeg.append(m.group(2) + ' 0')
    return retPos + retNeg
    
def cll(posProb, negProb):
    llSum = 0
    for prob in posProb:
        if prob == 0:
            prob = 1e-6
        llSum += math.log(prob)
    for prob in negProb:
        if prob == 1:
            prob = 1 - 1e-6
        llSum += math.log(1 - prob)
    return llSum/(len(posProb) + len(negProb))
    
def get_cll(infer, pos, neg):
    retPos = []
    retNeg = []
    for line in infer:
        m = re.search('^(\w+\([\w, +"]*\)) (.*)$', line)
        if m:
            if m.group(1) in pos:
                retPos.append(results_to_float(m.group(2)))
            elif m.group(1) in neg or len(neg) == 0:
                retNeg.append(results_to_float(m.group(2)))
    return cll(retPos, retNeg)

def results_to_float(string):
    '''Results can be printed with comma format.'''
    return float(string.replace(',','.'))
    
def get_results(infer, pos, neg):
    with open('testauc.txt', 'r') as f:
        text = f.read()
    PRPattern = 'Area Under the Curve for Precision \- Recall is ([\d\.eE-]+)'
    ROCPattern = 'Area Under the Curve for ROC is ([\d\.eE-]+)'
    aucpr = re.findall(PRPattern, text)
    aucroc = re.findall(ROCPattern, text)
    #print(aucroc)
    #print(aucpr)
    results = {
            'AUC ROC': results_to_float(aucroc[0]),
            'AUC PR': results_to_float(aucpr[0]),
            'CLL': get_cll(infer, pos, neg),
        }
    return results
    
def do_experiment(identifier, source, target, predicate):
    global bk

    delete_train_files()
    target_data = datasets.load(target, target=predicate)
    target_data = target_data[0]
    gen_test_files(target, target_data)
    
    experiment_title = identifier + '_' + source + '_' + target
    #create_dir('experiments/' + experiment_title)
    count_dir = count_directories('experiments/' + experiment_title)
    for fold in range(count_dir):
        #start = time.time()
        exp_number = fold + 1
        
        print('Scoring experiment number ' + str(exp_number))
        #gen_source_bk(target)
        
        for i in range(len(target_data)):
            #delete_models()
            experiment_path = 'experiments/' + experiment_title + '/experiment' + str(exp_number) + '/fold' + str(i+1)
            
            print('\nScoring fold ' + str(i+1))
            
            #for each fold test on remaining folds
            inferences = set()
            pos = set()
            neg = set()
            remaining = [p for p in range(len(target_data)) if p != i]
            for p in remaining:
                tgt_data = datasets.load(target, target=predicate, seed=results['save']['seed'], balanced=1)
                exs = tgt_data[1][p] + tgt_data[2][p]
                pos = pos.union(set(tgt_data[1][p]))
                neg = neg.union(set(tgt_data[2][p]))
                print('Scoring fold ' + str(i+1) + ' with fold ' + str(p+1))
                last = get_last_model(experiment_path)
                #os.remove('test.result')
                scoring = time.time()
                model = fix_symbols_model(read_from_file(experiment_path + '/model.mln'))
                write_to_file(model, 'model.mln')
                #CALL = '(./infer -i ' + experiment_path + '/model-' + str(last) + '.mln -r ' + experiment_path + '/fold' + str(p+1) + '.result -e test/' + source + '-fold' + str(p+1) + '.db -q ' + predicate.capitalize() + ' > infer.txt 2>&1)'
                #CALL = '(./infer -i model.mln -r test.result -e test/' + target + '-fold' + str(p+1) + '.db -q "' + ','.join(exs) + '" > infer.txt 2>&1)'
                CALL = '(./infer -i model.mln -r test.result -e test/' + target + '-fold' + str(p+1) + '.db -q ' + predicate.capitalize() + ' > infer.txt 2>&1)'
                call_process(CALL)
                scoring = time.time() - scoring
                print('Time taken: %s' % scoring)
                content = read_from_file('test.result')
                inferences = inferences.union(set(content))
            #neg = []
            #write_to_file(pos, 'pos.txt')
            #write_to_file(neg, 'neg.txt')
            #write_to_file(inferences, 'inferences.txt')
            write_to_file(get_test_infer(inferences, pos, neg), 'test.txt')
            CALL = '(java -jar auc.jar test.txt list 0 > testauc.txt 2>&1)'
            call_process(CALL)
            try:
                print(get_results(inferences, pos, neg))
            except:
                pass
    
#def do_experiment(identifier, source, target, predicate):
#    global bk
#    delete_train_files()
#    tar_data = datasets.load(target, target=predicate, seed=results['save']['seed'], balanced=1)
#    #t_target_data = datasets.load(target)
#    #gen_test_files(target, t_target_data[0])
#    #target_data = tar_data[0]
#    #gen_test_files(target, target_data)
#    experiment_title = identifier + '_' + source + '_' + target
#    #create_dir('experiments/' + experiment_title)
#    count_dir = count_directories('experiments/' + experiment_title)
#    for fold in range(count_dir):
#        #start = time.time()
#        exp_number = fold + 1
#        print('Scoring experiment number ' + str(exp_number))
#        
#        #gen_source_bk(target)
#        
#        for i in range(len(tar_data[0])):
#            #delete_models()
#            experiment_path = 'experiments/' + experiment_title + '/experiment' + str(exp_number) + '/fold' + str(i+1)
#            
#            # Group and shuffle
#            if target not in ['nell_sports', 'nell_finances', 'yago2s']:
#                [tar_train_facts, tar_test_facts] =  datasets.get_kfold_small(i, tar_data[0])
#                [tar_train_pos, tar_test_pos] =  datasets.get_kfold_small(i, tar_data[1])
#                [tar_train_neg, tar_test_neg] =  datasets.get_kfold_small(i, tar_data[2])
#            else:
#                [tar_train_facts, tar_test_facts] =  [tar_data[0][0], tar_data[0][0]]
#                to_folds_pos = datasets.split_into_folds(tar_data[1][0], n_folds=n_folds, seed=results['save']['seed'])
#                to_folds_neg = datasets.split_into_folds(tar_data[2][0], n_folds=n_folds, seed=results['save']['seed'])
#                [tar_train_pos, tar_test_pos] =  datasets.get_kfold_small(i, to_folds_pos)
#                [tar_train_neg, tar_test_neg] =  datasets.get_kfold_small(i, to_folds_neg)
#                
#            gen_test_file(tar_test_facts)
#            
#            print('Scoring fold ' + str(i+1))
#
#            inferences = set()
#            pos = tar_test_pos
#            neg = tar_test_neg
#            #tgt_data = datasets.load(target, target=predicate, seed=results['save']['seed'], balanced=1)
#            #remaining = [p for p in range(len(target_data)) if p != i]
#            #for p in remaining:
#            #pos = pos.union(tgt_data[1][p])
#            #pos += tgt_data[1][p]
#            #neg = neg.union(tgt_data[2][p])
#            #neg += tgt_data[2][p]
#            #print('Scoring fold ' + str(i+1) + ' with fold ' + str(p+1))
#            #last = get_last_model(experiment_path)
#            #os.remove('test.result')
#            scoring = time.time()
#            model = fix_symbols_model(read_from_file(experiment_path + '/model.mln'))
#            write_to_file(model, 'model.mln')
#            #exs = list(pos)+list(neg)
#            #exs = ','.join(exs)
#            #write_to_file(exs, 'queries.txt')
#            exs = tar_test_pos + tar_test_neg
#            CALL = '(./infer -i model.mln -ms 1 -r test.result -e test/test.db -q "' + ','.join(exs) + '" > infer.txt 2>&1)'
#            #CALL = '(./infer -i model.mln -ms 1 -r test.result -e test/test.db -q '+ predicate +' > infer.txt 2>&1)'
#            call_process(CALL)
#            scoring = time.time() - scoring
#            print('Time taken: %s' % scoring)
#            content = read_from_file('test.result')
#            inferences = inferences.union(set(content))
#            write_to_file(get_test_infer(inferences, pos, neg), 'test.txt')
#            CALL = '(java -jar auc.jar test.txt list 0 > testauc.txt 2>&1)'
#            call_process(CALL)
#            try:
#                print(get_results(inferences, pos, neg))
#            except:
#                pass
    
bk = {
      'dummy': ['Follows(person,person)',
                'Loves(person,person)',
                'Hates(person,person)'],
      'imdb': ['Workedunder(person,person)',
              'Gender(person,+gender)',
              'Isa(person,+isa)',
              'Movie(movie,person)',
              'Genre(person,genre)'],
      'uwcse': ['Isa(person,+isa)',
        'Advisedby(person,person)',
        'Tempadvisedby(person,person)',
        #'Ta(course,person,quarter).',
        'Hasposition(person,faculty)',
        'Publication(title,person)',
        'Inphase(person,prequals)',
        'Courselevel(course,level)',
        'Yearsinprogram(person,year)',
        'Projectmember(project,person)',
        'Sameproject(project,project)',
        'Samecourse(course,course)',
        'Sameperson(person,person)',],
      'cora': ['Sameauthor(author,author)',
              'Samebib(class,class)',
              'Sametitle(title,title)',
              'Samevenue(venue,venue)',
              'Author(class,author)',
              'Title(class,title)',
              'Venue(class,venue)',
              'Haswordauthor(author,+word)',
              'Haswordtitle(title,+word)',
              'Haswordvenue(venue,+word)'],
      'twitter': ['Accounttype(account,type)',
                  'Tweets(account,word)',
                  'Follows(account,account)'],
      'yeast': ['Location(protein,loc)',
                'Interaction(protein,protein)',
                'Proteinclass(protein,class)',
                'Enzyme(protein,enz)',
                'Function(protein,fun)',
                'Complex(protein,com)',
                'Phenotype(protein,phe)'],
      'nell_sports': ['Athleteledsportsteam(athlete,sportsteam)',
              'Athleteplaysforteam(athlete,sportsteam)',
              'Athleteplaysinleague(athlete,sportsleague)',
              'Athleteplayssport(athlete,sport)',
              'Teamalsoknownas(sportsteam,sportsteam)',
              'Teamplaysagainstteam(sportsteam,sportsteam)',
              'Teamplaysinleague(sportsteam,sportsleague)',
              'Teamplayssport(sportsteam,sport)'],
      'nell_finances': ['Countryhascompanyoffice(country,company)',
                        'Companyeconomicsector(company,sector)',
                        'Economicsectorcompany(sector,company)',
                        'Companyceo(company,person)',
                        'Companyalsoknownas(company,company)',
                        'Cityhascompanyoffice(city,company)',
                        'Acquired(company,company)',
                        'Bankbankincountry(person,country)',
                        'Bankboughtbank(company,company)',
                        'Bankchiefexecutiveceo(company,person)'],
    'facebook': ['Edge(person,person)',
            'Middlename(person,middlename)',
            'Lastname(person,lastname)',
            'Educationtype(person,educationtype)',
            'Workprojects(person,workprojects)',
            'Educationyear(person,educationyear)',
            'Educationwith(person,educationwith)',
            'Location(person,location)',
            'Workwith(person,workwith)',
            'Workenddate(person,workenddate)',
            'Languages(person,languages)',
            'Religion(person,religion)',
            'Political(person,political)',
            'Workemployer(person,workemployer)',
            'Hometown(person,hometown)',
            'Educationconcentration(person,educationconcentration)',
            'Workfrom(person,workfrom)',
            'Workstartdate(person,workstartdate)',
            'Worklocation(person,worklocation)',
            'Educationclasses(person,educationclasses)',
            'Workposition(person,workposition)',
            'Firstname(person,firstname)',
            'Birthday(person,birthday)',
            'Educationschool(person,educationschool)',
            'Name(person,name)',
            'Gender(person,gender)',
            'Educationdegree(person,educationdegree)',
            'Locale(person,locale)']
        }
        
experiments = [
            #{'id': '0', 'source':'dummy', 'target':'twitter', 'predicate':'proteinclass', 'to_predicate':'Accounttype'},
            {'id': '15', 'source':'yeast', 'target':'twitter', 'predicate':'proteinclass', 'to_predicate':'Accounttype'},
            {'id': '2', 'source':'uwcse', 'target':'imdb', 'predicate':'advisedby', 'to_predicate':'Workedunder'},
            {'id': '1', 'source':'imdb', 'target':'uwcse', 'predicate':'workedunder', 'to_predicate':'Advisedby'},
            {'id': '16', 'source':'yeast', 'target':'twitter', 'predicate':'interaction', 'to_predicate':'Follows'},
            ]

firstRun = False
n_runs = 12
folds = 3
            
if os.path.isfile('experiments/transfer_experiment.json'):
    with open('experiments/transfer_experiment.json', 'r') as fp:
        results = json.load(fp)
else:
    results = { 'save': { }}
    firstRun = True
        
if firstRun:
    results['save'] = {
        'experiment': 0,
        'n_runs': 0,
        'seed': 441773,
        'folds' : 3,      
        }
  
for experiment in experiments:
    experiment_title = experiment['id'] + '_' + experiment['source'] + '_' + experiment['target']
    print('Run: ' + experiment_title)
    #try:
    do_experiment(experiment['id'], experiment['source'], experiment['target'], experiment['to_predicate'])
    #except Exception as e:
    #    print(e)
    #    print('Error in experiment of ' + experiment_title)
    #    pass
    #results['save']['experiment'] += 1
    #results['save']['n_runs'] += 1
    #save(results)