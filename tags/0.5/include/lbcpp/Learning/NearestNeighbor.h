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

extern FunctionPtr binaryNearestNeighbor(size_t numNeighbors, bool includeTheNearestNeighbor = true, bool useWeightedScore = false);
extern FunctionPtr classificationNearestNeighbor(size_t numNeighbors, bool includeTheNearestNeighbor = true);
extern FunctionPtr regressionNearestNeighbor(size_t numNeighbors, bool includeTheNearestNeighbor = true);

extern FunctionPtr classificationStreamBasedNearestNeighbor(const StreamPtr& stream, size_t numNeighbors, bool includeTheNearestNeighbor);
extern FunctionPtr binaryClassificationStreamBasedNearestNeighbor(const StreamPtr& stream, size_t numNeighbors, bool includeTheNearestNeighbor);
// Without explicit Stream
extern FunctionPtr classificationStreamBasedNearestNeighbor(size_t numNeighbors, bool includeTheNearestNeighbor);
extern FunctionPtr binaryClassificationStreamBasedNearestNeighbor(size_t numNeighbors, bool includeTheNearestNeighbor);

extern FunctionPtr nearestNeighborLearningMachine(size_t numNeighbors, bool includeTheNearestNeighbor);
extern FunctionPtr nearestNeighborLearningMachine(const StreamPtr& stream, size_t numNeighbors, bool includeTheNearestNeighbor);

extern FunctionPtr binaryLocalitySensitiveHashing(size_t numNeighbors);

}; /* namespace lbcpp */

#endif //!LBCPP_NEAREST_NEIGHBOR_H_
