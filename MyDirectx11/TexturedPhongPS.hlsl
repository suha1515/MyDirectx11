//텍스처가 입혀진 오브젝트를 대상의 픽셀쉐이더 
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
    float padding[2];
};

Texture2D tex;

SamplerState splr;

float4 main(float3 worldPos : POSITION, float3 n : NORMAL, float2 tc : Texcoord) : SV_Target
{
	//물체의 조각 (정점) 에서  광원으로의 벡터(단위x)
    const float3 vToL = lightPos - worldPos;
	//정점부터 광원까지의 거리
    const float distToL = length(vToL);
	//정점부터 광원까지의 방향벡터
    const float3 dirToL = vToL / distToL;
	//***점광원***//
    // 빛 섞기
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
	// 최종 색 텍스쳐이므로 샘플을이용하여 머터리얼이 아닌 텍스쳐색깔을 가져온다.
    return float4(saturate(diffuse + ambient + specular), 1.0f) * tex.Sample(splr, tc);
}