#version 430
layout(local_size_x = 16, local_size_y = 16) in;

const int resX = 256;
const int resY = 256;
const float PI = 3.14159;
uniform float wavelen;
uniform float distance;
uniform float dirx;
uniform float diry;
uniform float dirz;

layout(binding = 0, rgba32F) coherent uniform image2D inTex;


float sinc(float x) {
	if (x==0) return 1;
	return sin(x) / x;
}

void main() {
	float cintensity = 0;
	float sintensity = 0;
	float dist = 0;
	float d2 = distance * distance;
	float dcenter = 0;
	float att = 0;
	int posx = int( gl_GlobalInvocationID[0]);
	int posy = int( gl_GlobalInvocationID[1]);
	float intexel = 0;
	int b = 0;
	
	int b2 = 0;
	float texCounter = 0.01;
	float centerx, centery;
	centerx = dirx * distance / dirz;
	centery = diry * distance / dirz;
	for (int i = 0; i < resX; i++) {
		for (int j = 0; j < resY; j++) {
			
			dist = sqrt(float(d2 + (i - posx) * (i - posx) + (j - posy) * (j - posy)));
			dcenter = sqrt((i - posx - centerx) * (i - posx -centerx) + (j - posy - centery) * (j - posy - centery)) / wavelen / distance;
			att = distance/dist * sinc(dcenter * PI);
			intexel = imageLoad(inTex, ivec2(i, j)).y;

			cintensity += intexel * cos(2 * PI *dist / wavelen) * att;
			sintensity += intexel * sin(2 * PI *dist / wavelen) * att;
			//if (cintensity > 0.000001) b = 1;
			//if (sintensity > 0.1) b2 = 1;
			texCounter+=intexel;
		}

	}
	//cintensity /= 500;
	//sintensity /= 600000;
	/*if (cintensity > 500) {
		imageStore(inTex, ivec2(posx, posy), vec4(cintensity * cintensity + sintensity * sintensity, imageLoad(inTex, ivec2(posx, posy)).x, 0.0, 0.0));
	}
	if (sintensity > 300000) {
		imageStore(inTex, ivec2(posx, posy), vec4(0.0, imageLoad(inTex, ivec2(posx, posy)).x, cintensity * cintensity + sintensity * sintensity, 0.0));
	}
	*/	//imageStore(inTex, ivec2(posx, posy), vec4(1.0,0.0,0.0, 0.0));
	
	imageStore(inTex, ivec2(posx, posy), vec4((cintensity * cintensity + sintensity * sintensity) / (texCounter*wavelen), imageLoad(inTex, ivec2(posx, posy)).y, 0.0, 0.0));

//	if (b==1) imageStore(inTex, ivec2(posx, posy), vec4(cintensity * cintensity + sintensity * sintensity, imageLoad(inTex, ivec2(posx, posy)).y, 0.5, 0.0));
//	if (b2==1) imageStore(inTex, ivec2(posx, posy), vec4(cintensity * cintensity + sintensity * sintensity, imageLoad(inTex, ivec2(posx, posy)).y, 1.0, 0.0));
	
}	
