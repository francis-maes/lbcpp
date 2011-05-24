/*-----------------------------------------.---------------------------------.
| Filename: RosettaProtein.h               | RosettaProtein                  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 24 mai 2011  07:47:58          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PROTEIN_H_
# define LBCPP_PROTEINS_ROSETTA_PROTEIN_H_

# include "RosettaUtils.h"

namespace lbcpp
{

class RosettaProtein : public Object
{
public:
  RosettaProtein(const core::pose::PoseOP& pose) : pose(pose) {}
  RosettaProtein() {}

  void getPose(core::pose::PoseOP& returnPose)
    {returnPose = pose;}

protected:
  friend class RosettaProteinClass;

  core::pose::PoseOP pose;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_H_
