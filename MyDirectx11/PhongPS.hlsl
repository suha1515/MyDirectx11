#include "ShaderOps.hlsl"
#include "LightVectorData.hlsl"

#include "PointLight.hlsl"

//������Ʈ ������ ���� ������������ ���Ѵ�.
cbuffer ObjectCBuf
{
    //float3  materialColor;
    float   specularIntensity;  //���ݻ� ����
    float   specularPower;      //���ݻ� �¼�   
    //float   padding[2];          //�е�
};

Texture2D tex;
SamplerState splr;


float4 main(float3 viewFragPos : POSITION, float3 viewNormal : NORMAL, float2 tc : Texcoord) : SV_TARGET
{
	viewNormal = normalize(viewNormal);
	const LightVectorData lv = CalculateLightVectorData(viewLightPos, viewFragPos);

    //***���ݻ�***//
	// attenuation
	const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
	// diffuse
	const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, viewNormal);
	// specular
	const float3 specular = Speculate(diffuseColor, diffuseIntensity, viewNormal, lv.vToL, viewFragPos, att, specularPower);
    //************//

	// ���� �� (�ؽ�ó ��� ��)
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular), 1.0f);
}

//attenuation �� ������ ������ ���� �˰������ �Ʒ�����Ʈ�� �����Ұ�
//�������� ���� ���ĵ� ����.

//wiki.orgre3d.org/-point+lightPos+Attenuation