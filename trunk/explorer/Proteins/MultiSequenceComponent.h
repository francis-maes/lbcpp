/*-----------------------------------------.---------------------------------.
| Filename: MultiSequenceComponent.h       | Components to display multiple  |
| Author  : Francis Maes                   |  aligned sequences              |
| Started : 14/06/2010 18:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_MULTI_SEQUENCE_COMPONENT_H_
# define EXPLORER_COMPONENTS_MULTI_SEQUENCE_COMPONENT_H_

# include "../Components/common.h"

namespace lbcpp
{

class MultiSequenceComponent : public Component
{
public:
  void clearSequences()
    {sequences.clear();}

  void addSequenceGroup(const String& friendlyName, const std::vector<ObjectContainerPtr>& sequences)
    {this->sequences.push_back(std::make_pair(friendlyName, sequences));}

private:
  std::vector< std::pair<String, std::vector<ObjectContainerPtr> > > sequences;

};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_MULTI_SEQUENCE_COMPONENT_H_

