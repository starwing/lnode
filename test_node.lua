local node = require 'node'

local n = node {
    node {
        node(),
        node(),
    },
    node {
    },
}
print(n, n[1], n[2], n[3], n[-1], n[-2], n[-3])
