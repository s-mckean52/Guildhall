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

`Up Arrow` and `Down Arrow`: Control dissolve on cube
`Left Arrow` and `Right Arrow`: Control intensity of Projection 
`F9`: Reload currently loaded shaders
`F`: Will toggle fog
`N` and `M`: Adjust the depth of the parallax

### Goal [80/80]
- [x] Be able to support up to at least 8 concurrent lights
- [x] Be able to support different light types...
    - [x] Point Light...
        - [x] Local 
        - [x] Infinite
    - [x] Infinite Directional Light...
    - [x] Spot Light
       - [x] Local
       - [x] Infinite
    - *Have all these visible in the scene concurrently*
    - *Note: Local and Infinite are just attenuation values...*
- [x] Shader should be branchless using a unified lighting structure 
- [x] Support linear fog...
      - [x] `RenderContext::EnableFog( float nearFog, float farFog, rgba nearFogColor, rgba farFogColor );` 
      - [x] `RenderContext::DisableFog()` (shader is just 'always' going to do fog, so what do you set this to to make it not have an effect?)
- [x] Dissolve Shader Effect
    - [x] Support a `Material Block` uniform buffer with your `RenderContext`
        - Suggest a `RenderContext::SetMaterialBuffer( RenderBuffer* buf )`  
    - [x] When binding a dissolve pattern to use, be sure to use a free slot.   
        - Suggest having a slot titled `USER_SLOT_START`, defined to likely 8.  That way when binding a texture 
          a user can just say `USER_SLOT_START + idx` for the slot they want.  
          You can also move your common textures (diffuse, normal, et.al.) to a later slot if you like users starting at 0. 
    - [x] Have your dissolve shader expose the following controls...
        - [x] A dissolve "depth" or value set to 0 to 1 to control how dissolved something is.
        - [x] A dissolve "range" to give a *burned edge* to the dissolve effect.  This is the range near the edge at which is interpolates between burn_start_color and burn_end_color.  
        - [x] A burn start color.
        - [x] A burn end color
        - *Note, the full range you will be moving through with this effect is not 0 to 1, but `(1 + 2 * dissolve_range)` (why?).  You can think of it kinda like a scroll bar.  Meaning - how does the `dissolve_value` or depth that is 0 to 1 actually affect our dissolve range we're using in the shader?*

### Extras
- [x] *X02.10: 02pts*: Shader Reloading. On key press (F10 suggested), reload all shaders in your shader database
- [x] *X07.10: 05%*: Triplanar Shader
- [x] *X07.11: 05%*: Projected Texture Effect
- [x] *X07.12: 05%*: Interior Mapping Shader
- [x] *X07.13: 03%*: Fresnel or Highlight Shader
- [x] *X07.20: 05%*: Parallax Mapping Shader
    - [x] *X07.21 05%*: Self Occluding Parallax Shader, aka Deep/Steep Parallax Shader