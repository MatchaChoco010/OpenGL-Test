#version 460

layout (location = 0) out vec3 outRadiance;

uniform sampler2D GBuffer0;
uniform sampler2D GBuffer1;
uniform sampler2D GBuffer2;
uniform sampler2D GBuffer3;

uniform vec3 worldLightPosition;
uniform float LightIntensity; // lm
uniform vec3 LightColor;
uniform float LightRange;

uniform vec3 worldCameraPos;
uniform mat4 ViewProjectionI;
uniform vec2 ProjectionParams; // x: near, y: far

uniform vec2 resolution;


const float PI = 3.14159265358979323846;


// ##################
// Calc gbuffer texcoord
// ##################
vec2 CalcTexCoord()
{
   return gl_FragCoord.xy / resolution;
}


// ##################
// world pos from depth texture
// ##################
float DecodeDepth(float d)
{
  return -d * (ProjectionParams.y - ProjectionParams.x) - ProjectionParams.x;
}

vec3 worldPosFromDepth(float d, vec2 uv)
{
  float depth = DecodeDepth(d);
  float m22 = -(ProjectionParams.y + ProjectionParams.x) / (ProjectionParams.y - ProjectionParams.x);
  float m23 = -2.0 * ProjectionParams.y * ProjectionParams.x / (ProjectionParams.y - ProjectionParams.x);
  float z = depth * m22 + m23;
  float w = -depth;
  vec4 projectedPos = vec4(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0, z / w, 1.0);
  vec4 worldPos = ViewProjectionI * projectedPos;
  return worldPos.xyz / worldPos.w;
}


// ##################
// attenuation
// ##################
float DistanceAttenuation(float distance)
{
  float EPSILON = 0.01;
  float att = 1.0 / (distance * distance + EPSILON);
  float smoothatt = 1 - pow(distance / LightRange, 4.0);
  smoothatt = max(smoothatt, 0.0);
  smoothatt =  smoothatt * smoothatt;
  return att * smoothatt;
}

vec3 LightIrradiance(float intensity, vec3 color, vec3 L, vec3 N, float distance)
{
  return 1.0 / (4.0 * PI) * intensity * color * dot(L, N) * DistanceAttenuation(distance);
}


// ##################
// Disney BRDF
// ##################
float SchlickFresnel(float cosTheta)
{
  return pow(clamp((1 - cosTheta), 0, 1), 5.0);
}

float D_GTR1(float NdotH, float a)
{
  float a2 = a * a;
  float tmp = 1 + (a2 - 1) * NdotH * NdotH;
  return (a2 - 1) / (PI * log(a2) * tmp);
}

float D_GTR2aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
{
  float tmp = (HdotX * HdotX) / (ax * ax) + (HdotY * HdotY) / (ay * ay) + NdotH * NdotH;
  return 1 / (PI * ax * ay * tmp * tmp);
}

float G_GGX(float NdotV, float a)
{
  float a2 = a * a;
  float down = NdotV + sqrt(a2 + NdotV * NdotV - a2 * NdotV * NdotV);
  return 1 / down;
}

float G_GGXaniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
  float tmp = VdotX * VdotX * ax * ax + VdotY * VdotY * ay * ay + NdotV * NdotV;
  float down = NdotV + sqrt(tmp);
  return 1 / down;
}

