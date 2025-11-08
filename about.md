# Geometron

This mod adds a custom Lua scripting engine for the Geometry Dash level editor, intended as a versatile tool for automating some parts of level building.
The engine does not currently support running outside the editor.

For more information on the scripting environment and API, refer to the folder `docs`.

Functionality of the editor API includes:
- Placing any type of object in a level
- Changing the parameters of most triggers
- Transforming objects (moving, rotating, scaling, skewing)
- Changing colors of objects (base, detail, HSV)
- Changing properties of objects (NoTouch, Don't Fade, Hide, etc.)
- Adding and removing group IDs (including parent groups)
- Converting objects to and from string representation (`"1,1,2,0,3,0..."`)

Functionality of the engine itself includes:
- Standard Lua 5.4 support
- In-game console that can display output and take input
- Reading and writing files in the "workspace" directory in the mod's save data
- Rudimentary "scheduler" that allows scripts to pause execution in some circumstances without freezing the game (e.g. waiting for input)
- Configurable "preemption" system that will automatically pause the script for one frame if it is taking too long, so an infinite loop will not freeze the game
- Basic sandboxing (not fully tested for security; do not run untrusted scripts!)