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

float4 main(float3 worldPos : Position, float3 n : Normal,float3 tan : Tangent,float3 bitan : Bitangent, float2 tc : Texcoord) : SV_TARGET
{
    //노맙맵으로부터 매핑
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
   //물체의 조각 (정점) 에서  광원으로의 벡터(단위x)
    const float3 vToL = lightPos - worldPos;
	//정점부터 광원까지의 거리
    const float distToL = length(vToL);
	//정점부터 광원까지의 방향벡터
    const float3 dirToL = vToL / distToL;
	//***점광원***//
    // 빛 감쇄치 정하기
    const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// 빛 강도
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, n));
    //************//

    //***정반사***//
    //빛벡터에 대한 반사벡터
    const float3 w = n * dot(vToL, n);
    const float3 r = w * 2.0f - vToL;
    //시야벡터와 빛반사벡터 사이의 각도를 기반으로 정반사의 강도를 제곱수와 함께 구한다
    const float3 specular = att * (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(worldPos))), specularPower);
    //************//

	// 최종 색 (텍스처 기반 색)
    //return float4(saturate(diffuse + ambient + specular), 1.0f) * tex.Sample(splr, tc);
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular), 1.0f);
}