static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
layout(location = 0) in vec3 Position;												\n\
layout(location = 1) in vec2 TexCoord;												\n\
layout(location = 2) in vec3 Normal;												\n\
layout(location = 3) in vec3 Tangent;												\n\
																					\n\
uniform mat4 gWVP;																	\n\
uniform mat4 gLightWVP;																\n\
uniform mat4 gWorld;																\n\
																					\n\
out vec2 TexCoord0;																	\n\
out vec3 Normal0;                                                                   \n\
out vec3 WorldPos0;																	\n\
out vec4 LightSpacePos;																\n\
out vec3 Tangent0;																	\n\
																					\n\
void main()																			\n\
{																					\n\
	gl_Position = gWVP * vec4(Position, 1.0);										\n\
	LightSpacePos = gLightWVP * vec4(Position, 1.0);								\n\
	Tangent0 = (gWorld * vec4(Tangent, 0.0)).xyz;									\n\
	TexCoord0 = TexCoord;															\n\
	Normal0 = (gWorld * vec4(Normal, 0.0)).xyz;										\n\
	WorldPos0   = (gWorld * vec4(Position, 1.0)).xyz;								\n\
};";

static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
																					\n\
in vec2 TexCoord0;																	\n\
in vec3 Normal0;                                                                    \n\
in vec3 WorldPos0;																	\n\
in vec3 Tangent0;																	\n\
in vec4 LightSpacePos;                                                              \n\
const int MAX_POINT_LIGHTS = 3;                                                     \n\
																					\n\
out vec4 FragColor;																	\n\
																					\n\
struct BaseLight                                                                    \n\
{                                                                                   \n\
	vec3 Color;																		\n\
	float AmbientIntensity;															\n\
	float DiffuseIntensity;															\n\
};                                                                                  \n\
																					\n\
struct DirectionalLight                                                             \n\
{                                                                                   \n\
	BaseLight Base;																	\n\
	vec3 Direction;                                                                 \n\
};                                                                                  \n\
																					\n\
struct Attenuation                                                                  \n\
{                                                                                   \n\
	float Constant;																	\n\
	float Linear;																	\n\
	float Exp;																		\n\
};                                                                                  \n\
																					\n\
struct PointLight                                                                   \n\
{                                                                                   \n\
	BaseLight Base;							                                        \n\
	vec3 Position;                                                                  \n\
	Attenuation Atten;                                                              \n\
};																					\n\
																					\n\
struct SpotLight																	\n\
{																					\n\
	PointLight Base;																\n\
	vec3 Direction;																	\n\
	float Cutoff;																	\n\
};																					\n\
																					\n\
uniform int gNumPointLights;                                                        \n\
uniform int gNumSpotLights;															\n\
uniform DirectionalLight gDirectionalLight;                                         \n\
uniform PointLight gPointLights[MAX_POINT_LIGHTS];									\n\
uniform SpotLight gSpotLights[MAX_POINT_LIGHTS];									\n\
uniform sampler2D gSampler;															\n\
uniform sampler2D gShadowMap;														\n\
uniform sampler2D gNormalMap;														\n\
                                                                                    \n\
uniform vec3 gEyeWorldPos;                                                          \n\
uniform float gMatSpecularIntensity;                                                \n\
uniform float gSpecularPower;														\n\
																					\n\
