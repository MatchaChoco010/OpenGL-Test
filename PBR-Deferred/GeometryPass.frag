#version 460

in vec3 vWorldNormal;
in vec3 vWorldTangent;
in float vDepth;
in vec2 vUv;

layout (location = 0) out vec4 GBuffer0; // rgb: albedo, a: ambient occlusion
layout (location = 1) out vec4 GBuffer1; // rgb: world normal, a: metallic
layout (location = 2) out vec4 GBuffer2; // rgb: world tangent, a: roughness
layout (location = 3) out vec4 GBuffer3; // rgb: emissive, a: depth

uniform sampler2D albedoMap;
uniform sampler2D aoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D normalMap;
uniform sampler2D emissiveMap;
uniform float emissiveIntensity;

void getNormalAndTangent(out vec3 normal, out vec3 tangent)
{
  vec3 vNormal = normalize(vWorldNormal);
  vec3 vTangent = normalize(vWorldTangent);
  vec3 bitangent = normalize(cross(vTangent, vNormal));
  vec3 normalFromMap = texture(normalMap, vUv).xyz;
  mat3 TBN = mat3(vTangent, bitangent, vNormal);
  normal = normalize(TBN * (normalFromMap * 2.0 - 1.0));
  tangent = normalize(cross(bitangent, normal));
}

void main()
{
  vec4 albedo = texture(albedoMap, vUv);
  if  (albedo.a < 0.5) discard;
  vec4 ao = texture(aoMap, vUv);
  float metallic = texture(metallicMap, vUv).r;
  float roughness = texture(roughnessMap, vUv).r;
  vec3 normal;
  vec3 tangent;
  getNormalAndTangent(normal, tangent);
  vec3 emissive = texture(emissiveMap, vUv).rgb * emissiveIntensity;

  GBuffer0 = vec4(albedo.rgb, ao);
  GBuffer1 = vec4(normal * 0.5 + 0.5, metallic);
  GBuffer2 = vec4(tangent * 0.5 + 0.5, roughness);
  GBuffer3 = vec4(emissive, vDepth);
}
