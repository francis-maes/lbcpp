-- Francis Maes, 11/08/2011
-- Evaluators

Evaluator = {}

local function isNumberFinite(x)
  return x > -math.huge and x < math.huge
end

function Evaluator.meanSquaredError(f, dataset)
  local res = 0
  for i,example in ipairs(dataset) do
    local prediction = f(unpack(example, 1, #example - 1))
    if not isNumberFinite(prediction) then
      return math.huge
    end
    local error = prediction - example[#example]
    res = res + error * error
  end
  return res / #dataset
end

function Evaluator.accuracy(f, examples)
  local stats = Statistics.mean()  
  for i,example in ipairs(examples) do
    local prediction = f(unpack(example, 1, #example - 1))
    stats:observe(prediction == example[#example] and 1 or 0)
  end
  return stats:getMean()
end

return Evaluator