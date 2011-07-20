
local t = {}

local is_key = {}
is_key.tag = true
is_key.lineinfo = true

function t.__index(t_ast, k_ast)
  if k_ast.tag == 'String' then
    local k_name = k_ast[1]
    if not is_key[k_name] then
      error{'field ' .. k_name .. ' not in AST'}
    end
  end
end
function t.add_field(name)
  is_key[name] = true
end


return t

