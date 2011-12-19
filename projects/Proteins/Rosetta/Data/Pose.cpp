/*-----------------------------------------.---------------------------------.
| Filename: Pose.cpp                       | Pose source                     |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 2, 2011  10:55:09 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "Pose.h"

namespace lbcpp
{
#ifdef LBCPP_PROTEIN_ROSETTA

void Pose::createFromSequence(const String& sequence)
{
  core::chemical::make_pose_from_sequence(*pose, (const char*)sequence,
      core::chemical::ChemicalManager::get_instance()->nonconst_residue_type_set("fa_standard"));
}

void Pose::createFromPDB(const File& pdbFile)
{
  pose = new core::pose::Pose((const char*)pdbFile.getFullPathName());
  if (pose() == NULL)
    jassert(false);
}


# else
void Pose::createFromSequence(String& sequence)
  {jasser(false);}

void Pose::createFromPDB(const File& pdbFile)
  {jassert(false);}

#endif //! LBCPP_PROTEIN_ROSETTA
}; /* namespace lbcpp */
