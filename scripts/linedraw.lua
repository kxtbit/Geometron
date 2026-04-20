--This script should be in workspace to be usable by other scripts!

local id_square = 211
local id_slope = 693
local id_quarterCircle = 1837
local id_circle = 3637
local sizemul_circle = 0.6

local cornerLengthLimit = 16

local function debugPoint(p, col)
    local obj = editor.createObject(3818)
    obj.pos = p
    obj.scaleX = 0.25
    obj.scaleY = 0.25
    obj.baseColorID = ({red = 8, green = 9, blue = 10, yellow = 11})[col]
    obj.zLayer = ZLayer.T1
end
local function debugPoints(p, col)
    for _, point in ipairs(p) do
        debugPoint(point, col)
    end
end

local function pointIsValid(p)
    return p.x == p.x and p.y == p.y and p.x ~= math.huge and p.x ~= -math.huge and p.y ~= math.huge and p.y ~= -math.huge
end

local function intersect(a1, a2, b1, b2)
    return point.new(
        (a1.x * a2.y - a1.y * a2.x) * (b1.x - b2.x) - (a1.x - a2.x) * (b1.x * b2.y - b1.y * b2.x),
        (a1.x * a2.y - a1.y * a2.x) * (b1.y - b2.y) - (a1.y - a2.y) * (b1.x * b2.y - b1.y * b2.x)
    ) / ((a1.x - a2.x) * (b1.y - b2.y) - (a1.y - a2.y) * (b1.x - b2.x))
end
local function shift(a, b, amount)
    local vec = (b - a):unit()
    local pvec = point.new(-vec.y, vec.x)
    return (a + pvec * amount), (b + pvec * amount)
end
local function shiftAux(a, b, a2, b2, amount)
    local vec = (b - a):unit()
    local pvec = point.new(-vec.y, vec.x)
    return (a2 + pvec * amount), (b2 + pvec * amount)
end

