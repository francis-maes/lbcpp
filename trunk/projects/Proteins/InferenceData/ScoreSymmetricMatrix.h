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

class ScoreSymmetricMatrix : public NameableObject
{
public:
  ScoreSymmetricMatrix(const String& name, size_t dimension, double initialValue = 0.0)
    : NameableObject(name), dimension(dimension), matrix((dimension * (dimension + 1)) / 2, initialValue) {}
  ScoreSymmetricMatrix() {}

  virtual String toString() const
  {
    String res = NameableObject::toString() + T(" dimension = ") + lbcpp::toString(dimension) + T(":\n");
    for (size_t i = 0; i < dimension; ++i)
    {
      String line;
      size_t j;
      for (j = 0; j < i; ++j)
        line += T("    ");
      for (; j < dimension; ++j)
        line += scoreToStringFixedSize(getScore(i, j), 3) + T(" ");
      res += line + T("\n");
    }
    return res;
  }

  static String scoreToStringFixedSize(double value, int fixedSize)
  {
    String str(value, 1);
    while (str.length() < fixedSize)
      str += T(" ");
    return str;
  }

  virtual ObjectPtr clone() const
  {
    ReferenceCountedObjectPtr<ScoreSymmetricMatrix> res = Object::createAndCast<ScoreSymmetricMatrix>(getClassName());
    res->dimension = dimension;
    res->matrix = matrix;
    res->name = name;
    return res;
  }

  double getScore(size_t i, size_t j) const
    {return matrix[getIndex(i, j)];}

  void setScore(size_t i, size_t j, double score)
    {matrix[getIndex(i, j)] = score;}

protected:
  size_t dimension;
  std::vector<double> matrix;

  size_t getIndex(size_t i, size_t j) const
  {
    jassert(i < dimension);
    jassert(j < dimension);
    if (i > j)
      {size_t tmp = i; i = j; j = tmp;}
    size_t res = (j - i) + (i * dimension) - ((i * (i - 1)) / 2);
    jassert(res < matrix.size());
    return res;
  }

  virtual bool load(InputStream& istr)
    {return NameableObject::load(istr) && lbcpp::read(istr, dimension) && lbcpp::read(istr, matrix);}

  virtual void save(OutputStream& ostr) const
    {NameableObject::save(ostr); lbcpp::write(ostr, dimension); lbcpp::write(ostr, matrix);}
};

typedef ReferenceCountedObjectPtr<ScoreSymmetricMatrix> ScoreSymmetricMatrixPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_SCORE_SYMMETRIC_MATRIX_H_
