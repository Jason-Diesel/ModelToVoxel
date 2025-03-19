//
RWTexture2D<float4> TestTexture : register (u0);

[numthreads(8, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	if(DTid.x > 6)
	{
		return;
	}
	float4 pixelColor = TestTexture[uint2(DTid.x, 0)].rgba;
	pixelColor = pixelColor + float4(0.01f, 0.02f, 0.03f, 0);
	if (pixelColor.r > 1)
	{
		pixelColor.r = 0;
	}
	if (pixelColor.g > 1)
	{
		pixelColor.g = 0;
	}
	if (pixelColor.b > 1)
	{
		pixelColor.b = 0;
	}
	
	TestTexture[uint2(DTid.x, 0)] = pixelColor;
}