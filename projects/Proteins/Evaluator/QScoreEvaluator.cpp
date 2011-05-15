/*-----------------------------------------.---------------------------------.
| Filename: QScoreEvaluator.cpp            | QScoreEvaluator                 |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 14 avril 2011                  |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"

#ifdef LBCPP_PROTEIN_ROSETTA
#include "QScoreEvaluator.h"

namespace lbcpp
{

CAlphaDist copyCAlphaDist(CAlphaDist toCopy)
{
  CAlphaDist temp;
  temp.i = toCopy.i;
  temp.j = toCopy.j;
  temp.dist1 = toCopy.dist1;
  temp.dist2 = toCopy.dist2;
  temp.score = toCopy.score;
  return temp;
}

QScoreObject::QScoreObject()
{
  scores = new std::vector<CAlphaDist>;
  mean = 0;
}

QScoreObject::QScoreObject(QScoreObjectPtr copy)
{
  scores = new std::vector<CAlphaDist>(copy->getScores());
  mean = copy->getMean();
}

QScoreObject::QScoreObject(std::vector<CAlphaDist>& copy)
{
  scores = new std::vector<CAlphaDist>(copy);
  double acc = 0;
  for (size_t i = 0; i < copy.size(); i++)
    acc += copy.at(i).score;
  mean = acc / copy.size();
}
QScoreObject::~QScoreObject()
{
  delete (scores);
}

bool QScoreObject::isEmpty()
{
  return (scores->size() == 0);
}

int QScoreObject::size()
{
  return (scores->size());
}

double QScoreObject::getScoreToMinimize() const
{
  return (1.0 / mean);
}

double QScoreObject::getMean()
{
  this->update();
  return mean;
}

void QScoreObject::update()
{
  double acc = 0;
  for (size_t i = 0; i < scores->size(); i++)
    acc += scores->at(i).score;
  mean = acc / scores->size();
}

void QScoreObject::addScore(const CAlphaDist cad)
{
  //CAlphaDist temp = copyCAlphaDist(cad);
  CAlphaDist temp(cad);
  scores->push_back(temp);
}

CAlphaDist QScoreObject::getScore(size_t index)
{
  CAlphaDist temp;
  if ((index >= 0) && (index < scores->size()))
    temp = copyCAlphaDist(scores->at(index));
  return temp;
}

CAlphaDist QScoreObject::getScore(int aa1, int aa2)
{
  CAlphaDist temp;
  for (size_t i = 0; i < scores->size(); i++)
  {
    if (((scores->at(i).i == aa1) && (scores->at(i).j == aa2)) || ((scores->at(i).j == aa1)
        && (scores->at(i).i == aa2)))
      temp = copyCAlphaDist(scores->at(i));
  }
  return temp;
}

CAlphaDist QScoreObject::getMaxScore()
{
  CAlphaDist temp;
  double max;
  // Initalization of max value.
  if (scores->size() == 1)
    return temp;
  else
  {
    temp = copyCAlphaDist(scores->at(0));
    max = temp.score;
  }
  // Loop to find max.
  for (size_t i = 1; i < scores->size(); i++)
  {
    if (scores->at(i).score > max)
    {
      temp = copyCAlphaDist(scores->at(i));
      max = temp.score;
    }
  }
  return temp;
}
CAlphaDist QScoreObject::getMinScore()
{
  CAlphaDist temp;
  double min;
  // Initalization of min value.
  if (scores->size() == 1)
    return temp;
  else
  {
    temp = copyCAlphaDist(scores->at(0));
    min = temp.score;
  }
  // Loop to find min.
  for (size_t i = 1; i < scores->size(); i++)
  {
    if (scores->at(i).score < min)
    {
      temp = copyCAlphaDist(scores->at(i));
      min = temp.score;
    }
  }
  return temp;
}

std::vector<CAlphaDist> QScoreObject::getScores()
{
  std::vector<CAlphaDist> temp((*scores));
  return temp;
}

bool compare4sortCAlphaScore(CAlphaDist calpha1, CAlphaDist calpha2)
{
  return (calpha1.score > calpha2.score);
}

std::vector<CAlphaDist> QScoreObject::getSortedScores()
{
  // Create list and sort.
  std::list<CAlphaDist> tempList;
  for (size_t i = 0; i < scores->size(); i++)
    tempList.push_back(copyCAlphaDist(scores->at(i)));
  tempList.sort(compare4sortCAlphaScore);

  // Turn back into vector and return.
  std::vector<CAlphaDist> tempVec;
  std::list<CAlphaDist>::iterator it;
  for (it = tempList.begin(); it != tempList.end(); it++)
  {
    tempVec.push_back(copyCAlphaDist(*it));
  }
  return tempVec;
}

void QScoreObject::sort()
{
  std::vector<CAlphaDist> tempVec = getSortedScores();
  delete (scores);
  scores = new std::vector<CAlphaDist>(tempVec);
}

std::vector<CAlphaDist> QScoreObject::getScoresByDist(int minDist, int maxDist)
{
  std::vector<CAlphaDist> tempVec;
  for (size_t i = 0; i < scores->size(); i++)
  {
    CAlphaDist ca(scores->at(i));
    if ((std::abs((long int)(ca.i - ca.j)) >= minDist) && (std::abs((long int)(ca.i - ca.j))
        <= maxDist))
      tempVec.push_back(copyCAlphaDist(ca));
  }
  return tempVec;
}

QScoreObjectPtr QScoreObject::getQScoreObjectByDist(int minDist, int maxDist)
{
  std::vector<CAlphaDist> tempVec = this->getScoresByDist(minDist, maxDist);
  QScoreObjectPtr temp = new QScoreObject(tempVec);
  return temp;
}

QScoreObjectPtr QScoreObject::createQScoreFromMatrices(SymmetricMatrixPtr matTarget,
    SymmetricMatrixPtr matModel, int minDist, int maxDist)
{
  std::vector<CAlphaDist> tempVec;

  int numRows = matTarget->getNumRows();
  int numCols = matTarget->getNumColumns();

  for (int i = 0; i < numRows; i++)
  {
    for (int j = i + 1; j < numCols; j++)
    {
      if ((std::abs((long int)(i - j)) >= minDist) && (std::abs((long int)(i - j)) <= maxDist))
      {
        if ((matTarget->getElement(i, j)).isDouble() && (matModel->getElement(i, j)).isDouble())
        {
          double dist1 = (matTarget->getElement(i, j)).getDouble();
          double dist2 = (matModel->getElement(i, j)).getDouble();
          double score = std::exp(-std::pow(dist1 - dist2, 2));
          CAlphaDist ca(i, j, dist1, dist2, score);
          tempVec.push_back(copyCAlphaDist(ca));
        }

      }
    }
  }
  QScoreObjectPtr qs = new QScoreObject(tempVec);
  return qs;
}

QScoreObjectPtr QScoreSingleEvaluator(ProteinPtr target, ProteinPtr model, int minDist, int maxDist)
{
  if (maxDist < 0)
    maxDist = target->getLength();

  if (target->getLength() != model->getLength())
    return NULL;

  SymmetricMatrixPtr matTarget = (target->getTertiaryStructure())->makeCAlphaDistanceMatrix();
  SymmetricMatrixPtr matModel = (model->getTertiaryStructure())->makeCAlphaDistanceMatrix();

  return QScoreObject::createQScoreFromMatrices(matTarget, matModel, minDist, maxDist);
}

QScoreObjectPtr QScoreSingleEvaluator(core::pose::PoseOP target, core::pose::PoseOP model,
    int minDist, int maxDist)
{
  if (maxDist < 0)
    maxDist = target->n_residue();

  if ((size_t)target->n_residue() != (size_t)model->n_residue())
    return NULL;

  SymmetricMatrixPtr matTarget = createCalphaMatrixDistance(target);
  SymmetricMatrixPtr matModel = createCalphaMatrixDistance(model);

  return QScoreObject::createQScoreFromMatrices(matTarget, matModel, minDist, maxDist);
}

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_ROSETTA
