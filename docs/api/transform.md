# Transform
A data type representing an affine transform, which can encode any combination of translation, rotation, scale, and skew.
Internally, the transformations are stored in a format akin to a 3x3 matrix whose bottom row is always `(0, 0, 1)`.
Similarly to the [point](point.md) data type, transforms use double-precision float values while Geometry Dash objects
use only single-precision float values.

### (static) `transform.new(xx, xy, xc, yx, yy, yc)`
Constructs a new transform from its individual components.
In most cases, it is more intuitive to construct a transform from an individual operation using the other static functions.
For the exact meaning of these components, see `transform.xx`, etc. below.
### (static) `transform.identity`
An immutable static value containing the identity transform
(meaning the transform that has no position, rotation, or scale and does nothing when applied).
Equivalent to `transform.new(1, 0, 0, 0, 1, 0)`.
### (static) `transform.translation(x, y)`, `transform.translation(point)`
Creates a transform that encodes a translation from the origin to the specified point (or individual X and Y components).
Equivalent to `transform.new(1, 0, x, 0, 1, y)`.
### (static) `transform.scaling(x, y)`, `transform.scaling(point)`
Creates a transform that encodes a scaling from the X and Y scale `(1, 1)` to the new scale `(x, y)`.
Similarly to `transform.translation`, this function accepts a point or individual components.
Equivalent to `transform.new(x, 0, 0, 0, y, 0)`
### (static) `transform.shear(x, y)`, `transform.shear(point)`
Creates a transform that encodes a shear `x` units to the right for each unit up and `y` units up for each unit right.
This function accepts a point or individual X and Y components.
Equivalent to `transform.new(1, x, 0, y, 1, 0)`.
### (static) `transform.rotation(rad)`
Creates a transform that encodes a rotation `rad` radians counter-clockwise.
Note that Geometry Dash objects instead interpret rotation values in degrees, not radians, and clockwise instead of counter-clockwise.
Equivalent to `transform.new(cos(rad), -sin(rad), 0, sin(rad), cos(rad), 0)`.
### (static) `transform.dualRotation(radX, radY)`
Creates a transform that encodes a `radX` radian rotation of the X axis and a `radY` radian rotation of the Y axis, both counter-clockwise.
Again, Geometry Dash objects instead use degrees and clockwise rotation.
Equivalent to `transform.new(cos(radX), -sin(radY), 0, sin(radX), cos(radY), 0)`.
### (static) `transform.fromVectors(vecX, vecY)`
Creates a transform that maps each unit on the X axis to one `vecX` and each unit on the Y axis to one `vecY`.
Equivalent to `transform.new(vecX.x, vecY.x, 0, vecX.y, vecY.y, 0)`.
### `transform.xx`, `transform.xy`, `transform.xc`, `transform.yx`, `transform.yy`, `transform.yc`
Properties representing the individual components of the transform.
`xx` and `xy` determine the amounts that input X and Y values contribute to the output X value,
and `yx` and `yy` determine the amounts that input X and Y values contribute to the output Y value.
The remaining properties `xc` and `yc` are constant offsets and are added to output X and Y values after other calculations.
These properties can also be thought of as the elements of a 3x3 matrix,
where 2D points have an implied third element always equal to 1 and where the bottom row is always `(0, 0, 1)`.
### `transform.pos`
An immutable property that returns the positional component of the transform as a point.
Equivalent to `point.new(xc, yc)`.
### `transform.scale`
An immutable property that returns the X and Y scale components of the transform as a point.
Equivalent to `point.new(sqrt(xx*xx + yx*yx), sqrt(xy*xy + yy*yy))`.
### `transform.rotXY`
An immutable property that returns the X and Y axis counter-clockwise rotational components of the transform, in radians, as a point.
Equivalent to `point.new(atan(yx, xx), atan(yy, xy) - pi * 0.5)`.
### `transform.degXY`
The same as `transform.rotXY`, but returns values in degrees instead of radians.
### `transform.inverse`
An immutable property that returns a transform performing the opposite action as this transform.
For example, the inverse of a translation `(x, y)` is `(-x, -y)`, and the inverse of a scaling `(x, y)` is `(1/x, 1/y)`.
More rigorously, `transform:apply(transform.inverse) == transform.identity`.
Note that some transform values are not actually invertible for the same reason it is impossible to undo a multiplication by zero.
### `transformA:apply(pointB)`
Applies the transformations encoded in this transform value to the point `pointB` and returns the result.
This function effectively performs a matrix multiplication.
Equivalent to `point.new(xx * pointB.x + xy * pointB.y + xc, yx * pointB.x + yy * pointB.y + yc)`.
### `transformA:apply(transformB)`
Applies the transformations encoded in the transform `transformA` to the transform `transformB` and returns the result.
In other words, returns a new transform that performs the transformations in `transformB`
and then performs the transformations in `transformA`.
Mathematically, if `A` and `B` are transforms, then `(A:apply(B)):apply(P) == A:apply(B:apply(P))` for any point `P` -
note that the two might not be perfectly equal in programming due to the limited precision of floating point numbers.
### `transformA:applyVector(pointB)`
Same as `transformA:apply(pointB)`, but excludes positional transformations (translations) from the result.
Equivalent to `transform.new(xx, xy, 0, yx, yy, 0):apply(pointB)`.
### `transformA:chain(transformB)`
Returns a transform that performs the transformations in `transformA` then performs the transformations in `transformB`.
This function is the same as `transform:apply` but backwards;
it is mainly useful for writing sequences of transformations in a more readable format.
Equivalent to `transformB:apply(transformA).`
### `transformA:around(transformB)`
Returns a copy of this transform with its origin remapped from the identity transform to `transformB`.
Essentially, applies the inverse of `transformB` to the "coordinate plane".
This can be used to change the center point of a rotation or scale, rotate the axes for X and Y scaling, etc.
Equivalent to `transformB.inverse:chain(transformA):chain(transformB)`.
### `transformA:around(pointB)`
Returns a copy of this transform with its origin translated from `(0, 0)` to `pointB`.
This function does the same thing as the previous, but is simplified to only the case of translation.
Equivalent to `transformA:around(transform.translation(pointB))`.
### `transformA + pointB`, `transformA - pointB`
Translates this transform by `point` or `-point` respectively.
Equivalent to `transform.translation(pointB):apply(transformA)` and `transform.translation(-pointB):apply(transformA)`.
### `transformA == transformB`
Returns `true` if the individual components (`xx, xy, xc, yx, yy, yc`) of `transformA`
are equal to the corresponding individual components of `transformB`, otherwise returns false.
### `tostring(transform)`
Returns a string representation of the provided transform.