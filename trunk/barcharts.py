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
import os

#%%
class MeanAndVariance:
  def __init__(self, data):
    samplesSum, samplesCount, samplesSumOfSquares = data
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

def parse_results(tracefile):
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

def load_plot(tracefile):
  tree = ET.parse(tracefile)
  root = tree.getroot()
  # get workunit node
  wu_node = root.find('./variable/node')
  problem_nodes = wu_node.findall('./node')

  results = defaultdict(lambda : defaultdict(list))

  for problem_node in problem_nodes:
    problem_name = problem_node.get('description')
    alg_nodes = wu_node.findall('./node/node')
    for alg_node in alg_nodes:
      fold_nodes = alg_node.findall("./node")
      fold_nodes = [node for node in fold_nodes if node.attrib['description'].startswith('Fold')]
      curve = list()
      for f in fold_nodes:
        points = f.findall("./node")
        points = [p for p in points if p.attrib['description'].startswith('Iteration')]
        fold_points = list()
        for p in points:
          result_nodes = p.findall("./result")
          if (len(result_nodes) > 1):
            iteration = int(p.find("./result[@resultName='iteration']").text)
            rrse = float(p.find("./result[@resultName='RRSE']").text)
            fold_points.append((iteration,rrse))
        if (len(curve) == 0):
          curve = fold_points
        else:
          if (len(curve) != len(fold_points)):
            print("Folds have different number of result points for " + alg_node.get('description'))
          else:
            for i in range(len(curve)):
              if (curve[i][0] != fold_points[i][0]):
                print("Folds have results on different points for " + alg_node.get('description'))
              else:
                curve[i] = (curve[i][0], curve[i][1] + fold_points[i][1])
      num_folds = len(fold_nodes)
      curve = [(i[0], i[1] / num_folds) for i in curve]
      results[problem_name][alg_node.get('description')] = curve

  return results

def draw_plot(results, filename):
  legend = ['iMauve', 'FIMT']
  colors = ['b','-g','r','c','m','y','k','g']
  markers = ['o','v','^','s','x','*','+','D']
  #algos = results[problem].keys()
  algos = ['iTotalMauveLLSQ', 'FIMTp']
  fig, ax = plt.subplots()
  alg_nb = 0
  for algo in algos:
    X = [x[0] for x in results[algo]]
    Y = [x[1] for x in results[algo]]
    ax.plot(X, Y, '-' + colors[alg_nb])
    alg_nb = alg_nb + 1
  ax.legend(legend)
  plt.grid(b=True, which='major', color='k', linestyle=':')
  ax.set_ybound(0, 1.5)
  plt.xlabel('# data points observed')
  plt.ylabel('RRSE')
  plt.savefig(filename, bbox_inches='tight')
  plt.close(fig)

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
  print("Loading " + tracefile)
  results = parse_results(tracefile)
  plot_results = load_plot(tracefile)
  outdir = os.path.dirname(os.path.abspath(tracefile))
  width = 0.8
  filenames = list()
  printHeader = True
  for problem in results.keys():
    for resultName in results[problem].keys():
      algNames = sorted(results[problem][resultName].keys())
      if printHeader:
        printHeader = False
        column_spec = '|l|'
        for a in algNames:
          column_spec = column_spec + 'c|'
        print('\\begin{table}[htp]')
        print('\\tiny')
        print('\\begin{tabular}{' + column_spec + '}')
        print('\\hline')
        header_row = ''
        for a in algNames:
          header_row = header_row + ' & ' + a
        print(header_row + '\\\\')
        print('\\hline')
      row = resultName
      minimum = 1e9
      for key in algNames:
        if (minimum > results[problem][resultName][key].mean()):
          minimum = results[problem][resultName][key].mean()
      for key in algNames:
        row = row + ' & '
        if (results[problem][resultName][key].mean() == minimum):
          row = row + '\\cellcolor{blue!25} '
        row = row + '$' + "{:.3f}".format(results[problem][resultName][key].mean()) + '_{' + "{:.3f}".format(results[problem][resultName][key].stddev()) + '}$'
      print(row + ' \\\\')
      print('\\hline')
      means = [results[problem][resultName][key].mean() for key in algNames]
      stddevs = [results[problem][resultName][key].stddev() for key in algNames]
      ind = np.arange(len(means))
      ind = [x + (1-width) / 2 for x in ind]
      fig, ax = plt.subplots()
      ax.bar(ind, means, width, yerr=stddevs, color='r')
      ax.set_ylabel(resultName)
      ax.set_title(problem)
      ax.set_xticks(ind)
      ax.set_xticklabels(algNames, rotation=20)
      ax.set_ylim(bottom=0)
      plt.grid(b=True, which='major', color='k', linestyle=':')
      plt.savefig(os.path.join(outdir, problem.replace(' ', '-') + "-" + resultName.replace(' ', '-') + ".pdf"), bbox_inches='tight')
      filenames.append((problem.replace(' ', '-') + "-" + resultName.replace(' ', '-') + ".pdf", resultName))
      plt.close(fig)
    print('\\end{tabular}')
    print('\\caption{Results for problem ' + problem + '}')
    print('\\end{table}')

    draw_plot(plot_results[problem], os.path.join(outdir, problem.replace(' ', '-') + "-curve.pdf"))
    filenames.append((problem.replace(' ', '-') + "-curve.pdf","RRSE Curve"))
    print('\\begin{figure}[htp]')
    print('\\centering')


    for f in filenames:
      print('\\begin{subfigure}{0.45\\textwidth}')
      print('  \\includegraphics[width=\\textwidth]{' + f[0] + '}')
      print('  \\caption{' + f[1] + '}')
      print('\\end{subfigure}')
    print('\\caption{Results for problem ' + problem + '}')
    print('\\end{figure}')
    print('\\clearpage')

#%%
if __name__ == "__main__":
  main()