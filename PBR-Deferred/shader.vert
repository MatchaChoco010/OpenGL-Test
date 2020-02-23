#version 460

uniform mat4 Model;
uniform mat4 ModelIT;
uniform mat4 ModelView;
uniform mat4 Projection;

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

out vec3 vWorldNormal;
out vec3 vWorldTangent;
out vec4 vWorldPos;
out vec2 vUv;

void main()
{
  vWorldNormal = mat3(ModelIT) * normal;
  vWorldTangent = mat3(ModelIT) * tangent;
  vWorldPos = Model * position;
  vUv = uv;

  gl_Position = Projection * ModelView * position;
}
