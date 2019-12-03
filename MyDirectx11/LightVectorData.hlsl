struct LightVectorData
{
	float3 vToL;//물체의 조각 (정점) 에서  광원으로의 벡터(단위x);
	float3 dirToL;//정점부터 광원까지의 거리;
	float distToL;//정점부터 광원까지의 방향벡터;
};

LightVectorData CalculateLightVectorData(const in float3 lightPos, const in float3 fragPos)
{
	LightVectorData lv;
	lv.vToL = lightPos - fragPos;
	lv.distToL = length(lv.vToL);
	lv.dirToL = lv.vToL / lv.distToL;
	return lv;
}