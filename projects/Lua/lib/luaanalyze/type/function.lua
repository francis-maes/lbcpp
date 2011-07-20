
local function tfunction(...)
  local params = {...}
  local nparams
  for i,v in ipairs(params) do
    params[i] = v == '->' and not nparams and v or type(v) == 'string' and require(v) or v
    if v == '->' then
      nparams = i - 1
    end
  end
  nparams = nparams or #params

  local t = {}
  function t.__call(...)
    local args = {...}
    local nargs = select('#', ...)
    if nargs ~= nparams then
      error{nargs .. ' arguments, expected ' .. nparams}
    end

    for i,param in ipairs(params) do
    if i <= nparams then
      if param and param.__isa then
        param.__isa(args[i])
      end
    end end
  end
  return t
end

return tfunction

