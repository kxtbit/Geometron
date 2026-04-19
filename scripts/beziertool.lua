---@diagnostic disable: undefined-global
---@diagnostic disable: trailing-space
debug.sethook()

local linedraw = assert(load(engine.readFile("linedraw.lua"), "=linedraw.lua"))()

local thickness = 30/8
local defaultSegmentCount = 20
local maxSegmentCount = 50
local segmentTolerance = 0.002

local id_curvePoint = 3911
local id_saveData = 914
--putting a null character in a GD string prevents the rest from being rendered
--but the rest of the data is still stored, even if invalid as text characters
local saveDataHeader = "Bezier\nData\0"

local function canBeCurvePoint(object)
    return object.id == id_curvePoint and object.scale == 1 and object.baseColorID == 1011
end
local function isSaveContainer(object)
    return object.id == id_saveData and string.sub(object.text, 1, #saveDataHeader) == saveDataHeader
end

local pointDataSize = string.packsize("!1<dddd")
if pointDataSize ~= 32 then
	error("unexpected double size, should be 32 but is "..pointDataSize, 0)
end


local curvePoints = {}
local controlPoints = {}
local segments = {}
local segmentsPoints = {}
local segmentsToUpdate = {}

local isCurvePoint = {}

local editorLayer = editor.getLayer()
if editorLayer == -1 then
    editorLayer = 0
end
local editingMarker = editor.createObject(3850)
editingMarker.scale = 2
editingMarker.editorLayer = editorLayer
editingMarker.baseColorID = 1011 --white

local function createControlPoint(pos)
    local controlPoint = editor.createObject(id_curvePoint)
    controlPoint.pos = pos
    controlPoint.scale = 0.5
    controlPoint.editorLayer = editorLayer
    controlPoint.baseColorID = 1010 --black
    controlPoint.baseColorHSV = hsv.new(60, 1, 1, true, true)
    return controlPoint
end
local function addPoint(pos, controlPos)
    local curvePoint = editor.createObject(id_curvePoint)
    curvePoint.pos = pos
    curvePoint.editorLayer = editorLayer
    curvePoint.baseColorID = 1011 --white
    table.insert(curvePoints, curvePoint)
    isCurvePoint[curvePoint.uid] = true

    table.insert(controlPoints, createControlPoint(controlPos or curvePoint.pos + point.new(60, 60)))

    return #curvePoints
end
local function makeIntoPoint(object)
    table.insert(curvePoints, object)
    isCurvePoint[object.uid] = true
    table.insert(controlPoints, createControlPoint(object.pos + point.new(60, 60)))

    return #curvePoints
end
local function lerp(t, a, b)
    return a + (b - a) * t
end
local function quadraticBezierInterp(t, a, b, c)
    return lerp(t, lerp(t, a, b), lerp(t, b, c))
end
local function cubicBezierInterp(t, a, b, c, d)
    local k1, k2 = 1 - t, t
    local k1_2, k2_2 = k1*k1, k2*k2
    local k1_3, k2_3 = k1_2*k1, k2_2*k2
    return k1_3*a + 3*k1_2*k2*b + 3*k1*k2_2*c + k2_3*d
    --local ab, bc, cd = lerp(t, a, b), lerp(t, b, c), lerp(t, c, d)
    --local ab_bc, bc_cd = lerp(t, ab, bc), lerp(t, bc, cd)
    --return lerp(t, ab_bc, bc_cd)
end
local function segmentControlPoints(i)
    local a = curvePoints[i].pos
    local b = controlPoints[i].pos
    local d = curvePoints[i + 1].pos
    local c = -(controlPoints[i + 1].pos - d) + d
    return a, b, c, d
end
local function distPointLine(p, a, b)
    local ab = b - a
    return math.abs(ab.y * p.x - ab.x * p.y + a.y * b.x - a.x * b.y) / ab.magnitude
end
local function splitSegment(a, b, c, d, out)
    local borders = {0, 1}
    local orderedBorders = {0, 1}

    local len = (d - a).magnitude
    local drySteps = 0
    local maxDrySteps = 3
    while #borders - 1 < maxSegmentCount do
        local greatestDist = -1
        local bestSplitPoint, bestSplitSegment
        for i = 1, #borders - 1 do
            local l, r = borders[i], borders[i + 1]
            local lp, rp = cubicBezierInterp(l, a, b, c, d), cubicBezierInterp(r, a, b, c, d)
            if lp == rp then
                goto finish
            end
            local splitPoint = lerp(0.5, l, r)
            local dist = distPointLine(cubicBezierInterp(splitPoint, a, b, c, d), lp, rp) / len

            if dist > greatestDist then
                greatestDist = dist
                bestSplitPoint = splitPoint
                bestSplitSegment = i
            end
        end
        if greatestDist < segmentTolerance then
            if drySteps >= maxDrySteps then
                table.move({}, 1, maxDrySteps, #orderedBorders - maxDrySteps + 1, orderedBorders)
                break
            else
                drySteps = drySteps + 1
            end
        else
            drySteps = 0
        end
        table.insert(borders, bestSplitSegment + 1, bestSplitPoint)
        table.insert(orderedBorders, bestSplitPoint)
    end

    ::finish::
    table.sort(orderedBorders)
    for i, border in ipairs(orderedBorders) do
        out[#out + 1] = cubicBezierInterp(border, a, b, c, d)
    end
    return borders
end
local function plotSegment(i)
    if i < 1 or i >= #curvePoints then return end

    local a, b, c, d = segmentControlPoints(i)
    local st = os.clock()
    local steps = {}
    if false then
        for j = 0, defaultSegmentCount do
            local t = j / defaultSegmentCount
            steps[#steps + 1] = cubicBezierInterp(t, a, b, c, d)
        end
    else
        splitSegment(a, b, c, d, steps)
    end
    segmentsPoints[i] = steps

    --update these too since the junctions between segments might change
    if i > 1 then
        segmentsToUpdate[i - 1] = true
    end
    segmentsToUpdate[i] = true
    if i < #curvePoints - 1 then
        segmentsToUpdate[i + 1] = true
    end
end
local function updateSegment(i)
    if i < 1 or i >= #curvePoints then return end

    local extraFirst, extraLast
    if i > 1 then
        local lastPoints = segmentsPoints[i - 1]
        extraFirst = lastPoints[#lastPoints - 1]
    end
    if i < #curvePoints - 1 then
        local nextPoints = segmentsPoints[i + 1]
        if nextPoints then
            extraLast = nextPoints[2]
        end
    end

    local steps = {extraFirst}
    table.move(segmentsPoints[i], 1, #segmentsPoints[i], #steps + 1, steps)
    if extraLast then
        table.insert(steps, extraLast)
    end

    local result = linedraw(steps, thickness, segments[i], (extraFirst and 1 or 0) | (extraLast and 2 or 0), true)
    segments[i] = result
end
local curvePointsPrevPos = {}
local controlPointsPrevPos = {}
local function updatePoint(i)
    if i < 1 or i > #curvePoints then return end

    plotSegment(i)
    plotSegment(i - 1)

    --update these too since the junctions between segments might change
    plotSegment(i + 1)
    plotSegment(i - 2)

    local curvePoint = curvePoints[i]
    local controlPoint = controlPoints[i]

    curvePointsPrevPos[i] = curvePoint.pos
    controlPointsPrevPos[i] = controlPoint.pos
end

local function loadSaveData(saveContainer, selectedObjects)
    local saveData = string.sub(saveContainer.text, #saveDataHeader + 1)
    local pos = 1

    --Lua can't mix variable definitions and writes to existing variables :/
    local numPoints, numObjects, x1, y1, x2, y2
    numPoints, numObjects, pos = string.unpack("!1<I4I4", saveData, pos)
    if selectedObjects and #selectedObjects ~= numObjects then
        error("either no objects or the whole curve exactly must be selected along with the curve data object to load", 0)
    end

    local basePos = saveContainer.pos
    for i = 1, numPoints do
        x1, y1, x2, y2, pos = string.unpack("!1<dddd", saveData, pos)
        addPoint(basePos + point.new(x1, y1), basePos + point.new(x2, y2))
    end
end

local initialSelection = editor.getSelectedObjects()
if #initialSelection <= 1 then
    if #initialSelection == 1 and isSaveContainer(initialSelection[1]) then
        loadSaveData(initialSelection[1])
        editor.removeObjects(initialSelection)
    else
        addPoint(#initialSelection == 1 and initialSelection[1].pos or editor.getViewGridCenter())
    end
else
    local saveContainer = nil
    for i, object in ipairs(initialSelection) do
        if isSaveContainer(object) then
            if saveContainer then
                error("multiple curve data objects are selected, should be one or zero", 0)
            else
                saveContainer = object
            end
        end
    end
    if not saveContainer then
        error("too many objects selected, should be one or include a curve data object", 0)
    end
    loadSaveData(saveContainer, initialSelection)
    editor.removeObjects(initialSelection)
end

if #curvePoints == 0 then
	error("curve has no points", 0)
end
editor.setSelectedObject(curvePoints[1])
editingMarker.pos = curvePoints[1].pos + point.new(150, 150)

while true do
    local i = 1
    while i <= #curvePoints do
        local curvePoint = curvePoints[i]
        local controlPoint = controlPoints[i]
        if not curvePoint.exists or not controlPoint.exists then
            isCurvePoint[curvePoint.uid] = nil
            if curvePoint.exists then
                editor.removeObject(curvePoint)
            end
            if controlPoint.exists then
                editor.removeObject(controlPoint)
            end
            if i < #curvePoints then
                editor.removeObjects(segments[i])
                table.remove(segments, i)
                table.remove(segmentsPoints, i)
            elseif i > 1 then
                editor.removeObjects(segments[i - 1])
                table.remove(segments, i - 1)
                table.remove(segmentsPoints, i - 1)
            end

            table.remove(curvePoints, i)
            table.remove(controlPoints, i)
            i = i - 1
            updateSegment(i - 1)
            updateSegment(i)
            updateSegment(i + 1)
        elseif curvePoint.pos ~= curvePointsPrevPos[i] or controlPoint.pos ~= controlPointsPrevPos[i] then
            updatePoint(i)
        end
        i = i + 1
    end

    if not editingMarker.exists then
        --exit
        break
    end

    local selectedObject = editor.getSelectedObject()
    if selectedObject then
        if isCurvePoint[selectedObject.uid] then
            local markerPos = selectedObject.pos + point.new(150, 150)
            if editingMarker.pos ~= markerPos then editingMarker.pos = markerPos end
        else
            if canBeCurvePoint(selectedObject) then
                local id = makeIntoPoint(selectedObject)
                editor.setSelectedObjects({selectedObject, controlPoints[id]})
            end
        end
    end

    for segment, _ in pairs(segmentsToUpdate) do
        updateSegment(segment)
        segmentsToUpdate[segment] = nil
    end

    engine.sleep(0)
end

if #curvePoints < 1 then return end

--finalize curve
local allObjects = {}
for i = 1, #curvePoints - 1 do
    updateSegment(i)
    local segment = segments[i]
    table.move(segment, 1, #segment, #allObjects + 1, allObjects)
end

local basePos = curvePoints[1].pos

--save curve data for future editing
local saveContainer = editor.createObject(id_saveData)
saveContainer.editorLayer = editorLayer
saveContainer.baseColorID = 1011 --white
saveContainer.pos = basePos
saveContainer.scale = 0.125
saveContainer.isHide = true

table.insert(allObjects, saveContainer)

local saveString = string.pack("!1<I4I4", #curvePoints, #allObjects)
for i = 1, #curvePoints do
    local curvePoint = curvePoints[i]
    local controlPoint = controlPoints[i]
    local pos1 = curvePoint.pos - basePos
    local pos2 = controlPoint.pos - basePos
    saveString = saveString..string.pack("!1<dddd", pos1.x, pos1.y, pos2.x, pos2.y)
end
saveContainer.text = saveDataHeader..saveString

editor.removeObjects(curvePoints)
editor.removeObjects(controlPoints)
editor.linkObjects(allObjects)
