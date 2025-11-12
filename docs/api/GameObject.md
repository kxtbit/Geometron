# GameObject
A type representing an in-game object (or more accurately, a reference to one) in the Geometry Dash level editor.
This is generally the most complex part of the Geometron interface due to the sheer number of properties available.

There is no constructor function for a GameObject.
To create a new GameObject, use `editor.createObject(id)` to add a new object to the editor and get a reference to it.

## Basic Properties
This section contains properties and methods that are common to every type of `GameObject`.

### `GameObject.id`
An immutable property containing the object ID of the object.
Every distinct type of object in Geometry Dash has a unique object ID; for example, the default block is ID `1`.
### `GameObject.uid`
An immutable property containing the unique identifier of the object.
This differs for every object, even if the objects are the same type,
and can be used to determine if two `GameObject` values refer to the same actual object.
### `GameObject.linkID`
An immutable property containing the linking identifier of the object.
Objects that are linked together (see `editor.linkObjects()`) will have the same `linkID` value.
A value of `0` means the object is not linked to any others,
although it is also possible for an object to have a non-zero ID that is simply not shared with any others.
### `GameObject.pos`
A property containing the current position of the object, represented as a point.
Assigning a new value to this property will move the object to that position in the level.
### `GameObject.x`, `GameObject.y`
Properties containing the X and Y positions of the object; equivalent to `GameObject.pos.x` and `GameObject.pos.y` respectively.
These properties are especially useful as shortcuts to change one component of an object's position at a time, since there is no need to construct a new point.
### `GameObject.scale`
A property containing the current scale of the object.
When read, this property returns the larger of the object's X or Y scale values,
but assigning a value to it will overwrite both.
### `GameObject.scaleX`, `GameObjectscaleY`
Properties containing the X and Y scale values of the object.
Each property only refers to one of the scale dimensions, but both otherwise act similar to `GameObject.scale`.
### `GameObject.rot`
A property containing the current rotation value of the object, measured in degrees counter-clockwise.
If the X and Y angles of the object differ, reading this property returns only the X angle.
Assigning a new value to this property will rotate the object to the specified angle (overwriting both the X and Y angles of the object).
This property will accept any value, although Geometry Dash normally only uses values ranging from `-90` to `270`.
### `GameObject.rotX`, `GameObject.rotY`
Properties containing the current "X angle" and "Y angle" of the object respectively.
Normally, both of these values are identical (or nearly identical), but if the object has been transformed with the warp tool, these may differ.
Each of these properties only read and write to their respective angle values, but otherwise function the same as `GameObject.rot`.

