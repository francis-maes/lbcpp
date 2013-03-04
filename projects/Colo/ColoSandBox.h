/*-----------------------------------------.---------------------------------.
| Filename: ColoSandBox.h                  | Compiler Optimization Level     |
| Author  : Francis Maes                   | SandBox                         |
| Started : 14/09/2012 18:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_COLO_SANDBOX_H_
# define LBCPP_COLO_SANDBOX_H_

# include "ColoProblem.h"
# include "SurrogateBasedColoSolver.h"
# include "ColoVariableEncoder.h"
# include <ml/RandomVariable.h>

# include <ml/ExpressionSampler.h>
# include <ml/SplittingCriterion.h>
# include <ml/SelectionCriterion.h>

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class ColoSandBox : public WorkUnit
{
public:
  ColoSandBox() : numEvaluations(1000), numRuns(10), verbose(false) {}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);

    ColoProblemPtr problem = new ColoProblem(context, javaDirectory, modelDirectory);
    if (!problem->getNumObjectives())
      return new Boolean(false);

    ParetoFrontPtr referenceFront = makeReferenceParetoFront(context, problem);
    context.resultCallback("referenceFront", referenceFront);
    context.informationCallback("Reference HyperVolume: " + string(referenceFront->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness())));

    std::vector<SolverInfo> infos;
    context.enterScope("Running");

    infos.push_back(runSolver(context, problem, randomSolver(new ColoSampler(), numEvaluations), "random"));

#if 0
    //infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler(), 100, 10, numEvaluations / 100, false), "eda1-100-10"));
    //infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler(), 100, 30, numEvaluations / 100, false), "eda1-100-30"));
    //infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler(), 100, 10, numEvaluations / 100, true), "eda1-100-10-el"));
    infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler(), 100, 30, numEvaluations / 100, true), "eda1-100-30-el"));

    //infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler2(), 100, 10, numEvaluations / 100, false), "eda2-100-10"));
    //infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler2(), 100, 30, numEvaluations / 100, false), "eda2-100-30"));
    //infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler2(), 100, 10, numEvaluations / 100, true), "eda2-100-10-el"));
    infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler2(), 100, 30, numEvaluations / 100, true), "eda2-100-30-el"));
    
    infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler(), 1000, 100, numEvaluations / 1000, false), "eda1-1000-100"));
    infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler2(), 1000, 100, numEvaluations / 1000, false), "eda2-1000-100"));
    /*infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler2(), 1000, 300, numEvaluations / 1000, false), "eda2-1000-300"));
    infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler2(), 1000, 100, numEvaluations / 1000, true), "eda2-1000-100-el"));
    infos.push_back(runSolver(context, problem, crossEntropySolver(new ColoSampler2(), 1000, 300, numEvaluations / 1000, true), "eda2-1000-300-el"));*/
#endif // 0

    
    SamplerPtr expressionVectorSampler = subsetVectorSampler(scalarExpressionVectorSampler(), 33);
    SolverPtr conditionLearner = randomSplitConditionLearner(expressionVectorSampler);
    SolverPtr surrogateLearner = treeLearner(vectorStddevReductionSplittingCriterion(), conditionLearner);
    surrogateLearner = simpleEnsembleLearner(surrogateLearner, 10);
    SolverPtr surrogateSolver = crossEntropySolver(new ColoSampler(), 100, 10, 10, true);
    if (verbose)
      surrogateSolver->setVerbosity(verbosityDetailed);
    IterativeSolverPtr surrogateBasedSolver = new SurrogateBasedMOSolver(new ColoSampler(), 100, surrogateLearner, surrogateSolver, coloVariableEncoder(), greedySelectionCriterion(), numEvaluations);
    infos.push_back(runSolver(context, problem, surrogateBasedSolver, "surrogate-100-10-eda1-100-10-10-el"));

    surrogateSolver = crossEntropySolver(new ColoSampler2(), 100, 10, 10, true);
    if (verbose)
      surrogateSolver->setVerbosity(verbosityDetailed);
    surrogateBasedSolver = new SurrogateBasedMOSolver(new ColoSampler(), 100, surrogateLearner, surrogateSolver, coloVariableEncoder(), greedySelectionCriterion(), numEvaluations);
    infos.push_back(runSolver(context, problem, surrogateBasedSolver, "surrogate-100-100-eda2-100-10-10-el"));
    
    context.leaveScope();
    
    context.enterScope("Results");
    displayResults(context, infos);
    context.leaveScope();

    /*runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler(), 2, 20, 10, 5, false));
    runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler(), 2, 20, 10, 5, true));
    runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler2(), 2, 20, 10, 5, false));
    runOptimizer(context, problem, new NestedCrossEntropySolver(new ColoSampler2(), 2, 20, 10, 5, true));*/
    return new Boolean(true);
  }

