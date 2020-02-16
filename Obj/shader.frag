#version 460

in vec3 viewNormal;
in vec4 viewPos;

layout (location = 0) out vec4 fragment;

uniform mat4 View;

const vec4 PointLightPos = vec4(2.0, 1.0, 1.0, 1.0);
const vec3 Ldiff = vec3(1.0);
const vec3 Lspec = vec3(1.0);
const vec3 Lamb = vec3(0.1);

const vec3 Kdiff = vec3(0.8, 0.2, 0.2);
const vec3 Kspec = vec3(1.0);
const vec3 Kamb = vec3(0.8, 0.2, 0.2);
const float Kshi = 30.0;

void main ()
{
  vec3 N = normalize(viewNormal);
  vec3 V = -normalize(viewPos.xyz);

  vec4 viewLightPos = View * PointLightPos;
  vec3 L = normalize(viewLightPos.xyz - viewPos.xyz);

  vec3 H = normalize(L + V);

  vec3 Idiff = max(dot(N, L), 0.0) * Kdiff * Ldiff;
  vec3 Ispec = pow(max(dot(N, H), 0.0), Kshi) * Kspec * Lspec;
  vec3 Iamb = Kamb * Lamb;

  fragment = vec4(Idiff + Ispec + Iamb, 1.0);
}
