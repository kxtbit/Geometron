# Geometron

This mod adds a custom Lua scripting engine for the Geometry Dash level editor, intended as a versatile tool for automating some parts of level building.
The engine does not currently support running outside the editor. If you want to run scripts outside the editor,
[SerpentLua](https://geode-sdk.org/mods/yellowcat98.serpentlua) may be closer to what you want.

This mod is still experimental, and not all features are documented yet!
I chose to release this because I felt such a mod could be helpful for creators and I didn't want the effort I already
put into this mod to go to waste if I ended up moving on to something else in the future.

For more information on the scripting environment and API, refer to the documentation on GitHub.
There are also some sample scripts available for use with this mod.

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