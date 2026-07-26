# 1.0.7
- Fixed a bug where the table returned by `editor.getGroupsParentOf` used indices starting at 0 instead of at 1

# 1.0.6
- Fixed the `engine.removeFile` function being able to delete only directories instead of files

# 1.0.5
- Fixed the AdvRand target and spawn remap functions being completely broken
- Made `GameObject:advRandSetTarget` and `GameObject:spawnSetRemap` take the index as the first argument instead of the last
- Removed the pointless `support.md` file after pawPatoes brought it to my attention

# 1.0.4
- Re-enable macOS support as the crash no longer seems to occur

# 1.0.3
- Removed an unnecessary bit of debug code that was sometimes causing harmless but annoying crashes on game exit

# 1.0.2
- Remove a piece of broken debug code from `beziertool.lua`

# 1.0.1
- Remove macOS support; the mod seems to just crash on load and I don't have a Mac to debug on anyway

# 1.0.0
- Initial release