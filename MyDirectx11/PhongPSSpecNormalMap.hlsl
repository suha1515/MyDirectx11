 cbuffer LightBuf
{
	float3 viewLightPos;            //������ġ
    float3 ambient;             //�ֺ���
    float3 diffuseColor;        //Ȯ�걤
    float  diffuseIntensity;    //Ȯ�걤 ����
    float attConst;             //�� ���� ���
    float attLin;               //�� ���� �Ÿ�
    float attQuad;              //�� ���� ����ü
};

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

float3 MapNormal(const in float3 tan,
	const in float3 bitan,
	const in float3 normal,
	const in float2 tc,
	uniform Texture2D nmap,
	uniform SamplerState splr)
{
	// tan/bitan/normal ���̿��Ͽ� ��������(Ÿ��)���� ��ȯ��Ű�� ��ȯ��� ����
	const float3x3 tanToTarget = float3x3(
		normalize(tan),
		normalize(bitan),
		normalize(normal));

	const float3 normalSample = nmap.Sample(splr, tc).xyz;
	const float3 tanNormal = normalSample * 2.0f - 1.0f;

	return normalize(mul(tanNormal, tanToTarget));
}

//const in float <<- ? ���� Ű������?
float Attenuate(uniform float attConst, uniform float attLin, uniform float attQuad, const in float distFragToL)
{
	return 1.0f / (attConst + attLin * distFragToL + attQuad * (distFragToL * distFragToL));
}

float3 Diffuse(
	uniform float3 diffuseColor,
	uniform float diffuseIntensity,
	const in float att,
	const in float3 viewDirFragToL,
	const in float3 viewNormal)
{
	return diffuseColor * diffuseIntensity * att * max(0.0f, dot(viewDirFragToL, viewNormal));
}

float3 Speculate(
	const in float3 specularColor,
	uniform float specularIntensity,
	const in float3 viewNormal,
	const in float3 viewFragToL,
	const in float3 viewPos,
	const in float att,
	const in float specularPower)
{
	//�ݻ� ���� ���
	const float3 w = viewNormal * dot(viewFragToL, viewNormal);
	const float3 r = normalize(w * 2.0f - viewFragToL);
	//�佺���̽����� ī�޶󿡼� ���������� ����
	const float3 viewCamToFrag = normalize(viewPos);
	//���ݻ��� ���� �þߺ��Ϳ� �ݻ纤�ͻ����� �������� ���Ѵ�. ( ������ �����¼��� ������)
	return att * specularColor * specularIntensity * pow(max(0.0f, dot(-r, viewCamToFrag)), specularPower);
}

float4 main(float3 viewPos : Position, float3 viewNormal : Normal, float3 tan : Tangent, float3 bitan : Bitangent, float2 tc : Texcoord) : SV_TARGET
{
    if(normalMapEnabled)
    {
		viewNormal = MapNormal(tan, bitan, viewNormal, tc, nmap, splr);
    }
	//��ü�� ���� (����) ����  ���������� ����(����x)
    const float3 viewFragToL = viewLightPos - viewPos;
	//�������� ���������� �Ÿ�
    const float distFragToL = length(viewFragToL);
	//�������� ���������� ���⺤��
    const float3 viewDirFragToL = viewFragToL / distFragToL;
	//***������***//
    // �� ����ġ ���ϱ�
	const float att = Attenuate(attConst, attLin, attQuad, distFragToL);
	// �� ����
	const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, viewDirFragToL, viewNormal);
    //************//

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
	// ���ݻ�
	const float3 specularReflected = Speculate(specularReflectionColor, 1.0f, viewNormal, viewFragToL, viewPos, att, specularPower);
	// ���� �� (�ؽ�ó ��� ��)
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specularReflected * specularReflectionColor), 1.0f);
}