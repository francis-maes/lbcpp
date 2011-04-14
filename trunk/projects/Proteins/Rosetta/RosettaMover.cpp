/**
 * author: Alejandro Marcos Alvarez
 * date: 12/04/2011
 * name: RosettaMovers.cpp
 */

#include "RosettaMover.h"

namespace lbcpp
{

void phiPsiMover(core::pose::PoseOP pose, std::vector<void*>* vec)
{
	int k = ((int) rand() % pose->n_residue()) + 1;
	double step;
	if ((vec == NULL) || (vec->size() == 0))
		step = 180;
	else
		step = (*(double*) vec->at(0));
	pose->set_phi(k, pose->phi(k) + step * (2 * generateRand() - 1));
	pose->set_psi(k, pose->psi(k) + step * (2 * generateRand() - 1));
}

void phiPsiGaussMover(core::pose::PoseOP pose, std::vector<void*>* vec)
{
	int k = ((int) rand() % pose->n_residue()) + 1;
	double step;
	if ((vec == NULL) || (vec->size() == 0))
		step = 180;
	else
		step = (*(double*) vec->at(0));

	double ang = step * generateNormalRand();
	if (ang >= 180)
		ang = 180;
	else if (ang <= -180)
		ang = -180;
	pose->set_phi(k, pose->phi(k) + ang);

	ang = step * generateNormalRand();
	if (ang >= 180)
		ang = 180;
	else if (ang <= -180)
		ang = -180;
	pose->set_psi(k, pose->psi(k) + ang);
}

}
; /* namespace lbcpp */
