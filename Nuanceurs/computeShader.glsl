#version 430
layout(local_size_x = 16, local_size_y = 16) in;


const float PI = 3.14159;
const int resX = 200;
const int resY = 200;
uniform float wavelen;
uniform float distance;
uniform float dirx;
uniform float diry;
uniform float dirz;

layout(binding = 0, rgba32F) coherent uniform image2D inTex;


float sinc(float x) { // cardinal sine
	if (x==0) return 1;
	return sin(x) / x;
}

void main() {
	float cintensity = 0; // cosine component
	float sintensity = 0; // sine component
	float dist = 0; 
	float d2 = distance * distance; // not to recalculate everytime
	float dcenter = 0; //distance to the projection of the center of the laser
	float att = 0; //attenuation
	int posx = int( gl_GlobalInvocationID[0]); //position of the element on the grid
	int posy = int( gl_GlobalInvocationID[1]);
	float intexel = 0; 
	float texCounter = 0.01; // for scaling luminosity
	float centerx, centery; //store the relative position of the center of the laser
	centerx = -diry * distance / dirz;
	centery = -dirx * distance / dirz;
	for (int i = 0; i < resX; i++) {
		for (int j = 0; j < resY; j++) {
			
			dist = sqrt(float(d2 + (i - posx - centerx) * (i - posx - centerx) + (j - posy - centery) * (j - posy - centery)));
			dcenter = sqrt((i - posx - centerx) * (i - posx -centerx) + (j - posy - centery) * (j - posy - centery)) / wavelen / distance;
			//attenuation in 1/d² * sinc around the center, the "distance" is constant just for scaling
			att = distance/dist * sinc(dcenter * PI);

			intexel = imageLoad(inTex, ivec2(i, j)).y;

			cintensity += intexel * cos(2 * PI *dist / wavelen) * att;
			sintensity += intexel * sin(2 * PI *dist / wavelen) * att;
			texCounter+=intexel;
		}

	}
	
	imageStore(inTex, ivec2(posx, posy), vec4((cintensity * cintensity + sintensity * sintensity) / (texCounter*wavelen), imageLoad(inTex, ivec2(posx, posy)).y, 0.0, 0.0));


}	
