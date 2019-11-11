// Solid 쉐이더
// 단순히 색깔만 표현하는 쉐이더이다.

cbuffer CBuf
{
    float4 color;
}

float4 main() : SV_TARGET
{
    return color;
}