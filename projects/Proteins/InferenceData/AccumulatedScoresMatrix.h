/*-----------------------------------------.---------------------------------.
| Filename: AccumulatedScoresMatrix.h      | Accumulated Scores Sequence     |
| Author  : Francis Maes                   |                                 |
| Started : 16/04/2010 14:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_DATA_ACCUMULATED_SCORES_MATRIX_H_
# define LBCPP_INFERENCE_DATA_ACCUMULATED_SCORES_MATRIX_H_

namespace lbcpp
{

class AccumulatedScoresMatrix : public Object
{
public:
  AccumulatedScoresMatrix(FeatureDictionaryPtr dictionary, size_t length)
    : dictionary(dictionary), accumulators(length) {}

  const std::vector<double>& getAccumulatedScores(size_t index) const
    {jassert(index < accumulators.size()); return accumulators[index];}

  std::vector<double>& getAccumulatedScores(size_t index)
    {jassert(index < accumulators.size()); return accumulators[index];}

  FeatureGeneratorPtr sumFeatures(size_t begin, size_t end) const;
  
  FeatureGeneratorPtr entropyFeatures(size_t begin, size_t end) const;

  size_t getLength() const
    {return accumulators.size();}

  FeatureDictionaryPtr getDictionary() const
    {return dictionary;}

private:
  FeatureDictionaryPtr dictionary;
  std::vector<std::vector<double> > accumulators; // position -> number -> count; number = label + 1 if label exists, 0 otherwise
};

typedef ReferenceCountedObjectPtr<AccumulatedScoresMatrix> AccumulatedScoresMatrixPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_ACCUMULATED_SCORES_MATRIX_H_
