/*-----------------------------------------.---------------------------------.
| Filename: MorpionBoard.h                 | Solitaire Morpion Board         |
| Author  : Francis Maes                   |                                 |
| Started : 30/06/2012 12:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MORPION_BOARD_H_
# define LBCPP_MORPION_BOARD_H_

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
  
  static Direction fromString(const String& str)
  {
    if (str == T("NE")) return NE;
    if (str == T("E")) return E;
    if (str == T("SE")) return SE;
    if (str == T("S")) return S;
    return none;
  }

  bool operator ==(const MorpionDirection& other) const
    {return dir == other.dir;}

  bool operator !=(const MorpionDirection& other) const
    {return dir != other.dir;}

  bool operator ==(Direction d) const
    {return dir == d;}

  bool operator !=(Direction d) const
    {return dir != d;}

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

class MorpionBoard
{
public:
  MorpionBoard() : width(0), height(0) {}

  enum
  {
    flagOccupied = 0x01,
    flagNE = 0x02,
    flagE = 0x04,
    flagSE = 0x08,
    flagS = 0x10,
    flagNeighbor = 0x20,
  };

  void clear()
    {board.clear();}

  void initialize(size_t crossLength)
  {
    size_t dim = crossLength * 10;
    resizeBoard(dim, dim);
    int refX = crossLength * 4;
    int refY = crossLength * 4;

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
      int x = lines[i] + refX;
      int y = lines[i+1] + refY;
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

  size_t getWidth() const
    {return width;}

  size_t getHeight() const
    {return height;}

  char getState(const MorpionPoint& point) const
    {return board[point.getY() * width + point.getX()];}

  char getState(int x, int y) const
    {return board[y * width + x];}
  
  char& getState(const MorpionPoint& point)
    {return board[point.getY() * width + point.getX()];}

  char& getState(int x, int y)
    {return board[y * width + x];}

  bool isOccupied(int x, int y) const
    {return (getState(x, y) & flagOccupied) == flagOccupied;}

  bool isOccupied(const MorpionPoint& point) const
    {return isOccupied(point.getX(), point.getY());}
  
  void markAsOccupied(int x, int y, bool occupied = true)
  {
    if (occupied)
    {
      getState(x, y) |= flagOccupied;
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
      getState(x, y) &= ~flagOccupied;
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
    {return (getState(x, y) & flagNeighbor) == flagNeighbor;}

  void addSegment(const MorpionPoint& point, const MorpionDirection& direction)
    {getState(point) |= getFlag(direction);}

  void removeSegment(const MorpionPoint& point, const MorpionDirection& direction)
    {getState(point) &= ~getFlag(direction);}

  bool hasSegment(int x, int y, const MorpionDirection& direction) const
    {return (getState(x, y) & getFlag(direction)) != 0;}

  bool hasSegment(const MorpionPoint& point, const MorpionDirection& direction) const
    {return hasSegment(point.getX(), point.getY(), direction);}

  void getXYRange(int& minX, int& minY, int& maxX, int& maxY) const
  {
    minX = 0x7FFFFFFF;
    minY = 0x7FFFFFFF;
    maxX = -0x7FFFFFFF;
    maxY = -0x7FFFFFFF;
    for (size_t i = 0; i < board.size(); ++i)
      if (board[i] != 0)
      {
        int x = ((int)i % width);
        int y = ((int)i / width);
        if (x < minX) minX = x;
        if (y < minY) minY = y;
        if (x > maxX) maxX = x;
        if (y > maxY) maxY = y;
      }
  }

private:
  size_t width;
  size_t height;
  std::vector<char> board;

  void resizeBoard(size_t width, size_t height)
    {this->width = width; this->height = height; board.resize(width * height, 0);}

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
      getState(x, y) &= ~flagNeighbor;
  }

  bool computeIfNeighbor(int x, int y) const
  {
    return isOccupied(x - 1, y - 1) || isOccupied(x - 1, y) || isOccupied(x - 1, y + 1) ||
            isOccupied(x, y - 1) || isOccupied(x, y + 1) ||
            isOccupied(x + 1, y - 1) || isOccupied(x + 1, y) || isOccupied(x + 1, y + 1);
  }

  void markAsNeighbor(int x, int y)
    {getState(x, y) |= flagNeighbor;}
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
    int minX, minY, maxX, maxY;
    board.getXYRange(minX, minY, maxX, maxY);
    for (int x = minX; x <= maxX; ++x)
      for (int y = minY; y <= maxY; ++y)
        if (board.isOccupied(x, y))
        {
          BitMask mask = makeMask(board, x, y, complexity);
          size_t maskIndex = toIndex(mask);
          //if (maskIndex)
            counts[maskIndex]++;
        }
    SparseDoubleVectorPtr res = new SparseDoubleVector(simpleSparseDoubleVectorClass);
    for (std::map<size_t, size_t>::const_iterator it = counts.begin(); it != counts.end(); ++it)
      res->setElement(it->first, (double)it->second);
    return res;
  }

private:
  typedef std::pair<juce::int64, size_t> BitMask;

  size_t toIndex(const BitMask& mask) const
    {return (size_t)mask.first;}

  BitMask makeMask(const MorpionBoard& board, int x, int y, size_t complexity) const
  {
    BitMask res;
    if (complexity >= 1)
      addBits(res, board.isOccupied(x - 1, y - 1), board.isOccupied(x, y - 1), board.isOccupied(x + 1, y - 1),
                   board.isOccupied(x + 1, y),
                   board.isOccupied(x + 1, y + 1), board.isOccupied(x, y + 1), board.isOccupied(x - 1, y + 1),
                   board.isOccupied(x - 1, y));

    if (complexity >= 2)
      addBits(res, board.hasSegment(x - 1, y - 1, MorpionDirection::SE), // top-left
                    board.hasSegment(x, y - 1, MorpionDirection::S), // top
                    board.hasSegment(x, y, MorpionDirection::NE), // top-right
                    board.hasSegment(x, y, MorpionDirection::E), // right
                    board.hasSegment(x, y, MorpionDirection::SE), // bottom-right
                    board.hasSegment(x, y, MorpionDirection::S), // bottom
                    board.hasSegment(x - 1, y + 1, MorpionDirection::NE), // bottom-left
                    board.hasSegment(x - 1, y, MorpionDirection::E)); // left
    
    if (complexity >= 3)
      addBits(res, board.isOccupied(x - 2, y - 2), board.isOccupied(x, y - 2), board.isOccupied(x + 2, y - 2),
                   board.isOccupied(x + 2, y),
                   board.isOccupied(x + 2, y + 2), board.isOccupied(x, y + 2), board.isOccupied(x - 2, y + 2),
                   board.isOccupied(x - 2, y));
                   
    if (complexity >= 4)
      addBits(res, board.hasSegment(x - 1, y, MorpionDirection::NE),
                   board.hasSegment(x, y - 2, MorpionDirection::S),
                   board.hasSegment(x, y - 1, MorpionDirection::SE),
                   board.hasSegment(x + 1, y, MorpionDirection::E),
                   board.hasSegment(x, y + 1, MorpionDirection::NE),
                   board.hasSegment(x, y + 1, MorpionDirection::S),
                   board.hasSegment(x - 1, y, MorpionDirection::SE),
                   board.hasSegment(x - 2, y, MorpionDirection::E));
                   
    // canonize and add current position bit
    res = canonizeMask(res);
    //res.push_back(board.isOccupied(x, y));
    return res;
  }

  BitMask canonizeMask(const BitMask& mask) const
  {
    //std::cout << String::toHexString((int)toIndex(mask)) << " => "; 
    BitMask res = mask;

    static const int orders[] = {
      0, 1, 2, 3, 4, 5, 6, 7,
      6, 7, 0, 1, 2, 3, 4, 5,
      4, 5, 6, 7, 0, 1, 2, 3,
      2, 3, 4, 5, 6, 7, 0, 1,

      6, 5, 4, 3, 2, 1, 0, 7,
      4, 3, 2, 1, 0, 7, 6, 5,
      2, 1, 0, 7, 6, 5, 4, 3,
      0, 7, 6, 5, 4, 3, 2, 1
    };

    jassert(reverseBits(mask, orders) == mask);
    for (size_t i = 1; i < 8; ++i)
    {
      BitMask tmp = reverseBits(mask, orders + i * 8);
      //std::cout << String::toHexString((int)toIndex(tmp)) << " "; 
      if (tmp.first < res.first)
        res = tmp;
    }
    //std::cout << " ==> " << String::toHexString((int)toIndex(res)) << std::endl;
    return res;
  }

  bool getBit(const BitMask& mask, size_t index) const
  {
    jassert(index < mask.second);
    juce::int64 bit = (1 << index);
    return (mask.first & bit) == bit;
  }
  
  void addBits(BitMask& mask, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7, bool b8) const
  {
    juce::int64 m = 0;
    if (b1) m |= 0x01;
    if (b2) m |= 0x02;
    if (b3) m |= 0x04;
    if (b4) m |= 0x08;
    if (b5) m |= 0x10;
    if (b6) m |= 0x20;
    if (b7) m |= 0x40;
    if (b8) m |= 0x80;
    mask.first |= (m << mask.second);
    mask.second += 8;
    jassert(mask.second <= 64);
  }

  void reverseBits(const BitMask& mask, size_t offset, const int order[8], BitMask& res) const
  {
    addBits(res,
      getBit(mask, order[0] + offset), 
      getBit(mask, order[1] + offset), 
      getBit(mask, order[2] + offset), 
      getBit(mask, order[3] + offset), 
      getBit(mask, order[4] + offset), 
      getBit(mask, order[5] + offset), 
      getBit(mask, order[6] + offset), 
      getBit(mask, order[7] + offset));
  }

  BitMask reverseBits(const BitMask& mask, const int order[8]) const
  {
    jassert(mask.second % 8 == 0);
    BitMask res;
    for (size_t i = 0; i < mask.second; i += 8)
      reverseBits(mask, i, order, res);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MORPION_BOARD_H_
