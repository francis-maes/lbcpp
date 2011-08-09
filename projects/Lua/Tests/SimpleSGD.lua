require 'Vector'
require 'Dictionary'
require 'Parser'
require 'Data'
require 'Statistics'
require 'IterationFunction'

local filename = "C:/Projets/lbcpp/projects/Examples/Data/BinaryClassification/a1a.test"
local labels = Dictionary.new()
local examples = Data.load(Parser.libSVMClassification, 100, filename, labels)

print (#examples .. " examples, " .. #labels .. " labels")

--for k,v in ipairs(examples) do 
--  for k2,v2 in ipairs(v) do
--    print (k2,v2)
--  end
--end


subspecified function learnBinaryClassifier(examples)

  parameter rate = {default = IterationFunction.constant{1}}
  parameter normalizeRate = {default = true}

  local parameters = Vector.newDense()
  local epoch = 0
  local numExamples = #examples

  local rateNormalizer = 1
  if normalizeRate then
    -- make statistics on features sparsity
    local featureStats = Statistics.meanAndVariance()
    for i=1,numExamples do
      featureStats:observe(examples[i][1]:l0norm())
    end
    context:result("featureStats", featureStats)
    rateNormalizer = 1 / featureStats:getMean()
  end

  local function iteration(parameters, iteration)
    context:result("iteration", iteration)

    local lossStats = Statistics.mean()
    for i=1,#examples do
      local example = examples[i]
      --print (parameters, example[1])
      local prediction = parameters.dot(parameters, example[1])
      local sign = examples[2] == 2 and 1 or -1
      local score = prediction * sign
      --print (score)
      --assert(sign == 1 or sign == -1)
      local hingeLoss = math.max(0, 1 - score)
      lossStats:observe(hingeLoss)
      
      if score < 1 then
        local currentRate = rate(epoch)
        parameters:add(example[1], rateNormalizer * currentRate * sign)
      end
      epoch = epoch + 1
    end
    context:result("loss stats", lossStats)
    context:result("mean hinge loss", lossStats:getMean())
    context:result("parameters l0norm", parameters:l0norm())
    context:result("parameters l1norm", parameters:l1norm())
    context:result("parameters l2norm", parameters:l2norm())
    context:result("parameters", parameters:clone())
    context:result("rate", rate(epoch))
    context:result("epoch", epoch)
  end

  for i = 1,10 do
    context:call("iteration " .. i,  iteration, parameters, i)
  end
end


learner = learnBinaryClassifier{rate = IterationFunction.invLinear{2.0, 1000}}

params = context:call("learn binary classifier", learner, examples)
