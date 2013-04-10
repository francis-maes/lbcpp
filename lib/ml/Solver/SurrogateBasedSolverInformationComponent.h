/*-----------------------------------------------------.-------------------------------------.
| Filename: SurrogateBasedSolverInformationComponent.h | Surrogate Based Solver Information  |
| Author  : Denny Verbeeck                             | UI Component			               	   |
| Started : 06/04/2013 11:35                           |										                 |
`------------------------------------------------------/                                     |
                               |                                                             |
                               `------------------------------------------------------------*/

#ifndef ML_SOLVER_SURROGATE_BASED_INFORMATION_COMPONENT_H_
# define ML_SOLVER_SURROGATE_BASED_INFORMATION_COMPONENT_H_

# include "SurrogateBasedSolverInformation.h"
# include <oil/UserInterface/ObjectComponent.h>
# include "../../oil/UserInterface/Plot/ContourPlotDrawable.h"

namespace lbcpp
{

class SurrogateBasedSolverInformation2DDrawable : public ContourPlotDrawable
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
    ContourPlotDrawable::draw(g, transform);
    if (information->getDrawSolutions())
    {
      SolutionVectorPtr solutions = information->getSolutions();
      for (size_t i = 0; i < information->getStepNumber(); ++i)
      {
        DenseDoubleVectorPtr vector = solutions->getSolution(i).staticCast<DenseDoubleVector>();
        int x, y;
        getPixelPosition(vector, x, y, getContourPlotTransform(transform));

        juce::uint8 k = (juce::uint8)((255 * i) / information->getStepNumber());
        g.setColour(juce::Colour(k, 0, 255 - k));
        paintPoint(g, x, y);
      }
    }
  }

  /**
   * Sets up the data for the ContourPlot. This should be called before a call to draw().
   */
  void initialize(const juce::AffineTransform& transform)
  {
    switch (information->getColourScaleType())
    {
    case SurrogateBasedSolverInformation::linear:
      setColourScale(new LinearColourScale());
      break;
    case SurrogateBasedSolverInformation::percentile:
      setColourScale(new PercentileBasedColourScale());
      break;
    default:
      jassertfalse;
    }

    juce::Rectangle rect = getFrameRectangle(transform);
    juce::AffineTransform inverseTransform = transform.inverted();

    int w = (int)(rect.getWidth() * information->getPrecision());
    int h = (int)((rect.getHeight() - colourBarHeight - colourBarMargin) * information->getPrecision());
    if (w <= 1 || h <= 1)
      return;

    SurrogateBasedSolverInformation::Layer layer = information->getLayerToDraw();

    FitnessPtr bestFitness;
    if (layer == SurrogateBasedSolverInformation::expectedImprovementLayer ||
        layer == SurrogateBasedSolverInformation::probabilityOfImprovementLayer)
    {
      bestFitness = information->getSolutions()->getFitness(0);
      for (size_t i = 1; i < information->getStepNumber(); ++i)
      {
        FitnessPtr fitness = information->getSolutions()->getFitness(i);
        if (fitness->strictlyDominates(bestFitness))
          bestFitness = fitness;
      }
    }
    
    SelectionCriterionPtr criterion;
    if (layer == SurrogateBasedSolverInformation::expectedImprovementLayer)
      criterion = expectedImprovementSelectionCriterion(bestFitness);
    else if (layer == SurrogateBasedSolverInformation::probabilityOfImprovementLayer)
      criterion = probabilityOfImprovementSelectionCriterion(bestFitness);
    else if (layer == SurrogateBasedSolverInformation::optimistic1Layer)
      criterion = optimisticSelectionCriterion(1.0);
    else if (layer == SurrogateBasedSolverInformation::optimistic2Layer)
      criterion = optimisticSelectionCriterion(2.0);
    else if (layer == SurrogateBasedSolverInformation::optimistic5Layer)
      criterion = optimisticSelectionCriterion(5.0);

    if (criterion)
      criterion->initialize(information->getProblem());
    
    data = new Table(h);
    for (int i = 0; i < w; ++i)
      data->addColumn(T("c" + i), doubleClass);
    for (int y = 0; y < h; ++y)
      for (int x = 0; x < w; ++x)
      {
        double vx = rect.getX() + (double)x * rect.getWidth() / (w - 1);
        double vy = rect.getY() + (double)y * rect.getHeight() / (h - 1);
        inverseTransform.transformPoint(vx, vy);

        DenseDoubleVectorPtr vector(new DenseDoubleVector(2, 0.0));
        vector->setValue(0, vx);
        vector->setValue(1, vy);
        double value = computeLayer(defaultExecutionContext(), vector, criterion);
        data->setElement(y, x, new Double(value));
      }
  }

