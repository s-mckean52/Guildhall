SD1-A4: Protogame2D

Known Issues / Items done differently:
    - There is an issue where the app window opens up behind other windows when building
      it from Visual Studio. Windows thinks that the window is still the focus so you
      have to select another window before reselecting the game.

    - For drawing AABB2s and specifically the tiles I used appendAABB2Verts and then call
      DrawVertexArray on the whole thing in an attempt to have faster render times in.


How to Use:
  Keyboard:
    - Press the 'P' key to pause. Press it again to unpause

    - Press and hold the 'T' key to enter slow-mo

    - Press and hold the 'Y' key to speed time up

    - Press the 'Escape' key to exit the app.

    - 'F1': When pressed enables debug mode. Press again to disable.

    - 'F3': When pressed enables NoClip allowing the player to move through solid blocks.

    - 'F4': When pressed shows the entire map.

    - 'F8': When pressed the game is recreated.

    - The arrow keys control the movement of the player's tank. Pressing the up arrow accelerates the tank
      in its forward direction. The left and right arrow keys rotate it in the respective direction; pressing
      both at the same time does not rotate the tank.

  Xbox Controller:
    - The left stick controls the direction and acceleration of the player's tank

    - The Right stick controls the direction of the tank's turret


Deep Learning:
        While I worked on this project the thing that I tried to prioritize was making clean and readable code.
        In this area I have a lot to work on, I found. Knowing when to break a function that is too long into 
        the approptiate sub functions is hard for me to see. Improvement will come with practice. Moving forward
        I need to make sure functions work properly while implementing them. I found issues with multiple engine
        functions that went unnoticed until they caused a problem. I did manage my time way better for this project.
     