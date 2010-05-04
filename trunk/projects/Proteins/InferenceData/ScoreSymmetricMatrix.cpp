/*-----------------------------------------.---------------------------------.
| Filename: ScoreSymmetricMatrix.cpp       | Symmetric Matrix of scores      |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 19:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ScoreSymmetricMatrix.h"
using namespace lbcpp;

ScoreSymmetricMatrix::ScoreSymmetricMatrix(const String& name, size_t dimension, double initialValue)
  : NameableObject(name), dimension(dimension), matrix((dimension * (dimension + 1)) / 2, initialValue)
{
}

ScoreSymmetricMatrixPtr ScoreSymmetricMatrix::makeThresholdedMatrix(const String& name, double threshold, double belowValue, double aboveValue) const
{
  ScoreSymmetricMatrixPtr res = new ScoreSymmetricMatrix(name, dimension);
  for (size_t i = 0; i < dimension; ++i)
    for (size_t j = i; j < dimension; ++j)
      if (hasScore(i, j))
        res->setScore(i, j, getScore(i, j) > threshold ? aboveValue : belowValue);
  return res;
}

static inline String scoreToStringFixedSize(double value, int fixedSize)
{
  String str(value, 1);
  while (str.length() < fixedSize)
    str += T(" ");
  return str;
}

String ScoreSymmetricMatrix::toString() const
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

ObjectPtr ScoreSymmetricMatrix::clone() const
{
  ReferenceCountedObjectPtr<ScoreSymmetricMatrix> res = Object::createAndCast<ScoreSymmetricMatrix>(getClassName());
  res->dimension = dimension;
  res->matrix = matrix;
  res->name = name;
  return res;
}

size_t ScoreSymmetricMatrix::getIndex(size_t i, size_t j) const
{
  jassert(i < dimension);
  jassert(j < dimension);
  if (i > j)
    {size_t tmp = i; i = j; j = tmp;}
  size_t res = (j - i) + (i * dimension) - ((i * (i - 1)) / 2);
  jassert(res < matrix.size());
  return res;
}
