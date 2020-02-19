#version 460

in vec3 vWorldNormal;
in vec4 vWorldPos;
in vec3 vWorldTangent;
in vec2 vUv;

layout (location = 0) out vec4 fragment;

uniform vec3 worldCameraPos;

uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D normalMap;
uniform sampler2D emissiveMap;

const float PI = 3.14159265358979323846;

const vec3 directionalLightDir = vec3(0.0, -1.0, 0.0) ;
const float directionalLightIlluminance = 6;
const vec3 directionalLightColor = vec3(1.0, 1.0, 1.0);

#define PointLightMax 2
const vec3[PointLightMax] pointLightPositions = vec3[](
  vec3(0.0, 0.0, 3.0),
  vec3(2.0, 0.0, 2.0)
);
const float[PointLightMax] pointLightLuminousFlux = float[](
  10,
  5
);
const vec3[PointLightMax] pointLightColor = vec3[](
  vec3(1.0, 1.0, 0.8),
  vec3(0.8, 1.0, 1.0)
);

// gamma correction
const float  SCALE_0= 1.0/12.92;
const float  SCALE_1= 1.0/1.055;
const float  OFFSET_1= 0.055 * SCALE_1;
float LinearToSRGB_F( float color )
{
    color= clamp( color, 0.0, 1.0 );
    if( color < 0.0031308 ){
        return  color * 12.92;
    }
    return  1.055 * pow( color, 0.41666 ) - 0.055;
}
vec3 LinearToSRGB( vec3 color )
{
    return  vec3(
        LinearToSRGB_F( color.x ),
        LinearToSRGB_F( color.y ),
        LinearToSRGB_F( color.z ) );
}

vec3 baseColor = vec3(1.0, 0.0, 0.0);
float metallic = 0;
float subsurface = 0;
float specular = .5;
float roughness = .2;
float specularTint = 0;
float anisotropic = 0;
float sheen = 0;
float sheenTint = .5;
float clearcoat = 0;
float clearcoatGloss = 1;


float sqr(float x) { return x*x; }


// // Disney BRDF
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

void main ()
{
  vec4 baseColor = texture(albedoMap, vUv);
  if (baseColor.a < 0.5) discard;
  float subsurface = 0.0;
  float metallic = texture(metallicMap, vUv).r;
  float specular = 0.5;
  float specularTint = 0.0;
  float roughness = texture(roughnessMap, vUv).r;
  float anisotropic = 0.0;
  float sheen = 0.0;
  float sheenTint = 0.5;
  float clearcoat = 0.0;
  float clearcoatGloss = 1.0;

  vec3 normal = normalize(vWorldNormal);
  vec3 tangent = normalize(vWorldTangent);
  vec3 binormal = normalize(cross(normal, tangent));
  vec3 normalFromMap = texture(normalMap, vUv).xyz;
  normalFromMap = 2 * normalFromMap - vec3(1.0);
  mat3 normalMat = mat3(tangent, binormal, normal);
  normal = normalMat * normalFromMap;
  tangent = normalize(cross(normal, binormal));
  binormal = normalize(cross(normal, tangent));

  vec3 emissive = texture(emissiveMap, vUv).rgb;

  // vec3 ao

  vec3 V = normalize(worldCameraPos - vWorldPos.xyz);
  vec3 N = normalize(normal);

  vec3 reflectedLight = vec3(0.0);

  {
    // Directional Light
    vec3 L = normalize(-directionalLightDir);
    vec3 H = normalize(L + V);
    vec3 irradiance = directionalLightIlluminance * directionalLightColor * dot(N, L);

    reflectedLight += DisneyBRDF(L, V, N, H, tangent, binormal, baseColor.rgb, subsurface, metallic, specular, specularTint, roughness, anisotropic, sheen, sheenTint, clearcoat, clearcoatGloss) * irradiance;
  }

  {
    // Point Lights
    for (int i = 0; i < PointLightMax; i++)
    {
      vec3 L = normalize(pointLightPositions[i] - vWorldPos.xyz);
      vec3 H = normalize(L + V);
      float attenuation = 1 / pow(length(pointLightPositions[i] - vWorldPos.xyz), 2);
      vec3 irradiance = pointLightLuminousFlux[i] * attenuation * pointLightColor[i] * dot(N, L);

      reflectedLight += DisneyBRDF(L, V, N, H, tangent, binormal, baseColor.rgb, subsurface, metallic, specular, specularTint, roughness, anisotropic, sheen, sheenTint, clearcoat, clearcoatGloss) * irradiance;
    }
  }

  vec3 outgoingLight = emissive + reflectedLight;

  fragment = vec4(LinearToSRGB(outgoingLight), 1.0);
}
