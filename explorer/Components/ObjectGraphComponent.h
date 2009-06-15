/*-----------------------------------------.---------------------------------.
| Filename: ObjectGraphComponent.h         | Object Graph Component          |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2009 13:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_OBJECT_GRAPH_H_
# define EXPLORER_COMPONENTS_OBJECT_GRAPH_H_

# include "../Juce/juce_amalgamated.h"
# include <lbcpp/lbcpp.h>
# include <opengl/gl.h>
# include <opengl/glu.h>

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
    Random& random = Random::getInstance();
    for (size_t i = 0; i < nodes.size(); ++i)
    {
      Node& n = nodes[i];
      n.object = nodeObjects[i];
      n.position = Point(random.sampleFloat(), random.sampleFloat());
    }
   // std::cout << "Num Nodes: " << nodes.size() << std::endl;
  }
  
  void update()
  {
    static const float damping = 0.9;
    /*float energy = */layoutIteration(damping);
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
    assert(it != inverseTable.end());
    return it->second;
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
  
  float layoutIteration(float damping)
  {
    static const float minimumSquaredDistance = 0.01;
    static const float maximumSquaredDistance = 2500;
    static const float timeStep = 1.0;
    static const float nodeMass = 1.0;

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
          addWeighted(forces[i], deltaPosition, 1.0 / squaredDistance);
        //std::cout << forceX << ", " << forceY << " ";
      }
      //std::cout << std::endl;
      //std::cout << "\tfx = " << forceX << " fy = " << forceY << std::endl;
      
      // successors
      //std::cout << "Successors: ";
      size_t count = getNumSuccessors(i);
      static const float edgesK = 0.01;
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
  ObjectGraphComponent(ObjectGraphPtr graph) : layouter(graph), viewportCenter(0.5f, 0.5f), pixelsPerUnit(10)
  {
    startTimer(20);
  }
  
  virtual void renderOpenGL()
  {
    layouter.update();

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
    size_t numNodes = layouter.getNumNodes();

    glPointSize(10.f);
    glColor3f(0, 0, 0); 
    glBegin(GL_POINTS);
    for (size_t i = 0; i < numNodes; ++i)
    {
      Point pos = layouter.getNodePosition(i);
      glVertex(pos);
    }
    glEnd();
  }
  
  void glVertex(const Point& p)
  {
    float x = (p.getX() - viewportCenter.getX()) * pixelsPerUnit / getWidth() + 0.5f;
    float y = (p.getY() - viewportCenter.getY()) * pixelsPerUnit / getHeight() + 0.5f;
    glVertex2f(x, y);
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
    
private:
  ObjectGraphLayouter layouter;
  Point viewportCenter;
  float pixelsPerUnit;  
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_OBJECT_GRAPH_H_

