/*-----------------------------------------.---------------------------------.
| Filename: ObjectGraphComponent.h         | Object Graph Component          |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2009 13:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_OBJECT_GRAPH_H_
# define EXPLORER_COMPONENTS_OBJECT_GRAPH_H_

# include "common.h"

# ifdef JUCE_WIN32
# undef T
# undef Rectangle
#  include <windows.h>
#  include <gl/gl.h>
#  include <gl/glu.h>
# else
#  include <opengl/gl.h>
#  include <opengl/glu.h>
# endif

namespace lbcpp
{

class ObjectGraphLayouter
{
public:
  ObjectGraphLayouter(ObjectGraphPtr graph)
    : graph(graph)
  {
    std::vector<ObjectPtr> nodeObjects;
    graph->enumerateNodes(nodeObjects, inverseTable);
    nodes.resize(nodeObjects.size());
    RandomGenerator& random = RandomGenerator::getInstance();
    for (size_t i = 0; i < nodes.size(); ++i)
    {
      Node& n = nodes[i];
      n.object = nodeObjects[i];
      n.position = Point(random.sampleFloat(0, 100.f), random.sampleFloat(0, 100.f));
    }
   // std::cout << "Num Nodes: " << nodes.size() << std::endl;
  }
  
  void update()
  {
    /*float energy = */layoutIteration();
    //std::cout << "Energy = " << energy << std::endl;
  }
  
  /*
  ** Accessors
  */
  size_t getNumNodes() const
    {return nodes.size();}

  ObjectPtr getNode(size_t index) const
    {return nodes[index].object;}

  Point getNodePosition(size_t index) const
    {return nodes[index].position;}
    
  size_t getNumSuccessors(size_t node) const
    {return graph->getNumSuccessors(getNode(node));}
    
  size_t getSuccessor(size_t node, size_t index) const
  {
    std::map<ObjectPtr, size_t>::const_iterator it = inverseTable.find(graph->getSuccessor(getNode(node), index));
    jassert(it != inverseTable.end());
    return it->second;
  }
  
  /*
  ** Operation on nodes
  */
  void setNodePosition(size_t index, const Point& position)
  {
    nodes[index].position = position;
    nodes[index].velocity = Point(); // reset velocity
  }

  // returns -1 if the nearest point is farther from position than maxDistance
  int getNearestNode(const Point& position, double maxDistance) const
  {
    int nearest = -1;
    float minDistance = FLT_MAX;
    for (size_t i = 0; i < nodes.size(); ++i)
    {
      float distance = sumOfSquares(substract(position, nodes[i].position));
      if (distance < minDistance)
        minDistance = distance, nearest = (int)i;
    }
    return minDistance < maxDistance ? nearest : -1;
  }

private:
  ObjectGraphPtr graph;
  
  struct Node
  {
    ObjectPtr object;
    Point position;
    Point velocity;
  };
  std::vector<Node> nodes;
  std::map<ObjectPtr, size_t> inverseTable;
  
  static Point substract(const Point& a, const Point& b)
    {return Point(a.getX() - b.getX(), a.getY() - b.getY());}

  static void addWeighted(Point& a, const Point& b, float k)
    {a.setXY(a.getX() + b.getX() * k, a.getY() + b.getY() * k);}
  
  static void multiplyByScalar(Point& p, float k)
    {p.setXY(p.getX() * k, p.getY() * k);}
          
  static float sumOfSquares(const Point& p)
    {return p.getX() * p.getX() + p.getY() * p.getY();}
  
  float layoutIteration()
  {
    static const float minimumSquaredDistance = 0.01f;
    static const float maximumSquaredDistance = 2500.f;
    static const float timeStep = 1.f;
    static const float nodeMass = 0.1f;
    static const float damping = 0.9f;
    static const float edgesK = 0.01f;

    std::vector<Point> forces(nodes.size(), Point(0.f, 0.f));

    for (size_t i = 0; i < nodes.size(); ++i)
    {
      //std::cout << "Node " << (i + 1) << " / " << n.size() << " px = " << n[i].px << " py = " << n[i].py << " vx = " << n[i].vx << " vy = " << n[i].vy << std::endl;
      
      // other nodes
      //std::cout << "Other nodes: ";
      for (size_t j = 0; j < nodes.size(); ++j)
      {
        if (i == j)
          continue;
        
        Point deltaPosition = substract(nodes[i].position, nodes[j].position);
        float squaredDistance = jmax(minimumSquaredDistance, sumOfSquares(deltaPosition));
        if (squaredDistance < maximumSquaredDistance)
          addWeighted(forces[i], deltaPosition, 1.f / squaredDistance);
        //std::cout << forceX << ", " << forceY << " ";
      }
      //std::cout << std::endl;
      //std::cout << "\tfx = " << forceX << " fy = " << forceY << std::endl;
      
      // successors
      //std::cout << "Successors: ";
      size_t count = getNumSuccessors(i);
      for (size_t j = 0; j < count; ++j)
      {
        size_t successor = getSuccessor(i, j);
        Point deltaPosition = substract(nodes[successor].position, nodes[i].position);
        addWeighted(forces[i], deltaPosition, edgesK);
        addWeighted(forces[successor], deltaPosition, -edgesK);
        //std::cout << forceX << ", " << forceY << " ";
      }
      //std::cout << std::endl;
    }
    
    float energy = 0.0;
//    std::cout << "FORCES: ";
    for (size_t i = 0; i < nodes.size(); ++i)
    {
  //    std::cout << forces[i].getX() << ", " << forces[i].getY() << " ";
      addWeighted(nodes[i].velocity, forces[i], timeStep);
      multiplyByScalar(nodes[i].velocity, damping);
      addWeighted(nodes[i].position, nodes[i].velocity, timeStep);
      energy += nodeMass * sumOfSquares(nodes[i].velocity);
    }
//   std::cout << std::endl;
    return energy;
  }
};

