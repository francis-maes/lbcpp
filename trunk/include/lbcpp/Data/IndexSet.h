/*-----------------------------------------.---------------------------------.
| Filename: IndexSet.h                     | Index Set                       |
| Author  : Francis Maes                   |                                 |
| Started : 17/12/2011 13:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_INDEX_SET_H_
# define LBCPP_DATA_INDEX_SET_H_

# include "RandomGenerator.h"

namespace lbcpp
{

class IndexSet;
typedef ReferenceCountedObjectPtr<IndexSet> IndexSetPtr;

class IndexSet : public Object
{
public:
  IndexSet(size_t begin, size_t end);
  IndexSet();
  
  struct iterator
  {
    iterator(IndexSet* owner, size_t chunkNumber, size_t indexInChunk) : owner(owner), chunkNumber(chunkNumber), indexInChunk(indexInChunk) {}
    iterator(const iterator& other) : owner(other.owner), chunkNumber(other.chunkNumber), indexInChunk(other.indexInChunk) {}
    iterator() : owner(NULL), chunkNumber(0), indexInChunk(0) {}

    iterator& operator =(const iterator& other)
      {owner = other.owner; chunkNumber = other.chunkNumber; indexInChunk = other.indexInChunk; return *this;}

    iterator& operator ++()
    {
      jassert(chunkNumber < owner->chunks.size() && indexInChunk < owner->chunks[chunkNumber].size());
      ++indexInChunk;
      if (indexInChunk == owner->chunks[chunkNumber].size())
      {
        ++chunkNumber;
        indexInChunk = 0;
      }
      return *this;
    }

    size_t& operator *() const
    {
      jassert(chunkNumber < owner->chunks.size() && indexInChunk < owner->chunks[chunkNumber].size());
      return owner->chunks[chunkNumber][indexInChunk];
    }

    bool operator ==(const iterator& other) const
      {return owner == other.owner && chunkNumber == other.chunkNumber && indexInChunk == other.indexInChunk;}
    bool operator !=(const iterator& other) const
      {return owner != other.owner || chunkNumber != other.chunkNumber || indexInChunk != other.indexInChunk;}

  private:
    friend class IndexSet;

    IndexSet* owner;
    size_t chunkNumber;
    size_t indexInChunk;
  };

  iterator begin()
    {return iterator(this, 0, 0);}

  iterator end()
    {return iterator(this, chunks.size(), 0);}

  typedef iterator const_iterator; // FIXME: we should duplicate the iterator class, and modify the return type of the * operator

  const_iterator begin() const
    {return iterator(const_cast<IndexSet* >(this), 0, 0);}

  const_iterator end() const
    {return iterator(const_cast<IndexSet* >(this), chunks.size(), 0);}

  size_t size() const
    {return totalSize;}

  bool empty() const
    {return totalSize == 0;}

  size_t front() const
    {jassert(chunks.size()); return chunks.front().front();}

  size_t back() const
    {jassert(chunks.size()); return chunks.back().back();}

  void append(size_t index, double minimumSparsity = 0.1);
  void addFilledChunk(size_t begin, size_t end);

  size_t getNumChunks() const
    {return chunks.size();}

  size_t getChunkBegin(size_t index) const
    {jassert(index < chunks.size()); return chunks[index].front();}

  size_t getChunkNumElements(size_t index) const
    {jassert(index < chunks.size()); return chunks[index].size();}

  size_t getChunkElement(size_t chunkIndex, size_t indexInChunk) const
    {jassert(chunkIndex < chunks.size() && indexInChunk < chunks[chunkIndex].size()); return chunks[chunkIndex][indexInChunk];}

  size_t getIndexOfFirstChunkAfter(size_t position) const;

  struct IntersectionRegion
  {
    IntersectionRegion() : begin(0), end(0), firstChunkNumber(-1), firstIndexInChunk(0), firstCount(0), secondChunkNumber(-1), secondIndexInChunk(0), secondCount(0) {}

    String toString() const;

    size_t begin;
    size_t end;

    bool hasFirst() const
      {return firstChunkNumber >= 0;}

    int firstChunkNumber; // -1 if no first chunk here
    size_t firstIndexInChunk;
    size_t firstCount;

    bool hasSecond() const
      {return secondChunkNumber >= 0;}

    int secondChunkNumber; // -1 if no second chunk here
    size_t secondIndexInChunk;
    size_t secondCount;

    bool operator ==(const IntersectionRegion& other) const
      {return begin == other.begin && end == other.end &&
            firstChunkNumber == other.firstChunkNumber && firstIndexInChunk == other.firstIndexInChunk && firstCount == other.firstCount &&
            secondChunkNumber == other.secondChunkNumber && secondIndexInChunk == other.secondIndexInChunk && secondCount == other.secondCount;}
  };

  static void getIntersectionRegions(const IndexSetPtr& first, const IndexSetPtr& second, std::vector<IntersectionRegion>& res);

  // grow size to newSize; all new samples are taken from source starting from random positions and using contiguous blocks
  // this set is assumed to be a subset of the source set
  void randomlyExpandUsingSource(ExecutionContext& context, size_t newSize, const IndexSetPtr& source, bool contiguous = true);

protected:
  friend class IndexSetClass;

  typedef std::vector<size_t> Chunk;

  std::vector<Chunk> chunks;
  size_t totalSize;

  void fillChunk(Chunk& chunk, size_t begin, size_t end);

  // the newChunk should not overlap with existing chunks
  size_t insertChunk(const Chunk& newChunk);
  void expandChunkUsingSource(Chunk& targetChunk, size_t targetSize, const IntersectionRegion& region, const IndexSetPtr& source, size_t sourceOffset = 0);

  // primitives for getIntersectionRegions()
  size_t getNextIntersection(bool insideChunk, const_iterator it) const;
  bool isInsideChunk(size_t position, size_t chunkNumber) const;
  void initializeRegion(size_t begin, size_t end, const_iterator& it, int& resChunkNumber, size_t& resIndexInChunk, size_t& resCount);
  static IntersectionRegion makeRegion(const IndexSetPtr& first, const IndexSetPtr& second, size_t begin, size_t end,
                                        bool& insideFirst, const_iterator& firstIt, bool& insideSecond, const_iterator& secondIt);
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_INDEX_SET_H_
