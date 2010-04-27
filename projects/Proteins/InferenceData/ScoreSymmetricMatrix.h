/*-----------------------------------------.---------------------------------.
| Filename: ScoreSymmetricMatrix.h         | Symmetric Matrix of scores      |
| Author  : Francis Maes                   |                                 |
| Started : 17/04/2010 16:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_DATA_SCORE_SYMMETRIC_MATRIX_H_
# define LBCPP_INFERENCE_DATA_SCORE_SYMMETRIC_MATRIX_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class ScoreSymmetricMatrix;
typedef ReferenceCountedObjectPtr<ScoreSymmetricMatrix> ScoreSymmetricMatrixPtr;

class ScoreSymmetricMatrix : public NameableObject
{
public:
  ScoreSymmetricMatrix(const String& name, size_t dimension, double initialValue = DBL_MAX);
  ScoreSymmetricMatrix() {}

  size_t getDimension() const
    {return dimension;}

  ScoreSymmetricMatrixPtr makeThresholdedMatrix(const String& name, double threshold, double belowValue = 0.0, double aboveValue = 1.0) const;

  virtual String toString() const;

  virtual ObjectPtr clone() const;

  bool hasScore(size_t i, size_t j) const
    {return getScore(i, j) != DBL_MAX;}

  double getScore(size_t i, size_t j) const
    {return matrix[getIndex(i, j)];}

  void setScore(size_t i, size_t j, double score)
    {matrix[getIndex(i, j)] = score;}

protected:
  size_t dimension;
  std::vector<double> matrix;

  size_t getIndex(size_t i, size_t j) const;

  virtual bool load(InputStream& istr)
    {return NameableObject::load(istr) && lbcpp::read(istr, dimension) && lbcpp::read(istr, matrix);}

  virtual void save(OutputStream& ostr) const
    {NameableObject::save(ostr); lbcpp::write(ostr, dimension); lbcpp::write(ostr, matrix);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_SCORE_SYMMETRIC_MATRIX_H_
