local attr = require 'c-node.attr'
for i, v in pairs(attr.attrs) do
    if type(i) == 'number' then
        assert(attr.tostring(i) == v)
        assert(attr.fromstring(v) == i)
    else
        assert(attr.tostring(v) == i)
        assert(attr.fromstring(i) == v)
    end
end
for i = 1, 1000 do
    local s
    repeat
        local len = math.random(5)
        local t = {}
        for i = 1, len do
            t[#t+1] = string.char(math.random(255))
        end
        s = table.concat(t)
    until not attr.attrs[s]
    assert(not attr.fromstring(s))
    assert(not attr.tostring(math.random(255, 10000)))
end
