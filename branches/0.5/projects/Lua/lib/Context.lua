-- Francis Maes, 01/08/2011
-- Sparse, Dense and Composed Vectors

--[[
Interface:

 Context:error(what, where = "")
 Context:warning(what, where = "")
 Context:information(what, where = "")
 Context:result(string, value)
 Context:progress(value, total = 1.0, unit = "")

 Context:enter(description)
 Context:leave(returnValue)

 Context:call(description, function, arguments)  -- self:enter(description) res = function(arguments) self:leave(res)
]]


Context = {}

function Context.call(self, description, fun, ...)
  self:enter(description)
  local ok, res = pcall(fun, ...)
  if not ok then
    __errorHandler(res)
    res = "Failure"
  end
  self:leave(res)
  return res
end

function Context.random(self)
  return self.randomGenerator:sample()
end

return Context