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

float4 main(float3 worldPos : Position, float3 n : Normal,float3 tan : Tangent,float3 bitan : Bitangent, float2 tc : Texcoord) : SV_TARGET
{
    //�븿�����κ��� ����
    if(normalMapEnabled)
    {
        // build the tranform (rotation) into tangent space
        const float3x3 tanToView = float3x3(
            normalize(tan),
            normalize(bitan),
            normalize(n)
        );
        // unpack the normal from map into tangent space        
        const float3 normalSample = nmap.Sample(splr, tc).xyz;
        n.x = normalSample.x * 2.0f - 1.0f;
        n.y = -normalSample.y * 2.0f + 1.0f;
        n.z = normalSample.z;
        // bring normal from tanspace into view space
        n = mul(n, tanToView);
    }
   //��ü�� ���� (����) ����  ���������� ����(����x)
    const float3 vToL = lightPos - worldPos;
	//�������� ���������� �Ÿ�
    const float distToL = length(vToL);
	//�������� ���������� ���⺤��
    const float3 dirToL = vToL / distToL;
	//***������***//
    // �� ����ġ ���ϱ�
    const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// �� ����
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, n));
    //************//

    //***���ݻ�***//
    //�����Ϳ� ���� �ݻ纤��
    const float3 w = n * dot(vToL, n);
    const float3 r = w * 2.0f - vToL;
    //�þߺ��Ϳ� ���ݻ纤�� ������ ������ ������� ���ݻ��� ������ �������� �Բ� ���Ѵ�
    const float3 specular = att * (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(worldPos))), specularPower);
    //************//

	// ���� �� (�ؽ�ó ��� ��)
    //return float4(saturate(diffuse + ambient + specular), 1.0f) * tex.Sample(splr, tc);
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular), 1.0f);
}