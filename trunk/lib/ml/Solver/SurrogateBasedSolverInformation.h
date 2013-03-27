/*--------------------------------------------.-------------------------------------.
| Filename: SurrogateBasedSolverInformation.h | Surrogate Based Solver Information  |
| Author  : Francis Maes                      |									               	    |
| Started : 26/03/2013 19:13                  |										                  |
`---------------------------------------------/                                     |
                               |                                                    |
                               `---------------------------------------------------*/

#ifndef ML_SOLVER_SURROGATE_BASED_INFORMATION_H_
# define ML_SOLVER_SURROGATE_BASED_INFORMATION_H_

# include <ml/Solver.h>
# include <ml/Problem.h>

# include <oil/UserInterface/ObjectComponent.h>
# include "../oil/UserInterface/Plot/TwoDimensionalPlotDrawable.h"

namespace lbcpp
{

// TODO-eventually: better color-scales
// TODO: understand the strange behavior on problem 3
class SurrogateBasedSolverInformation : public Object
{
public:
  SurrogateBasedSolverInformation(size_t stepNumber = 0)
    : stepNumber(stepNumber), drawSolutions(true), drawObjective(true), drawModel(false), drawSelectionCriterion(false), drawStddev(false) {}

  ProblemPtr getProblem() const
    {return problem;}

  void setProblem(ProblemPtr problem)
    {this->problem = problem;}

  SolutionVectorPtr getSolutions() const
    {return solutions;}

  void setSolutions(SolutionVectorPtr solutions)
    {this->solutions = solutions;}

  ExpressionPtr getSurrogateModel() const
    {return surrogateModel;}

  void setSurrogateModel(ExpressionPtr surrogateModel)
    {this->surrogateModel = surrogateModel;}

  SelectionCriterionPtr getSelectionCriterion() const
    {return selectionCriterion;}

  void setSelectionCriterion(SelectionCriterionPtr selectionCriterion)
    {this->selectionCriterion = selectionCriterion;}

  size_t getStepNumber() const
    {return stepNumber;}

  bool getDrawSolutions() const
    {return drawSolutions;}

  void setDrawSolutions(bool draw)
    {drawSolutions = draw;}

  bool getDrawObjective() const
    {return drawObjective;}

  void setDrawObjective(bool draw)
    {drawObjective = draw;}

  bool getDrawModel() const
    {return drawModel;}
  
  void setDrawModel(bool draw)
    {drawModel = draw;}

  bool getDrawSelectionCriterion() const
    {return drawSelectionCriterion;}

  void setDrawSelectionCriterion(bool draw)
    {drawSelectionCriterion = draw;}

  bool getDrawStddev() const
    {return drawStddev;}

  void setDrawStddev(bool draw)
    {drawStddev = draw;}

private:
  friend class SurrogateBasedSolverInformationClass;

  bool drawSolutions;
  bool drawObjective;
  bool drawModel;
  bool drawSelectionCriterion;
  bool drawStddev;

  ProblemPtr problem;
  SolutionVectorPtr solutions;
  ExpressionPtr surrogateModel;
  SelectionCriterionPtr selectionCriterion;
  size_t stepNumber;
};

typedef ReferenceCountedObjectPtr<SurrogateBasedSolverInformation> SurrogateBasedSolverInformationPtr;

class SurrogateBasedSolverInformation2DDrawable : public TwoDimensionalPlotDrawable
{
public:
  SurrogateBasedSolverInformation2DDrawable(SurrogateBasedSolverInformationPtr information) : information(information)
  {
    xAxis = makeAxis(information->getSolutions(), 0);
    yAxis = makeAxis(information->getSolutions(), 1);
    computeBounds();
  }

  virtual Drawable* createCopy() const
    {return new SurrogateBasedSolverInformation2DDrawable(information);}

  virtual PlotAxisPtr getXAxis() const
    {return xAxis;}

