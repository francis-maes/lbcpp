#!/usr/bin/python

from math import sqrt
import os
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

def parseResult(item):
  if (item.get('type') == 'ScalarVariableMeanAndVariance'):
    sum = float(item.find("./variable[@name='samplesSum']").text)
    count = float(item.find("./variable[@name='samplesCount']").text)
    sumSq = float(item.find("./variable[@name='samplesSumOfSquares']").text)
    return MeanAndVariance((sum, count, sumSq))
  else:
    return 0.0

def load_trace(tracefile):
  tree = ET.parse(tracefile)
  root = tree.getroot()
  # get workunit node
  wu_node = root.find('./variable/node')
  problem_node = wu_node.find('./node')
  problem_name = problem_node.attrib['description']
  curve_node = wu_node.find("./node[@description='Curve']")
  curve = list()
  for node in curve_node.findall('./node'):
    item = dict()
    for result in node.findall('./result'):
      if 'resultName' in result.attrib:
	  item[result.attrib['resultName']] = float(result.text)
    curve.append(item)
  results = defaultdict(dict)
  for node in problem_node.findall('./node'):
    result_nodes = node.findall('./result')
    for r in result_nodes:
      if (r.get('ref') is None):
        # parse into MeanAndVariance object
        mv = parseResult(r)
        results[node.get('description')][r.get('resultName')] = mv
      else:
        # find the referenced result
        refResult = node.find("./*[@id='" + r.get('ref') + "']")
        mv = parseResult(refResult)
        results[node.get('description')][r.get('resultName')] = mv
  problem_name = problem_name[:20] if len(problem_name) > 20 else problem_name
  return {'name': problem_name, 'curve': curve, 'results': results}


#%%
def main():
  description = "Create plots from LBCPP trace files"
  epilog = """There must be a node "curve", the contents of this node will be plotted."""
  parser = argparse.ArgumentParser(description=description, epilog=epilog)
  parser.add_argument("--path", help="Path to the tracefiles")
  args = parser.parse_args()
  create_plots(args.path)
#%%
def create_plots(path):

  legend = ['iMauve','FIMT']
  colors = ['b','-g','r','c','m','y','k']

  problem_results = dict()
  plot_filenames = list()
  for tf in os.listdir(path):
    if not tf.endswith('.trace'):
      continue
    result = load_trace(os.path.join(path, tf))
    curves = result['curve']
    problem_name = result['name']
    problem_results[problem_name] = result['results']
    
    plt.clf()
    alg_lines = []
    alg_nb = 0
    ax = plt.subplot(111)
    X = [i['Iterations'] for i in curves]
    algs = [i for i in curves[0] if i != 'Iterations']
    
    for a in algs:
      yvals = [i[a] for i in curves]
      alg_lines.append(plt.plot(X, yvals, '-' + colors[alg_nb]))
      alg_nb += 1
    ax.grid()
    ax.legend([l[0] for l in alg_lines], legend, loc='upper right')
    ax.set_xlabel('Iterations')
    ax.set_ylabel('RRSE')
    
    filename = problem_name.replace(" ", "-") + ".pdf"
    plt.savefig(os.path.join(path, filename), bbox_inches='tight')
    plot_filenames.append((problem_name.replace(" ", "-") + ".pdf", problem_name))
    
  texfile = open(os.path.join(path, "tables.tex"),'w')

  problem_names = problem_results.keys()
  alg_names = problem_results[problem_names[0]].keys()
  result_names = problem_results[problem_names[0]][alg_names[0]].keys()
  
  texfile.write('\\documentclass{article}\n')
  texfile.write('\\usepackage{graphicx}\n')
  texfile.write('\\begin{document}\n')  
  for result_name in result_names:
    column_spec = '|l|'    
    for a in alg_names:
      column_spec = column_spec + 'c|'
    texfile.write('\\begin{table}[htp]\n')
    texfile.write('\\centering\n')
    texfile.write('\\begin{tabular}{' + column_spec + '}\n')
    texfile.write('\\hline\n')
    header_row = ''
    for a in alg_names:
      header_row = header_row + ' & ' + a
    texfile.write(header_row + '\\\\\n')
    texfile.write('\\hline\n')
    for p in problem_names:
      row = p
      for a in alg_names:
        row = row + " & " + "{:.3f}".format(problem_results[p][a][result_name].mean())
      texfile.write(row + "\\\\\n")
    texfile.write("\\hline")
    texfile.write('\\end{tabular}\n')
    texfile.write('\\caption{Results for ' + result_name + '}\n')
    texfile.write('\\end{table}\n')

  for f in plot_filenames:
    texfile.write('\\begin{figure}[htp]\n')
    texfile.write('\\centering\n')
    texfile.write('  \\includegraphics[width=0.7\\textwidth]{' + f[0] + '}\n')
    texfile.write('\\caption{Results for problem ' + f[1] + '}\n')
    texfile.write('\\end{figure}\n')
  texfile.write('\\end{document}\n')
  texfile.close()

#%%
if __name__ == "__main__":
  main()