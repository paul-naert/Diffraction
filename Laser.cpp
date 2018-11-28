#include "Laser.h"

CLaser::CLaser(void) {
	pos[0] = 0;
	pos[1] = 0;
	pos[2] = 0;

	dir[0] = 0;
	dir[1] = 0;
	dir[2] = 0;

	waveLen = 1;
	cosAngle = 0;
}
CLaser::CLaser(GLfloat posx, GLfloat posy, GLfloat posz, GLfloat dirx, GLfloat diry, GLfloat dirz, GLfloat waveLength,GLfloat cos) {
	
	pos[0] = posx;
	pos[1] = posy;
	pos[2] = posz;

	dir[0] = dirx;
	dir[1] = diry;
	dir[2] = dirz;

	waveLen = waveLength;
	cosAngle = cos;
}

CLaser::~CLaser(void)
{
}

void CLaser::move(GLfloat dx, GLfloat dy, GLfloat dz) {
	pos[0] += dx;
	pos[1] += dy;
	pos[2] += dz;
}

void CLaser::turn(GLfloat dx, GLfloat dy, GLfloat dz) {
	dir[0] += dx;
	dir[1] += dy;
	dir[2] += dz;
}

void CLaser::changeWL(GLfloat incr) {
	waveLen += incr;
	if (waveLen <= 0) {
		waveLen -= incr;
		waveLen /= 2;
	}
}

