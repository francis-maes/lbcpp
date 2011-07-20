local t = {}
t.__basictype = 'number'
function t.__index()
  error{'number cannot be indexed'}
end
function t.__call()
  error{'number cannot be called'}
end
function t.__isa(ast)
  if ast.type and ast.type.__basetype and ast.type.__basetype ~= 'number' then
    error{'got ' .. tostring(ast.type.__basetype) .. ' expected number'}
  end
end

return t