#if 0
  void runOptimizer(ExecutionContext& context, ProblemPtr problem, SolverPtr optimizer)
  {
    context.enterScope(optimizer->toShortString());

    DVectorPtr cpuTimes = new DVector();
    DVectorPtr hyperVolumes = new DVector();
    size_t evaluationPeriod = numEvaluations > 250 ? numEvaluations / 250 : 1;
    ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
    SolverCallbackPtr callback = compositeSolverCallback(
      fillParetoFrontSolverCallback(front),
      hyperVolumeEvaluatorSolverCallback(evaluationPeriod, cpuTimes, hyperVolumes),
      maxEvaluationsSolverCallback(numEvaluations));

    //optimizer->setVerbosity(verbosityProgressAndResult);
    optimizer->setVerbosity(verbosityDetailed);

    optimizer->solve(context, problem, callback);

    for (size_t i = 0; i < hyperVolumes->getNumElements(); ++i)
    {
      size_t numEvaluations = (i + 1) * evaluationPeriod;
      context.enterScope(string((int)numEvaluations));
      context.resultCallback("numEvaluations", numEvaluations);
      context.resultCallback("log(numEvaluations)", log10((double)numEvaluations));
      context.resultCallback("hyperVolume", hyperVolumes->get(i));
      context.resultCallback("cpuTime", cpuTimes->get(i));
      context.leaveScope();
    }

    double hyperVolume = front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness());
    context.informationCallback("HyperVolume: " + string(hyperVolume));

    context.leaveScope(hyperVolume);
  }
#endif // 0

