#include "ShaderOps.hlsl"
#include "LightVectorData.hlsl"
#include "PointLight.hlsl" 


//오브젝트 고유의 색과 빛나는정도를 정한다.
cbuffer ObjectCBuf
{
    bool normalMapEnabled;
    bool specularMapEnabled;
    bool hasGloss;
    float specularPowerConst;
    float3 specularColor;
    float specularMapweight;
};

Texture2D tex;
// 스페큘러 맵.
Texture2D spec;
Texture2D nmap;

SamplerState splr;

float4 main(float3 viewFragPos : Position, float3 viewNormal : Normal, float3 viewTan : Tangent, float3 viewBitan : Bitangent, float2 tc : Texcoord) : SV_TARGET
{
	//메쉬 노멀 정규화
	viewNormal = normalize(viewNormal);
	//만약 노말매핑이 활성화 되었다면 법선을 매핑된 값으로 대체한다.
    if(normalMapEnabled)
    {
		viewNormal = MapNormal(normalize(viewTan), normalize(viewBitan), viewNormal, tc, nmap, splr);
    }
	const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);

    //시선벡터와 반사벡터사이의 각에 기반하여 정반사 강도를 구한다.
    float3 specularReflectionColor;
    //스페큘러 맵에서 승수값을 받기전까진 기본으로 머터리얼값을 사용한다.
    float specularPower = specularPowerConst;
    if(specularMapEnabled)
    {
        //수치화된 스페큘러가 아닌. 스페큘러맵에서 강도를 가져온다.
        const float4 specularSample = spec.Sample(splr, tc);
        specularReflectionColor = specularSample.rgb * specularMapweight;

        //스페큘러맵의 알파값이 승수값이므로 있는경우 파워값을 받아온다
        if(hasGloss)
        {
            specularPower = pow(2.0f, specularSample.a * 13.0f);
        }
    }
    else
    {
        specularReflectionColor = specularColor;
    }
	//빛감쇄
	const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
	// 확산광
	const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
	// 정반사
	const float3 specularReflected = Speculate(specularReflectionColor, 1.0f, viewNormal, lv.vToL, viewFragPos, att, specularPower);
	//최종색 = 감쇄된 색상 + 텍스쳐색상에의한 주변광 +정반사광
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specularReflected * specularReflectionColor), 1.0f);
}