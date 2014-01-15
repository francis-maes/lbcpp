/*-----------------------------------------.---------------------------------.
| Filename: SPEA2Comparator.h              | Compares solutions based on     |
| Author  : Denny Verbeeck                 | SPEA2 fitness assignment        |
| Started : 13/01/2014 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_COMPARATOR_SPEA2_H_
# define ML_COMPARATOR_SPEA2_H_

# include <ml/SolutionComparator.h>
# include <algorithm>

namespace lbcpp
{

class SPEA2Comparator : public SolutionComparator
{
public:
  virtual void initialize(const SolutionContainerPtr& solutions)
  {
    SolutionComparatorPtr dominance = dominanceComparator();
    dominance->initialize(solutions);
    size_t n = solutions->getNumSolutions();
    fitness = std::vector<double>(n, 0.0);

    double* strength = new double[n];
    std::fill_n(strength, n, 0.0);
    
    double* rawFitness = new double[n];
    std::fill_n(rawFitness, n, 0.0);

    double** distance = new double*[n];
    for (size_t i = 0; i < n; ++n)
      distance[n] = new double[n];
    calculateDistanceMatrix(solutions, distance);

    // calculate strength value
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < n; ++j)
        if (dominance->compareSolutions(i, j) == -1)
          strength[i] += 1.0;

    // calculate raw fitness
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < n; ++j)
        if (dominance->compareSolutions(i, j) == 1)
          rawFitness[i] += strength[j];

    // Add the distance to the k-th individual. In the reference paper of SPEA2, 
    // k = sqrt(population.size()), but a value of k = 1 recommended. See
    // http://www.tik.ee.ethz.ch/pisa/selectors/spea2/spea2_documentation.txt
    size_t k = 1;
    double kDistance = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      std::sort(distance[i], distance[i] + n);
      kDistance = 1.0 / (distance[i][k] + 2.0);
      fitness[i] = rawFitness[i] + kDistance;
    }
    
    delete[] strength;
    delete[] rawFitness;
    for (size_t i = 0; i < n; ++n)
      delete[] distance[n];
    delete[] distance;
  }

  virtual int compareSolutions(size_t index1, size_t index2)
  {
    if (fitness[index1] < fitness[index2])
      return -1;
    if (fitness[index1] > fitness[index2])
      return 1;
    return 0;
  }
protected:
  std::vector<double> fitness;

  void calculateDistanceMatrix(const SolutionContainerPtr& solutions, double** matrix)
  {
    for (size_t i = 0; i < solutions->getNumSolutions(); ++i)
      for (size_t j = i; j < solutions->getNumSolutions(); ++j)
      {
        double distance = 0.0;
        DenseDoubleVectorPtr s1 = solutions->getSolution(i).staticCast<DenseDoubleVector>();
        DenseDoubleVectorPtr s2 = solutions->getSolution(j).staticCast<DenseDoubleVector>();
        for (size_t k = 0; k < s1->getNumValues(); ++k)
          distance += (s1->getValue(k) - s2->getValue(k)) * (s1->getValue(k) - s2->getValue(k));
        matrix[i][j] = sqrt(distance);
        matrix[j][i] = matrix[i][j];
      }
  }
};

} /* namespace lbcpp */

#endif //!ML_COMPARATOR_SPEA2_H_