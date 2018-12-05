# Simulating diffraction on a GPU compute shader

This is a project for INF8702 in which I try to represent the diffraction figure of a laser on a screen after going through a user-defined mask. 
The code basis for this project is TP2.

## My additions to the code

### Laser Class
In files [laser.cpp](laser.cpp) and [laser.h](laser.h), I define a new class Laser with characteristics such as position, direction, wavelength and aperture.

### Compute Shader
[computeShader.glsl](computeShader.glsl) is a shader I wrote to calculate the intensity of light at a given point on the screen. It adds the contributions of all the points on the mask (green component of the input/output texture) and calculates the norm.
[compute.cpp](compute.cpp) contains just one function which does the compiling of the shader. I adapted code found on   http://wili.cc/blog/opengl-cs.html.

### Main
I have added a few functions to [main.cpp](main.cpp). 

initDiffraction() builds the arrays that will become the textures for the screen and the mask. This is where the original mask layout is defined. It can be changed at runtime by pressing num1-4 which will call the setMaskN() functions, each having a predefined layout.

laserIntersect() calculates the light pattern on the mask considering its layout and the position, the direction and the angle of the laser. It stores the result in the texture arrays for the screen and the mask.

updateLaser() is where I update the mask texture and animate the laser if a predefined animation is set. Warning : since I no longer calculate the diffraction pattern at every frame, animation should be placed somewhere else.

calculDiffraction() is the function which dispatches the calculation of the pattern to the GPU, and updates the texture for the screen.

setWindowFPS() just displays the number of frames per second displayed and the current wavelength in the title of the window.
