SDST Assignment 01 - Protogame3D

Controls:
	Movement:
		'W' - Move Relative Forward on XY-Plane
		'A' - Move Relative Left on XY-Plane
		'S' - Move Relative Backward on XY-Plane
		'D' - Move Relative Right on XY-Plane
		'Q' - Move Up
		'E' - Move Down

	Other:
		'~' - Open the dev console
		'I' - Set Yaw, Pitch, and Roll to 0
		'O' - Set Position and Rotations to 0

Notes:
	The rendering of the world basis and the 'compass' are done in the same way.
	Both use the debug rendering system that we put in place last semester. This
	draws arrows instead of 'ribbons'. Arrows are drawn using a cylinder and a 
	cone at the pointing direction, and they use backface culling. They are still 
	visible from all sides except from the inside of the arrow itself. Each basis
	arrow is its own mesh and is drawn in seperate draw calls.