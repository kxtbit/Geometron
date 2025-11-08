# Engine
A library of miscellaneous functions for interacting with the Geometron scripting engine.
All of its functions are static.

### `engine.sleep(seconds)`
Pauses execution of the Lua script for the provided number of seconds. When the time has passed, the function will return.
Passing a sleep duration of zero or less will cause the script to sleep until the next in-game frame.
### `engine.readFile(path)`
Reads the file at the provided path inside the workspace directory.
If successful, the entire content of the file is returned as a string; otherwise, this function returns `nil` and an error message.
### `engine.writeFile(path, content)`
Writes the string `content` to the file at the provided path inside the workspace directory.
If the file already exists, it is overwritten.
Additionally, this function will automatically create any missing directories in the provided path.
If successful, this function returns `true`; otherwise, it returns `false` and an error message.
### `engine.removeFile(path[, recursive])`
Removes the file at the provided path inside the workspace directory.
If the path refers to a non-empty directory, the deletion will fail unless `recursive` is `true`.
If successful, this function returns `true`; otherwise, it returns `false` and an error message.
### `engine.makeDir(path)`
Creates a new directory at the provided path inside the workspace directory, also creating any missing directories leading up to it.
If a file or directory already exists at the provided path, this function will fail.
If successful, this function returns `true`; otherwise, it returns `false` and an error message.
### `engine.exists(path)`
Tests if there exists a file or directory at the provided patrh inside the workspace directory.
If no file or directory exists at the provided path, this function returns `false`.
If a file exists at the provided path, this function returns `"file"`.
If a directory exists at the provided path, this function returns `"directory"`.
### `engine.listFiles(path)`
Lists the files and directories within the provided path inside the workspace directory.
If the provided path does not refer to a directory, this function returns `nil` and an error message.
Otherwise, this function returns an array of strings containing the names of any files or directories present inside the provided path.
The results are returned in no particular order, and the names of directories are suffixed with `"/"`.