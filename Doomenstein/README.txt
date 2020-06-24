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
	When parsing XML files I check for nodes that are present but unsupported.
	For attribute parsing, however, I parse them by name and only error if that
	element was unable to be parsed.