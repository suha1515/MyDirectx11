#include "ShaderOps.hlsl"
#include "PointLight.hlsl" 

//������Ʈ ������ ���� ������������ ���Ѵ�.
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
// ����ŧ�� ��.
Texture2D spec;
Texture2D nmap;

SamplerState splr;

float4 main(float3 viewPos : Position, float3 viewNormal : Normal, float3 viewTan : Tangent, float3 viewBitan : Bitangent, float2 tc : Texcoord) : SV_TARGET
{
	//�޽� ��� ����ȭ
	viewNormal = normalize(viewNormal);
	//���� �븻������ Ȱ��ȭ �Ǿ��ٸ� ������ ���ε� ������ ��ü�Ѵ�.
    if(normalMapEnabled)
    {
		viewNormal = MapNormal(normalize(viewTan), normalize(viewBitan), viewNormal, tc, nmap, splr);
    }
	//��ü�� ���� (����) ����  ���������� ����(����x)
    const float3 viewFragToL = viewLightPos - viewPos;
	//�������� ���������� �Ÿ�
    const float distFragToL = length(viewFragToL);
	//�������� ���������� ���⺤��
    const float3 viewDirFragToL = viewFragToL / distFragToL;

    //�ü����Ϳ� �ݻ纤�ͻ����� ���� ����Ͽ� ���ݻ� ������ ���Ѵ�.
    float3 specularReflectionColor;
    //����ŧ�� �ʿ��� �¼����� �ޱ������� �⺻���� ���͸����� ����Ѵ�.
    float specularPower = specularPowerConst;
    if(specularMapEnabled)
    {
        //��ġȭ�� ����ŧ���� �ƴ�. ����ŧ���ʿ��� ������ �����´�.
        const float4 specularSample = spec.Sample(splr, tc);
        specularReflectionColor = specularSample.rgb * specularMapweight;

        //����ŧ������ ���İ��� �¼����̹Ƿ� �ִ°�� �Ŀ����� �޾ƿ´�
        if(hasGloss)
        {
            specularPower = pow(2.0f, specularSample.a * 13.0f);
        }
    }
    else
    {
        specularReflectionColor = specularColor;
    }
	//������
	const float att = Attenuate(attConst, attLin, attQuad, distFragToL);
	// Ȯ�걤
	const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, viewDirFragToL, viewNormal);
	// ���ݻ�
	const float3 specularReflected = Speculate(specularReflectionColor, 1.0f, viewNormal, viewFragToL, viewPos, att, specularPower);
	//������ = ����� ���� + �ؽ��Ļ������� �ֺ��� +���ݻ籤
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specularReflected * specularReflectionColor), 1.0f);
}