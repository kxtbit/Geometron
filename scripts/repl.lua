local running = true
function exit()
    running = false
    error("exited", 0)
end

local function compile(input)
    --try expression first
    local func, err = load("return " .. input, "=interactive")
    if not func then
        func, err = load(input, "=interactive")
    end
    return func, err
end
local function run(func)
    local ret = table.pack(pcall(func))
    if ret.n >= 2 then
        print(table.unpack(ret, 2, ret.n))
    end
end
local function isIncompleteStatement(err)
    local i, j = string.find(err, "<eof>", 1, true)
    if i and j == #err then
        return true
    end
    return false
end
local function multiline(firstInput)
    local str = firstInput
    while true do
        io.write(" .. > ")

        local input = io.read()
        if input == "" then return end

        str = str .. "\n" .. input
        local func, err = compile(str)

        if func then
            run(func)
            return
        else
            if not isIncompleteStatement(err) then
                print(err)
                return
            end
        end
    end
end

while running do
    io.write("> ")

    local input = io.read()
    local func, err = compile(input)

    if not func then
        if isIncompleteStatement(err) then
            multiline(input)
        else
            print(err)
        end
    else
        run(func)
    end

    if not running then break end
end
