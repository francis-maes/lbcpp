/**
 * author: Alejandro Marcos Alvarez
 * date: 14/04/2011
 * name: QScoreEvaluator.cpp
 */

#include "QScoreEvaluator.h"

namespace lbcpp
{
CAlphaDist copyCAlphaDist(CAlphaDist toCopy)
{
	CAlphaDist temp;
	temp.i = toCopy.i;
	temp.j = toCopy.j;
	temp.dist = toCopy.dist;
	temp.score = toCopy.score;
	return temp;
}

QScoreObject::QScoreObject()
{
	scores = new std::vector<CAlphaDist>;
	mean = 0;
}

QScoreObject::~QScoreObject()
{
	delete (scores);
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
	for (int i = 0; i < scores->size(); i++)
		acc += scores->at(i).score;
	mean = acc / scores->size();
}

void QScoreObject::addScore(const CAlphaDist cad)
{
	CAlphaDist temp = copyCAlphaDist(cad);
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
	for (int i = 0; i < scores->size(); i++)
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
	for (int i = 1; i < scores->size(); i++)
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
	for (int i = 1; i < scores->size(); i++)
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

bool compare4sortCAlphaDist(CAlphaDist calpha1, CAlphaDist calpha2)
{
	return (calpha1.dist > calpha2.dist);
}

bool compare4sortCAlphaScore(CAlphaDist calpha1, CAlphaDist calpha2)
{
	return (calpha1.score > calpha2.score);
}

std::vector<CAlphaDist> QScoreObject::getSortedScores()
{
	// Create list and sort.
	std::list<CAlphaDist> tempList;
	for (int i = 0; i < scores->size(); i++)
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

QScoreObjectPtr QScoreSingleEvaluator(ProteinPtr target, ProteinPtr model, size_t minDist,
		size_t maxDist)
{
	if( target->getLength() != model->getLength())
		return NULL;
	QScoreObjectPtr scores = new QScoreObject;




	return scores;
}

}
; /* namespace lbcpp */
