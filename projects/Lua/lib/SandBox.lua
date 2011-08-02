require 'Vector'
require 'Dictionary'
require 'Parser'
require 'Data'
require 'Context'
require 'Statistics'
filename = "C:/Projets/lbcpp/projects/Examples/Data/BinaryClassification/a1a.test"
labels = Dictionary.new()
examples = Data.load(Parser.libSVMClassification, 100, filename, labels)

print (#examples .. " examples, " .. labels:size() .. " labels")

--for k,v in ipairs(examples) do 
--  for k2,v2 in ipairs(v) do
--    print (k2,v2)
--  end
--end


function learnBinaryClassifier(examples)

  -- parameters
  --parameter rate = {default = function (i) return 1 end}
  --parameter normalizeRate = {default = true}
  local rate = function (i) return 1 end
  local normalizeRate = true
  --

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

    local totalHingeLoss = 0.0
    for i=1,#examples do
      local example = examples[i]
       --print (parameters, example[1])
      local prediction = parameters.dot(parameters, example[1])
      local sign = examples[2] == 2 and 1 or -1
      local score = prediction * sign
      local hingeLoss = math.max(0, 1 - score)
      totalHingeLoss = totalHingeLoss + hingeLoss
      if score < 1 then
        parameters:add(example[1], rateNormalizer * rate(epoch) * sign)
      end
      epoch = epoch + 1
    end
    context:result("mean hinge loss", totalHingeLoss / numExamples)
    context:result("parameters l0norm", parameters:l0norm())
    context:result("parameters l1norm", parameters:l1norm())
    context:result("parameters l2norm", parameters:l2norm())
    context:result("parameters", parameters:clone())
    context:result("epoch", epoch)
  end

  for i = 1,10 do
    context:call("iteration " .. i,  iteration, parameters, i)
  end
end

params = context:call("learn binary classifier", learnBinaryClassifier, examples)