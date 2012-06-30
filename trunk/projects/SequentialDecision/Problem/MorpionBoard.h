/*-----------------------------------------.---------------------------------.
| Filename: MorpionBoard.h                 | Solitaire Morpion Board         |
| Author  : Francis Maes                   |                                 |
| Started : 30/06/2012 12:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_MORPION_BOARD_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_MORPION_BOARD_H_

# include <lbcpp/Core/Object.h>

namespace lbcpp
{

/*
** Direction
*/
class MorpionDirection
{
public:
  enum Direction
  {
    NE = 0, E, SE, S, none
  };

  MorpionDirection(Direction direction)
    : dir(direction) {}
  MorpionDirection() : dir(none) {}

  operator Direction () const
    {return dir;}

  int getDx() const
    {static int dxs[] = {1, 1, 1, 0, 0}; return dxs[dir];}

  int getDy() const
    {static int dys[] = {-1, 0, 1, 1, 0}; return dys[dir];}

  String toString() const
    {static String strs[] = {T("NE"), T("E"), T("SE"), T("S"), T("none")}; return strs[dir];}
    
  bool operator ==(const MorpionDirection& other) const
    {return dir == other.dir;}

  bool operator !=(const MorpionDirection& other) const
    {return dir != other.dir;}

private:
  Direction dir;
};

/*
** Point
*/
class MorpionPoint
{
public:
  MorpionPoint(int x, int y)
    : x(x), y(y) {}
  MorpionPoint() : x(0), y(0) {}

  int getX() const
    {return x;}

  int getY() const
    {return y;}

  String toString() const
    {return T("(") + String(x) + T(", ") + String(y) + T(")");}

  MorpionPoint moveIntoDirection(const MorpionDirection& direction, int delta) const
    {return MorpionPoint(x + delta * direction.getDx(), y + delta * direction.getDy());}
  
  void incrementIntoDirection(const MorpionDirection& direction)
    {x += direction.getDx(); y += direction.getDy();}

  bool operator ==(const MorpionPoint& other) const
    {return x == other.x && y == other.y;}
    
  bool operator !=(const MorpionPoint& other) const
    {return x != other.x || y != other.y;}

private:
  int x, y;
};

/*
** Board
*/
template<class T>
class MorpionBidirectionalVector
{
public:
  MorpionBidirectionalVector(size_t initialNegSize, size_t initialPosSize, const T& initialValue)
    : neg(initialNegSize, initialValue), pos(initialPosSize, initialValue), initialValue(initialValue) {}
  MorpionBidirectionalVector() {}

  const T& operator [](int index) const
    {return index >= 0 ? get(pos, posIndex(index)) : get(neg, negIndex(index));}
  
  T& operator [](int index)
    {return index >= 0 ? resizeAndGet(pos, posIndex(index)) : resizeAndGet(neg, negIndex(index));}

  size_t getPositiveSize() const
    {return pos.size();}

  size_t getNegativeSize() const
    {return neg.size();}

  int getMinIndex() const
    {return -(int)neg.size();}

  int getMaxIndex() const
    {return pos.size() - 1;}

private:
  std::vector<T> neg;
  std::vector<T> pos;
  T initialValue;

  inline size_t posIndex(int index) const
    {return (size_t)index;}

  inline size_t negIndex(int index) const
    {return (size_t)(-(index + 1));}

  const T& get(const std::vector<T>& v, size_t index) const
    {return index < v.size() ? v[index] : initialValue;}

  T& resizeAndGet(std::vector<T>& v, size_t index)
  {
    if (v.size() <= index)
      v.resize(index + 1, initialValue);
    return v[index];
  }
};

class MorpionBoard
{
public:
  MorpionBoard() : b(0, 0, MorpionBidirectionalVector< char >(0, 0, 0)) {}

  enum
  {
    flagOccupied = 0x01,
    flagNE = 0x02,
    flagE = 0x04,
    flagSE = 0x08,
    flagS = 0x10,
    flagNeighbor = 0x20,
  };

  void initialize(size_t crossLength)
  {
    int n = crossLength - 2;
    int lines[] = {
            n, 0, 1, 0,
      0, n, 1, 0,    2 * n, n, 1, 0,
      0, 2 * n, 1, 0,    2 * n, 2 * n, 1, 0,
            n, 3 * n, 1, 0,
      n, 0, 0, 1,     2 * n, 0, 0, 1,
      0, n, 0, 1,     3 * n, n, 0, 1,
      n, 2 * n, 0, 1, 2 * n, 2 * n, 0, 1
    };

    for (size_t i = 0; i < sizeof (lines) / sizeof (int); i += 4)
    {
      int x = lines[i];
      int y = lines[i+1];
      int dx = lines[i+2];
      int dy = lines[i+3];
      for (int j = 0; j <= n; ++j)
      {
        markAsOccupied(x, y);
        x += dx;
        y += dy;
      }
    }
  }

