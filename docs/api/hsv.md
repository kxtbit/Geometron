# HSV
A data type representing the HSV settings for a Geometry Dash object.
The hue, saturation, and value parameters are represented with single-precision floats.

### (static) `hsv.new(h, s, v, addS, addV)`
Constructs a new HSV value with the specified parameters.
`addS` and `addV` specify whether the saturation and value should be treated as additive instead of multiplicative,
equivalent to the check box next to "Saturation" or "Brightness" in Geometry Dash.
### `hsv.h, hsv.s, hsv.v`
Properties containing the hue, saturation, and value parameters of the HSV value respectively.
Not mutable.
### `hsv.addS, hsv.addV`
Properties specifying whether the saturation or value parameters respectively are counted as additive or multiplicative; see the constructor for details.
Not mutable.
### `tostring(hsvA)`
Returns a string representation of the provided HSV value.