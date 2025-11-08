# Editor
A library of functions for interacting with the Geometry Dash level editor and the objects in it.
All of its functions are static.

### `editor.getSelectedObjects()`
Returns an array containing all of the GameObjects that are currently selected in the editor.
### `editor.getSelectedObject()`
Returns the GameObject currently selected in the editor if there is exactly one (not zero, not multiple); otherwise, returns `nil`.
### `editor.setSelectedObjects(objects)`
Deselects all currently selected objects in the editor and selects every GameObject in the array `objects`.
### `editor.setSelectedObject(object)`
Deselects all currently selected objects in the editor and selects the GameObject `object`.
### `editor.getAllObjects()`
Returns an array of every GameObject that currently exists in the editor.
### `editor.createObject(id)`
Creates and returns a new GameObject with the object ID `id` and positioned at exactly `0, 0`.
If the provided object ID is not valid, this function returns `nil`.
Additionally, creating a linked teleport portal object (ID 747) will also create a linked orange portal to go with it, accessible with `object.orangePortal`.
It is not possible to create a linked orange portal (ID 749) on its own.
### `editor.loadObjects(string)`
Loads the GameObjects encoded in the GD object string `string` (e.g. `"1,1,2,15,3,15;1,1,2,45,3,15;"`) into the editor and returns the objects as an array.
### `editor.saveObjects(objects)`
Saves every GameObject in the array `objects` and returns the resulting object string.
The data for each object is suffixed with a semicolon to act as a delimiter.
### `editor.saveObject(object)`
Saves the GameObject `object` and returns the resulting object string.
The resulting string will not contain semicolons.
### `editor.removeObjects(objects)`
Removes every GameObject in the array `objects` from the editor.
The deleted objects will no longer work properly from Lua.
### `editor.removeObject(object)`
Removes the GameObject `object` from the editor.
The object will no longer work properly from Lua.
### `editor.getGroupParent(group)`
Returns the object currently set as the parent of group ID `group` (the object that has it as a pink group).
If the group has no parent object, this function returns `nil`.
### `editor.setGroupParent(group, object)`
Sets the GameObject `object` as the parent of group ID `group`.
If the group already has a parent, the parent will be replaced with `object`.
### `editor.removeGroupParent(group)`
Removes the parent from group ID `group`; any object set as its parent will still exist, but the group will no longer have a parent.
### `editor.getGroupsParentOf(object)`
Returns an array of group IDs that the GameObject `object` is the parent of.
### `editor.linkObjects(objects)`
Links every GameObject in the array `objects` together, as when using link controls in the editor.
### `editor.unlinkObjects(objects)`
Unlinks every GameObject in the array `objects` from any objects they are linked to.
Objects not present in the provided array will stay linked.
### `editor.getAnyLinkedObjects(objects)`
Returns an array containing all GameObjects in the provided array `objects` as well as the objects they are linked to (if any).
### `editor.getLinkedObjects(object)`
Returns an array containing all GameObjects that are linked to `object`.
If `object` is not linked to any other objects, this function returns `{object}`.
### `editor.linkKeyframes(objects)`
Links up the keyframes (GameObject ID 3032) in the array `objects` such that the editor recognizes them all as one animation.
This function is sensitive to the order of the keyframes provided; keyframes that appear later in the array will occur later in the animation.
### `editor.getLinkedKeyframes(object)`
Returns an array of keyframes that are part of the same animation as the keyframe `object`, in order.
If the provided keyframe is not linked to any other, this function returns `object`.
### `editor.unlinkKeyframeGroup(id)`
Separates all of the keyframes in the keyframe group (`object.keyframeGroup`) `id`.
### `editor.nextFreeColorChannel()`
Returns the next free color channel ID in the editor.
Does the same thing as the "next free" button in the Edit Object menu.
### `editor.nextFreeGroupID()`
Returns the next free group ID in the editor.
Does the same thing as the "next free" button in the Edit Group menu.
### `editor.nextFreeItemID()`
Returns the next free item ID in the editor.
### `editor.nextFreeGradientID()`
Returns the next free gradient ID in the editor.
### `editor.nextFreeAreaEffectID()`
Returns the next free area effect ID in the editor.
### `editor.getObjectRect(object)`
Returns two points referring to the bottom left and top right corner respectively of the GameObject `object`'s bounding box.
### `editor.getGridSize()`
Returns the current size in GD units of one grid square in the editor.
This value differs based on what the user is currently doing in the editor.
### `editor.getViewCenter()`
Returns the exact coordinate, in GD units, of the point that would currently appear in the exact center of the editor view.
This is mostly useful to determine what part of the level the user is currently viewing.
Note that "the exact center of the editor view" in this case means the center of the area that is not covered by the toolbar,
usually the upper two thirds or so of the screen.
### `editor.getViewGridCenter()`
Same as `getViewCenter`, but returns coordinates aligned to the center of the nearest grid square in the editor.
This is analogous to the behavior of the "paste" button in the editor, which always pastes objects in the center of the screen.
### `editor.setViewCenter(pos)`
Moves the editor view to be centered on the point `pos`.
When this function returns, subsequent calls to `getViewCenter()` should return the same value as `pos` discounting floating point rounding error.
### `editor.getLayer()`
Returns the number for the current layer being viewed in the editor (note that layer in this sense refers to "editor layer").
A value of `-1` means that the editor is currently viewing all layers at once (the "All" layer in the interface).
### `editor.setLayer(layer)`
Changes the layer currently being viewed in the editor to the layer with the number `layer`.
Specifying a `layer` value of `-1` (or any number less than zero) will cause the editor to view all layers at once.