  char getState(int x, int y) const
    {return b[x][y];}

  bool isOccupied(int x, int y) const
    {return (b[x][y] & flagOccupied) == flagOccupied;}

  bool isOccupied(const MorpionPoint& point) const
    {return isOccupied(point.getX(), point.getY());}
  
  void markAsOccupied(int x, int y, bool occupied = true)
  {
    if (occupied)
    {
      b[x][y] |= flagOccupied;
      markAsNeighbor(x - 1, y - 1);
      markAsNeighbor(x - 1, y);
      markAsNeighbor(x - 1, y + 1);
      markAsNeighbor(x, y - 1);
      markAsNeighbor(x, y + 1);
      markAsNeighbor(x + 1, y - 1);
      markAsNeighbor(x + 1, y);
      markAsNeighbor(x + 1, y + 1);
    }
    else
    {
      b[x][y] &= ~flagOccupied;
      undoNeighborState(x - 1, y - 1);
      undoNeighborState(x - 1, y);
      undoNeighborState(x - 1, y + 1);
      undoNeighborState(x, y - 1);
      undoNeighborState(x, y + 1);
      undoNeighborState(x + 1, y - 1);
      undoNeighborState(x + 1, y);
      undoNeighborState(x + 1, y + 1);
    }
  }

  void markAsOccupied(const MorpionPoint& point, bool occupied = true)
    {markAsOccupied(point.getX(), point.getY(), occupied);}

  bool isNeighbor(int x, int y) const
    {return (b[x][y] & flagNeighbor) == flagNeighbor;}

  void addSegment(const MorpionPoint& point, const MorpionDirection& direction)
    {b[point.getX()][point.getY()] |= getFlag(direction);}

  void removeSegment(const MorpionPoint& point, const MorpionDirection& direction)
    {b[point.getX()][point.getY()] &= ~getFlag(direction);}

  bool hasSegment(int x, int y, const MorpionDirection& direction) const
    {return (b[x][y] & getFlag(direction)) != 0;}

  bool hasSegment(const MorpionPoint& point, const MorpionDirection& direction) const
    {return hasSegment(point.getX(), point.getY(), direction);}

  int getMinX() const
    {return b.getMinIndex();}

  int getMaxX() const
    {return b.getMaxIndex();}

  int getMinY(int x) const
    {return b[x].getMinIndex();}

  int getMaxY(int x) const
    {return b[x].getMaxIndex();}

  void getXRange(int& minIndex, int& maxIndex) const
    {minIndex = getMinX(); maxIndex = getMaxX();}

  void getYRange(int& minIndex, int& maxIndex) const
  {
    int x1 = getMinX();
    int x2 = getMaxX();
    minIndex = 0x7FFFFFFF;
    maxIndex = -0x7FFFFFFF;
    for (int x = x1; x <= x2; ++x)
    {
      minIndex = juce::jmin(minIndex, getMinY(x));
      maxIndex = juce::jmax(maxIndex, getMaxY(x));
    }
  }

private:
  MorpionBidirectionalVector< MorpionBidirectionalVector< char > > b;

  static int getFlag(const MorpionDirection::Direction& dir)
  {
    switch (dir)
    {
    case MorpionDirection::NE: return flagNE;
    case MorpionDirection::E: return flagE;
    case MorpionDirection::SE: return flagSE;
    case MorpionDirection::S: return flagS;
    default: jassert(false); return 0;
    }
  }

  void undoNeighborState(int x, int y)
  {
    if (!computeIfNeighbor(x, y))
      b[x][y] &= ~flagNeighbor;
  }

  bool computeIfNeighbor(int x, int y) const
  {
    return isOccupied(x - 1, y - 1) || isOccupied(x - 1, y) || isOccupied(x - 1, y + 1) ||
            isOccupied(x, y - 1) || isOccupied(x, y + 1) ||
            isOccupied(x + 1, y - 1) || isOccupied(x + 1, y) || isOccupied(x + 1, y + 1);
  }

  void markAsNeighbor(int x, int y)
    {b[x][y] |= flagNeighbor;}
};

