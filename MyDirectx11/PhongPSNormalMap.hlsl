#include "ShaderOps.hlsl"
#include "LightVectorData.hlsl"

#include "PointLight.hlsl"

cbuffer ObjectCBuf
{
    float specularIntensity;
    float specularPower;
    bool normalMapEnabled;      //bool 형변수는 hlsl 에서 int 와 같은 4byte이다 염두해두자.
    float padding[1];
};

cbuffer TransformCBuf
{
    matrix modelView;
    matrix modelViewProj;
};

Texture2D tex;
Texture2D nmap : register(t2);

SamplerState splr;

float4 main(float3 viewFragPos : Position, float3 viewNormal : Normal, float3 viewTan : Tangent, float3 viewBitan : Bitangent, float2 tc : TEXcoord) : SV_TARGET
{
	// normalize the mesh normal
	viewNormal = normalize(viewNormal);
    //노맙맵으로부터 매핑
    if(normalMapEnabled)
    {
         viewNormal = MapNormal(normalize(viewTan), normalize(viewBitan), viewNormal, tc, nmap, splr);
    }
	// fragment to light vector data
	const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);
	// attenuation
	const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
	// diffuse
	const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
	// specular
	const float3 specular = Speculate(
		diffuseColor, diffuseIntensity, viewNormal,
		lv.vToL, viewFragPos, att, specularPower
	);

	// 최종 색 (텍스처 기반 색)
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular), 1.0f);
}