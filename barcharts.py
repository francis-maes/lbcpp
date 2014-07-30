# -*- coding: utf-8 -*-
"""
Created on Wed Jul 30 10:03:57 2014

@author: Denny Verbeeck
"""

#!/usr/bin/python

from math import sqrt
import argparse
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt
import numpy as np
from collections import defaultdict
#%%
class MeanAndVariance:
  def __init__(self, (samplesSum, samplesCount, samplesSumOfSquares)):
    self.sS = samplesSum
    self.sC = samplesCount
    self.sSS = samplesSumOfSquares
  def __str__(self):
    mean = self.mean()
    stddev = self.stddev()
    return "{:.3e} +/- {:.3e}".format(mean, stddev)
  def mean(self):
    return self.sS / self.sC
  def stddev(self):
    variance = (self.sSS / self.sC) - self.mean() * self.mean()
    if (variance > 1e-6):
      return sqrt(variance)
    else:
      return 0.0

# parse a result node and return its contents
def parseResult(item):
  if (item.get('type') == 'ScalarVariableMeanAndVariance'):
    sum = float(item.find("./variable[@name='samplesSum']").text)
    count = float(item.find("./variable[@name='samplesCount']").text)
    sumSq = float(item.find("./variable[@name='samplesSumOfSquares']").text)
    return MeanAndVariance((sum, count, sumSq))
  else:
    return 0.0

def load_trace(tracefile):
  print('Parsing ' + tracefile)
  tree = ET.parse(tracefile)
  root = tree.getroot()
  # get workunit node
  wu_node = root.find('./variable/node')
  problem_nodes = wu_node.findall('./node')

  results = defaultdict(lambda : defaultdict(dict))

  for problem_node in problem_nodes:
    problem_name = problem_node.get('description')
    alg_nodes = wu_node.findall('./node/node')
    for alg_node in alg_nodes:
      result_nodes = alg_node.findall('./result')
      for r in result_nodes:
        if (r.get('ref') is None):
          # parse into MeanAndVariance object
          mv = parseResult(r)
          results[problem_name][r.get('resultName')][alg_node.get('description')] = mv
        else:
          # find the referenced result
          refResult = alg_node.find("./*[@id='" + r.get('ref') + "']")
          mv = parseResult(refResult)
          results[problem_name][r.get('resultName')][alg_node.get('description')] = mv
  return results

#%%
def main():
  description = "Create bar charts from LBCPP trace files"
  epilog = """Top-level <node>s must be problems, each algorithm is a child of a problem.
              Each algorithm node must contain at least one result."""
  parser = argparse.ArgumentParser(description=description, epilog=epilog)
  parser.add_argument("--tracefile", help="Path to the tracefile")
  args = parser.parse_args()
  run(args.tracefile)

def run(tracefile):
  results = load_trace(tracefile)
  width = 0.8
  for problem in results.keys():
    for resultName in results[problem].keys():
      means = [results[problem][resultName][key].mean() for key in results[problem][resultName].keys()]
      stddevs = [results[problem][resultName][key].stddev() for key in results[problem][resultName].keys()]
      ind = np.arange(len(means))
      ind = [x + (1-width) / 2 for x in ind]
      fig, ax = plt.subplots()
      ax.bar(ind, means, width, yerr=stddevs, color='r')
      ax.set_ylabel(resultName)
      ax.set_title(problem)
      ax.set_xticks(ind)
      ax.set_xticklabels(results[problem][resultName].keys(), rotation=15)
      ax.set_ylim(bottom=0)


#%%
def create_plots(curves):
  legend = ['NSGA-II','SMPSO','OMOPSO','AbYSS']
  colors = ['b','g','r','c','m','y','k']
  markers = ['o','v','^','s','x']

  for p in range(0, 16):
    plt.clf()
    alg_lines = []
    alg_nb = 0
    ax = plt.subplot(111)
    for alg in legend:
      xvals = [int(x) for x in curves[alg][str(p)]]
      xvals.sort()
      yvals = [curves[alg][str(p)][str(x)][-1][1].mean() for x in xvals]
      yerrs = [curves[alg][str(p)][str(x)][-1][1].stddev() for x in xvals]
      alg_lines.append(plt.errorbar(xvals, yvals, yerr=yerrs, fmt='-' + colors[alg_nb] + markers[alg_nb]))
      alg_nb += 1
    box = ax.get_position()
    ax.grid()
    ax.set_position([box.x0, box.y0, box.width*0.8, box.height])
    ax.legend([l[0] for l in alg_lines], legend, loc='center left', numpoints=1, bbox_to_anchor=(1, 0.8))
    ax.set_xlabel('Search-space dimensionality')
    ax.set_ylabel('Spread')
    savefig('dimimpact-spread-' + getName(p) + '.pdf')

#%%
if __name__ == "__main__":
  main()