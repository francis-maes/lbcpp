/*-----------------------------------------.---------------------------------.
| Filename: QScoreEvaluator.h              | QScoreEvaluator                 |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 14 avril 2011                  |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_EVALUATOR_QSCOREEVALUATOR_H_
# define LBCPP_PROTEINS_EVALUATOR_QSCOREEVALUATOR_H_

# include <vector>
# include <list>
# include <iostream>
# include <cmath>

# include "../Data/Protein.h"
# include "lbcpp/Function/Evaluator.h"
# include "../Rosetta/RosettaUtils.h"

/*# undef T
#  include <core/conformation/Residue.hh>
#  include <core/pose/Pose.hh>
#  include <numeric/xyzVector.hh>
# define T JUCE_T*/

namespace lbcpp
{
/*
 * Temporary structure to store the distances and scores
 * for each pair of amino acids.
 */
struct CAlphaDist;
typedef struct CAlphaDist CAlphaDist;

struct CAlphaDist
{
  int i;
  int j;
  double dist1;
  double dist2;
  double score;
  CAlphaDist() :
    i(0), j(0), dist1(-1), dist2(-1), score(-1)
  {
  }
  CAlphaDist(int x, int y, double d1, double d2, double s) :
    i(x), j(y), dist1(d1), dist2(d2), score(s)
  {
  }
  CAlphaDist(const CAlphaDist& x) :
    i(x.i), j(x.j), dist1(x.dist1), dist2(x.dist2), score(x.score)
  {
  }
};

CAlphaDist copyCAlphaDist(CAlphaDist toCopy);

class QScoreObject;
typedef ReferenceCountedObjectPtr<QScoreObject> QScoreObjectPtr;

class QScoreObject: public ScoreObject
{
public:
  /**
   * Constructor and destructors
   */
  QScoreObject();
  QScoreObject(QScoreObjectPtr copy);
  QScoreObject(std::vector<CAlphaDist>& copy);
  ~QScoreObject();

  /**
   * Tells if the QScore object is empty.
   * @retur true if empty.
   */
  bool isEmpty();

  /**
   * Gives the size of the QScoreObject.
   * @return the number of scores stored in the QScoreObject.
   */
  int size();

  /**
   * @brief Returns the score that has to be minimized. A call to
   * update() is necessary before calling this method to ensure that
   * the value returned is up to date.
   * @return the score to minimize
   */
  virtual double getScoreToMinimize() const;

  /**
   * @brief Returns the mean of all QScores calculated for the object.
   * @return the mean of all QScores.
   */
  double getMean();

  /**
   * @brief Updates the mean variable, necessary to obtain
   * an up-to-date value for the function getScoreToMinimize().
   */
  void update();

  /**
   * @brief Adds score to the object.
   * @param cad, the CAlphaDist object that has to be added to
   * the QScore object.
   */
  void addScore(const CAlphaDist cad);

  /**
   * @brief Get the CAlphaDist object for the specified
   * index in QScore (objects stored in a vector and non-sorted).
   * @param index the position of the desired object.
   * @return the CAlphaDist object at position index. Returns an object
   * containing (0, 0, -1, -1) if nothing found.
   */
  CAlphaDist getScore(size_t index);

  /**
   * @brief Get the CAlphaDist object for the specified amino
   * acids in QScore (objects stored in a vector and non-sorted).
   * @param aa1 index of the first amino acid considered.
   * @param aa2 index of the second amino acid considered.
   * @return the CAlphaDist object that corresponds to the two
   * desired amino acids. Returns an object containing (0, 0, -1, -1)
   * if nothing found.
   */
  CAlphaDist getScore(int aa1, int aa2);

  /**
   * @brief Returns the object containing the max score.
   * @return the object containing the max score.
   */
  CAlphaDist getMaxScore();

  /**
   * @brief Returns the object containing the min score.
   * @return the object containing the min score.
   */
  CAlphaDist getMinScore();

  /**
   * @brief This method returns a vector containing all the scores
   * that were stored in the QScore object.
   * @return a vector containing the CAlphaDist objects.
   */
  std::vector<CAlphaDist> getScores();

  /**
   * @brief This method returns a vector containing all the scores
   * that were stored in the QScore object and in a sorted fashion.
   * @return a sorted vector containing the CAlphaDist objects.
   */
  std::vector<CAlphaDist> getSortedScores();

  /**
   * Sorts the QScoreObject that is, by default, unsorted.
   */
  void sort();

  std::vector<CAlphaDist> getScoresByDist(int minDist, int maxDist);

  QScoreObjectPtr getQScoreObjectByDist(int minDist, int maxDist);

  static QScoreObjectPtr createQScoreFromMatrices(SymmetricMatrixPtr matrix1,
      SymmetricMatrixPtr matrix2, int minDist, int maxDist);

private:
  std::vector<CAlphaDist>* scores;
  double mean;
};

/**
 * @brief Computes QScore for the target and model. Target and model are supposed
 * to be the same protein (same primary sequence) only the tertiary structure is
 * different. The proteins' primary sequence has to be written in the same order
 * for both arguments (i.e. prot1=AAC and prot2=AAC : OK but prot1=AAC and prot2=
 * CAA : FAIL).
 * @param target the target protein.
 * @param model the model protein, that has to match the target.
 * @param minDist the minimum distance in the chain of amino acids between two
 * amino acids that are evaluated for the QScore. -1 means no minimum distance.
 * @param maxDist the maximum distance in the chain of amino acids between two
 * amino acids that are evaluated for the QScore. -1 means no maximum distance.
 * @return a QScoreObjectPtr that represents the similarity between the proteins.
 */
QScoreObjectPtr QScoreSingleEvaluator(ProteinPtr target, ProteinPtr model, int minDist = -1,
    int maxDist = -1);

QScoreObjectPtr QScoreSingleEvaluator(core::pose::PoseOP target, core::pose::PoseOP model,
    int minDist = -1, int maxDist = -1);

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_EVALUATOR_QSCOREEVALUATOR_H_
