#pragma once
#include  <GL/glew.h>

class CLaser 
{
public:

	CLaser(void);
	CLaser(GLfloat posx, GLfloat posy, GLfloat posz, GLfloat dirx, GLfloat diry, GLfloat dirz, GLfloat waveLength, GLfloat cos);
	~CLaser(void);

	inline void getDir(GLfloat direction[3]);
	inline GLfloat getWL();
	inline void getPos(GLfloat position[3]);
	inline GLfloat getCosAngle(void);

	void move(GLfloat dx, GLfloat dy, GLfloat dz);
	void turn(GLfloat dx, GLfloat dy, GLfloat dz);
	void changeWL(GLfloat incr);
	void changeCos(GLfloat incr);

private:
	GLfloat pos[3];
	GLfloat dir[3];
	GLfloat waveLen;
	GLfloat cosAngle;
};

inline void CLaser::getDir(GLfloat direction[3]) {
	direction[0] = dir[0];
	direction[1] = dir[1];
	direction[2] = dir[2];
}


inline void CLaser::getPos(GLfloat position[3]) {
	position[0] = pos[0];
	position[1] = pos[1];
	position[2] = pos[2];
}

inline GLfloat CLaser::getCosAngle(void) {
	return cosAngle;
}

inline GLfloat CLaser::getWL() {
	return waveLen;
}

