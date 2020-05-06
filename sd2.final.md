Project: FPS
Controls:
`1`: will draw a world line to the origin from the camera's position
`2`: will draw a world arrow from the origin to the camera's position
`3`: will draw the camera's transform at its location
`4`: will draw world text at the camera's location and matching its orientaiton
`5`: will draw a world quad from the origin to the camera's position
`6`: will draw a screen arrow from (0,0) to (100,100)
`7`: will draw a textured quad on the screen

All lighting controls are displayed on screen

`V`: Toggles Grayscale
`B`: Toggles Bloom
`F9`: Reload currently loaded shaders

## Tasks
- [x] `NamedProperties`
    - [x] Switch `EventArgs` to use `NamedProperties` instead `NamedStrings`
- [x] Add ability to subscribe methods to your event system. 
- [x] Add ability to unsubscribe an object from the event system (unsubscribes all methods on that object)

- [x] Color Transform (ex: Grayscale) Effect
    - [x] Create/Recycle a color target matching your swapchain's output.
    - [x] Render game as normal
    - [x] Create/Recycle another match target
    - [x] Apply an effect on the original image, outputting to this new texture
        - [x] Effect applies a per-pixel color transform using a matrix. 
        - [x] Make it so you can specify a "strength" for this transform, where 0 
              has no effect, and 1 is the full effect so you can blend between them.
        - [x] Be able to specify a tint and tint strength that pixels trend toward (useful for fades)
              - `0` would have no effect
              - `1` the resultant color would be the tint 
        - [x] **Optional Challenge: Have all the above be done with a single mat44.**
    - [x] Copy this final image to the swapchain before present
   
- [x] Bloom Effect
    - [x] Set your normal color target, and a secondary "bloom" target
        - [x] Camera can set set tertiary render targets
        - [x] Shader has a secondary output specified
    - [x] When done, be able to blur the bloom target
        - [x] Create/Recycle a matching color/render target
        - [x] Can use any blur algorithm you can find online, some suggestions...
            - Simple single pass box blur
            - Single pass guassian blur
            - Multiple pass guassian blur
    - [x] Take the result of the blur, and the normal color output, and combine them
          into the final image.
    - [x] Be able to toggle blur on-and-off to see it working or not
        - [x] Disabling the blur just doesn't run the blur and composite steps;

- [x] Texture Pool
    - [x] Be able to ask your `RenderContext` for a temporary render target of a given resolution and size.
        - [x] Search for an existing free texture first, and return it if you find one.
        - [x] If there are none available, create and return a new one.
    - [x] Be able to relinquish back these temporary textures back to the `RenderContext` when you're done with them.
        - This will allow them to be reused.
    - [x] Add a `RenderContext::GetTotalTexturePoolCount()` to tell you how many textures have been created this way.
    - [x] Add a `RenderContext::GetTexturePoolFreeCount()` to tell you how many are currently in the pool to be recycled
    - [x] Debug render these counts to the screen to you can make sure you're properly recycling during this assignment.
        - At any given time you likely will not have more than 3 textures in use at once, meaning your pool should never exceed that count.  This can really depend on your scene though.  For eaxmple, in this assignment for bloom... 
          1. Camera color output
          2. Camera bloom target
          3. Temporaries
             - Blurring secondary output
             - Composite output 