/*
** Features
*/
class MorpionFeatures
{
public:
  SparseDoubleVectorPtr compute(const MorpionBoard& board, size_t complexity) const
  {
    std::map<size_t, size_t> counts;
    for (int x = board.getMinX(); x <= board.getMaxX(); ++x)
      for (int y = board.getMinY(x); y <= board.getMaxY(x); ++y)
      {
        BitMask mask = makeMask(board, x, y, complexity);
        size_t maskIndex = toIndex(mask);
        if (maskIndex)
          counts[maskIndex]++;
      }
    SparseDoubleVectorPtr res = new SparseDoubleVector(simpleSparseDoubleVectorClass);
    for (std::map<size_t, size_t>::const_iterator it = counts.begin(); it != counts.end(); ++it)
      res->setElement(it->first, (double)it->second);
    return res;
  }

private:
  typedef std::vector<bool> BitMask;

  size_t toIndex(const BitMask& mask) const
  {
    size_t res = 0;
    size_t bit = 1;
    for (int i = mask.size() - 1; i >= 0; --i)
    {
      if (mask[i])
        res |= bit;
      bit <<= 1;
    }
    return res;
  }

  void addBits(BitMask& mask, bool b1, bool b2, bool b3, bool b4) const
  {
    mask.push_back(b1);
    mask.push_back(b2);
    mask.push_back(b3);
    mask.push_back(b4);
  }

  BitMask makeMask(const MorpionBoard& board, int x, int y, size_t complexity) const
  {
    BitMask res;
    res.reserve(complexity * 4 + 1);

    if (complexity >= 1)
      addBits(res, board.isOccupied(x - 1, y), // left
                   board.isOccupied(x, y - 1), // top
                   board.isOccupied(x + 1, y), // right
                   board.isOccupied(x, y + 1)); // bottom
    if (complexity >= 2)
      addBits(res, board.hasSegment(x - 1, y, MorpionDirection::E), // left
                   board.hasSegment(x, y - 1, MorpionDirection::S), // top
                   board.hasSegment(x, y, MorpionDirection::E), // right
                   board.hasSegment(x, y, MorpionDirection::S)); // bottom
    if (complexity >= 3)
      addBits(res, board.isOccupied(x - 1, y - 1), // top-left
                   board.isOccupied(x + 1, y - 1), // top-right
                   board.isOccupied(x + 1, y + 1), // bottom-right
                   board.isOccupied(x - 1, y + 1)); // bottom-left
    if (complexity >= 4)
      addBits(res, board.hasSegment(x - 1, y - 1, MorpionDirection::SE), // top-left
                   board.hasSegment(x, y, MorpionDirection::NE), // top-right
                   board.hasSegment(x, y, MorpionDirection::SE), // bottom-right
                   board.hasSegment(x - 1, y + 1, MorpionDirection::NE)); // bottom-left
    if (complexity >= 5)
      addBits(res, board.isOccupied(x - 2, y), // left
                   board.isOccupied(x, y - 2), // top
                   board.isOccupied(x + 2, y), // right
                   board.isOccupied(x, y + 2)); // bottom
    if (complexity >= 6)
      addBits(res, board.isOccupied(x - 2, y - 2), // top-left
                   board.isOccupied(x + 2, y - 2), // top-right
                   board.isOccupied(x + 2, y + 2), // bottom-right
                   board.isOccupied(x - 2, y + 2)); // bottom-left

    // canonize and add current position bit
    res = canonizeMask(res);
    res.push_back(board.isOccupied(x, y));
    return res;
  }

  BitMask canonizeMask(const BitMask& mask) const
  {
    BitMask res = mask;
    BitMask tmp = mask;
    shiftByFour(tmp); if (tmp < res) res = tmp;
    shiftByFour(tmp); if (tmp < res) res = tmp;
    shiftByFour(tmp); if (tmp < res) res = tmp;
    jassert((res < tmp) == (toIndex(res) < toIndex(tmp)));
    
    tmp = mask;
    invertByFour(tmp); if (tmp < res) res = tmp;
    shiftByFour(tmp); if (tmp < res) res = tmp;
    shiftByFour(tmp); if (tmp < res) res = tmp;
    shiftByFour(tmp); if (tmp < res) res = tmp;
    jassert((res < tmp) == (toIndex(res) < toIndex(tmp)));
    return res;
  }

  void shiftByFour(BitMask& mask) const
  {
    jassert(mask.size() % 4 == 0);
    for (size_t i = 0; i < mask.size(); i += 4)
    {
      bool tmp = mask[i];
      mask[i] = mask[i+1];
      mask[i+1] = mask[i+2];
      mask[i+2] = mask[i+3];
      mask[i+3] = tmp;
    }
  }

  void invertByFour(BitMask& mask) const
  {
    jassert(mask.size() % 4 == 0);
    for (size_t i = 0; i < mask.size(); i += 4)
    {
      bool tmp = mask[i];
      mask[i] = mask[i+3];
      mask[i+3] = tmp;
      tmp = mask[i+1];
      mask[i+1] = mask[i+2];
      mask[i+2] = tmp;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_MORPION_BOARD_H_
