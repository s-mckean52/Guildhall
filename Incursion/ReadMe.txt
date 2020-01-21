SD1-A6: Incursion Alpha

Known Issues / Items done differently:
    - There is an issue where the app window opens up behind other windows when building
      it from Visual Studio. Windows thinks that the window is still the focus so you
      have to select another window before reselecting the game.

    - GAME_STATE_PLAYING and GAME_STATE_DEATH are their own states but behave as required


How to Use:
  Keyboard:
    - Press the 'P' to pause. Press it again to unpause
    - While paused press 'Esc' to return to title screen

    - Press and hold the 'T' key to enter slow-mo

    - Press and hold the 'Y' key to speed time up

    - Press the 'Escape' key to exit the app from the title screen.
    - Press 'P' to start the game.

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

    - The A button fires a bullet from the nose of the turret

    - Press 'Start' to puase. press it again to unpause.
    
   General Control Rules:
    - Press 'Start' or 'P' to continue
    - Press 'Back' or 'Escape' to leave
    


Deep Learning:
        While working on this project I found mysef focusing heavily on a top down approach. Whenever I came to something that
    seemed rather difficult to address in the scope that I was in I would make a function for doing that thing where it was appropriate.
    However, I did find myself occasionally working through a problem just by coding it out. This resulted in long functions that
    should have been broken up. With tank collisions specifically I found myself doing all of the logic in one function when it could have
    been broken up. Moving forward something that I am going to work on is doing some pre-planning of what kinds of functions I am going
    to need. I have been doing this to some extent but I believe that writing it out will help me process what is actually needed in
    its entirety. 
     