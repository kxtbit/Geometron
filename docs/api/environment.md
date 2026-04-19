# Basic Environment

Geometron provides several new APIs to Lua scripts in the global environment to allow automating the Geometry Dash editor:
- `engine.*` - contains miscellaneous functions for interacting with the Geometron engine, as well as filesystem operations; see [engine.md](engine.md)
- `editor.*` - contains functions to interact with the GD editor; see [editor.md](editor.md)
- `point` - A basic data type representing a 2D point, creatable with `point.new(x, y)`; see [point.md](point.md)
- `transform` - A data type representing an affine transform (any combination of position, rotation, scale, and skew); see [transform.md](transform.md)
- `color` - A basic data type representing a 24-bit RGB color value, creatable with `color.new(r, g, b)`; see [color.md](color.md)
- `hsv` - A special data type designed to represent the HSV settings for a Geometry Dash object, creatable with `hsv.new(h, s, v, addS, addV)`; see [hsv.md](hsv.md)
- `GameObject` - A complex data type representing a Geometry Dash object in the editor; see [GameObject.md](GameObject.md)
- `EasingType, ZLayer, GameObjectType, GameObjectClassType, Speed` - Enumerations with a finite number of states; see [enums.md](enums.md)

### Extra Quirks
Yielding (with `coroutine.yield`) from the top level coroutine in a script will effectively pause execution of the script until the next frame.
However, it is probably clearer to do this with `engine.sleep(0)` instead; this method also functions within any coroutine instead of only the top level coroutine.

### Missing or Disabled Features
Geometron scripts run in a standard Lua 5.4 environment with the following exceptions:
- The `package.*` library, as well as the function `require`, are not available.
- `loadfile` and `dofile` are not available.
- The only functions available in `os.*` are `os.date`, `os.time`, `os.clock`, and `os.difftime`.
- By default, all functions in `debug.*` are removed except for `debug.traceback`, although this can be changed in the mod settings. Note that enabling the rest of the debug library is likely a security risk for untrusted code.
- The `io.*` library has been reimplemented, and many functions do not work properly