#ifndef _LBCPP_KOLMOGOROV_PERFECT_MATCHING_H_
# define _LBCPP_KOLMOGOROV_PERFECT_MATCHING_H_

# include <lbcpp/Core/Function.h>

# include "../BlossomV/PerfectMatching.h"

namespace lbcpp
{

class KolmogorovPerfectMatchingFunction : public SimpleUnaryFunction
{
public:
  KolmogorovPerfectMatchingFunction(double threshold = 0.f)
    : SimpleUnaryFunction(doubleSymmetricMatrixClass(doubleType), doubleSymmetricMatrixClass(doubleType), T("Kolmogorov")),
      threshold(threshold)
    {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    DoubleSymmetricMatrixPtr matrix = input.getObjectAndCast<DoubleSymmetricMatrix>();
    jassert(matrix);

    std::vector<size_t> vertexMap(matrix->getDimension());
    for (size_t i = 0; i < vertexMap.size(); ++i)
      vertexMap[i] = i;

    std::vector<size_t> matching(matrix->getDimension(), (size_t)-1);
    computePerfectMatching(matrix, vertexMap, matching);

    DoubleSymmetricMatrixPtr res = new DoubleSymmetricMatrix(matrix->getElementsType(), matrix->getDimension(), 0.f);
    for (size_t i = 0; i < matching.size(); ++i)
      if (matching[i] != (size_t)-1)
        res->setValue(i, matching[i], matrix->getValue(i, matching[i]));
    return res;
  }

protected:
  double threshold;

private:
  size_t getValidVertices(const DoubleSymmetricMatrixPtr& matrix, const std::vector<size_t>& vertexMap, std::vector<size_t>& result) const
  {
    size_t numEdges = 0;
    for (size_t i = 0; i < vertexMap.size(); ++i)
    {
      bool hasEdge = false;
      for (size_t j = 0; j < vertexMap.size(); ++j)
      {
        if (i == j)
          continue;
        if (matrix->getValue(vertexMap[i], vertexMap[j]) > threshold)
        {
          hasEdge = true;
          ++numEdges;
        }
      }
      if (hasEdge)
        result.push_back(vertexMap[i]);
    }
    jassert(numEdges % 2 == 0);
    return numEdges / 2;
  }


  double computePerfectMatching(const DoubleSymmetricMatrixPtr& matrix, const std::vector<size_t>& verticesMap, std::vector<size_t>& result) const
  {
    std::vector<size_t> validVertices;
    const size_t numEdges = getValidVertices(matrix, verticesMap, validVertices);
    const size_t numVertices = validVertices.size();

    // Base Cases
    if (numVertices <= 1)
      return 0.f;
    if (numVertices == 2)
    {
      result[validVertices[0]] = validVertices[1];
      result[validVertices[1]] = validVertices[0];
      return matrix->getValue(validVertices[0], validVertices[1]);
    }
    if (numVertices % 2 == 0)
      return applyKolmogorovAlgorithm(matrix, validVertices, numEdges, result);

    // General Case
    double bestScore = -DBL_MAX;
    for (size_t i = 0; i < numVertices; ++i)
    {
      // Copy numVertice - 1
      std::vector<size_t> subVerticesMap;
      for (size_t j = 0; j < numVertices; ++j)
        if (j != i)
          subVerticesMap.push_back(validVertices[j]);
      std::vector<size_t> subMatching(matrix->getDimension(), (size_t)-1);
      double score = computePerfectMatching(matrix, subVerticesMap, subMatching);
      jassert(subMatching[validVertices[i]] == (size_t)-1);
      if (score > bestScore)
      {
        bestScore = score;
        result = subMatching;
      }
    }
    return bestScore;
  }

  double applyKolmogorovAlgorithm(const DoubleSymmetricMatrixPtr& matrix, const std::vector<size_t>& verticesMap, size_t numEdges, std::vector<size_t>& result) const
  {
    //std::cout << "Kolmogorov Perfect Matching - #Vertex: " << verticesMap.size() << " - #Edge: " << numEdges << std::endl;
    //jassert(verticesMap.size() % 2 == 0 && numEdges >= verticesMap.size() - 1);
    PerfectMatching matching(verticesMap.size(), verticesMap.size() * (verticesMap.size() - 1) / 2);//numEdges);
    matching.options.verbose = false;
    // Add edges
//std::cout << "applyKolmogorovAlgorithm" << std::endl;
    for (size_t i = 0; i < verticesMap.size(); ++i)
      for (size_t j = i + 1; j < verticesMap.size(); ++j)
      {
        const double value = matrix->getValue(verticesMap[i], verticesMap[j]);
        if (value > threshold || true)
        {
          matching.AddEdge(i, j, -(int)(value * 10000.f));
//std::cout << "Edge: " << i << " " << j << " > " << value << std::endl;
        }
      }
//std::cout << std::endl;
    matching.Solve(true);

    double res = 0.f;
    for (size_t i = 0; i < verticesMap.size(); ++i)
    {
      const size_t j = matching.GetMatch(i);
      jassert(j < verticesMap.size());
      if (matrix->getValue(verticesMap[i], verticesMap[j]) <= threshold)
        continue;
      jassert(result[verticesMap[i]] == (size_t)-1);
      result[verticesMap[i]] = verticesMap[j];
      res += matrix->getValue(verticesMap[i], verticesMap[j]);
    }
    return res / 2.f;
  }
};

};

#endif // !_LBCPP_KOLMOGOROV_PERFECT_MATCHING_H_
