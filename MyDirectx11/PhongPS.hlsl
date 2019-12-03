#include "ShaderOps.hlsl"
#include "LightVectorData.hlsl"

#include "PointLight.hlsl"

//오브젝트 고유의 색과 빛나는정도를 정한다.
cbuffer ObjectCBuf
{
    //float3  materialColor;
    float   specularIntensity;  //정반사 강도
    float   specularPower;      //정반사 승수   
    //float   padding[2];          //패딩
};

Texture2D tex;
SamplerState splr;


float4 main(float3 viewFragPos : POSITION, float3 viewNormal : NORMAL, float2 tc : Texcoord) : SV_TARGET
{
	viewNormal = normalize(viewNormal);
	const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);

    //***정반사***//
	// attenuation
	const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
	// diffuse
	const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
	// specular
	const float3 specular = Speculate(diffuseColor, diffuseIntensity, viewNormal, lv.vToL, viewFragPos, att, specularPower);
    //************//

	// 최종 색 (텍스처 기반 색)
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular), 1.0f);
}

//attenuation 빛 감쇠의 적절한 값을 알고싶으면 아래사이트를 참고할것
//점광원에 대한 공식도 볼것.

//wiki.orgre3d.org/-point+lightPos+Attenuation