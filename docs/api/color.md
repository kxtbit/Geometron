# Color
A data type representing an RGB color value, where each color channel ranges from 0 to 255.

### (static) `color.new(r, g, b)`
Constructs a new color value with the specified RGB values.
The provided color channels should be integers ranging from 0 to 255.
### `color.r, color.g, color.b`
Properties containing the red, green, and blue color intensities of the color respectively.
Not mutable.
### `tostring(colorA)`
Returns a string representation of the provided color.