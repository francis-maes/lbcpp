#ifndef LBCPP_PROTEIN_GABOW_PATTERN_H_
# define LBCPP_PROTEIN_GABOW_PATTERN_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class GabowPatternFunction : public SimpleUnaryFunction
{
public:
  GabowPatternFunction()
    : SimpleUnaryFunction(doubleSymmetricMatrixClass(doubleType), doubleSymmetricMatrixClass(doubleType), T("GabowPattern"))
    {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    DoubleSymmetricMatrixPtr matrix = input.getObjectAndCast<DoubleSymmetricMatrix>(context);
    if (!matrix)
      return Variable::missingValue(getOutputType());

    const size_t numVertices = matrix->getDimension();
    const size_t numEdges = getNumEdges(matrix);
    // E0 - Initialize
    GabowState state(numVertices, numEdges);
    convertGraph(matrix, state);
    state.printGraph();
    // E1 - Find unmatched vertex
    //for (size_t ii = 0; ii < 2; ++ii)
    for (size_t startVertex = findUnmatchedVertex(state);
         startVertex != (size_t)-1;
         startVertex = findUnmatchedVertex(state, startVertex))
    {
      jassert(!state.isMatched(startVertex));
      state.setStartLabel(startVertex);
      state.setFirstNonOuter(startVertex, 0);
      // E2 - Choose edge
      std::vector<size_t> edgeIds;
      state.getEdgesWithOuterVertex(edgeIds);
      for (size_t i = 0; i < edgeIds.size(); ++i)
      {
        const size_t edgeId = edgeIds[i];
        const std::pair<size_t, size_t> edge = state.getVertices(edgeId);
        jassert(state.isOuter(edge.first));
        
        std::cout << "Selected Edge: " << edge.first << " <-> " << edge.second << std::endl;
        // E3 - Augment the matching
        if (!state.isMatched(edge.second) && edge.second != startVertex)
        {
          state.setMatching(edge.second, edge.first);
          rematcheEdges(state, edge.first, edge.second);
          break; // Go to E7
        }
        // E4 - Assign edge labels
        if (state.isOuter(edge.second))
        {
          assignEdgeLabelToNonOuterVertices(state, edgeId);
          continue; // Go to E2
        }
        // E5 - Assign a vertex label
        size_t endEdge = state.getMatching(edge.second);
        if (!state.isOuter(endEdge))
        {
          std::cout << "Set Vertex Label: " << endEdge << std::endl;
          state.setVertexLabel(endEdge, edge.first);
          state.setFirstNonOuter(endEdge, edge.second);
          continue; // Go to E2
        }
        // E6 - Get next edge
        jassert(!state.isOuter(edge.second) && state.isOuter(state.getMatching(edge.second)));
        // Go to E2
        state.printState();
      }
      state.printState();
      // E7 - Stop the search
      state.resetLabel(0);
      for (size_t i = 1; i <= numVertices; ++i)
      {
        if (state.isOuter(i))
        {
          state.resetLabel(i);
          state.setFirstNonOuter(i, 0);
          state.resetLabel(state.getMatching(i));
        }
      }
      // Go to E1
    }

    std::cout << "Matching found !" << std::endl;
    for (size_t i = 1; i <= numVertices; ++i)
    {
      std::cout << i << " <-> " << state.getMatching(i) << std::endl;
    }
    std::cout << std::endl;
    return buildMatrix(state);
  }
  
protected:
  class GabowState
  {
  public:
    const size_t numVertices;
    const size_t numEdges;
    
    GabowState(size_t numVertices, size_t numUndirectedEdges)
      : numVertices(numVertices), numEdges(numUndirectedEdges * 2)
      , edges(NULL), nextEdge(0)
    {
      edges = new std::pair<size_t, size_t>[numEdges];
      matching = new size_t[numVertices + 1]; // + dummy vertex 0
      labels = new int[numVertices + 1];
      firstNonOuter = new size_t[numVertices + 1];
      for (size_t i = 0; i <= numVertices; ++i)
      {
        matching[i] = 0;                  // unmatched (matched to dummy vertex)
        labels[i] = -1;                   // not outer vertex
        firstNonOuter[i] = 0;
      }
    }

    ~GabowState()
    {
      delete[] edges;
      delete[] matching;
      delete[] labels;
      delete[] firstNonOuter;
    }

    void printGraph() const
    {
      for (size_t i = 0; i <= numVertices; ++i)
        std::cout << "Vertex " << i << std::endl;
      for (size_t i = 0; i < numEdges; ++i)
        std::cout << "Edge " << (i + numVertices + 1) << ": " << edges[i].first << " <-> " << edges[i].second << std::endl;
    }

    void printState() const
    {
      std::cout << "#\t\tMatching\t\tLabels\t\tFirst" << std::endl;
      for (size_t i = 0; i <= numVertices; ++i)
      {
        std::cout << i << "\t\t"
                  << matching[i] << "\t\t"
                  << labels[i] << "\t\t"
                  << firstNonOuter[i] << std::endl;
      }
    }

    void addEdge(size_t firstVertex, size_t secondVertex)
    {
      jassert(nextEdge < numEdges);
      jassert(firstVertex > 0 && firstVertex <= numVertices);
      jassert(secondVertex > 0 && secondVertex <= numVertices);
      edges[nextEdge++] = std::pair<size_t, size_t>(firstVertex, secondVertex);
      edges[nextEdge++] = std::pair<size_t, size_t>(secondVertex, firstVertex);
    }

    /* Label */
    void resetLabel(size_t vertex)
      {jassert(vertex <= numVertices); labels[vertex] = -1;}

    bool isOuter(size_t vertex) const
      {jassert(vertex <= numVertices); return vertex == 0 ? false : labels[vertex] >= 0;}

    bool isVertexLabel(size_t vertex) const
      {jassert(isOuter(vertex)); return labels[vertex] <= (int)numVertices && labels[vertex] > 0;}

    bool isEdgeLabel(size_t vertex) const
      {jassert(isOuter(vertex)); return labels[vertex] > (int)numVertices;}

    void setStartLabel(size_t vertex)
      {jassert(vertex <= numVertices && vertex > 0); labels[vertex] = 0;}

    void setVertexLabel(size_t vertex, int label)
      {jassert(vertex <= numVertices); jassert(label <= (int)numVertices && label > 0); labels[vertex] = label;}

    void setEdgeLabel(size_t vertex, size_t edgeLabel)
      {jassert(vertex <= numVertices); jassert(edgeLabel > numVertices && edgeLabel <= numEdges + numVertices); labels[vertex] = edgeLabel;}

    void setFlaggedLabel(size_t vertex, int label)
      {jassert(vertex <= numVertices && label < -(int)numVertices); labels[vertex] = label;}

    bool isFlagged(size_t vertex) const
      {jassert(vertex <= numVertices); return labels[vertex] < -(int)numVertices;}
    
    int getVertexLabel(size_t vertex) const
      {jassert(isVertexLabel(vertex)); return labels[vertex];}

    int getEdgeLabel(size_t vertex) const
      {jassert(isEdgeLabel(vertex)); return labels[vertex];}

    /* First */
    void setFirstNonOuter(size_t vertex, size_t nonOuterVetex)
      {jassert(vertex <= numVertices && !isOuter(nonOuterVetex)); firstNonOuter[vertex] = nonOuterVetex;}

    size_t getFirstNonOuter(size_t vertex) const
      {jassert(isOuter(vertex) && !isOuter(firstNonOuter[vertex])); return firstNonOuter[vertex];}

    size_t getFirst(size_t vertex) const
      {jassert(isOuter(vertex)); return firstNonOuter[vertex];}

    const std::pair<size_t, size_t>& getVertices(size_t edge) const
      {jassert(edge > numVertices && edge <= numVertices + numEdges); return edges[edge - numVertices - 1];}

    // Outer vertex first
    void getEdgesWithOuterVertex(std::vector<size_t>& results) const
    {
      for (size_t i = 0; i < numEdges; ++i)
      {
        const std::pair<size_t, size_t> edge = edges[i];
        jassert(edge.first > 0 && edge.second > 0);
        if (isOuter(edge.first))
          results.push_back(i + numVertices + 1);
      }
    }

    bool isMatched(size_t vertex) const
      {jassert(vertex <= numVertices); return matching[vertex] > 0;}
    
    size_t getMatching(size_t vertex) const
      {jassert(vertex <= numVertices); return matching[vertex];}
    
    void setMatching(size_t firstVertex, size_t secondVertex)
      {jassert(firstVertex <= numVertices && secondVertex <= numVertices
               && firstVertex != 0 && secondVertex != 0);
        matching[firstVertex] = secondVertex;}

    size_t getNextNonOuterVertex(size_t startVertex) const
      {return getFirstNonOuter(getVertexLabel(getMatching(startVertex)));}

  private:
    std::pair<size_t, size_t>* edges;   // Indices of vertex
    size_t nextEdge;

    size_t* matching;                    // Index of vertex
    int* labels;                       // -1: nonouter > vertex > edge
    size_t* firstNonOuter;               // Index of vertex
  };
  
  size_t getNumEdges(const DoubleSymmetricMatrixPtr& matrix) const
  {
    const size_t dimension = matrix->getDimension();
    size_t res = 0;
    for (size_t i = 0; i < dimension; ++i)
      for (size_t j = i + 1; j < dimension; ++j)
        if (matrix->getValue(i, j) > 0.f)
          ++res;
    return res;
  }

  void convertGraph(const DoubleSymmetricMatrixPtr& matrix, GabowState& state) const
  {
    const size_t dimension = matrix->getDimension();
    for (size_t i = 0; i < dimension; ++i)
      for (size_t j = i + 1; j < dimension; ++j)
        if (matrix->getValue(i, j) > 0.f)
          state.addEdge(i + 1, j + 1);
  }

  size_t findUnmatchedVertex(const GabowState& state, size_t startVertex = 0) const
  {
    for (size_t i = startVertex + 1; i <= state.numVertices; ++i)
    {
      std::cout << "----> findUnmatchedVertex - " << i;
      if (!state.isMatched(i))
      {
        std::cout << " ***** Not Matched *****" << std::endl;
        return i;
      }
      std::cout << std::endl;
    }
    return -1;
  }

  // L routine
  void assignEdgeLabelToNonOuterVertices(GabowState& state, int edgeId) const
  {
    const std::pair<size_t, size_t>& edge = state.getVertices(edgeId);
    std::cout << "Assign edge: " << edge.first << " <-> " << edge.second << std::endl;

    // L0 - Initialize
    size_t r = state.getFirstNonOuter(edge.first);
    size_t s = state.getFirstNonOuter(edge.second);
    if (r == s)
      return;
    state.setFlaggedLabel(r, -edgeId);
    state.setFlaggedLabel(s, -edgeId);

    while (true)
    {
      // L1 - Switch paths
      if (s != 0)
      {
        size_t tmp = r;
        r = s;
        s = tmp;
      }
      // L2 - Next nonouter vertex
      r = state.getNextNonOuterVertex(r);
      if (state.isFlagged(r))
        break;
      state.setFlaggedLabel(r, -edgeId);
      // Go to L1
    }
    const size_t joinVertex = r;
    jassert(!state.isOuter(joinVertex));
    // L3 - Label vertices in P(firstVertex), P(secondVertex)
    assingEdgeLabelAlongPath(state, state.getFirstNonOuter(edge.first), joinVertex, edgeId);
    assingEdgeLabelAlongPath(state, state.getFirstNonOuter(edge.second), joinVertex, edgeId);
    // L5 - Update FirstNonOuter
    for (size_t i = 1; i <= state.numVertices; ++i)
      if (state.isOuter(i) && state.isOuter(state.getFirst(i)))
        state.setFirstNonOuter(i, joinVertex);
    // L6 - Done
  }
  // L4 - Label
  void assingEdgeLabelAlongPath(GabowState& state, size_t vertex, size_t joinVertex, size_t edgeId) const
  {
    while (vertex != joinVertex)
    {
      state.setEdgeLabel(vertex, edgeId);
      state.setFirstNonOuter(vertex, joinVertex);
      vertex = state.getNextNonOuterVertex(vertex);
    }
  }

  // R routine
  void rematcheEdges(GabowState& state, size_t firstVertex, size_t secondVertex) const
  {
    std::cout << "Rematch" << std::endl;
    // R1 - Match firstVertex to secondVertex
    size_t nextVertex = state.getMatching(firstVertex);
    state.setMatching(firstVertex, secondVertex);
    if (state.getMatching(nextVertex) != firstVertex)
      return;
    // R2 - Rematch a path
    if (state.isVertexLabel(firstVertex))
    {
      state.setMatching(nextVertex, state.getVertexLabel(firstVertex));
      rematcheEdges(state, state.getVertexLabel(firstVertex), nextVertex);
      return;
    }
    // R3 - Rematch two paths
    const std::pair<size_t, size_t>& edge = state.getVertices(state.getEdgeLabel(firstVertex));
    rematcheEdges(state, edge.first, edge.second);
    rematcheEdges(state, edge.second, edge.first);
  }

  DoubleSymmetricMatrixPtr buildMatrix(const GabowState& state) const
  {
    DoubleSymmetricMatrixPtr res = symmetricMatrix(doubleType, state.numVertices);

    for (size_t i = 1; i <= state.numVertices; ++i)
      if (state.getMatching(i) != 0)
        res->setValue(i - 1, state.getMatching(i) - 1, 1.f);

    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_GABOW_PATTERN_H_