return function(points, thickness, objects, ignoreFirstLast, circleJoints)
    local segments = {}
    for i = 1, #points - 1 do
        segments[i] = {a = points[i], b = points[i + 1]}
    end
    local leftSegments = {}
    local rightSegments = {}
    for i, s in ipairs(segments) do
        local a, b = shift(s.a, s.b, thickness / 2)
        leftSegments[i] = {a = a, b = b}
        a, b = shift(s.a, s.b, -thickness / 2)
        rightSegments[i] = {a = a, b = b}
    end

    objects = objects or {}
    ignoreFirstLast = ignoreFirstLast or 0
    local cornerID
    if circleJoints then cornerID = id_circle else cornerID = id_quarterCircle end
    local recyclableCorners = {}
    local recyclableSquares = {}
    for _, obj in ipairs(objects) do
        if not obj.exists then goto continue end
        if obj.id == cornerID then
            table.insert(recyclableCorners, obj)
        elseif obj.id == id_square then
            table.insert(recyclableSquares, obj)
        else
            editor.removeObject(obj)
        end
        ::continue::
    end
    local nextCornerID = 1
    local nextSquareID = 1
    local function createCorner()
        local id = nextCornerID
        nextCornerID = nextCornerID + 1
        if id <= #recyclableCorners then
            return recyclableCorners[id]
        else
            return editor.createObject(cornerID)
        end
    end
    local function createSquare()
        local id = nextSquareID
        nextSquareID = nextSquareID + 1
        if id <= #recyclableSquares then
            return recyclableSquares[id]
        else
            return editor.createObject(id_square)
        end
    end

    local straightJunction = {false}
    local leftBorderPoints = {leftSegments[1].a}
    local rightBorderPoints = {rightSegments[1].a}
    for i = 2, #segments do
        local lFirst, lLast = leftSegments[i - 1], leftSegments[i]
        local rFirst, rLast = rightSegments[i - 1], rightSegments[i]

        local vecFirst = (segments[i - 1].b - segments[i - 1].a):unit()
        local vecLast = (segments[i].b - segments[i].a):unit()
        if vecFirst:dot(vecLast) >= 0.999999 then
            straightJunction[i] = true
            leftBorderPoints[i] = lFirst.b
            rightBorderPoints[i] = rFirst.b
            goto continue
        end

        local lBorder = intersect(lFirst.a, lFirst.b, lLast.a, lLast.b)
        local rBorder = intersect(rFirst.a, rFirst.b, rLast.a, rLast.b)
        if lBorder.x ~= lBorder.x or lBorder.y ~= lBorder.y then
            lBorder = lFirst.b
        end
        if rBorder.x ~= rBorder.x or rBorder.y ~= rBorder.y then
            rBorder = rFirst.b
        end
        straightJunction[i] = false
        leftBorderPoints[i] = lBorder
        rightBorderPoints[i] = rBorder

        ::continue::
    end
    leftBorderPoints[#points] = leftSegments[#leftSegments].b
    rightBorderPoints[#points] = rightSegments[#rightSegments].b

    local finalSegments = {}
    for i = 1, #points - 1 do
        local la, lb = shiftAux(points[i], points[i + 1], leftBorderPoints[i], leftBorderPoints[i + 1], -thickness / 2)
        local ra, rb = shiftAux(points[i], points[i + 1], rightBorderPoints[i], rightBorderPoints[i + 1], thickness / 2)
        local center = (segments[i].a + segments[i].b) / 2

        local a, b
        if not straightJunction[i] then
            a = ((la - center).magnitude >= (ra - center).magnitude) and la or ra
        else
            a = points[i]
        end
        if not straightJunction[i + 1] then
            b = ((lb - center).magnitude >= (rb - center).magnitude) and lb or rb
        else
            b = points[i + 1]
        end

        finalSegments[i] = {a = a, b = b}
    end

    local objects = {}
    for i = 1, #finalSegments - 1 do
        local first, last = finalSegments[i], finalSegments[i + 1]
        local vecFirst = (first.b - first.a):unit()
        local vecLast = (last.b - last.a):unit()

        if vecFirst:dot(vecLast) < -0.0001 then
            local xAxis = (first.b - segments[i].b):unit()
            local yAxis = (last.a - segments[i + 1].a):unit()
            local div = math.sin(math.acos(xAxis:dot(yAxis)))
            xAxis = xAxis / div
            yAxis = yAxis / div

            if (circleJoints or (xAxis + yAxis).magnitude <= cornerLengthLimit) and (i + 1 < #finalSegments or ignoreFirstLast & 2 == 0) then
                local obj
                if circleJoints then
                    obj = createCorner()
                    obj.pos = segments[i].b
                    obj.scale = (thickness / 30) * sizemul_circle
                else
                    --quarter circle because it needs to overlap a bit to prevent a hole from showing
                    obj = createCorner()
                    obj.pos = segments[i].b
                    obj.vectorX = xAxis * thickness * 0.5 / 30
                    obj.vectorY = -yAxis * thickness * 0.5 / 30
                    obj.baseColorID = 1011
                end

                table.insert(objects, obj)
            end

            first.b = segments[i].b
            last.a = segments[i + 1].a
        end
    end

    for i, segment in ipairs(finalSegments) do
        local a = segment.a
        local b = segment.b

        local center = (a + b) / 2
        local angle = math.atan(b.y - a.y, b.x - a.x)
        local length = (b - a).magnitude

        if (i > 1 or ignoreFirstLast & 1 == 0) and (i < #finalSegments or ignoreFirstLast & 2 == 0) and pointIsValid(center) then
            local obj = createSquare()
            obj.pos = center
            obj.scaleX = length / 30
            obj.scaleY = thickness / 30
            obj.rot = -math.deg(angle)
            obj.baseColorID = 1011 --white

            table.insert(objects, obj)
        end
    end

    --remove unused objects
    for i = nextCornerID, #recyclableCorners do
        editor.removeObject(recyclableCorners[i])
    end
    for i = nextSquareID, #recyclableSquares do
        editor.removeObject(recyclableSquares[i])
    end

    return objects
end
