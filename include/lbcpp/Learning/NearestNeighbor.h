/*-----------------------------------------.---------------------------------.
| Filename: NearestNeighbor.h              | Nearest Neighbor                |
| Author  : Julien Becker                  |                                 |
| Started : 04/08/2011 16:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NEAREST_NEIGHBOR_H_
# define LBCPP_NEAREST_NEIGHBOR_H_

# include "../Core/Function.h"

namespace lbcpp
{

extern FunctionPtr binaryNearestNeighbor(size_t numNeighbors, bool autoNormalizeFeatures = false, bool includeTheNearestNeighbor = true, bool useWeightedScore = false);
extern FunctionPtr classificationNearestNeighbor(size_t numNeighbors, bool autoNormalizeFeatures = false, bool includeTheNearestNeighbor = true);
extern FunctionPtr regressionNearestNeighbor(size_t numNeighbors, bool autoNormalizeFeatures = false, bool includeTheNearestNeighbor = true);

extern FunctionPtr nearestNeighborLearningMachine(size_t numNeighbors, bool autoNormalizeFeatures, bool includeTheNearestNeighbor);

}; /* namespace lbcpp */

#endif //!LBCPP_NEAREST_NEIGHBOR_H_
