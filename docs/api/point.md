# Point
A data type representing a 2D point. It has support for arithmetic operations such as addition, multiplication, and scaling.

Note that these points represent X and Y position with double-precision float values,
while Geometry Dash represents object position with only single-precision float values.

### (static) `point.new(x, y)`
Constructs a new point with the specified X and Y position.
### `point.x`, `point.y`
Properties containing the X and Y position of the point respectively.
### `point.magnitude`
An immutable property that returns the distance of the point from the origin (effectively the same as treating treating the point as a vector and taking its magnitude).
Functionally equivalent to `math.sqrt(point.x * point.x + point.y * point.y)`.
### `point:dot(other)`
Returns the dot product of this point and the point `other`, when both are treated as vectors.
Functionally equivalent to `point.x * other.x + point.y * other.y`.
### `point:unit()`
Returns a normalized copy of this point, treating this point as a vector.
Functionally equivalent to `point / point.magnitude`.
### `pointA + pointB`, `pointA - pointB`, `pointA * pointB`, `pointA / pointB`
Adds, subtracts, multiplies, or divides respectively the X values and the Y values of the two points, and returns a new point with the resulting values.
Functionally equivalent to `point.new(pointA.x (+, -, *, /) pointB.x, pointA.y (+, -, *, /) pointB.y)`.
### `pointA * numberB`, `pointA / numberB`
Returns a copy of point A where both its X and Y values have been multiplied or divided respectively by number B.
Functionally equivalent to `pointA (*, /) point.new(numberB, numberB)`.
Both operations also work in reverse, with a number on the left side and a point on the right side.
### `-pointA`
Returns a copy of point A with negated X and Y values.
Functionally equivalent to `pointA * -1`.
### `pointA == pointB`
Returns `true` if `pointA.x == pointB.x and pointA.y == pointB.y`; otherwise returns `false`.
### `tostring(pointA)`
Returns a string representation of the provided point.