vec3 CalcBumpedNormal()																\n\
{																					\n\
	vec3 Normal = normalize(Normal0);												\n\
	vec3 Tangent = normalize(Tangent0);												\n\
	Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);					\n\
	vec3 Bitangent = cross(Tangent, Normal);										\n\
	vec3 BumpMapNormal = texture(gNormalMap, TexCoord0).xyz;						\n\
	BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);						\n\
	vec3 NewNormal;																	\n\
	mat3 TBN = mat3(Tangent, Bitangent, Normal);									\n\
	NewNormal = TBN * BumpMapNormal;												\n\
	NewNormal = normalize(NewNormal);												\n\
	return NewNormal;																\n\
}																					\n\
float CalcShadowFactor(vec4 LightSpacePos){															\n\
	vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;											\n\
	vec2 UVCoords;																					\n\
	UVCoords.x = 0.5 * ProjCoords.x + 0.5;															\n\
	UVCoords.y = 0.5 * ProjCoords.y + 0.5;															\n\
	float z = 0.5 * ProjCoords.z + 0.5;																\n\
	float Depth = texture(gShadowMap, UVCoords).x;													\n\
	if(Depth < (z + 0.00000001))																	\n\
		return 0.5;																					\n\
	else																							\n\
		return 1.0;																					\n\
}																									\n\
vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal, float ShadowFactor)		\n\
{																									\n\
	vec4 AmbientColor = vec4(Light.Color, 1.0f) * Light.AmbientIntensity;							\n\
	float DiffuseFactor = dot(Normal, -LightDirection);											\n\
	float DiffuseFactor1 = 1;																		\n\
																									\n\
	vec4 DiffuseColor = vec4(0, 0, 0, 0);															\n\
	vec4 SpecularColor = vec4(0, 0, 0, 0);															\n\
																									\n\
	if (DiffuseFactor > 0) {																		\n\
		DiffuseColor = vec4(Light.Color, 1.0f) * Light.DiffuseIntensity * DiffuseFactor;			\n\
		vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0);										\n\
		vec3 LightReflect = normalize(reflect(LightDirection, Normal));								\n\
		float SpecularFactor = dot(VertexToEye, LightReflect);										\n\
		SpecularFactor = pow(SpecularFactor, gSpecularPower);										\n\
		if (SpecularFactor > 0) {																	\n\
			SpecularColor = vec4(Light.Color, 1.0f) *												\n\
				gMatSpecularIntensity * SpecularFactor;												\n\
		}																							\n\
	}																								\n\
																									\n\
	return (AmbientColor + (ShadowFactor * (DiffuseColor +  SpecularColor)));						\n\
}																									\n\
vec4 CalcDirectionalLight(vec3 Normal)																\n\
{																									\n\
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal, 1.0);		\n\
}																									\n\
vec4 CalcPointLight(PointLight l, vec3 Normal, vec4 LightSpacePos)									\n\
{																									\n\
	vec3 LightDirection = WorldPos0 - l.Position;													\n\
	float Distance = length(LightDirection);														\n\
	LightDirection = normalize(LightDirection);														\n\
	float ShadowFactor = CalcShadowFactor(LightSpacePos);											\n\
																									\n\
	vec4 Color = CalcLightInternal(l.Base, LightDirection, Normal, ShadowFactor);					\n\
	float Attenuation = l.Atten.Constant +															\n\
		l.Atten.Linear * Distance +																	\n\
		l.Atten.Exp * Distance * Distance;															\n\
																									\n\
	return Color / Attenuation;																		\n\
}																									\n\
vec4 CalcSpotLight(SpotLight l, vec3 Normal, vec4 LightSpacePos)									\n\
{																									\n\
	vec3 LightToPixel = normalize(WorldPos0 - l.Base.Position);										\n\
	float SpotFactor = dot(LightToPixel, normalize(l.Direction));									\n\
																									\n\
	if (SpotFactor > l.Cutoff) {																	\n\
		vec4 Color = CalcPointLight(l.Base, Normal, LightSpacePos);									\n\
		return Color * (1.0 - (1.0 - SpotFactor) * 1.0 / (1.0 - l.Cutoff));							\n\
	}																								\n\
	else {																							\n\
		return vec4(0, 0, 0, 0);																	\n\
	}																								\n\
}																									\n\
void main()																							\n\
{																									\n\
	vec3 Normal = CalcBumpedNormal();																\n\
    vec4 TotalLight = CalcDirectionalLight(Normal);													\n\
																									\n\
    for (int i = 0 ; i < gNumPointLights ; i++) {													\n\
        TotalLight += CalcPointLight(gPointLights[i], Normal, LightSpacePos);						\n\
    }																								\n\
	for (int i = 0 ; i < gNumSpotLights ; i++) {													\n\
		TotalLight += CalcSpotLight(gSpotLights[i], Normal, LightSpacePos);							\n\
	}																								\n\
																									\n\
    FragColor = texture2D(gSampler, TexCoord0.xy) * TotalLight;										\n\
};";


static const char* shadow_pVS = "													\n\
#version 330																		\n\
																					\n\
layout (location = 0) in vec3 Position;												\n\
layout (location = 1) in vec2 TexCoord;												\n\
layout (location = 2) in vec3 Normal;												\n\
																					\n\
uniform mat4 gWVP;																	\n\
																					\n\
out vec2 TexCoordOut;																\n\
																					\n\
void main()																			\n\
{																					\n\
    gl_Position = gWVP * vec4(Position, 1.0);										\n\
    TexCoordOut = TexCoord;															\n\
}";

static const char* shadow_pFS = "													\n\
#version 330																		\n\
																					\n\
in vec2 TexCoordOut;																\n\
uniform sampler2D gShadowMap;														\n\
																					\n\
out vec4 FragColor;																	\n\
																					\n\
void main()																			\n\
{																					\n\
    float Depth = texture(gShadowMap, TexCoordOut).x;								\n\
    Depth = 1.0 - (1.0 - Depth) * 25.0;												\n\
    FragColor = vec4(vec3(Depth), 1.0);												\n\
}";

static const char* skybox_pVS = "													\n\
#version 330																		\n\
																					\n\
layout(location = 0) in vec3 Position;												\n\
																					\n\
uniform mat4 gWVP;																	\n\
																					\n\
out vec3 TexCoord0;																	\n\
																					\n\
void main()																			\n\
{																					\n\
	vec4 WVP_Pos = gWVP * vec4(Position, 1.0);										\n\
    gl_Position = WVP_Pos.xyww;														\n\
    TexCoord0 = Position;															\n\
}";

static const char* skybox_pFS = "													\n\
#version 330																		\n\
																					\n\
in vec3 TexCoord0;																	\n\
uniform samplerCube gCubemapTexture;												\n\
																					\n\
out vec4 FragColor;																	\n\
																					\n\
void main()																			\n\
{																					\n\
    FragColor = texture(gCubemapTexture, TexCoord0);								\n\
}";