vec3 DisneyBRDF(vec3 L, vec3 V, vec3 N, vec3 H, vec3 X, vec3 Y, vec3 baseColor, float subsurface, float metallic, float specular, float specularTint, float roughness, float anisotropic, float sheen, float sheenTint, float clearcoat, float clearcoatGloss)
{
  float NdotL = dot(N, L);
  float NdotV = dot(N, V);
  if (NdotL < 0 || NdotV < 0) return vec3(0);

  float NdotH = dot(N, H);
  float LdotH = dot(L, H);

  float luminance = 0.3 * baseColor.r + 0.6 * baseColor.g + 0.1 * baseColor.b;
  vec3 C_tint = luminance > 0 ? baseColor / luminance : vec3(1);
  vec3 C_spec = mix(specular * 0.08 * mix(vec3(1), C_tint, specularTint), baseColor, metallic);
  vec3 C_sheen = mix(vec3(1), C_tint, sheenTint);

  // diffuse
  float F_i = SchlickFresnel(NdotL);
  float F_o = SchlickFresnel(NdotV);
  float F_d90 = 0.5 + 2 * LdotH * LdotH * roughness;
  float F_d = mix(1.0, F_d90, F_i) * mix(1.0, F_d90, F_o);

  float F_ss90 = LdotH * LdotH * roughness;
  float F_ss = mix(1.0, F_ss90, F_i) * mix(1.0, F_ss90, F_o);
  float ss = 1.25 * (F_ss * (1 / (NdotL + NdotV) - 0.5) + 0.5);

  float FH = SchlickFresnel(LdotH);
  vec3 F_sheen = FH * sheen * C_sheen;

  vec3 BRDFdiffuse = ((1 / PI) * mix(F_d, ss, subsurface) * baseColor + F_sheen) * (1 - metallic);

  // specular
  float aspect = sqrt(1 - anisotropic * 0.9);
  float roughness2 = roughness * roughness;
  float a_x = max(0.001, roughness2 / aspect);
  float a_y = max(0.001, roughness2 * aspect);
  float D_s = D_GTR2aniso(NdotH, dot(H, X), dot(H, Y), a_x, a_y);
  vec3 F_s = mix(C_spec, vec3(1), FH);
  float G_s = G_GGXaniso(NdotL, dot(L, X), dot(L, Y), a_x, a_y) * G_GGXaniso(NdotV, dot(V, X), dot(V, Y), a_x, a_y);

  vec3 BRDFspecular = G_s * F_s * D_s;

  // clearcoat
  float D_r = D_GTR1(NdotH, mix(0.1, 0.001, clearcoatGloss));
  float F_r = mix(0.04, 1.0, FH);
  float G_r = G_GGX(NdotL, 0.25) * G_GGX(NdotV, 0.25);

  vec3 BRDFclearcoat = vec3(0.25 * clearcoat * G_r * F_r * D_r);

  return BRDFdiffuse + BRDFspecular + BRDFclearcoat;
}


// ###################
// main
// ###################
void main()
{
  vec2 uv = CalcTexCoord();

  vec4 gbuffer0 = texture(GBuffer0, uv);
  vec4 gbuffer1 = texture(GBuffer1, uv);
  vec4 gbuffer2 = texture(GBuffer2, uv);
  vec4 gbuffer3 = texture(GBuffer3, uv);

  vec3 albedo = gbuffer0.rgb;
  float ao = gbuffer0.a;
  vec3 normal = gbuffer1.rgb * 2.0 - 1.0;
  float metallic = gbuffer1.a;
  vec3 tangent = gbuffer2.rgb * 2.0 - 1.0;
  float roughness = gbuffer2.a;
  vec3 emissive = gbuffer3.rgb;
  float depth = gbuffer3.a;

  float subsurface = 0.0;
  float specular = 0.5;
  float specularTint = 0.0;
  float anisotropic = 0.0;
  float sheen = 0.0;
  float sheenTint = 0.5;
  float clearcoat = 0.0;
  float clearcoatGloss = 1.0;

  vec3 bitangent = normalize(cross(tangent, normal));

  vec3 worldPos = worldPosFromDepth(depth, uv);

  vec3 V = normalize(worldCameraPos - worldPos);
  vec3 N = normalize(normal);
  vec3 L = normalize(worldLightPosition - worldPos);
  vec3 H = normalize(L + V);

  float distance = length(worldLightPosition - worldPos);
  vec3 irradiance = LightIrradiance(LightIntensity, LightColor, L, N, distance);
  outRadiance = emissive + DisneyBRDF(L, V, N, H, tangent, bitangent, albedo, subsurface, metallic, specular, specularTint, roughness, anisotropic, sheen, sheenTint, clearcoat, clearcoatGloss) * irradiance;
}
