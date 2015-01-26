Course:		FGA2.GRP2
Student #:	130048
Student name:	Eric Polman

Description
===========
The source folder contains a VS2013 project that compiles to a software rasterized 3D version of Asteroids.
Every huge asteroid breaks down into 2 medium sized asteroids, which break down into 2 small asteroids.
The player can shoot the asteroids and needs to avoid them. The goal is to shoot every asteroid without getting hit 3 times.

Instructions
============
A & D to adjust Yaw
W & S to adjust Pitch
Space to accelerate
Left Ctrl to shoot
P to switch between game and 100k dragon-view mode
B to toggle bilinear filtering on skybox
K & L to decrease and increase light intensity
Arrow keys to change light direction

Features
========
Bilinear filtering on the skybox	Implemented to show that I am able to do this, not on objects because of speed considerations
Dynamic lighting			Lighting can be adjusted real-time
Scene graph				The rotating asteroids will demonstrate this. Sometimes, a smaller asteroid is a child of a bigger asteroid and rotating around it.
					Also, the camera is a child of the Spaceship, thus following it everywhere.
Correct rasterizer			No jaggies, overlap, and ugly edges
Tinted textures				Textures can be colorized as well. For instance, check the player, when it hits an asteroid.
Skybox					The skybox assures a constantly filled screen.
OBJ & MTL Parser + Managers		When an OBJ is loaded, it will load the corresponding MTL file and if there is a texture associated with the material, that is loaded as well.
Leak free				Visual Leak Detector assured me the code is leak free.
Sutherland-Hodgeman clipping		I clip against the left, right, and near planes.
Home-made models			I made the bullets, player, and asteroids myself. I did not make the dragon ofcourse.
Sounds					No engine is complete without some sounds playing when you shoot and destroy asteroids. (Sounds are from SoundBible.com)