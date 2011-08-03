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
  
function instanceMT.__call(self, ...)
  return self.__get(...)
end

function instanceMT.__index(self, key)
  if key == "__get" then
    return self.__expression.functor(self.__parameters)
  else
    return self.__get[key]
  end
end

function instanceMT.__newindex(self, key, value)
 -- if key == "__parameters" or key == "__expression" then
 --   self.key = value
 -- else
    self.__get[key] = value
end

function instanceMT.__tostring(self)
  res = ""
  for identifier,value in pairs(self.__parameters) do
    if #res > 0 then res = res .. ", " end
    res = res .. identifier .. " = " .. tostring(value)
  end
  return "ssExpr instance{" .. res .. "}: " .. tostring(self.__get)
end


--
-- class metatable
--
MT = {}

function MT.__call(ssExpr, paramValues)

 -- fill parameters
 local parameters = {}
 for identifier,properties in pairs(ssExpr.parameters) do
   parameters[identifier] = paramValues[identifier] or properties.default
 end

 -- create instance
 return setmetatable({__expression = ssExpr, __parameters = parameters}, instanceMT)

end

function MT.__tostring(ssExpr)
  res = ""
  for identifier,properties in pairs(ssExpr.parameters) do
    if #res > 0 then res = res .. ", " end
    res = res .. identifier .. " (def " .. tostring(properties.default) .. ")"
  end
  return  "sub-specified expression{" .. res .. "}"
end