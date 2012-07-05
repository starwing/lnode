local node = require 'node'

local n = node {
    node {
        node(),
        node(),
    },
    node {
    },
}
print(#n)

local function print_nodes(n, lvl)
    local lvl = lvl or 0
    print(('  '):rep(lvl)..tostring(n))
    for k, v in ipairs(n) do
        print_nodes(v, lvl+1)
    end
    if lvl == 0 then
        print '-----------------'
    end
end

print_nodes(n)

n:append (node { node(), node() })
print(#n)

print_nodes(n)