  virtual PlotAxisPtr getYAxis() const
    {return yAxis;}

  virtual void draw(juce::Graphics& g, const juce::AffineTransform& transform = juce::AffineTransform::identity) const
  {
    if (!areBoundsValid())
      return;

    drawBackgroundLayer(g, transform);

    TwoDimensionalPlotDrawable::draw(g, transform);

    SolutionVectorPtr solutions = information->getSolutions();
    for (size_t i = 0; i < information->getStepNumber(); ++i)
    {
      DenseDoubleVectorPtr vector = solutions->getSolution(i).staticCast<DenseDoubleVector>();
      int x, y;
      getPixelPosition(vector, x, y, transform);

      juce::uint8 k = (juce::uint8)((255 * i) / information->getStepNumber());
      g.setColour(juce::Colour(k, 0, 255 - k));
      paintPoint(g, x, y);
    }
  }

protected:
  SurrogateBasedSolverInformationPtr information;
  PlotAxisPtr xAxis, yAxis;
  FitnessPtr currentFitness;

  void drawBackgroundLayer(juce::Graphics& g, const juce::AffineTransform& transform) const
  {
    juce::Rectangle rect = getFrameRectangle(transform);
    
    juce::AffineTransform inverseTransform = transform.inverted();

    double lowestValue = DBL_MAX;
    double highestValue = -DBL_MAX;

    int numSteps = 50;

    std::vector< std::vector< double > > values(numSteps + 1);

    juce::Image* imgLayer = new juce::Image(juce::Image::RGB, numSteps + 1, numSteps + 1, true);
    juce::Graphics gLayer(*imgLayer);
    //TODO check if selection of bg layer has changed, if not, we don't need to recompute the image, just resize it
    for (int y = 0; y <= numSteps; ++y)
    {
      values[y].resize(numSteps + 1);
      for (int x = 0; x <= numSteps; ++x)
      {
        double vx = rect.getX() + (double)x * rect.getWidth() / numSteps;
        double vy = rect.getY() + (double)y * rect.getHeight() / numSteps;
        inverseTransform.transformPoint(vx, vy);

        DenseDoubleVectorPtr vector(new DenseDoubleVector(2, 0.0));
        vector->setValue(0, vx);
        vector->setValue(1, vy);
        double value = 0.0;
        if (information->getDrawObjective())
          value = information->getProblem()->getObjective(0)->evaluate(defaultExecutionContext(), vector);
        if (information->getDrawModel())
        {
          std::vector<ObjectPtr> ovector(vector->getNumElements());
          for (size_t i = 0; i < vector->getNumElements(); ++i)
            ovector[i] = vector->getElement(i);
          value = information->getSurrogateModel()->compute(defaultExecutionContext(), ovector)->toDouble();
        }
        if (information->getDrawStddev())
        {
          std::vector<ObjectPtr> ovector(vector->getNumElements());
          for (size_t i = 0; i < vector->getNumElements(); ++i)
            ovector[i] = vector->getElement(i);
          value = information->getSurrogateModel()->compute(defaultExecutionContext(), ovector).staticCast<ScalarVariableMeanAndVariance>()->getStandardDeviation();
        }
        if (information->getDrawSelectionCriterion())
        {
          std::vector<ObjectPtr> ovector(vector->getNumElements());
          for (size_t i = 0; i < vector->getNumElements(); ++i)
            ovector[i] = vector->getElement(i);
          value = information->getSelectionCriterion()->evaluate(defaultExecutionContext(), information->getSurrogateModel()->compute(defaultExecutionContext(), ovector));
        }
        values[y][x] = value;

        if (value < lowestValue)
          lowestValue = value;
        if (value > highestValue)
          highestValue = value;
      }
    }

    double invDelta = highestValue > lowestValue ? 1.0 / (highestValue - lowestValue) : 1.0;
    for (int y = 0; y <= numSteps; ++y)
      for (int x = 0; x <= numSteps; ++x)
      {
        double normalizedValue = values[y][x] * invDelta;
        if (normalizedValue > 1.0)
          normalizedValue = 1.0;
        if (normalizedValue < 0.0)
          normalizedValue = 0.0;
        jassert(normalizedValue >= 0.0 && normalizedValue <= 1.0);
        juce::uint8 level = (juce::uint8)(255 * normalizedValue);
        gLayer.setColour(juce::Colour(level, level, level));
        gLayer.setPixel(x, y);
      }

    g.drawImage(imgLayer, rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight(), 0, 0, numSteps, numSteps);
  }
  
