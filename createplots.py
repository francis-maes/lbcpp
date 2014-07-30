#!/usr/bin/python

from math import sqrt
from os.path import splitext
from os import listdir
import argparse
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt
from collections import defaultdict
#%%
class MeanAndVariance:
  def __init__(self, (samplesSum, samplesCount, samplesSumOfSquares)):
    self.sS = samplesSum
    self.sC = samplesCount
    self.sSS = samplesSumOfSquares
  def mean(self):
    return self.sS / self.sC
  def stddev(self):
    variance = (self.sSS / self.sC) - self.mean() * self.mean()
    if (variance > 1e-6):
      return sqrt(variance)
    else:
      return 0.0

#%%

def getName(problemIdx):
  if problemIdx < 9:
    return "WFG" + str(problemIdx + 1)
  else:
    return "DTLZ" + str(problemIdx - 8)

def load_traces(tracefile, result_name):
  legend = ['NSGA-II','SMPSO','OMOPSO','AbYSS','']
  colors = ['b','g','r','c','m','y','k']
  markers = ['o','v','^','s','x']
  tracefiles = []
  if tracefile==None:
    for file in listdir('./'):
      if file.endswith('.trace'):
        tracefiles.append(file)
  else:
    tracefiles = [tracefile]
  curves = {}
  for tf in tracefiles:
    print('Parsing ' + tf)
    tree = ET.parse(tf)
    root = tree.getroot()
    # get workunit node
    wu_node = root.find('./variable/node')
    arguments = wu_node.get('description').split(',')
    numdims = arguments[1].strip()
    problemidx = arguments[6].strip()
    if int(problemidx) < 9:
      numdims = str(int(numdims) + 4)
    print("Problem " + str(problemidx) + " with " + str(numdims) + " dimensions")
    # get algorithm nodes
    alg_nodes = root.findall("./variable/node/node/node")
    refHv = float(root.find("./variable/node/node/result[@resultName='Reference front hypervolume']").text)
    plt.clf()
    ax = plt.subplot(111)
    alg_nb = 0
    alg_lines = []
    for alg_node in alg_nodes:
      alg_name = legend[alg_nb]
      curve_nodes = alg_node.findall("./node[@description='curve']/node")
      num_curve_points = len(curve_nodes)
      if not alg_name in curves:
        curves[alg_name] = {}
      if not problemidx in curves[alg_name]:
        curves[alg_name][problemidx] = {}
      if not numdims in curves[alg_name][problemidx]:
        curves[alg_name][problemidx][numdims] = []

      for i in range(0, num_curve_points):
        results = curve_nodes[i].find("./result[@resultName='" + result_name + "']")
        result_values = [float(x.text) for x in results if x.get('name') != 'name']
        curves[alg_name][problemidx][numdims].append((int(curve_nodes[i].find("./result[@resultName='NumEvaluations']").text),MeanAndVariance(result_values)))

      xvals = [x[0] for x in curves[alg_name][problemidx][numdims]]
      xvals.append(xvals[-1])
      hvvals = [x[1].mean() for x in curves[alg_name][problemidx][numdims]]
      hvvals.append(hvvals[-1])
      hverr = [x[1].stddev() for x in curves[alg_name][problemidx][numdims]]
      hverr.append(hverr[-1])
      alg_lines.append(ax.errorbar(xvals, hvvals, yerr=hverr, fmt='-' + markers[alg_nb]+colors[alg_nb], markevery=25, errorevery=25))
      alg_nb += 1
      if alg_nb == 4:   # the random search
        break;
    ax.grid()
    #alg_lines.append(ax.plot([0, 75000], [refHv, refHv], '-k'))
    #legend[4] = '$\mathcal{S}^* = ' + '{0:.3f}'.format(refHv) + '$'
    box = ax.get_position()
    ax.set_position([box.x0, box.y0, box.width*0.8, box.height])

    ax.legend([l[0] for l in alg_lines], legend, loc='center left', numpoints=1, bbox_to_anchor=(1, 0.8))
    ax.set_xlabel('# evaluations')
    ax.set_ylabel(result_name)
    plt.savefig(getName(int(problemidx)) + "-spread-" + str(numdims) + ".pdf")
  return curves

#%%
def main():
  description = "Create plots from LBCPP trace files"
  epilog = """Top-level <node>s must be problems, each algorithm is a child of a problem.
              Each algorithm must contain a <node> with the description "curve". Curves
              must have at least a result "x" and one other result."""
  parser = argparse.ArgumentParser(description=description, epilog=epilog)
  parser.add_argument("--numpoints", default="5", metavar="N", help="The number of points to draw.")
  parser.add_argument("--tracefile", help="Path to the tracefile")
  args = parser.parse_args()
  create_plots(args.tracefile, args.numpoints)
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