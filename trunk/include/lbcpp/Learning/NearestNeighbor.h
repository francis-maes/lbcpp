/*-----------------------------------------.---------------------------------.
| Filename: NearestNeighbo.h               | Nearest Neighbor                |
| Author  : Julien Becker                  |                                 |
| Started : 04/089/2011 16:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NEAREST_NEIGHBOR_H_
# define LBCPP_NEAREST_NEIGHBOR_H_

# include "../Core/Function.h"

namespace lbcpp
{

extern FunctionPtr binaryNearestNeighbor(size_t numNeighbors, bool useWeightedScore = false);

}; /* namespace lbcpp */

#endif //!LBCPP_NEAREST_NEIGHBOR_H_
