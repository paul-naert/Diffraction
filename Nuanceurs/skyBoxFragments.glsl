#version 430 core

uniform samplerCube colorMap;

in vec3 fragTexCoord;
out vec4 color;
void main (void) 
{
   // compléter le nuanceur ici pour texturer le cube
 
    vec4 out_color=texture(colorMap,fragTexCoord);
    color = clamp(out_color, 0.0, 1.0);

}
