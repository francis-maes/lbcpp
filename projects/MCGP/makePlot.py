import matplotlib.pyplot as plt
import matplotlib.figure as fig
import numpy
import sys
import os

directory = "results_fast"
problems = ["symbreg", "ant", "parity", "multiplexer"]

#methods = ["beagle", "random-postfix", "nmc1-postfix", "nmc2-postfix", "nmc3-postfix"]
#methods = ["beagle", "random-prefix", "nmc1-prefix", "nmc2-prefix", "nmc3-prefix"]
#methods = ["beagle", "random-prefix", "random-postfix", "nmc2-prefix", "nmc2-postfix"]

#methods = ["beagle", "random-postfix", "nrpa2-postfix", "nrpa3-postfix"]
#methods = ["beagle", "random-prefix", "nrpa2-prefix", "nrpa3-prefix"]
#methods = ["beagle", "random-prefix", "nrpa2-prefix", "bnrpa8-2-prefix", "nrpa3-prefix", "bnrpa8-3-prefix"]
#methods = ["beagle", "random-postfix", "nrpa2-postfix", "bnrpa8-2-postfix", "nrpa3-postfix", "bnrpa8-3-postfix"]
methods = ["beagle", "random-postfix", "nmc2-postfix", "nrpa3-postfix"]
#methods = ["beagle", "treegp-postfix", "treegp-samplers-postfix"]
inFunctionOfTime = False

def parseFile(filename):
    res = []
    for line in open(filename):
        values = []
        for token in line.split():
            values.append(float(token))
        res.append(values)
    return res

def getLongestLength(results):
    res = 0
    for row in results:
        if len(row) > res:
            res = len(row)
    return res

def getValue(row, index):
    if index >= len(row):
        return row[len(row)-1]
    else:
        return row[index]

def computeMean(results):
    l = getLongestLength(results)
    res = numpy.zeros(l)
    for i in range(l):
        for row in results:
            res[i] += getValue(row, i)
        res[i] /= len(results)
    return res

def computeXValues(isTime, length):
    res = numpy.zeros(length)
    if isTime:
        val = 0.001
    else:
        val = 1.5
    for i in range(length):
        res[i] = val
        val *= 1.5
    return res

def getYRange():
    if problem == "symbreg":
        return [0.3, 1.0]
    elif problem == "ant":
        return [0, 100]
    elif problem == "multiplexer":
        return [500,2100]
    elif problem == "parity":
        return [4,16]

results = []
for problem in problems:
  for method in methods:

    name = os.path.join(os.path.join(directory, problem), method)
    if inFunctionOfTime:
        name = name + "-times.txt"
    else:
        name = name + "-evals.txt"
    data = parseFile(name)
    results.append(computeMean(data))

#print "Evaluations:"
#print computeXValues(False, len(evals_means))
#print evals_means
#print "Times:"
#print computeXValues(True, len(times_means))
#print times_means

plt.figure(1, figsize=(12, 12))
plt.clf()

problemIndex = 0
resultsIndex = 0
for problem in problems:
    problemIndex = problemIndex + 1
    plt.subplot(2, 2, problemIndex)
    
    plt.xscale('log')
    if inFunctionOfTime:
        plt.xlabel("Time in seconds")
    else:
        plt.xlabel("Number of evaluations")
    plt.ylabel("Best fitness")
    plt.ylim( getYRange())
    plt.title(problem)
    for i in range(len(methods)):
      data = results[resultsIndex]
      resultsIndex = resultsIndex + 1
      plt.plot(computeXValues(inFunctionOfTime, len(data)), data, label=methods[i])
    plt.legend(loc=4)

plt.show()

