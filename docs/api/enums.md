# Enums
There are multiple types of enumerations available to Geometron scripts.
Each state of each enumeration is represented as a mapping between a name and an integer.
The numeric value of each state can be obtained with `<enum>.<state>`,
and the name of each state can be derived from a numeric value with `<enum>[<number>]`.

### `EasingType`
An enumeration representing the possible easing types for triggers in Geometry Dash.
It defines 19 mappings:
- `EasingType.None = 0`
- `EasingType.EaseInOut = 1`
- `EasingType.EaseIn = 2`
- `EasingType.EaseOut = 3`
- `EasingType.ElasticInOut = 4`
- `EasingType.ElasticIn = 5`
- `EasingType.ElasticOut = 6`
- `EasingType.BounceInOut = 7`
- `EasingType.BounceIn = 8`
- `EasingType.BounceOut = 9`
- `EasingType.ExponentialInOut = 10`
- `EasingType.ExponentialIn = 11`
- `EasingType.ExponentialOut = 12`
- `EasingType.SineInOut = 13`
- `EasingType.SineIn = 14`
- `EasingType.SineOut = 15`
- `EasingType.BackInOut = 16`
- `EasingType.BackIn = 17`
- `EasingType.BackOut = 18`
### `ZLayer`
An enumeration representing the possible Z layers for objects in Geometry Dash.
It defines 10 mappings:
- `ZLayer.Default = 0`
- `ZLayer.B5 = -5`
- `ZLayer.B4 = -3`
- `ZLayer.B3 = -1`
- `ZLayer.B2 = 1`
- `ZLayer.B1 = 3`
- `ZLayer.T1 = 5`
- `ZLayer.T2 = 7`
- `ZLayer.T3 = 9`
- `ZLayer.T4 = 11`
### `GameObjectClassType`
An enumeration representing the different internal "types" of objects in Geometry Dash.
It defines 5 mappings:
- `GameObjectClassType.Game = 0` - for solid, decoration, or hazard objects
- `GameObjectClassType.Effect = 1` - for triggers, pickups, pads, orbs, portals, collision blocks, etc.
- `GameObjectClassType.Animated = 2` - for animated objects
- `GameObjectClassType.Enhanced = 4` - for rotating objects
- `GameObjectClassType.Smart = 5` - for auto-building template objects
### `Speed`
An enumeration representing the different gameplay speed states for the player in Geometry Dash.
It defines 5 mappings:
- `Speed.Normal = 0` - 1x speed
- `Speed.Slow = 1` - 0.5x speed
- `Speed.Fast = 2` - 2x speed
- `Speed.Faster = 3` - 3x speed
- `Speed.Fastest = 4` - 4x speed
### `SingleColorType`
An enumeration representing the possible ways a single color object can be treated in the editor.
It defines 3 mappings:
- `Default`: single color object is treated as either base or detail color depending on the object type
- `Base`: single color object is treated as base color
- `Detail`: single color object is treated as detail color