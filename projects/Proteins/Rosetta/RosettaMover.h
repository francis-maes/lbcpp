/**
 * author: Alejandro Marcos Alvarez
 * date: 12/04/2011
 * name: RosettaMovers.h
 */

#ifndef LBCPP_PROTEINS_ROSETTAMOVER_H_
#define LBCPP_PROTEINS_ROSETTAMOVER_H_

#include "RosettaUtils.h"

#include <cmath>
#include <vector>

namespace lbcpp
{

/**
 * Increments the phi and psi angles of a randomly selected residue of the pose object.
 * The increments are randomly selected in the range [-x, x] where x is given by argument vec.
 * @param pose the object that is to be modified
 * @param vec a vector containing 1 element of type double* that gives the range where the
 * increments of the psi and phi angles lie.
 */
void phiPsiMover(core::pose::PoseOP pose, std::vector<void*>* vec);

/**
 * Increments the phi and psi angles of a randomly selected residue of the pose object.
 * The increments are randomly selected by a gaussian distribution of mean = x where x
 * is given by argument vec.
 * @param pose the object that is to be modified
 * @param vec a vector containing 1 element of type double* that gives the range where the
 * increments of the psi and phi angles lie.
 */
void phiPsiGaussMover(core::pose::PoseOP pose, std::vector<void*>* vec);

}
; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTAMOVER_H_
