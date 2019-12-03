#include "ShaderOps.hlsl"
#include "LightVectorData.hlsl"
#include "PointLight.hlsl"
cbuffer ObjectCBuf
{
    float specularIntensity;
    float specularPower;
    bool normalMapEnabled;
    float padding[1];
};

#include "Transform.hlsl"
Texture2D tex;
Texture2D nmap : register(t2);

SamplerState splr;


float4 main(float3 viewFragPos : Position, float3 viewNormal : Normal, float2 tc : Texcoord) : SV_Target
{
	// sample normal from map if normal mapping enabled
    if (normalMapEnabled)
    {
        // sample and unpack normal data
        const float3 normalSample = nmap.Sample(splr, tc).xyz;
		// 오브젝트 스페이스 노멀.
		const float3 objectNormal = normalSample * 2.0f - 1.0f;
		// 노멀을 오브젝트스페이스에서 뷰스페이스로 가져온다. 다시 정규화도진행
		viewNormal = normalize(mul(objectNormal, (float3x3) modelView));
    }
	// 빛계산
	const LightVectorData lv = CaculateLightVectorData(viewLightPos, viewFragPos);
	// 빛감쇄
	const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
	// 확산광 세기
    const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
	// 정반사
	const float3 specular = Speculate(
		specularIntensity.rrr, 1.0f, viewNormal, lv.vToL,
		viewFragPos, att, specularPower
	);
	// final color
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular), 1.0f);
}