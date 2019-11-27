float3 MapNormal(const in float3 tan,
	const in float3 bitan,
	const in float3 normal,
	const in float2 tc,
	uniform Texture2D nmap,
	uniform SamplerState splr)
{
	// tan/bitan/normal ���̿��Ͽ� ��������(Ÿ��)���� ��ȯ��Ű�� ��ȯ��� ����
	const float3x3 tanToTarget = float3x3(tan, bitan, normal);

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