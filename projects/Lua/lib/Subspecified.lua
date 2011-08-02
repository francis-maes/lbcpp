-- Francis Maes, 02/08/2011
-- Subspecified extension

--[[
Interface:

  Subspecified.instantiate(parameters, functor, paramValues)

]]

module("Subspecified", package.seeall)

--
-- instance metatable
--
instanceMT = {} 
  
function instanceMT.__call(self, param)
  return self:get()(param)
end

function instanceMT.__tostring(self)
  res = ""
  for identifier,value in pairs(self.parameters) do
    if #res > 0 then res = res .. ", " end
    res = res .. identifier .. " = " .. tostring(value)
  end
  return "ssExpr instance{" .. res .. "}"
end

--
-- class metatable
--
MT = {}

function MT.__call(ssExpr, paramValues)

 -- create instance
 local res = setmetatable({expression = ssExpr}, instanceMT)

 -- fill parameters
 res.parameters = {}
 for identifier,properties in pairs(ssExpr.parameters) do
   res.parameters[identifier] = paramValues[identifier] or properties.default
 end
 
 -- add the "get()" function
 function res:get()
   return self.expression.functor(self.parameters)
 end

 return res

end

function MT.__tostring(ssExpr)
  res = ""
  for identifier,properties in pairs(ssExpr.parameters) do
    if #res > 0 then res = res .. ", " end
    res = res .. identifier .. " (def " .. tostring(properties.default) .. ")"
  end
  return  "sub-specified expression{" .. res .. "}"
end