protected:
  friend class ColoSandBoxClass;

  juce::File javaDirectory;
  juce::File modelDirectory;
  size_t numEvaluations;
  size_t numRuns;
  juce::File outputDirectory;
  bool verbose;

  enum {evaluationPeriod = 100};

  struct SolverInfo
  {
    string name;
    std::vector<double> cpuTimes;
    std::vector<double> hyperVolumes;
  };

  SolverInfo runSolver(ExecutionContext& context, ProblemPtr problem, SolverPtr solver, const string& description)
  {
    OutputStream* hyperVolumesOutput = NULL;

    if (outputDirectory != juce::File::nonexistent)
    {
      outputDirectory.createDirectory();
      juce::File hyperVolumesFile = outputDirectory.getChildFile(description + ".txt");
      if (hyperVolumesFile.exists()) hyperVolumesFile.deleteFile();
      hyperVolumesOutput = hyperVolumesFile.createOutputStream();
    }

    context.enterScope(description);
    context.resultCallback("solver", solver);
	  std::vector<SolverInfo> runInfos(numRuns);
    for (size_t i = 0; i < numRuns; ++i)
    {
      SolverInfo& info = runInfos[i];
      if (verbose)
        context.enterScope("Run " + string((int)i));
      double res = runSolverOnce(context, problem, solver, info);
      writeToOutput(hyperVolumesOutput, info.hyperVolumes);
      if (verbose)
        context.leaveScope(res);
      context.progressCallback(new ProgressionState(i+1, numRuns, "Runs"));
    }


    if (hyperVolumesOutput)
      deleteAndZero(hyperVolumesOutput);

    SolverInfo res;
    res.name = description;
    res.hyperVolumes.resize(runInfos[0].hyperVolumes.size());
    mergeResults(res.hyperVolumes, runInfos, false);
    context.leaveScope(res.hyperVolumes.back());
    return res;
  }
  
  void writeToOutput(OutputStream* ostr, const std::vector<double>& values)
  {
    if (ostr)
    {
      string line;
      for (size_t i = 0; i < values.size(); ++i)
        line += string(values[i]) + " ";
      line += "\n";
      *ostr << line;
      ostr->flush();
    }
  }

  double runSolverOnce(ExecutionContext& context, ProblemPtr problem, SolverPtr solver, SolverInfo& info)
  {
    DVectorPtr cpuTimes = new DVector();
    DVectorPtr hyperVolumes = new DVector();
    ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
    SolverCallbackPtr callback = compositeSolverCallback(
      fillParetoFrontSolverCallback(front),
      hyperVolumeEvaluatorSolverCallback(evaluationPeriod, cpuTimes, hyperVolumes),
      maxEvaluationsSolverCallback(numEvaluations));

    solver->setVerbosity(verbose ? verbosityDetailed : verbosityQuiet);
    solver->solve(context, problem, callback);
    info.cpuTimes = cpuTimes->getNativeVector();
    info.hyperVolumes = hyperVolumes->getNativeVector();
    return front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness());
  }

  void mergeResults(std::vector<double>& res, const std::vector<SolverInfo>& infos, bool inFunctionOfCpuTime)
  {
    for (size_t i = 0; i < res.size(); ++i)
    {
      ScalarVariableMean mean;
      for (size_t j = 0; j < infos.size(); ++j)
        mean.push(infos[j].hyperVolumes[i]);
      res[i] = mean.getMean();
    }
  }

  void displayResults(ExecutionContext& context, const std::vector<SolverInfo>& infos)
  {
    for (size_t i = 0; i < infos[0].hyperVolumes.size(); ++i)
    {
      size_t numEvaluations = (i + 1) * evaluationPeriod;
      context.enterScope(string((int)numEvaluations));
      context.resultCallback("numEvaluations", numEvaluations);
      for (size_t j = 0; j < infos.size(); ++j)
        context.resultCallback(infos[j].name, infos[j].hyperVolumes[i]);
      context.leaveScope();
    }
  }

  ParetoFrontPtr makeReferenceParetoFront(ExecutionContext& context, ProblemPtr problem)
  {
    static const char* sequences[] = {
      "32-12-31-32-26",
		  "32-12-31-32-16",
		  "9-29-31-31-32-19-12-16",
		  "32-12-2-9-29-31-31-32-19-12-16",
		  "32-12-2-9-29-12-16-9-23-32-12-31-32-26",
		  "32-12-2-9-29-12-27-31-32-31-32-32-27-19-12-27-3-2-16-29-12-19-16-19-23-26",
		  "13-12-31-32-16",
		  "13-19-12-33-27-12-16",
		  "13-19-12-32-12-2-9-29-31-31-32-19-12-16",
		  "20-29-12-19-16-19-26-13-16-30",
		  "31-2-9-29-12-27-13-31-31-32-19-12-16",
		  "13-31-31-32-19-12-12-2-9-13-16-9-11-23-12-2-9-16-19-26-13-16-9-11-2-9-29-31-31-32-19-12-16",
		  "12-2-12-31-29-8-2-29-13-16-9-11-23-30",
		  "13-31-31-32-19-12-12-2-9-13-16-9-11-23-12-2-9-16-19-26-13-16-9-11-23-12-2-9-29-12-27-24-16",
		  "32-12-2-9-13-16-9-11-23-12-2-9-29-12-27-24-16",
		  "32-27-19-12-29-8-2-29-13-16-9-11-23-30",
		  "12-2-12-20-12-27-31-32-19-12-26-13-16-9-11-23-29-16-29-12-19-16-19-23-26",
		  "13-20-12-27-31-32-32-27-19-12-29-8-2-29-13-16-9-11-23-30",
		  "20-29-12-27-31-32-32-27-19-12-29-8-2-29-13-16-9-11-23-30",
		  "28-27-3-2-16-28-31-32-31-32-32-27-19-12-31-32-19-12-16",
		  "32-12-2-9-29-12-27-31-32-28-27-3-2-16-28-26",
		  "32-12-2-9-29-12-27-31-32-28-27-3-2-16-28-24-16",
		  "32-12-2-9-29-12-27-31-32-28-27-2-9-29-31-31-32-19-12-16",
		  "12-12-28-24-3-2-16-9-23-26",
		  "12-12-28-24-28-27-3-2-16-9-23-26",
		  "32-12-2-9-29-12-28-24-28-27-3-2-16-29-12-19-16-19-23-26",
		  "13-19-12-28-24-28-27-3-2-16-28-31-32-31-32-32-27-19-12-31-32-19-12-16",
		  "13-9-29-12-27-31-32-28-27-3-2-16-28-24-16",
		  "13-19-12-28-24-13-19-12-28-24-28-27-3-9-12-28-12-31-32-16",
		  "13-19-12-28-24-13-12-31-32-19-12-28-32-12-3-12-2-9-29-16",
		  "13-19-12-28-24-28-27-3-2-16-29-12-19-16-19-23-26",
		  "13-19-12-28-24-13-12-31-32-19-12-28-32-12-2-9-29-12-16-9-23-26",
		  "32-19-12-28-24-13-12-31-32-19-12-28-32-12-2-9-29-12-16-9-23-26",
		  "13-20-12-27-31-32-19-12-12-12-28-24-28-27-3-2-16-9-23-26",
		  "12-28-27-3-2-16-24-28-27-3-12-28-24-13-12-31-32-19-12-28-32-12-2-9-29-12-16-9-23-26",
		  "12-2-12-20-12-27-31-32-19-13-19-12-28-24-28-27-3-2-16-29-12-19-9-23-26",
		  "12-28-27-3-2-16-24-28-27-3-27-28-27-3-2-16-9-19-4-12-26-13-12-28-32-12-2-9-29-12-16-9-23-26",
		  "32-27-19-12-29-8-2-29-13-16-9-28-20-28-31-12-26-13-16-11-23-2-28-27-19-12-28-31-14-16",
		  "12-2-12-20-12-27-31-32-19-13-19-12-28-24-28-31-32-19-12-12-2-9-13-16-9-11-23-12-2-9-16-19-26-13-16-9-11-2-9-29-31-31-32-19-12-16",
		  "12-2-12-20-12-27-31-32-19-13-19-12-28-24-28-27-3-2-16-29-12-19-16-19-26-13-16-9-12-28-27-3-2-16-28-24-28-27-3-2-16-3-14-16-9-11-9-23-26",
		  "12-2-12-31-12-28-14-10-28-31-14-16",
		  "12-27-31-12-28-14-10-28-31-14-16",
		  "12-27-31-12-28-14-10-29-8-2-23-26",
		  "32-12-28-24-28-27-3-10-29-8-2-23-26",
		  "32-12-2-9-27-3-9-12-28-14-10-23-26",
		  "32-12-2-27-3-2-16-28-24-28-27-3-10-29-8-2-23-26",
		  "32-12-2-9-29-12-27-32-12-2-9-29-16-28-24-28-27-3-2-16-3-14-10-29-8-2-23-26",
		  "32-12-2-9-27-3-9-12-28-14-10-14-20-14-10-29-8-2-23-26",
		  "13-19-12-28-24-13-19-12-28-24-28-16-3-14-10-29-8-2-23-26",
		  "13-19-12-28-24-12-19-16-19-26-13-16-9-12-28-27-3-2-16-28-24-28-27-3-2-16-3-14-10-29-8-2-23-26",
		  "13-19-12-28-24-13-19-12-28-24-28-27-3-9-12-28-14-10-14-20-14-10-29-8-2-23-26",
		  "13-19-12-28-24-13-19-12-28-24-28-27-3-9-12-28-14-10-12-2-9-29-12-16-9-23-26",
		  "13-19-12-28-24-13-12-31-32-19-12-28-32-12-2-9-29-12-16-9-23-19-12-28-24-28-27-3-9-12-28-14-10-14-20-14-10-29-8-2-23-26",
		  "13-19-12-28-24-13-19-12-28-24-28-27-3-9-12-7-16-3-14-10-29-8-7-16-3-14-20-14-10-29-8-2-23-26",
		  "12-28-27-3-2-16-24-28-27-3-2-16-28-24-28-27-3-2-16-24-13-19-12-28-24-28-27-3-9-12-7-16-3-14-10-29-8-7-16-3-14-20-14-10-29-8-2-23-26",
		  "12-28-27-3-2-16-24-28-27-3-27-28-27-3-2-16-9-19-4-12-26-13-16-3-14-10-29-8-2-29-13-16-9-31-20-21-5-12-20-14-10-29-8-2-23-26",
		  "32-12-2-9-27-27-3-27-28-27-3-2-16-9-19-4-12-26-13-16-3-14-10-29-8-2-29-13-16-9-31-20-21-5-12-20-14-10-29-8-2-23-26",
		  "12-28-27-3-2-16-24-28-27-3-27-28-27-3-2-16-9-19-4-12-13-16-9-31-20-21-5-12-20-31-20-27-18-16-19-26-13-10-29-8-2-23-26",
		  "13-19-12-28-24-13-12-31-32-19-12-28-32-12-3-10-29-8-2-23-8-2-29-13-16-9-31-20-21-5-12-20-31-20-27-18-16-19-26-13-10-29-8-2-23-26",
		  "13-19-12-28-24-28-27-3-9-12-7-16-3-14-10-29-8-2-29-13-16-9-31-20-21-5-12-20-31-20-27-18-16-19-26-13-16-9-11-9-23-26"
    };

    ParetoFrontPtr res = new ParetoFront(problem->getFitnessLimits());
    for (size_t i = 0; i < sizeof (sequences) / sizeof (const char* ); ++i)
    {
      ColoObjectPtr sequence = new ColoObject();
      sequence->loadFromString(context, sequences[i]);
      FitnessPtr fitness = problem->evaluate(context, sequence);
      res->insertSolution(sequence, fitness);
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_COLO_SANDBOX_H_
