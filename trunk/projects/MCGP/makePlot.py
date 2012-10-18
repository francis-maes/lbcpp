import matplotlib.pyplot as plt
import matplotlib.figure as fig
import numpy
import sys
import os

problem = "multiplexer"
directory = os.path.join("results_linux", problem)
#methods = ["beagle", "linux-beagle", "random-postfix", "linux-random-postfix"]
methods = ["beagle", "random-prefix", "random-postfix"]#, "nmc1-prefix", "nmc1-postfix", "nmc2-prefix", "nmc2-postfix", "nmc3-prefix", "nmc3-postfix"]#, "nrpa2-postfix", "nrpa3-postfix"]

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
        return [0,2100]
    elif problem == "parity":
        return [4,16]

evals = []
times = []
for method in methods:

  name = os.path.join(directory, method)
  evalRuns = parseFile(name + "-evals.txt")
  timeRuns = parseFile(name + "-times.txt")
  evals.append(computeMean(evalRuns))
  times.append(computeMean(timeRuns))

#print "Evaluations:"
#print computeXValues(False, len(evals_means))
#print evals_means
#print "Times:"
#print computeXValues(True, len(times_means))
#print times_means

plt.figure(1, figsize=(16, 6))
plt.clf()

plt.subplot(1, 2, 1)
plt.xscale('log')
plt.xlabel("Number of evaluations")
plt.ylabel("Best fitness")
plt.ylim( getYRange())
for i in range(len(methods)):
  evals_means = evals[i]
  plt.plot(computeXValues(False, len(evals_means)), evals_means, label=methods[i])
plt.legend(loc=4)

plt.subplot(1, 2, 2)
plt.xscale('log')
plt.xlabel("Time in seconds")
plt.ylabel("Best fitness")
plt.ylim(getYRange())
for i in range(len(methods)):
  times_means = times[i]
  plt.plot(computeXValues(True, len(times_means)), times_means, label=methods[i]) 
plt.legend(loc=4)

plt.show()

