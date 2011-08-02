require 'Vector'
require 'Dictionary'
require 'Parser'
require 'Data'
require 'Context'

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

  local parameters = Vector.newDense()

  local function iteration(parameters, iteration)
    context:result("iteration", iteration)

    local totalHingeLoss = 0.0
    local numExamples = #examples
    for i=1,#examples do
      local example = examples[i]
       --print (parameters, example[1])
      local prediction = parameters.dot(parameters, example[1])
      local sign = examples[2] == 2 and 1 or -1
      local score = prediction * sign
      local hingeLoss = math.max(0, 1 - score)
      totalHingeLoss = totalHingeLoss + hingeLoss
      if score < 1 then
        parameters:add(example[1], 0.001 * sign)
      end
    end
    context:result("mean hinge loss", totalHingeLoss / numExamples)
    context:result("parameters norm", parameters:l2norm())
    context:result("parameters", parameters:clone())
  end

  for i = 1,10 do
    context:call("iteration " .. i,  iteration, parameters, i)
  end
end

params = context:call("learn binary classifier", learnBinaryClassifier, examples)