  void getPixelPosition(DenseDoubleVectorPtr vector, int& x, int& y, const juce::AffineTransform& transform) const
  {
    double dx = vector->getValue(0);
    double dy = vector->getValue(1);
    transform.transformPoint(dx, dy);
    x = (int)dx;
    y = (int)dy;
  }

  void paintPoint(juce::Graphics& g, int x, int y) const
  {
    if (information->getStepNumber() < 200)
    {
      const int pointHalfSize = 4;
      const float lineWidth = 2.f;

      float x1 = (float)(x - pointHalfSize);
      float y1 = (float)(y - pointHalfSize);
      float x2 = (float)(x + pointHalfSize);
      float y2 = (float)(y + pointHalfSize);
      g.drawLine(x1, y1, x2 + 1.f, y2 + 1.f, lineWidth);
      g.drawLine(x1, y2, x2 + 1.f, y1 + 1.f, lineWidth);
    }
    else
      g.setPixel(x, y);
  }
  
  PlotAxisPtr makeAxis(SolutionVectorPtr solutions, size_t inputDimension) const
  {
    double rangeMin = DBL_MAX;
    double rangeMax = -DBL_MAX;

    EnumerationPtr elementsEnumeration;
    for (size_t i = 0; i < solutions->getNumSolutions(); ++i)
    {
      DenseDoubleVectorPtr vector = solutions->getSolution(i).staticCast<DenseDoubleVector>();
      if (i == 0)
        elementsEnumeration = vector->getElementsEnumeration();
      jassert(inputDimension < vector->getNumValues());
      double value = vector->getValue(inputDimension);
      if (value < rangeMin)
        rangeMin = value;
      if (value > rangeMax)
        rangeMax = value;
    }
    return new PlotAxis(rangeMin, rangeMax, elementsEnumeration->getElementName(inputDimension), false);
  }
};

// TODO: add the possibility to draw the selection criterion (prediction, stddev, selection criterions that is used)
class SurrogateBasedSolverInformationConfigurationComponent : public Component, public juce::ChangeBroadcaster,
                                                              public juce::ComboBoxListener
{
public:
  SurrogateBasedSolverInformationConfigurationComponent(SurrogateBasedSolverInformationPtr information) 
    : information(information)
  {
    addAndMakeVisible(layerComboBox = createComboBox());
  }

  virtual ~SurrogateBasedSolverInformationConfigurationComponent()
    {deleteAllChildren();}

  virtual void comboBoxChanged(juce::ComboBox* box)
  {
    information->setDrawObjective(false);
    information->setDrawModel(false);
    information->setDrawSelectionCriterion(false);
    information->setDrawStddev(false);
    switch(box->getSelectedId()) 
    {
      case 1: information->setDrawObjective(true); break;
      case 2: information->setDrawModel(true); break;
      case 3: information->setDrawSelectionCriterion(true); break;
      case 4: information->setDrawStddev(true); break;
      default: jassertfalse;
    }
    sendSynchronousChangeMessage(this);
  }

  virtual void resized()
  {
    layerComboBox->setBounds(10, 10, 250, 30);
  }

private:
  SurrogateBasedSolverInformationPtr information;

  juce::ComboBox* layerComboBox;

  juce::ComboBox* createComboBox()
  {
    juce::ComboBox* res = new juce::ComboBox(T("layerSelectionCombo"));
    res->addItem(T("Objective"), 1);
    res->addItem(T("Surrogate model"), 2);
    res->addItem(T("Selection criterion"), 3);
    res->addItem(T("Stddev"), 4);
    res->addListener(this);
    res->setSelectedId(1);
    return res;
  }
};

class SurrogateBasedSolverInformationContentComponent : public juce::Component
{
public:
  SurrogateBasedSolverInformationContentComponent(SurrogateBasedSolverInformationPtr information)
    : information(information)
  {
    jassert(information->getProblem()->getNumObjectives() == 1);
    jassert(information->getProblem()->getDomain().staticCast<ScalarVectorDomain>()->getNumDimensions() == 2); // asserts scalarvector domain and input type
    drawable = new SurrogateBasedSolverInformation2DDrawable(information);
  }

