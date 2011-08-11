-- Francis Maes, 11/08/2011
-- Evaluators

Evaluator = {}

local function isNumberFinite(x)
  return x > -math.huge and x < math.huge
end

function Evaluator.meanSquaredError(f, dataset)
  local res = 0
  for i,example in ipairs(dataset) do
    local prediction = f(unpack(example[1]))
    if not isNumberFinite(prediction) then
      return math.huge
    end
    local error = prediction - example[2]
    res = res + error * error
  end
  return res / #dataset
end


return Evaluator
