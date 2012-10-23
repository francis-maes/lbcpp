/*-----------------------------------------.---------------------------------.
| Filename: BeamNRPASolver.h               | Beam Nested Rollout Policy      |
| Author  : Francis Maes                   | Adaptation                      |
| Started : 12/09/2012 14:45               |  Cazenave and Teytaud, 2012     |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLVER_BEAM_NRPA_H_
# define LBCPP_ML_SOLVER_BEAM_NRPA_H_

# include "NRPASolver.h"
# include <algorithm>

namespace lbcpp
{

class BeamNRPASolver : public NRPASolver
{
public:
  BeamNRPASolver(SamplerPtr sampler, size_t level, size_t numIterationsPerLevel, size_t beamSizeAtFirstLevel, size_t beamSizeAtHigherLevels)
    : NRPASolver(sampler, level, numIterationsPerLevel), beamSizeAtFirstLevel(beamSizeAtFirstLevel), beamSizeAtHigherLevels(beamSizeAtHigherLevels) {}
  BeamNRPASolver() : beamSizeAtFirstLevel(0), beamSizeAtHigherLevels(0) {}
  
  virtual void optimize(ExecutionContext& context)
    {solveRecursively(context, sampler, level);}

protected:
  friend class BeamNRPASolverClass;

  size_t beamSizeAtFirstLevel;
  size_t beamSizeAtHigherLevels;
  
  struct Element
  {
    Element(const ObjectPtr& solution, const FitnessPtr& fitness, const SamplerPtr& sampler)
      : solution(solution), fitness(fitness), sampler(sampler) {}
    Element() {}

    ObjectPtr solution;
    FitnessPtr fitness;
    SamplerPtr sampler;

    bool operator <(const Element& element) const
    {
      if (fitness->strictlyDominates(element.fitness))
        return true;
      if (element.fitness->strictlyDominates(fitness))
        return false;
      return solution.get() < element.solution.get();
    }
  };

  typedef std::vector<Element> Beam;

  Beam solveRecursively(ExecutionContext& context, SamplerPtr sampler, size_t level)
  {
    if (problem->shouldStop())
      return Beam();
      
    if (level == 0)
    {
      ObjectPtr solution = sampler->sample(context);
      FitnessPtr fitness = evaluate(context, solution);
      return Beam(1, Element(solution, fitness, sampler));
    }
    else
    {
      Beam beam(1, Element(ObjectPtr(), problem->getFitnessLimits()->getWorstPossibleFitness(true), sampler));
      size_t beamSize = level > 1 ? beamSizeAtHigherLevels : beamSizeAtFirstLevel;

      for (size_t i = 0; i < numIterationsPerLevel; ++i)
      {
        Beam newBeam;
        for (size_t j = 0; j < beam.size(); ++j)
        {
          Element& element = beam[j];
          newBeam.push_back(element);
          Beam beam1 = solveRecursively(context, element.sampler, level - 1);
          for (size_t k = 0; k < beam1.size(); ++k)
            if (beam1[k].solution) // may be null if "problem->shouldStop()"
            {
              SamplerPtr sampler = element.sampler->cloneAndCast<Sampler>();
              sampler->reinforce(context, beam1[k].solution, 1.0);
              newBeam.push_back(Element(beam1[k].solution, beam1[k].fitness, sampler));
            }
        }
        
        beam = beamSize < newBeam.size() ? selectNBests(newBeam, beamSize) : newBeam;
        if (problem->shouldStop())
          break;
      }
      return beam;
    }
  }

  Beam selectNBests(const Beam& beam, size_t count) const
  {
    jassert(count <= beam.size());
    Beam res = beam;
    std::sort(res.begin(), res.end());
    res.resize(count);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLVER_BEAM_NRPA_H_
