#include <iostream>
#include "GL/glew.h";
#include "GL/glut.h";
#include "glm/glm.hpp";
#include "GL/freeglut.h"

GLuint VBO;
float Scale = 0.0f;
void RenderSceneCB() {
	glClear(GL_COLOR_BUFFER_BIT);
	glm::vec3 Vertices[3];
	Vertices[0] = glm::vec3(1.0f, 1.0f, 0.0f);
	Vertices[2] = glm::vec3(-1.0f, 1.0f, 0.0f);
	Vertices[1] = glm::vec3(0.0f, -1.0f, 0.0f);

	Scale += 0.001f;
	glm::mat4x4 World;
	World[0][0] = sinf(Scale); World[0][1] = 0.0f;        World[0][2] = 0.0f;        World[0][3] = 0.0f;
	World[1][0] = 0.0f;        World[1][1] = cosf(Scale); World[1][2] = 0.0f;        World[1][3] = 0.0f;
	World[2][0] = 0.0f;        World[2][1] = 0.0f;        World[2][2] = sinf(Scale); World[2][3] = 0.0f;
	World[3][0] = 0.0f;        World[3][1] = 0.0f;        World[3][2] = 0.0f;        World[3][3] = 1.0f;
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