class ObjectGraphComponent : public OpenGLComponent, public Timer
{
public:
  ObjectGraphComponent(ObjectGraphPtr graph)
    : layouter(graph), viewportCenter(50, 50), pixelsPerUnit(10), selectedNode(-1)
  {
    startTimer(20);
  }
  
  virtual void selectionChanged(int selectedNode) {}
  
  virtual void renderOpenGL()
  {
    layouter.update();
    if (selectedNode >= 0 && isMouseButtonDown())
    {
      int x, y;
      getMouseXYRelative(x, y);
      layouter.setNodePosition(selectedNode, pixelsToLogicalPosition(Point((float)x, (float)y)));
    }
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, getWidth(), getHeight(), 0, -100, 100);

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    renderLinks(layouter);
    renderNodes(layouter);
  }
  
  void renderLinks(ObjectGraphLayouter& layouter)
  {
    size_t numNodes = layouter.getNumNodes();

    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
    for (size_t i = 0; i < numNodes; ++i)
    {
      size_t numSuccessors = layouter.getNumSuccessors(i);
      Point pos1 = layouter.getNodePosition(i);
      for (size_t j = 0; j < numSuccessors; ++j)
      {
        Point pos2 = layouter.getNodePosition(layouter.getSuccessor(i, j));
        glVertex(pos1);
        glVertex(pos2);
      }
    }
    glEnd();
  }

  void renderNodes(ObjectGraphLayouter& layouter)
  {
    static const float nodeColors[] = {0, 0, 0,  1, 0, 0,  0, 1, 0,  0, 0, 1};
    static const size_t numColors = sizeof (nodeColors) / (3 * sizeof (float));

    size_t numNodes = layouter.getNumNodes();
    
    glPointSize(10.f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < numNodes; ++i)
    {
      Point pos = layouter.getNodePosition(i);
      size_t classNumber = getNodeClassNumber(layouter.getNode(i)->getClassName());
      const float* c = nodeColors + (classNumber % numColors) * 3;
      glColor3f(c[0], c[1], c[2]);
      glVertex(pos);
    }
    glEnd();
    if (selectedNode >= 0)
    {
      glPointSize(20.f);
      glBegin(GL_POINTS);
      Point pos = layouter.getNodePosition(selectedNode);
      size_t classNumber = getNodeClassNumber(layouter.getNode(selectedNode)->getClassName());
      const float* c = nodeColors + (classNumber % numColors) * 3;
      glColor3f(c[0], c[1], c[2]);
      glVertex(pos);
      glEnd();
    }
  }

  virtual void newOpenGLContextCreated()
  {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, -100, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }
  
  virtual void timerCallback()
    {repaint();}
  
  virtual void mouseDown(const MouseEvent& e)
  {
    selectedNode = hitTestNode(e.getMouseDownX(), e.getMouseDownY());
    selectionChanged(selectedNode);
    mouseDownViewportCenter = viewportCenter;
  }
  
  virtual void mouseDrag(const MouseEvent& e)
  {
    if (selectedNode >= 0)
    {
      Point pixelsPosition((float)(e.getMouseDownX() + e.getDistanceFromDragStartX()),
                        (float)(e.getMouseDownY() + e.getDistanceFromDragStartY()));
      layouter.setNodePosition(selectedNode, pixelsToLogicalPosition(pixelsPosition));
    }
    else
      viewportCenter.setXY(mouseDownViewportCenter.getX() - e.getDistanceFromDragStartX() / pixelsPerUnit,
          mouseDownViewportCenter.getY() - e.getDistanceFromDragStartY() / pixelsPerUnit);
  }
  
  virtual void mouseUp(const MouseEvent& e)
  {
  }
    
  virtual void mouseWheelMove(const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
  {
    if (wheelIncrementY)
      pixelsPerUnit *= jlimit(0.00001f, 10000.f, powf(2.f, wheelIncrementY));
  }
  
protected:
  ObjectGraphLayouter layouter;
  Point viewportCenter;
  Point mouseDownViewportCenter;
  float pixelsPerUnit;
  int selectedNode;

  Point pixelsToLogicalPosition(const Point& p) const
  {
    return Point(
      (p.getX() - getWidth() / 2.f) / pixelsPerUnit + viewportCenter.getX(),
      (p.getY() - getHeight() / 2.f) / pixelsPerUnit + viewportCenter.getY());
  }
  
  Point logicalPositionToPixels(const Point& p) const
  {
    return Point(
        (p.getX() - viewportCenter.getX()) * pixelsPerUnit + getWidth() / 2.f,
        (p.getY() - viewportCenter.getY()) * pixelsPerUnit + getHeight() / 2.f);
  }
  
  void glVertex(const Point& p)
  {
    Point pp = logicalPositionToPixels(p);
    glVertex2f(pp.getX(), pp.getY());
  }

  int hitTestNode(int x, int y) const
  {
    static const int maxDistanceInPixels = 15;
    Point p = pixelsToLogicalPosition(Point((float)x, (float)y));
    return layouter.getNearestNode(p, maxDistanceInPixels / pixelsPerUnit);
  }
  
  std::map<String, size_t> nodeClassNumbers;

  size_t getNodeClassNumber(const String& className)
  {
    std::map<String, size_t >::iterator it = nodeClassNumbers.find(className);
    if (it == nodeClassNumbers.end())
    {
      size_t numClasses = nodeClassNumbers.size();
      return (nodeClassNumbers[className] = numClasses);
    }
    else
      return it->second;
  }
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_OBJECT_GRAPH_H_