  virtual void paint(juce::Graphics& g)
  {
    if (information->getSolutions()->isEmpty())
    {
      paintText(g, "Empty Solutions");
      return;
    }
    if (!drawable)
    {
      paintText(g, "This case is not supported yet");
      return;
    }

    if (getWidth() > leftMargin + rightMargin && getHeight() > topMargin + bottomMargin)
      drawable->draw(g, getTransform());
  }

private:
  SurrogateBasedSolverInformation2DDrawable* drawable;
  SurrogateBasedSolverInformationPtr information;

  void paintText(juce::Graphics& g, const string& text)
  {
    g.setColour(juce::Colours::grey);
    g.drawText(text, 0, 0, getWidth(), getHeight(), juce::Justification::centred, true);
  }

  juce::AffineTransform getTransform() const
  {
    const int destX = leftMargin;
    const int destY = topMargin;
    const int destW = getWidth() - leftMargin - rightMargin;
    const int destH = getHeight() - topMargin - bottomMargin;

    float x, y, w, h;
    drawable->getBounds(x, y, w, h);
    juce::RectanglePlacement placement(juce::RectanglePlacement::stretchToFit);
    return placement.getTransformToFit(x, y, w, h, (float)destX, (float)destY, (float)destW, (float)destH);
  }

  enum
  {
    leftMargin = 60,
    rightMargin = 20,
    topMargin = 20,
    bottomMargin = 40
  };
};

class SurrogateBasedSolverInformationComponent : public juce::Component, public juce::ChangeListener, public ComponentWithPreferedSize
{
public:
  SurrogateBasedSolverInformationComponent(SurrogateBasedSolverInformationPtr information, const string& name)
    : information(information), configuration(NULL), content(NULL)
  {
    addAndMakeVisible(configuration = new SurrogateBasedSolverInformationConfigurationComponent(information));
    addAndMakeVisible(content = new SurrogateBasedSolverInformationContentComponent(information));
    configuration->addChangeListener(this);
  }

  virtual void changeListenerCallback(void* objectThatHasChanged)
  {
    removeChildComponent(content);
    delete content;
    addAndMakeVisible(content = new SurrogateBasedSolverInformationContentComponent(information));
    resized();
  }

  virtual void resized()
  {
    const size_t h = 100;
    configuration->setBounds(0, 0,getWidth(), h);
    content->setBounds(0, h, getWidth(), getHeight() - h);
  }

  virtual ~SurrogateBasedSolverInformationComponent()
    {if (content) delete content;}

  virtual int getDefaultWidth() const
    {return 600;}

  virtual int getDefaultHeight() const
    {return 600;}

private:
  SurrogateBasedSolverInformationPtr information;
  SurrogateBasedSolverInformationConfigurationComponent* configuration;
  SurrogateBasedSolverInformationContentComponent* content;
};

} /* namespace lbcpp */

#endif // !ML_SOLVER_SURROGATE_BASED_INFORMATION_H_
