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
# include "../../oil/UserInterface/Plot/TwoDimensionalPlotDrawable.h"

namespace lbcpp
{

// TODO-eventually: better color-scales
// TODO: understand the strange behavior on problem 3
class SurrogateBasedSolverInformation : public Object
{
public:
  SurrogateBasedSolverInformation(size_t stepNumber = 0)
    : drawSolutions(true), layerToDraw(objectiveLayer), precision(0.2), stepNumber(stepNumber) {}

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

  size_t getStepNumber() const
    {return stepNumber;}

  bool getDrawSolutions() const
    {return drawSolutions;}

  void setDrawSolutions(bool draw)
    {drawSolutions = draw;}

  double getPrecision() const
    {return precision;}

  void setPrecision(double precision)
    {this->precision = precision;}

  enum Layer
  {
    objectiveLayer = 0,
    modelLayer,
    stddevLayer,
    expectedImprovementLayer,
    probabilityOfImprovementLayer,
    optimistic1Layer,
    optimistic2Layer,
    optimistic5Layer,

    numLayers
  };

  Layer getLayerToDraw() const
    {return layerToDraw;}

  void setLayerToDraw(Layer layerToDraw)
    {this->layerToDraw = layerToDraw;}
  
  static string getLayerName(Layer layer)
  {
    static const char* names[] = {"Objective", "Model", "Uncertainty", "Expected Improvement", "Probability of Improvement of 10%", "Optimistic C=1", "Optimistic C=2", "Optimistic C=5"};
    jassert(layer < numLayers);
    return names[layer];
  }

private:
  friend class SurrogateBasedSolverInformationClass;

  bool drawSolutions;
  Layer layerToDraw;
  double precision;

  ProblemPtr problem;
  SolutionVectorPtr solutions;
  ExpressionPtr surrogateModel;
  size_t stepNumber;
};

typedef ReferenceCountedObjectPtr<SurrogateBasedSolverInformation> SurrogateBasedSolverInformationPtr;

class PercentileBasedColourScale
{
public:
  void clearValues()
    {values.clear();}
  
  void pushValue(double value)
    {values.insert(value);}

  void computePercentiles()
  {
    size_t numValues = values.size();
    jassert(values.size() > 0);
    p0 = *values.begin();
    p100 = *values.rbegin();

    size_t i25 = numValues / 4;
    size_t i50 = numValues / 2;
    size_t i75 = 3 * numValues / 4;

    size_t i = 0;
    for (std::multiset<double>::const_iterator it = values.begin(); it != values.end(); ++it, ++i)
      if (i == i25)
        p25 = *it;
      else if (i == i50)
        p50 = *it;
      else if (i == i75)
        p75 = *it;
  }

  double normalizeValue(double value) const
  {
    if (value < p0)
      return 0.0;
    else if (value < p25)
      return normalize(value, p0, p25, 0.0, 0.25);
    else if (value < p50)
      return normalize(value, p25, p50, 0.25, 0.5);
    else if (value < p75)
      return normalize(value, p50, p75, 0.5, 0.75);
    else if (value < p100)
      return normalize(value, p75, p100, 0.75, 1.0);
    else
      return 1.0;
  }

  juce::Colour getColour(double value) const
  {
    return getColourFromScale(normalizeValue(value));
  }
  
  juce::Image* getColourBar(size_t width, size_t height) const
  {
    double rangeMin = DBL_MAX;
    double rangeMax = -DBL_MAX;
    for (std::multiset<double>::const_iterator it = values.begin(); it != values.end(); ++it)
    {
      if (rangeMin > *it)
        rangeMin = *it;
      if (rangeMax < *it)
        rangeMax = *it;
    }
    double step = (rangeMax - rangeMin) / width;
    juce::Image* res = new juce::Image(juce::Image::RGB, width, height, false);
    juce::Graphics g(*res);
    g.fillAll(juce::Colour(255, 255, 255));
    for (size_t i = 0; i < width; ++i)
    {
      g.setColour(getColour(rangeMin + i * step));
      g.drawVerticalLine(i, 15, height - 15);
    }
    char* number = new char[8];
    sprintf(number, "%8.3e", p0);
    g.setFont(juce::Font(12.0f));
    g.setColour(juce::Colour(0, 0, 0));
    g.drawText(string(number), 0, height - 12, 100, 12, juce::Justification::left, false);
    sprintf(number, "%8.3e", p25);
    g.drawText(string(number), p25 * width / p100, 0, 100, 12, juce::Justification::left, false);
    sprintf(number, "%8.3e", p50);
    g.drawText(string(number), p50 * width / p100, height - 12, 100, 12, juce::Justification::left, false);
    sprintf(number, "%8.3e", p75);
    g.drawText(string(number), p75 * width / p100, 0, 100, 12, juce::Justification::left, false);
    sprintf(number, "%8.3e", p100);
    g.drawText(string(number), width - 75, height - 12, 75, 12, juce::Justification::right, false);
    return res;
  }

