local event = require 'event'

local f = function(...) print("f", ...) end


local signal1 = event.new()
local signal2 = event.new()
print "connect" do
    signal1:connect(f)
    signal1:connect("abc", f)
    signal1:connect(function()error"abc"end)
    signal1:connect(function()error"def"end)
    signal2:connect(f)
    signal2:connect(f)
end
print "signal" do
    print(pcall(signal1, 1))
    signal1("abc", "abc")
    print(pcall(signal1, "def", "abc"))
    signal2(2)
    signal1:reset()
    signal1:connect(f, f, f, f)
end
print "signal" do
    signal1:disconnect(f, f)
    signal1(1)
end
print "signal" do
    signal1:disconnect(f, f)
    signal1(1)
end
print "reset" do
    signal2:reset()
    signal2(2)
end
print "end"
