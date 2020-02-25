#version 460

in vec4 worldFragPos;

uniform vec3 worldLightPos;
uniform float far;

void main()
{
  float lightDistance = length(worldFragPos.xyz - worldLightPos);
  lightDistance = lightDistance / far;
  gl_FragDepth = lightDistance;
}