  static juce::Colour getColourFromScale(double normalizedValue)
  {
    if (normalizedValue < 0.01f)
      return juce::Colour((float)normalizedValue, 1.f, 1.f - (float)normalizedValue, 1.f);
    else
      return juce::Colour((float)normalizedValue, 1.f, (1.f - (float)normalizedValue) * 0.66f, 1.f);
/*    jassert(normalizedValue >= 0.0 && normalizedValue <= 1.0);
    if (normalizedValue < 0.25)
    {
      juce::uint8 level = (juce::uint8)(normalizedValue * 4 * 255);
      return juce::Colour(255 - level, 255 - level, 0);
    }
    else if (normalizedValue < 0.75)
    {
      juce::uint8 level = (juce::uint8)(((normalizedValue - 0.25) * 2) * 255);
      return juce::Colour(level, level, level);
    }
    else
    {
      juce::uint8 level = (juce::uint8)((normalizedValue - 0.75) * 4 * 255);
      return juce::Colour(255 - level, 255, 255 - level);
    }*/
  }

private:
  std::multiset<double> values;
  double p0, p25, p50, p75, p100;

  double normalize(double value, double lower, double upper, double l, double u) const
  {
    if (upper == lower)
      return l;
    return l + (value - lower) * (u - l) / (upper - lower);
  }
};

class SurrogateBasedSolverInformation2DDrawable : public TwoDimensionalPlotDrawable
{
public:
  SurrogateBasedSolverInformation2DDrawable(SurrogateBasedSolverInformationPtr information) : information(information), scale(new PercentileBasedColourScale())
  {
    xAxis = makeAxis(information->getSolutions(), 0);
    yAxis = makeAxis(information->getSolutions(), 1);
    computeBounds();
  }
  ~SurrogateBasedSolverInformation2DDrawable()
    {delete scale;}

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

    if (information->getDrawSolutions())
    {
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
  }
  
  PercentileBasedColourScale* getScale() const
    {return scale;}

protected:
  SurrogateBasedSolverInformationPtr information;
  PlotAxisPtr xAxis, yAxis;
  FitnessPtr currentFitness;
  PercentileBasedColourScale* scale;

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

  void drawBackgroundLayer(juce::Graphics& g, const juce::AffineTransform& transform) const
  {
    juce::Rectangle rect = getFrameRectangle(transform);
    
    juce::AffineTransform inverseTransform = transform.inverted();

    int w = (int)(rect.getWidth() * information->getPrecision());
    int h = (int)(rect.getHeight() * information->getPrecision());
    if (w <= 1 || h <= 1)
      return;

    SurrogateBasedSolverInformation::Layer layer = information->getLayerToDraw();
    bool layerShouldBeMinimised = true;

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
      layerShouldBeMinimised = false;
    }

    if (layer == SurrogateBasedSolverInformation::stddevLayer)
      layerShouldBeMinimised = false;

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
    
    scale->clearValues();
    std::vector< std::vector< double > > values(h);
    for (int y = 0; y < h; ++y)
    {
      values[y].resize(w);
      for (int x = 0; x < w; ++x)
      {
        double vx = rect.getX() + (double)x * rect.getWidth() / (w - 1);
        double vy = rect.getY() + (double)y * rect.getHeight() / (h - 1);
        inverseTransform.transformPoint(vx, vy);

        DenseDoubleVectorPtr vector(new DenseDoubleVector(2, 0.0));
        vector->setValue(0, vx);
        vector->setValue(1, vy);
        double value = computeLayer(defaultExecutionContext(), vector, criterion);
        if (!layerShouldBeMinimised)
          value = -value;

        values[y][x] = value;
        scale->pushValue(value);
      }
    }

    scale->computePercentiles();

    juce::Image* imgLayer = new juce::Image(juce::Image::RGB, w, h, true);
    juce::Graphics gLayer(*imgLayer);
    for (int y = 0; y < h; ++y)
      for (int x = 0; x < w; ++x)
      {
        gLayer.setColour(scale->getColour(values[y][x]));
        gLayer.setPixel(x, y);
      }

    g.drawImage(imgLayer, rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight(), 0, 0, w, h);
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
    sendSynchronousChangeMessage(this);
  }

  virtual void resized()
  {
    layerComboBox->setBounds(10, 10, 250, 30);
    precisionComboBox->setBounds(270, 10, 100, 30);
    drawSolutions->setBounds(380, 10, 100, 30);
  }

private:
  SurrogateBasedSolverInformationPtr information;

  juce::ComboBox* layerComboBox;
  juce::ComboBox* precisionComboBox;
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
      drawable->draw(g, getTransform());
      size_t w = getWidth() - leftMargin - rightMargin;
      g.drawImage(drawable->getScale()->getColourBar(w, 45), leftMargin, 5, w, 45, 0, 0, w, 45);
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
    topMargin = 60,
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
