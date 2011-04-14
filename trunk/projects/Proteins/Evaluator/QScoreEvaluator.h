/**
 * author: Alejandro Marcos Alvarez
 * date: 14/04/2011
 * name: QScoreEvaluator.h
 */

#ifndef LBCPP_PROTEINS_EVALUATOR_QSCOREEVALUATOR_H_
#define LBCPP_PROTEINS_EVALUATOR_QSCOREEVALUATOR_H_

#include <vector>
#include <list>
#include <iostream>
#include <cmath>

#include "../Data/Protein.h"
#include "lbcpp/Function/Evaluator.h"

namespace lbcpp
{
/*
 * Temporary structure to store the distances and scores
 * for each pair of amino acids.
 */
struct CAlphaDist
{
	int i;
	int j;
	double dist;
	double score;
	CAlphaDist() :
		i(0), j(0), dist(-1), score(-1)
	{
	}
	CAlphaDist(int x, int y, double d, double s) :
		i(x), j(y), dist(d), score(s)
	{
	}
};
typedef struct CAlphaDist CAlphaDist;

CAlphaDist copyCAlphaDist(CAlphaDist toCopy);

class QScoreObject: public ScoreObject
{
public:
	/**
	 * Constructor and destructors
	 */
	QScoreObject();
	~QScoreObject();

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

private:
	std::vector<CAlphaDist>* scores;
	double mean;
};
typedef ReferenceCountedObjectPtr<QScoreObject> QScoreObjectPtr;

/**
 * @brief Computes QScore for the target and model. Target and model are supposed
 * to be the same protein (same primary sequence) only the tertiary structure is
 * different. The proteins' primary sequence has to be written in the same order
 * for both arguments (i.e. prot1=AAC and prot2=AAC : OK but prot1=AAC and prot2=
 * CAA : FAIL).
 * @param target the target protein.
 * @param model the model protein, that has to match the target.
 * @param minDist the minimum distance in the chain of amino acids between two
 * amino acids that are evaluated for the QScore. -1 if no min distance, default.
 * @param maxDist the maximum distance in the chain of amino acids between two
 * amino acids that are evaluated for the QScore. -1 if no max distance, default.
 * @return a QScoreObjectPtr that represents the similarity between the proteins.
 */
QScoreObjectPtr QScoreSingleEvaluator(ProteinPtr target, ProteinPtr model, size_t minDist = -1,
		size_t maxDist = -1);

}
; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_EVALUATOR_QSCOREEVALUATOR_H_
