#include <iostream>
#include "GL/glew.h";
#include "GL/glut.h";
#include "glm/glm.hpp";
#include "GL/freeglut.h"

GLuint VBO;
float Scale = 0.0f;
#define M_PI 3.14159265358979323846
#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)
void InitPers(glm::mat4x4& m) {
	const float ar = 1;// m_persProj.Width / m_persProj.Height;
	const float zNear = 0;// m_persProj.zNear;
	const float zFar = 10000;// m_persProj.zFar;
	const float zRange = zNear - zFar;
	const float tanHalfFOV = tanf(ToRadian(70 / 2.0));

	m[0][0] = 1.0f / (tanHalfFOV * 1);
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = 1.0f / tanHalfFOV;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = (-zNear - zFar) / zRange;
	m[2][3] = 2.0f * zFar * zNear / zRange;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 1.0f;
	m[3][3] = 0.0f;

}
/*void InitPerspectiveProj(Matrix4f& m) {
	const float ar = m_persProj.Width / m_persProj.Height;
	const float zNear = m_persProj.zNear;
	const float zFar = m_persProj.zFar;
	const float zRange = zNear - zFar;
	const float tanHalfFOV = tanf(ToRadian(m_persProj.FOV / 2.0));

	m.m[0][0] = 1.0f / (tanHalfFOV * ar);
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = 1.0f / tanHalfFOV;
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	m.m[2][2] = (-zNear - zFar) / zRange;
	m.m[2][3] = 2.0f * zFar * zNear / zRange;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	m.m[3][2] = 1.0f;
	m.m[3][3] = 0.0f;
}*/
void RenderSceneCB() {
	glClear(GL_COLOR_BUFFER_BIT);
	glm::vec3 Vertices[3];
	Vertices[0] = glm::vec3(1.0f, 1.0f, 0.0f);
	Vertices[2] = glm::vec3(-1.0f, 1.0f, 0.0f);
	Vertices[1] = glm::vec3(0.0f, -1.0f, 0.0f);

	Scale += 0.001f;
	glm::mat4x4 World;
	World[0][0] = 1.0f;		   World[0][1] = 0.0f;        World[0][2] = 0.0f;        World[0][3] = 0.0f;
	World[1][0] = 0.0f;        World[1][1] = 1.0f;		  World[1][2] = 0.0f;        World[1][3] = 0.0f;
	World[2][0] = 0.0f;        World[2][1] = 0.0f;        World[2][2] = 1.0f;		 World[2][3] = 0.0f;
	World[3][0] = 0.0f;        World[3][1] = 0.0f;        World[3][2] = 0.0f;        World[3][3] = 1.0f;
	glm::mat4x4 World;
	World[0][0] = 1.0f; World[0][1] = 0.0f; World[0][2] = 0.0f; World[0][3] = sinf(Scale);
	World[1][0] = 0.0f; World[1][1] = 1.0f; World[1][2] = 0.0f; World[1][3] = 0.0f;
	World[2][0] = 0.0f; World[2][1] = 0.0f; World[2][2] = 1.0f; World[2][3] = 0.0f;
	World[3][0] = 0.0f; World[3][1] = 0.0f; World[3][2] = 0.0f; World[3][3] = 1.0f;
	glm::mat4x4 World;
	World[0][0] = cosf(Scale); World[0][1] = -sinf(Scale); World[0][2] = 0.0f; World[0][3] = 0.0f;
	World[1][0] = sinf(Scale); World[1][1] = cosf(Scale);  World[1][2] = 0.0f; World[1][3] = 0.0f;
	World[2][0] = 0.0f;        World[2][1] = 0.0f;         World[2][2] = 1.0f; World[2][3] = 0.0f;
	World[3][0] = 0.0f;        World[3][1] = 0.0f;         World[3][2] = 0.0f; World[3][3] = 1.0f;
	InitPers(World);
	//m_transformation = PersProjTrans * TranslationTrans * RotateTrans * ScaleTrans;
	Vertices[0] = World * glm::vec4(Vertices[0], 1.0f);
	Vertices[1] = World * glm::vec4(Vertices[1], 1.0f);
	Vertices[2] = World * glm::vec4(Vertices[2], 1.0f);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(0);

	glutSwapBuffers();
}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Tutorial 06");
	glutDisplayFunc(RenderSceneCB);
	glutIdleFunc(RenderSceneCB);

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s' \n", glewGetErrorString(res));
		return 1;
	}
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glutMainLoop();
}