For a more thorough explanation, these properties function on the X and Y basis vectors of the object.
By default, the Y basis vector for an object points up, and the X basis vector for an object points right
(the lengths of the X and Y basis vectors are decided by the object's X and Y scale values).
The properties `GameObject.rotX` and `GameObject.rotY` refer to the rotational offsets of these vectors from their default values, measured in degrees counter-clockwise.
These two properties combine, albeit in an odd way, to represent any rotation or shear transformation of the object.
### `GameObject.vectorX`, `GameObject.vectorY`
Properties representing, as points, the X and Y basis vectors mentioned in the previous section.
Assigning a new value to either of these properties will also modify the corresponding scale and rotation properties.
### `GameObject.transform`
A property containing the transform of the object, which contains its position, rotation, scale, and warp in one value; see [transform.md](transform.md).
Writing to this property also changes `pos`, `scaleX`, `scaleY`, `rotX`, and `rotY`.
Note that the transform data type uses a much higher precision than Geometry Dash does,
so some information may be lost when writing a transform to this property.

### `GameObject.baseColorID`, `GameObject.detailColorID`
Properties representing the color channel IDs of the object's "base" and "detail" colors respectively.
Note that for object types that only have one color, that color will always be treated as a base color, even if it shows up as "Detail" in the editor.
Assigning a new value to these properties will change the corresponding color to the new color ID;
`0` or any number less than zero will reset the corresponding color to whatever the default is for the specific object type.

There are several special color ID values that refer to normally hidden or inaccessible color channels:
- `1000`: background color ("BG")
- `1001`: ground color 1 ("G1")
- `1009`: ground color 2 ("G2")
- `1013`: middle ground color 1 ("MG")
- `1014`: middle ground color 2 ("MG2")
- `1002`: ground line color ("L")
- `1003`: 3D line color ("3DL")
- `1004`: solid object color ("OBJ")
- `1005`: player color 1 ("P1")
- `1006`: player color 2 ("P2")
- `1007`: light background color ("LBG")
- `1010`: constant black ("Black")
- `1011`: constant white ("White")
- `1012`: ??? ("Lighter")
- `1008`, `1015-1101`: hidden extra color channels ("NA")
- `1102+`: same as `1101` ("NA")
### `GameObject.baseColorHSV`, `GameObject.detailColorHSV`
Properties representing the HSV settings of the object's base and detail colors (as set using the HSV button in the "Edit Object" menu).
The editor shows the HSV settings as unmodified when this value is set to `hsv.new(0, 1, 1, false, false)` (the default).
### `GameObject.isSelected`
An immutable property representing, as a boolean, whether the object is selected in the editor
(i.e. it is present in the return value of `editor.getSelectedObjects()`).
### `GameObject.exists`
An immutable property representing whether the object "exists."
If the object has been deleted either by the user or using `editor.removeObject[s]()`, this property is `false`, otherwise it is `true`.
### `GameObject.isFlipX`, `GameObject.isFlipY`
Properties representing whether the object is flipped horizontally or vertically.

### `GameObject:getGroups()`
Returns an array of every group ID the object belongs to.
### `GameObject:addGroup(group)`
Adds the object to the group ID `group`.
An object can be in a maximum of 10 different groups at a time; this is a Geometry Dash limitation.
This function returns `true` if successful, or `false` if the object already has 10 groups or is already in `group`.
### `GameObject:addGroups(groups)`
Adds the object to every group ID in the array `groups`.
This function returns `true` if every group addition was successful, or `false` if at least one was not (see `addGroup`).
### `GameObject:removeGroup(group)`
Removes the object from the group ID `group`.
If the object is the parent of the group, its parent status is removed.
This function returns `true` if the removal was successful, or `false` if the object does not belong to `group`.
### `GameObject:removeGroups(groups)`
Removes the object from every group ID in the array `groups`.
If the object is the parent of any group, its parent status is removed.
This function returns `true` if every removal was successful, or `false` if at least one was not (see `removeGroup`).
### `GameObject:resetGroups()`
Removes the object from all groups.
If the object is the parent of any groups, its parent status is removed.
### `GameObject:hasGroup(group)`
Returns `true` if the object is a member of the group ID `group`, otherwise returns `false`.

## Miscellaneous Properties
This section contains various properties common to every type of object but whose meanings are fairly trivial.
For more information about these properties, it is recommended to check the official GD editor guide.
Following are a list of properties and their meanings, along with their names in the GD editor in parentheses if applicable:
- `mainColorKeyIndex`, `detailColorKeyIndex` - unknown
- `editorLayer`, `editorLayer2` - editor layers 1 and 2 ("Editor L" and "Editor L2")
- `zLayer` - object Z layer, uses enum type `ZLayer` ("Z Layer")
- `fixedZLayer` - `true` if the object Z layer cannot be changed, as in a portal, otherwise `false`
- `zOrder` - object Z order ("Z Order")
- `classType` - object "class type," uses enum type `GameObjectClassType`
- `isDontFade` - disable edge fade ("Dont Fade")
- `isDontEnter` - disable edge transform ("Dont Enter")
- `isNoEffects` - disable special effects for object ("No Effects")
- `isGroupParent` - make object origin of editor transforms ("Group Parent")
- `isAreaParent` - make object origin of linked objects for area triggers ("Area Parent")
- `isDontBoostX` - disable moving object boosting player horizontally ("Dont BoostX")
- `isDontBoostY` - disable moving object boosting player vertically ("Dont BoostY")
- `isHighDetail` - skip loading object in low detail mode ("High Detail")
- `isNoTouch` - disable all collision and interaction ("NoTouch")
- `isPassable` - enable semi-solid collision behavior ("Passable")
- `isHide` - make object permanently invisible in gameplay ("Hide")
- `isNonStickX` - disable moving objects carrying player horizontally ("NonStickX")
- `isNonStickY` - disable moving objects carrying player vertically ("NonStickY")
- `isExtraSticky` - enable fast moving object carrying player down ("ExtraSticky")
- `isScaleStick` - make horizontal scaling object carry player proportionally to distance ("ScaleStick")
- `isExtendedCollision` - fix glitchy hitbox for objects larger than 6 units ("Extended Collision")
- `isIceBlock` - make object slippery in platformer mode ("IceBlock")
- `isGripSlope` - allow steep ascent on slope in platformer mode ("GripSlope")
- `isNoGlow` - disable edge glow for solid objects ("NoGlow")
- `isNoParticles` - disable particle effects for object ("NoParticle")
- `isNoAudioScale` - disable pulsing effect for object ("No Audio Scale")
- `enterChannel` - object enter effect channel ("Enter Channel")

## General Utilities
This section contains various utility functions that are available to every GameObject,
but are not required for full functionality.

### `GameObject:scaleAround(origin, scale)`
Scales the object by the factor `scale`, with the origin at the point `origin` instead of the object's center.
### `GameObject:rotateAround(origin, rotation)`
Rotates the object by the angle `rotation` (measured in degrees counter-clockwise),
with the origin at the point `origin` instead of the object's center.
### `GameObject:setPosition3P(origin, pointX, pointY[, flipMask])`
Positions, scales, and rotates the object such that its lower left corner is `origin`, its lower right corner is `pointX`,
and its upper left corner is `pointY`.
The `flipMask` parameter is optional and controls whether the object should also be flipped horizontally or vertically.
`0` is default, `1` is horizontal flip, `2` is vertical flip, and `3` is both; in this way, it acts like a bit mask.

This method is especially useful for positioning objects in complex shapes;
for example, the following code will create a triangle with corners at points `a, b, c`:
```lua
local obj = editor.createObject(693) --ID 693: single color slope
--a slope normally has its top corner on the right, but here it should be on the left
--so pass 1 as flipMask to flip the slope horizontally before transforming
obj:setPosition3P(a, b, c, 1)
```
### `GameObject:transformBy(transform)`
Transforms this object by the provided transform.
Equivalent to `object.transform = transform:apply(object.transform)`.

## Object Subclasses
Many types of objects in Geometry Dash can be configured in other ways; these objects are represented by subclasses:
- GameObject: every object
  - [EnhancedGameObject](EnhancedGameObject.md): animated and rotating objects
    - [EffectGameObject](EffectGameObject.md): triggers, portals, orbs, pads, etc.
      - RingObject: jump orbs (which are also called jump rings)
        - DashRingObject: dash orbs
        - TeleportPortalObject: teleport portals (yes, this is technically a subclass of RingObject)
  - [TextGameObject](TextGameObject.md): text objects