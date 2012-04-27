local node = require 'node'

local function check_one_node(node, lvl)
    print(("  "):rep(lvl).."+ single check id = "..node.id)
    if node.parent and node.parent[node.idx] ~= node then
        error("node:idx = "..
            tostring(node.idx or nil)..", parent[idx] = "..
            tostring(node.parent[node.idx])..", node = "..tostring(node))
    end
    local cur = node.firstchild
    assert(cur == node[1], "#node = "..#node..
        ", cur = "..tostring(cur)..", node[1] = "..tostring(node[1]))
    local count = 0
    for i, v in ipairs(node) do
        assert(node[i] == node[i-#node-1])
        node[i] = v
        node[i-#node-1] = v
        assert(node[v] == i, "node[v] = "..tostring(node[v])..", i = "..i)
        assert(v.idx == i, "v.idx = "..v.idx..", i = "..i)
        assert(v.parent == node,
            "v.parent = "..tostring(v.parent)..", node = "..tostring(node))
        assert(cur == v, "cur = "..tostring(cur)..", v = "..tostring(v))
        cur = assert(cur.nextsibling)
        count = count + 1
    end
    assert(cur == node.firstchild)
    local cur = node.lastchild
    for i = #node, 1, -1 do
        assert(node[i] == cur)
        cur = assert(cur.prevsibling)
    end
    assert(cur == node.lastchild)
    assert(#node == count)
end

local function check_tree(root, lvl)
    local lvl = lvl or 0
    print(("  "):rep(lvl).."check id = "..root.id)
    for i, v in ipairs(root) do
        check_tree(v, lvl+1)
    end
    check_one_node(root, lvl)
end

local n = node {
    id = "a",
    node {
        id = "b",
        node {id = "c"},
        node {id = "d"},
    },
    node {
        id = "e",
    },
    node {
        id = "f",
        node {id = "g"},
        node {
            id = "h",
            node {id = "i"},
        },
        node {id = "j"},
        node {id = "k"},
    },
}

print "----- check 1 ------"
check_tree(n)

n:appendchild(assert(n[1]))
print "----- check 2 ------"
check_tree(n)

n:appendchild(assert(n[-1]))
print "----- check 3 ------"
check_tree(n)

