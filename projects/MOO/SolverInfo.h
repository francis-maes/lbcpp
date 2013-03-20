/*-----------------------------------------.---------------------------------.
 | Filename: SolverInfo.h                   | Solver Info                     |
 | Author  : Denny Verbeeck                 |                                 |
 | Started : 14/03/2013 14:17               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef SOLVER_INFO_H_
# define SOLVER_INFO_H_

# include <ml/Solver.h>
# include <ml/SolutionContainer.h>

namespace lbcpp
{
  
struct ResultCurve
{
  IVectorPtr evaluations;
  DVectorPtr cpuTimes;
  DVectorPtr scores;
  
  void initialize()
  {
    evaluations = new IVector();
    cpuTimes = new DVector();
    scores = new DVector();
  }
};

struct SolverInfo
{
  string name;
  ResultCurve inFunctionOfEvaluations;
  ResultCurve inFunctionOfCpuTime;
  
  void initialize(string name)
  {
    this->name = name;
    inFunctionOfEvaluations.initialize();
    inFunctionOfCpuTime.initialize();
  }
  
  static void displayResults(ExecutionContext& context, const std::vector<SolverInfo>& infos)
  {
    size_t maxLengthEvaluations = 0;
    size_t maxLengthCpuTimes = 0;
    IVectorPtr longestEvaluationsVector;
    DVectorPtr longestCpuTimesVector;
    for (size_t i = 0; i < infos.size(); ++i)
    {
      if (infos[i].inFunctionOfEvaluations.scores->getNumElements() > maxLengthEvaluations)
      {
        maxLengthEvaluations = infos[i].inFunctionOfEvaluations.evaluations->getNumElements();
        longestEvaluationsVector = infos[i].inFunctionOfEvaluations.evaluations;
      }
      if (infos[i].inFunctionOfCpuTime.scores->getNumElements() > maxLengthCpuTimes)
      {
        maxLengthCpuTimes = infos[i].inFunctionOfCpuTime.cpuTimes->getNumElements();
        longestCpuTimesVector = infos[i].inFunctionOfCpuTime.cpuTimes;
      }
    }
    
    context.enterScope("score(evaluations)");
    for (size_t i = 0; i < maxLengthEvaluations; ++i)
    {
      context.enterScope(string(longestEvaluationsVector->get(i)));
      context.resultCallback("numEvaluations", longestEvaluationsVector->get(i));
      if (longestEvaluationsVector->get(i))
        context.resultCallback("log(numEvaluations)", log10((double)longestEvaluationsVector->get(i)));
      for (size_t j = 0; j < infos.size(); ++j)
        if (i < infos[j].inFunctionOfEvaluations.scores->getNumElements() &&
            infos[j].inFunctionOfEvaluations.scores->get(i) != DVector::missingValue)
          context.resultCallback(infos[j].name, infos[j].inFunctionOfEvaluations.scores->get(i));
      context.leaveScope();
    }
    context.leaveScope();
    
    context.enterScope("cpuTime(evaluations)");
    for (size_t i = 0; i < maxLengthEvaluations; ++i)
    {
      context.enterScope(string(longestEvaluationsVector->get(i)));
      context.resultCallback("numEvaluations", longestEvaluationsVector->get(i));
      if (longestEvaluationsVector->get(i))
        context.resultCallback("log(numEvaluations)", log10((double)longestEvaluationsVector->get(i)));
      for (size_t j = 0; j < infos.size(); ++j)
        if (i < infos[j].inFunctionOfEvaluations.cpuTimes->getNumElements() &&
            infos[j].inFunctionOfEvaluations.cpuTimes->get(i) != DVector::missingValue)
          context.resultCallback(infos[j].name, infos[j].inFunctionOfEvaluations.cpuTimes->get(i));
      context.leaveScope();
    }
    context.leaveScope();
    
    context.enterScope("score(cpuTime)");
    for (size_t i = 0; i < maxLengthCpuTimes; ++i)
    {
      context.enterScope(string(longestCpuTimesVector->get(i)));
      context.resultCallback("cpuTime", longestCpuTimesVector->get(i));
      if (longestCpuTimesVector->get(i))
        context.resultCallback("log(cpuTime)", log10((double)longestCpuTimesVector->get(i)));
      for (size_t j = 0; j < infos.size(); ++j)
        if (i < infos[j].inFunctionOfCpuTime.scores->getNumElements() &&
            infos[j].inFunctionOfCpuTime.scores->get(i) != DVector::missingValue)
          context.resultCallback(infos[j].name, infos[j].inFunctionOfCpuTime.scores->get(i));
      context.leaveScope();
    }
    context.leaveScope();
      
    context.enterScope("evaluations(cpuTime)");
    for (size_t i = 0; i < maxLengthCpuTimes; ++i)
    {
      context.enterScope(string(longestCpuTimesVector->get(i)));
      context.resultCallback("cpuTime", longestCpuTimesVector->get(i));
      if (longestCpuTimesVector->get(i))
        context.resultCallback("log(cpuTime)", log10((double)longestCpuTimesVector->get(i)));
      for (size_t j = 0; j < infos.size(); ++j)
        if (i < infos[j].inFunctionOfCpuTime.evaluations->getNumElements() &&
            infos[j].inFunctionOfCpuTime.evaluations->get(i) != DVector::missingValue)
          context.resultCallback(infos[j].name, infos[j].inFunctionOfCpuTime.evaluations->get(i));
      context.leaveScope();
    }
    context.leaveScope();
  }
};

class SolverSettings
{ 
public:
  SolverSettings() : numRuns(0), bestFitness(NULL), numEvaluations(0) {}
  SolverSettings(SolverPtr solver, size_t numRuns, size_t numEvaluations, 
                 double evaluationPeriod, double evaluationPeriodFactor, size_t verbosity,
                 size_t optimizerVerbosity, const string& description = string::empty, FitnessPtr* bestFitness = NULL)
    : solver(solver), numRuns(numRuns), 
      bestFitness(bestFitness), description(description.isEmpty() ? solver->toShortString() : description),
      numEvaluations(numEvaluations), evaluationPeriod(evaluationPeriod), evaluationPeriodFactor(evaluationPeriodFactor),
      verbosity((SolverVerbosity)verbosity), optimizerVerbosity((SolverVerbosity)optimizerVerbosity) {}
  
  SolverInfo runSolver(ExecutionContext& context, ProblemPtr problem)
  {
    context.enterScope(description);
    context.resultCallback("solver", solver);
    std::vector<SolverInfo> runInfos(numRuns);
    for (size_t i = 0; i < numRuns; ++i)
    {
      SolverInfo& info = runInfos[i];
      context.enterScope("Run " + string((int)i));
      double res = runSolverOnce(context, problem, info);
      context.leaveScope(res);
      context.progressCallback(new ProgressionState(i+1, numRuns, "Runs"));
    }
    
    SolverInfo res;
    res.initialize(description);
    double mean = mergeResults(res, runInfos);
    
    if (verbosity >= verbosityDetailed)
    {
      context.enterScope("curveWithFixedEvaluations");
      for (size_t i = 0; i < res.inFunctionOfEvaluations.evaluations->getNumElements(); ++i)
      {
        context.enterScope(string(res.inFunctionOfEvaluations.evaluations->get(i)));
        context.resultCallback("numEvaluations", res.inFunctionOfEvaluations.evaluations->get(i));
        context.resultCallback("score", res.inFunctionOfEvaluations.scores->get(i));
        if (res.inFunctionOfEvaluations.cpuTimes->get(i) != DVector::missingValue)
          context.resultCallback("cpuTime", res.inFunctionOfEvaluations.cpuTimes->get(i));
        context.leaveScope();
      }
      context.leaveScope();
      context.enterScope("curveWithFixedCpuTime");
      for (size_t i = 0; i < res.inFunctionOfCpuTime.cpuTimes->getNumElements(); ++i)
      {
        context.enterScope(string(res.inFunctionOfCpuTime.cpuTimes->get(i)));
        context.resultCallback("cpuTime", res.inFunctionOfCpuTime.cpuTimes->get(i));
        if (res.inFunctionOfCpuTime.scores->get(i) != DVector::missingValue)
          context.resultCallback("score", res.inFunctionOfCpuTime.scores->get(i));
        context.resultCallback("numEvaluations", res.inFunctionOfCpuTime.evaluations->get(i));
        context.leaveScope();
      }
      context.leaveScope();
    }
    
    context.leaveScope(mean);
    return res;
  }
  
  double runSolverOnce(ExecutionContext& context, ProblemPtr problem, SolverInfo& info)
  {
    problem->reinitialize(context);
    
    FitnessPtr defaultBestFitness;
    if (bestFitness)
      *bestFitness = FitnessPtr();
    else
      bestFitness = &defaultBestFitness;
    
    IVectorPtr evaluations = new IVector();
    DVectorPtr cpuTimes = new DVector();
    DVectorPtr scores = new DVector();
    SolverEvaluatorPtr evaluator = singleObjectiveSolverEvaluator(*bestFitness);
    
    SolverCallbackPtr callback1 = compositeSolverCallback(storeBestFitnessSolverCallback(*bestFitness), 
                                                          evaluationPeriodEvaluatorSolverCallback(evaluator, evaluations, cpuTimes, scores, 2));
    
    info.inFunctionOfEvaluations.evaluations = evaluations;
    info.inFunctionOfEvaluations.cpuTimes = cpuTimes;
    info.inFunctionOfEvaluations.scores = scores;
    
    evaluations = new IVector();
    cpuTimes = new DVector();
    scores = new DVector();
    SolverCallbackPtr callback2 = compositeSolverCallback(logTimePeriodEvaluatorSolverCallback(evaluator, evaluations, cpuTimes, scores, evaluationPeriod, evaluationPeriodFactor), maxEvaluationsSolverCallback(numEvaluations));
    
    info.inFunctionOfCpuTime.evaluations = evaluations;
    info.inFunctionOfCpuTime.cpuTimes = cpuTimes;
    info.inFunctionOfCpuTime.scores = scores;
    
    solver->setVerbosity((SolverVerbosity)optimizerVerbosity);
    solver->solve(context, problem, compositeSolverCallback(callback1, callback2));
    
    context.resultCallback("optimizer", solver);
    if (verbosity >= verbosityDetailed)
    {
      context.enterScope("curveWithFixedEvaluations");
      for (size_t i = 0; i < info.inFunctionOfEvaluations.evaluations->getNumElements(); ++i)
      {
        context.enterScope(string(info.inFunctionOfEvaluations.evaluations->get(i)));
        context.resultCallback("numEvaluations", info.inFunctionOfEvaluations.evaluations->get(i));
        context.resultCallback("score", info.inFunctionOfEvaluations.scores->get(i));
        if (info.inFunctionOfEvaluations.cpuTimes->get(i) != DVector::missingValue)
          context.resultCallback("cpuTime", info.inFunctionOfEvaluations.cpuTimes->get(i));
        context.leaveScope();
      }
      context.leaveScope();
      context.enterScope("curveWithFixedCpuTime");
      for (size_t i = 0; i < info.inFunctionOfCpuTime.cpuTimes->getNumElements(); ++i)
      {
        context.enterScope(string(info.inFunctionOfCpuTime.cpuTimes->get(i)));
        context.resultCallback("cpuTime", info.inFunctionOfCpuTime.cpuTimes->get(i));
        if (info.inFunctionOfCpuTime.scores->get(i) != DVector::missingValue)
          context.resultCallback("score", info.inFunctionOfCpuTime.scores->get(i));
        context.resultCallback("numEvaluations", info.inFunctionOfCpuTime.evaluations->get(i));
        context.leaveScope();
      }
      context.leaveScope();
    }
    return (*bestFitness)->toDouble();
  }
  
  /**
   *  Merges multiple results into a curve of the average of the results.
   *  \return The mean of the best found values for each of the curves
   */
  double mergeResults(SolverInfo& res, const std::vector<SolverInfo>& infos)
  {
    std::vector<double> best(infos.size(), DBL_MAX);
    size_t maxLengthEvaluations = 0;
    size_t maxLengthCpuTimes = 0;
    for (size_t i = 0; i < infos.size(); ++i)
    {
      maxLengthEvaluations = (infos[i].inFunctionOfEvaluations.evaluations->getNumElements() > maxLengthEvaluations ? infos[i].inFunctionOfEvaluations.evaluations->getNumElements() : maxLengthEvaluations);
      maxLengthCpuTimes = (infos[i].inFunctionOfCpuTime.cpuTimes->getNumElements() > maxLengthCpuTimes ? infos[i].inFunctionOfCpuTime.cpuTimes->getNumElements() : maxLengthCpuTimes);
    }
    for (size_t i = 0; i < maxLengthEvaluations; ++i)
    {
      ScalarVariableMean meanScore;
      ScalarVariableMean meanCpuTime;
      size_t evaluations = 0;
      for (size_t j = 0; j < infos.size(); ++j)
      {
        ResultCurve curve = infos[j].inFunctionOfEvaluations;
        if (i < curve.scores->getNumElements())
        {
          meanScore.push(curve.scores->get(i));
          meanCpuTime.push(curve.cpuTimes->get(i));
          if (evaluations)
            jassert(evaluations == curve.evaluations->get(i)); // if this fails there is smth wrong in EvaluationPeriodEvaluatorSolverCallback, results not aligned on nb of evaluations
          evaluations = curve.evaluations->get(i);
          best[j] = (best[j] < curve.scores->get(i) ? best[j] : curve.scores->get(i));
        }
        else
        {
          meanScore.push(curve.scores->get(curve.scores->getNumElements() - 1));
          meanCpuTime.push(curve.cpuTimes->get(curve.cpuTimes->getNumElements() - 1));
        }
      }
      res.inFunctionOfEvaluations.evaluations->append(evaluations);
      res.inFunctionOfEvaluations.scores->append(meanScore.getCount() ? meanScore.getMean() : DVector::missingValue);
      res.inFunctionOfEvaluations.cpuTimes->append(meanCpuTime.getCount() ? meanCpuTime.getMean() : DVector::missingValue);
    }
    for (size_t i = 0; i < maxLengthCpuTimes; ++i)
    {
      ScalarVariableMean meanScore;
      ScalarVariableMean meanEvaluations;
      double cpuTime = 0.0;
      for (size_t j = 0; j < infos.size(); ++j)
      {
        ResultCurve curve = infos[j].inFunctionOfCpuTime;
        if (i < curve.scores->getNumElements())
        {
          if (curve.scores->get(i) != DVector::missingValue)
            meanScore.push(curve.scores->get(i));
          if (curve.evaluations->get(i) != DVector::missingValue)
            meanEvaluations.push(curve.evaluations->get(i));
          if (cpuTime != 0.0)
            jassert(fabs(cpuTime - curve.cpuTimes->get(i)) < 1e-5); // if this fails there is smth wrong in TimePeriodEvaluatorSolverCallback, results not aligned on cputime
          cpuTime = curve.cpuTimes->get(i);
        }
        else
        {
          meanScore.push(curve.scores->get(curve.scores->getNumElements() - 1));
          meanEvaluations.push(curve.evaluations->get(curve.evaluations->getNumElements() - 1));
        }
      }
      res.inFunctionOfCpuTime.evaluations->append(meanEvaluations.getCount() ? meanEvaluations.getMean() : DVector::missingValue);
      res.inFunctionOfCpuTime.scores->append(meanScore.getCount() ? meanScore.getMean() : DVector::missingValue);
      res.inFunctionOfCpuTime.cpuTimes->append(cpuTime);
    }
    double meanOfBests = 0;
    for (size_t j = 0; j < infos.size(); ++j)
      meanOfBests += best[j];
    meanOfBests /= infos.size();
    return meanOfBests;
  }
  
protected:
  SolverPtr solver;
  size_t numRuns;
  FitnessPtr* bestFitness;
  string description;
  size_t numEvaluations;
  double evaluationPeriod;
  double evaluationPeriodFactor;
  SolverVerbosity verbosity;
  SolverVerbosity optimizerVerbosity;
};

}; /* namespace lbcpp */

#endif // !SOLVER_INFO_H_
