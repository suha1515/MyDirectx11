cbuffer LightCBuf
{
    float3 lightPos;
    float3 ambient;
    float3 diffuseColor;
    float diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

cbuffer ObjectCBuf
{
    float specularIntensity;
    float specularPower;
    bool normalMapEnabled;      //bool �������� hlsl ���� int �� ���� 4byte�̴� �����ص���.
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

float4 main(float3 viewPos : Position, float3 viewNormal : Normal, float3 tan : Tangent, float3 bitan : Bitangent, float2 tc : TEXcoord) : SV_TARGET
{
    //�븿�����κ��� ����
    if(normalMapEnabled)
    {
        // build the tranform (rotation) into tangent space
        const float3x3 tanToView = float3x3(
            normalize(tan),
            normalize(bitan),
            normalize(viewNormal)
        );
        // unpack the normal from map into tangent space        
        const float3 normalSample = nmap.Sample(splr, tc).xyz;
		float tanNormal;
		tanNormal = normalSample * 2.0f - 1.0f;
		tanNormal.y = -tanNormal.y;
        // bring normal from tanspace into view space
		viewNormal = normalize(mul(tanNormal, tanToView));
    }
   //��ü�� ���� (����) ����  ���������� ����(����x)
    const float3 vToL = lightPos - viewPos;
	//�������� ���������� �Ÿ�
    const float distToL = length(vToL);
	//�������� ���������� ���⺤��
    const float3 dirToL = vToL / distToL;
	//***������***//
    // �� ����ġ ���ϱ�
    const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// �� ����
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, viewNormal));
    //************//

    //***���ݻ�***//
    //�����Ϳ� ���� �ݻ纤��
    const float3 w = viewNormal * dot(vToL, viewNormal);
    const float3 r = w * 2.0f - vToL;
    //�þߺ��Ϳ� ���ݻ纤�� ������ ������ ������� ���ݻ��� ������ �������� �Բ� ���Ѵ�
    const float3 specular = att * (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), specularPower);
    //************//

	// ���� �� (�ؽ�ó ��� ��)
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular), 1.0f);
}