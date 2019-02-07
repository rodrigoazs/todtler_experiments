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

def gen_train_files(source, datas):
    count = 1
    for data in datas:
        write_to_file(data, 'domains/' + source + '-fold' + str(count) + '.db')
        count += 1
        
def gen_train_file(source, data):
    try:
        os.makedirs('domains')
    except:
        pass
    write_to_file(data, 'domains/' + source + '.db')
        
def gen_source_bk(source):
    global bk
    write_to_file(bk[source], 'domains/' + source + '.mln')
    
def do_experiment(identifier, source, target, src_predicate, predicate):
    global bk
    
    json = {}
    delete_train_files()
    
    if source not in ['nell_sports', 'nell_finances', 'yago2s']:
        #[tar_train_facts, tar_test_facts] =  datasets.get_kfold_small(i, tar_data[0])
        #[tar_train_pos, tar_test_pos] =  datasets.get_kfold_small(i, tar_data[1])
        #[tar_train_neg, tar_test_neg] =  datasets.get_kfold_small(i, tar_data[2])
        source_data = datasets.load(source)
    else:
        src_data = source_data = datasets.load(source, target=src_predicate, seed=results['save']['seed'], balanced=1)
        [src_train_facts, src_test_facts] =  [src_data[0][0], src_data[0][0]]
        to_folds_pos = datasets.split_into_folds(src_data[1][0], n_folds=n_folds, seed=results['save']['seed'])
        to_folds_neg = datasets.split_into_folds(src_data[2][0], n_folds=n_folds, seed=results['save']['seed'])
        #[tar_train_pos, tar_test_pos] =  datasets.get_kfold_small(i, to_folds_pos)
        #[tar_train_neg, tar_test_neg] =  datasets.get_kfold_small(i, to_folds_neg)
        source_data[0] = [src_train_facts + to_folds_pos[0], src_train_facts + to_folds_pos[1], src_train_facts + to_folds_pos[2]]
    
    #source_data = datasets.load(source)
    source_data = source_data[0]
    
    if target not in ['nell_sports', 'nell_finances', 'yago2s']:
        #[tar_train_facts, tar_test_facts] =  datasets.get_kfold_small(i, tar_data[0])
        #[tar_train_pos, tar_test_pos] =  datasets.get_kfold_small(i, tar_data[1])
        #[tar_train_neg, tar_test_neg] =  datasets.get_kfold_small(i, tar_data[2])
        target_data = datasets.load(target)
    else:
        tar_data = target_data = datasets.load(target, target=predicate, seed=results['save']['seed'], balanced=1)
        [tar_train_facts, tar_test_facts] =  [tar_data[0][0], tar_data[0][0]]
        to_folds_pos = datasets.split_into_folds(tar_data[1][0], n_folds=n_folds, seed=results['save']['seed'])
        to_folds_neg = datasets.split_into_folds(tar_data[2][0], n_folds=n_folds, seed=results['save']['seed'])
        #[tar_train_pos, tar_test_pos] =  datasets.get_kfold_small(i, to_folds_pos)
        #[tar_train_neg, tar_test_neg] =  datasets.get_kfold_small(i, to_folds_neg)
        target_data[0] = [tar_train_facts + to_folds_pos[0], tar_train_facts + to_folds_pos[1], tar_train_facts + to_folds_pos[2]]
    
    #target_data = datasets.load(target)
    #target_data = target_data[0]
    target_data = target_data[0]
    
    experiment_title = identifier + '_' + source + '_' + target
    create_dir('experiments/' + experiment_title)
    count_dir = count_directories('experiments/' + experiment_title)
    if count_dir < len(target_data): # n x n fold (target)
        start = time.time()
        exp_number = count_dir + 1
        
        print('Doing experiment number ' + str(exp_number))
        
        gen_source_bk(source) 
        gen_train_files(source, source_data)

        print('Generating templates for source domain')
        templating = time.time()        
        CALL = '(java -jar todtler-generator.jar -maxLiterals 3 -maxVariables 3 -domainFile domains/' + source + '.mln -templateFile ' + source + '-templates.csv -formulaFile ' + source + '-formulas.csv -formulaDirectory clauses/' + source + ' > todtler-generator.txt 2>&1)'
        call_process(CALL)
        json['Generating source template time'] = time.time() - templating
        print('Time taken: %s' % json['Generating source template time'])
        
        write_to_file(score(source, len(source_data)), 'score.sh')
        
        gen_source_bk(target)  
        gen_train_files(target, target_data)
        
        print('Generating templates for target domain')
        templating = time.time()  
        CALL = '(java -jar todtler-generator.jar -maxLiterals 3 -maxVariables 3 -domainFile domains/' + target + '.mln -templateFile ' + target + '-templates.csv -formulaFile ' + target + '-formulas.csv -formulaDirectory clauses/' + target + ' > todtler-generator.txt 2>&1)'
        call_process(CALL)
        json['Generating target template time'] = time.time() - templating
        print('Time taken: %s' % json['Generating target template time'])
        
        # sample clauses generated
        #clauses_size = 0.01
        #for clauses_path in ['clauses/' + source, 'clauses/' + target]:
        #    onlyfiles = [f for f in os.listdir(clauses_path) if os.path.isfile(os.path.join(clauses_path, f))]
        #    random.shuffle(onlyfiles)
        #    for file in onlyfiles[int(clauses_size * len(onlyfiles)):]:
        #        os.remove(os.path.join(clauses_path, file))
        #raise(Exception('aaa'))
        
        print('Scoring clauses')
        scoring = time.time()
        CALL = '(./score.sh > score.txt 2>&1)'
        call_process(CALL)
        scoring = time.time() - scoring
        print('Time taken: %s' % scoring)
        
        #raise(Exception('Scored clauses'))
        
        json['Learning time'] = []
        
        for i in range(len(target_data)):
            #delete_models()
            experiment_path = 'experiments/' + experiment_title + '/experiment' + str(exp_number) + '/fold' + str(i+1)
            create_dir(experiment_path)
            
            print('Learning fold ' + str(i+1))
            print(predicate)
            
            learning = time.time()
            CALL = '(java -jar todtler-learner.jar -sourceDirectory clauses/' + source + ' -targetDirectory clauses/' + target + ' -templateFileSource ' + source + '-templates.csv -formulaFileSource ' + source + '-formulas.csv -templateFileTarget ' + target + '-templates.csv -formulaFileTarget ' + target + '-formulas.csv -outputDirectory ' + experiment_path + ' -domainFile domains/' + target + '.mln -train domains/' + target + '-fold' + str(i+1) + '.db -ne ' + predicate.capitalize() + ' > todtler-learner.txt 2>&1)'
            call_process(CALL)
            learning = time.time() - learning
            json['Learning time'].append(learning)
            print('Time taken: %s' % learning)
            
            #raise(Exception('ue'))
            
            last = get_last_model(experiment_path)
            remove_all_but(experiment_path, last)
            #print('copying')
            #shutil.copyfile('models/' + source + '-' + target + '/model-'+ str(last) + '.mln', experiment_path + '/model.mln')
            #print('copying')
            #copy_all_models('models/' + source + '-' + target, experiment_path)
            #print('deleting')
            #delete_models()
            #print('deleted')

        end = time.time()
        json['Total experiment time'] = end - start
        print('Total experiment time taken %s' % json['Total experiment time'])
        save_experiment(json, experiment_title, exp_number)
    delete_train_files()
    
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
              'Haswordauthor(author,word)',
              'Haswordtitle(title,word)',
              'Haswordvenue(venue,word)'],
      'twitter': [#'Accounttype(account,+type)',
                  'Accounttype(account,type)',
                  #'Tweets(account,+word)',
                  'Tweets(account,word)',
                  'Follows(account,account)'],
      'yeast': ['Location(protein,loc)',
                'Interaction(protein,protein)',
                'Proteinclass(protein,class)',
                'Enzyme(protein,enz)',
                #'Function(protein,+fun)',
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
                        'Bankbankincountry(company,country)',
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
            ##{'id': '16', 'source':'yeast', 'target':'twitter', 'predicate':'interaction', 'to_predicate':'Follows'},
            #{'id': '17', 'source':'yeast', 'target':'twitter', 'predicate':'location', 'to_predicate':'tweets'},
            #{'id': '18', 'source':'yeast', 'target':'twitter', 'predicate':'enzyme', 'to_predicate':'tweets'},
            #{'id': '19', 'source':'yeast', 'target':'twitter', 'predicate':'function', 'to_predicate':'tweets'},
            #{'id': '20', 'source':'yeast', 'target':'twitter', 'predicate':'phenotype', 'to_predicate':'tweets'},
            #{'id': '21', 'source':'yeast', 'target':'twitter', 'predicate':'complex', 'to_predicate':'tweets'},
            #{'id': '22', 'source':'twitter', 'target':'yeast', 'predicate':'accounttype', 'to_predicate':'Proteinclass'},
            ##{'id': '23', 'source':'twitter', 'target':'yeast', 'predicate':'follows', 'to_predicate':'Interaction'},
            ##{'id': '1', 'source':'imdb', 'target':'uwcse', 'predicate':'workedunder', 'to_predicate':'Advisedby'},
            ##{'id': '2', 'source':'uwcse', 'target':'imdb', 'predicate':'advisedby', 'to_predicate':'Workedunder'},
            #{'id': '3', 'source':'imdb', 'target':'uwcse', 'predicate':'movie', 'to_predicate':'publication'},
            #{'id': '4', 'source':'uwcse', 'target':'imdb', 'predicate':'publication', 'to_predicate':'movie'},
            #{'id': '5', 'source':'imdb', 'target':'uwcse', 'predicate':'genre', 'to_predicate':'inphase'},
            #{'id': '6', 'source':'uwcse', 'target':'imdb', 'predicate':'inphase', 'to_predicate':'genre'},
            #{'id': '7', 'source':'imdb', 'target':'cora', 'predicate':'workedunder', 'to_predicate':'Samevenue'},
            #{'id': '53', 'source':'cora', 'target':'imdb', 'predicate':'samevenue', 'to_predicate':'Workedunder'},
            #{'id': '8', 'source':'imdb', 'target':'cora', 'predicate':'workedunder', 'to_predicate':'samebib'},
            ##{'id': '9', 'source':'imdb', 'target':'cora', 'predicate':'workedunder', 'to_predicate':'Sameauthor'},
            #{'id': '10', 'source':'imdb', 'target':'cora', 'predicate':'workedunder', 'to_predicate':'sametitle'},
            #{'id': '11', 'source':'uwcse', 'target':'cora', 'predicate':'advisedby', 'to_predicate':'samevenue'},
            #{'id': '12', 'source':'uwcse', 'target':'cora', 'predicate':'advisedby', 'to_predicate':'samebib'},
            #{'id': '13', 'source':'uwcse', 'target':'cora', 'predicate':'advisedby', 'to_predicate':'sameauthor'},
            #{'id': '14', 'source':'uwcse', 'target':'cora', 'predicate':'advisedby', 'to_predicate':'sametitle'},
            #{'id': '24', 'source':'twitter', 'target':'yeast', 'predicate':'tweets', 'to_predicate':'location'},
            #{'id': '25', 'source':'twitter', 'target':'yeast', 'predicate':'tweets', 'to_predicate':'enzyme'},
            #{'id': '26', 'source':'twitter', 'target':'yeast', 'predicate':'tweets', 'to_predicate':'function'},
            #{'id': '27', 'source':'twitter', 'target':'yeast', 'predicate':'tweets', 'to_predicate':'phenotype'},
            #{'id': '28', 'source':'twitter', 'target':'yeast', 'predicate':'tweets', 'to_predicate':'complex'},
            #{'id': '29', 'source':'nell_sports', 'target':'nell_finances', 'predicate':'teamalsoknownas', 'to_predicate':'companyalsoknownas'},
            #{'id': '30', 'source':'nell_sports', 'target':'nell_finances', 'predicate':'teamplaysagainstteam', 'to_predicate':'companyalsoknownas'},
            #{'id': '31', 'source':'nell_sports', 'target':'nell_finances', 'predicate':'teamplaysagainstteam', 'to_predicate':'acquired'},
            #{'id': '32', 'source':'nell_sports', 'target':'nell_finances', 'predicate':'teamplaysagainstteam', 'to_predicate':'bankboughtbank'},
            #{'id': '33', 'source':'nell_sports', 'target':'nell_finances', 'predicate':'athleteplayssport', 'to_predicate':'companyceo'},
            #{'id': '34', 'source':'nell_sports', 'target':'nell_finances', 'predicate':'athleteplayssport', 'to_predicate':'bankchiefexecutiveceo'},
            #{'id': '36', 'source':'nell_sports', 'target':'nell_finances', 'predicate':'athleteplaysforteam', 'to_predicate':'companyceo'},
            #{'id': '37', 'source':'nell_sports', 'target':'nell_finances', 'predicate':'teamplayssport', 'to_predicate':'Companyeconomicsector'},
            #{'id': '38', 'source':'nell_finances', 'target':'nell_sports', 'predicate':'companyalsoknownas', 'to_predicate':'teamalsoknownas'},
            #{'id': '39', 'source':'nell_finances', 'target':'nell_sports', 'predicate':'companyalsoknownas', 'to_predicate':'teamplaysagainstteam'},
            #{'id': '40', 'source':'nell_finances', 'target':'nell_sports', 'predicate':'acquired', 'to_predicate':'teamplaysagainstteam'},
            #{'id': '41', 'source':'nell_finances', 'target':'nell_sports', 'predicate':'bankboughtbank', 'to_predicate':'teamplaysagainstteam'},
            #{'id': '42', 'source':'nell_finances', 'target':'nell_sports', 'predicate':'companyceo', 'to_predicate':'athleteplayssport'},
            #{'id': '43', 'source':'nell_finances', 'target':'nell_sports', 'predicate':'bankchiefexecutiveceo', 'to_predicate':'athleteplayssport'},
            #{'id': '44', 'source':'nell_finances', 'target':'nell_sports', 'predicate':'bankchiefexecutiveceo', 'to_predicate':'athleteplaysforteam'},
            #{'id': '45', 'source':'nell_finances', 'target':'nell_sports', 'predicate':'companyceo', 'to_predicate':'athleteplaysforteam'},
            ##{'id': '46', 'source':'nell_finances', 'target':'nell_sports', 'predicate':'companyeconomicsector', 'to_predicate':'teamplayssport'},
            ##{'id': '35', 'source':'nell_sports', 'target':'nell_finances', 'predicate':'athleteplaysforteam', 'to_predicate':'bankchiefexecutiveceo'},
            ##{'id': '47', 'source':'yeast', 'target':'facebook', 'predicate':'interaction', 'to_predicate':'Edge'},
            ##{'id': '48', 'source':'twitter', 'target':'facebook', 'predicate':'follows', 'to_predicate':'Edge'},
            ##{'id': '49', 'source':'imdb', 'target':'facebook', 'predicate':'workedunder', 'to_predicate':'Edge'},
            ##{'id': '50', 'source':'uwcse', 'target':'facebook', 'predicate':'advisedby', 'to_predicate':'Edge'},
            ]

firstRun = False
n_runs = 1
folds = n_folds = 3
            
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
    
while results['save']['n_runs'] < n_runs:
    experiment = results['save']['experiment'] % len(experiments)
    experiment_title = experiments[experiment]['id'] + '_' + experiments[experiment]['source'] + '_' + experiments[experiment]['target']
    print('Run: ' + str(results['save']['n_runs']) + ' ' + experiment_title)
    #try:
    do_experiment(experiments[experiment]['id'], experiments[experiment]['source'], experiments[experiment]['target'], experiments[experiment]['predicate'], experiments[experiment]['to_predicate'])
    #except Exception as e:
    #    print(e)
    #    print('Error in experiment of ' + experiment_title)
    #    pass
    results['save']['experiment'] += 1
    results['save']['n_runs'] += 1
    save(results)
