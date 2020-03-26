Project: FPS
Controls:
'1': will draw a world line to the origin from the camera's position
'2': will draw a world arrow from the origin to the camera's position
'3': will draw the camera's transform at its location
'4': will draw world text at the camera's location and matching its orientaiton
'5': will draw a world quad from the origin to the camera's position
'6': will draw a screen arrow from (0,0) to (100,100)
'7': will draw a textured quad on the screen

## Checklist [90%]
- [x] RenderContext
    - [x] `RasterState` moved off shader, and added to `RenderContext`
    - [x] `RenderContext::SetCullMode`
    - [x] `RenderContext::SetFillMode`
    - [x] `RenderContext::SetFrontFaceWindOrder`
    - [x] Create a default raster state to set when `BeginCamera` is called.
    - [x] Have a transient raster state to create/set when above calls are used.

- [x] World Rendering
    - [x] Points
    - [x] Lines
    - [x] Arrows
    - [x] Basis
    - [x] Quad
    - [x] Wire Box
    - [x] Wire Sphere
    - [x] Text
    - [x] Billboarded Text
    - [x] All world commands support all rendering modes; 

- [x] Screen Rendering
    - [x] Points
    - [x] Lines
    - [x] Quads
    - [x] Textured Quads
    - [x] Text

- [x] Output
    - [x] Implement `DebugRenderWorldTo` to draw the debug objects into the passed camera's space.
    - [x] Implement `DebugRenderScreenTo` to add the screen-space debug calls to the passed in texture.
    - [x] Add a `DebugRenderWorldTo` call to your game after you render your scene
    - [x] Add a `DebugRenderScreenTo` call to your App before your present to render 2D objects

- [x] Controls
    - [x] Console command: `debug_render enabled=bool` 
    - [x] Console command: `debug_add_world_point position=vec3 duration=float`
    - [x] Console command: `debug_add_world_wire_sphere position=vec3 radius=float duration=float`
    - [x] Console command: `debug_add_world_wire_bounds min=vec3 max=vec3 duration=float`
    - [x] Console command: `debug_add_world_billboard_text position=vec3 pivot=vec2 text=string`
    - [x] Console command: `debug_add_screen_point position=vec2 duration=float`
    - [x] Console command: `debug_add_screen_quad min=vec2 max=vec2 duration=float`
    - [x] Console command: `debug_add_screen_text position=vec2 pivot=vec2 text=string`

## Extras
- [x] *X05.00: 03%*: MeshUtils: `AddCylinderToIndexedVertexArray`
- [x] *X05.00: 03%*: MeshUtils: `AddConeToIndexedVertexArray`