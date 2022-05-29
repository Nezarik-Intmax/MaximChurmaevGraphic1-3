#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "GL/glew.h";
#include "GL/glut.h";
#include "glm/glm.hpp";
#include "GL/freeglut.h";
#include <Magick++.h>;
#include <string>;
#define MAX_POINT_LIGHTS 3

struct BaseLight{
	glm::fvec3 Color;
	float AmbientIntensity;
	float DiffuseIntensity;

	BaseLight(){
		Color = glm::fvec3(0.0f, 0.0f, 0.0f);
		AmbientIntensity = 0.0f;
		DiffuseIntensity = 0.0f;
	}
};
struct DirectionalLight: public BaseLight{
	glm::fvec3 Direction;
};
struct PointLight: public BaseLight{
	glm::fvec3 Position;

	struct{
		float Constant;
		float Linear;
		float Exp;
	} Attenuation;

	PointLight(){
		Position = glm::fvec3(0.0f, 0.0f, 0.0f);
		Attenuation.Constant = 1.0f;
		Attenuation.Linear = 0.0f;
		Attenuation.Exp = 0.0f;
	}
};
struct SpotLight: public PointLight{
	glm::fvec3 Direction;
	float Cutoff;

	SpotLight(){
		Direction = glm::fvec3(0.0f, 0.0f, 0.0f);
		Cutoff = 0.0f;
	}
};
struct{
	GLuint Color;
	GLuint AmbientIntensity;
	GLuint DiffuseIntensity;
	GLuint Position;
	struct{
		GLuint Constant;
		GLuint Linear;
		GLuint Exp;
	} Atten;
} m_pointLightsLocation[MAX_POINT_LIGHTS];

struct{
	GLuint Color;
	GLuint AmbientIntensity;
	GLuint DiffuseIntensity;
	GLuint Position;
	GLuint Direction;
	GLuint Cutoff;
	struct{
		GLuint Constant;
		GLuint Linear;
		GLuint Exp;
	} Atten;
} m_spotLightsLocation[MAX_POINT_LIGHTS];