protected:
  SurrogateBasedSolverInformationPtr information;
  FitnessPtr currentFitness;

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

  double computeLayer(ExecutionContext& context, DenseDoubleVectorPtr vector, SelectionCriterionPtr criterion) const
  {
    SurrogateBasedSolverInformation::Layer layer = information->getLayerToDraw();

    if (layer == SurrogateBasedSolverInformation::objectiveLayer)
      return information->getProblem()->getObjective(0)->evaluate(context, vector);

    std::vector<ObjectPtr> objectVector(vector->getNumElements());
    for (size_t i = 0; i < vector->getNumElements(); ++i)
      objectVector[i] = vector->getElement(i);
    ScalarVariableMeanAndVariancePtr prediction = information->getSurrogateModel()->compute(context, objectVector).staticCast<ScalarVariableMeanAndVariance>();

    if (layer == SurrogateBasedSolverInformation::modelLayer)
      return prediction->getMean();
    else if (layer == SurrogateBasedSolverInformation::stddevLayer)
      return prediction->getStandardDeviation();
    else
    {
      jassert(criterion);
      return criterion->evaluate(context, prediction);
    }
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
};

class SurrogateBasedSolverInformationConfigurationComponent : public Component, public juce::ChangeBroadcaster,
                                                              public juce::ComboBoxListener,
                                                              public juce::ButtonListener
{
public:
  SurrogateBasedSolverInformationConfigurationComponent(SurrogateBasedSolverInformationPtr information) 
    : information(information)
  {
    addAndMakeVisible(layerComboBox = createLayerSelectionComboBox());
    addAndMakeVisible(precisionComboBox = createPrecisionComboBox());
    addAndMakeVisible(colourScaleTypeComboBox = createColourScaleTypeComboBox());
    addAndMakeVisible(drawSolutions = new juce::ToggleButton("Draw solutions"));
    drawSolutions->addButtonListener(this);
    drawSolutions->setToggleState(information->getDrawSolutions(), false);
  }

  virtual ~SurrogateBasedSolverInformationConfigurationComponent()
    {deleteAllChildren();}

  virtual void buttonClicked(juce::Button* button)
  {
    information->setDrawSolutions(drawSolutions->getToggleState());
    sendSynchronousChangeMessage(this);
  }

  virtual void comboBoxChanged(juce::ComboBox* box)
  {
    if (box == layerComboBox)
    {
      information->setLayerToDraw((SurrogateBasedSolverInformation::Layer)box->getSelectedItemIndex());
    }
    else if (box == precisionComboBox)
    {
      const double precisions[] = {1.0, 0.5, 0.2, 0.1, 0.05};
      information->setPrecision(precisions[box->getSelectedItemIndex()]);
    }
    else if (box == colourScaleTypeComboBox)
    {
      information->setColourScaleType((SurrogateBasedSolverInformation::ColourScaleType)box->getSelectedItemIndex());
    }
    sendSynchronousChangeMessage(this);
  }

  virtual void resized()
  {
    layerComboBox->setBounds(10, 10, 250, 30);
    precisionComboBox->setBounds(270, 10, 100, 30);
    drawSolutions->setBounds(380, 10, 100, 30);
    colourScaleTypeComboBox->setBounds(500, 10, 100, 30);
  }

private:
  SurrogateBasedSolverInformationPtr information;

  juce::ComboBox* layerComboBox;
  juce::ComboBox* precisionComboBox;
  juce::ComboBox* colourScaleTypeComboBox;
  juce::ToggleButton* drawSolutions;

  juce::ComboBox* createLayerSelectionComboBox()
  {
    juce::ComboBox* res = new juce::ComboBox(T("layerSelectionCombo"));
    for (size_t layer = 0; layer < SurrogateBasedSolverInformation::numLayers; ++layer)
      res->addItem(SurrogateBasedSolverInformation::getLayerName((SurrogateBasedSolverInformation::Layer)layer), (int)layer + 1);
    res->addListener(this);
    res->setSelectedId(1);
    return res;
  }

  juce::ComboBox* createPrecisionComboBox()
  {
    juce::ComboBox* res = new juce::ComboBox(T("precisionCombo"));
    res->addItem(T("1"), 1);
    res->addItem(T("1 / 2"), 2);
    res->addItem(T("1 / 5"), 3);
    res->addItem(T("1 / 10"), 4);
    res->addItem(T("1 / 20"), 5);
    res->addListener(this);
    res->setSelectedId(3);
    return res;
  }

  juce::ComboBox* createColourScaleTypeComboBox()
  {
    juce::ComboBox* res = new juce::ComboBox(T("colourScaleTypeCombo"));
    res->addItem(T("Linear"),1);
    res->addItem(T("Percentile"),2);
    res->addListener(this);
    res->setSelectedId(2);
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
    {
      drawable->initialize(getTransform());
      drawable->draw(g, getTransform());
    }
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
  {
    if (content) delete content;
    if (configuration) delete configuration;
  }

  virtual int getDefaultWidth() const
    {return 800;}

  virtual int getDefaultHeight() const
    {return 600;}

private:
  SurrogateBasedSolverInformationPtr information;
  SurrogateBasedSolverInformationConfigurationComponent* configuration;
  SurrogateBasedSolverInformationContentComponent* content;
};

}; /* namespace lbcpp */

#endif // !ML_SOLVER_SURROGATE_BASED_INFORMATION_COMPONENT_H_
