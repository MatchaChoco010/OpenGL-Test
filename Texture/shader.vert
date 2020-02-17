#version 460

uniform mat4 ModelView;
uniform mat4 ModelViewIT;
uniform mat4 Projection;

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

out vec3 viewNormal;
out vec4 viewPos;
out vec2 vUV;

void main()
{
  viewNormal = mat3(ModelViewIT) * normal;
  viewPos = ModelView * position;
  vUV = uv;

  gl_Position = Projection * ModelView * position;
}
