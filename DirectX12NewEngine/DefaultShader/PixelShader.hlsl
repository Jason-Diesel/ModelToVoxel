#include "Defines.hlsli"

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3x3 TBN : TBN;
    float2 uv : UV;
    float4 fragpos : FRAG_POS;
};

cbuffer MVP : register(b0)
{
    row_major matrix view;
    row_major matrix projection;
    float4 cameraPos;
};

cbuffer MaterialBuffer : register(b2)
{
    float4 ka;
    float4 kd;
    float4 ks;
    float4 ns;
    int materialFlag;
    int4 materialIndex;//Diffuse, Normal, Height, LightTexture
}

cbuffer LightData : register(b3)
{
    uint nrOfLights;
    row_major matrix ViewProjectionMatrix[MAXNROFLIGHTS];
    int4 ShadowMapInfo[MAXNROFLIGHTS];          //X = Shadowmap index in texture array, Y = ShadowSoftness 0 = hard, zw Not defined yet//have one for nroflights
    float4 LightPosoLightType[MAXNROFLIGHTS];   //(xyz) = lightPos, w = lightType
    float4 LightColor[MAXNROFLIGHTS];           //(xyz) = RGB, w = ShadowIntensity
};

Texture2D<float4> bindless_textures[MAXNROFMATERIALS] : register(t0, space0);

SamplerState samp : register(s0);

float rand_1_05(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5;
}

float2 getRandomPosition(uint size, float2 uvNoise)
{
    float x = rand_1_05(uvNoise);
    float y = rand_1_05(float2(x, uvNoise.y));
    float l = sqrt(x * x + y * y);
    x = (x / l) * size;
    y = (y / l) * size;
    return float2(x, y);
}

float shadowLevel(float bias, int lightIndex, float3 shadowMapChoords)
{
    if (LightPosoLightType[lightIndex].w == 0)
    {
        return 0;
    }
    const int shadowSoftness = int(ShadowMapInfo[lightIndex].y);
    uint shadowMapIndexInTextures = ShadowMapInfo[lightIndex].x;
    Texture2D shadowMap = bindless_textures[shadowMapIndexInTextures];
    uint width, height;
    shadowMap.GetDimensions(width, height);
    const float2 pixelSize = float2(1.0 / width, 1.0 / height);
    float shadowReturn = 0;
    
    if (shadowSoftness <= 0)
    {
        float sm = shadowMap.SampleLevel(samp, shadowMapChoords.xy, 0).r;
        if (sm + bias < shadowMapChoords.z)
        {
            shadowReturn = 1.0;
        }
        return shadowReturn;
    }
    //TODO : CHECK WICH ONE IS FASTER
    //SOFT SHADOWS BUT CAN BE BRUSIG
    const int numberOfTestsInOneGo = 7;
    int TotalNumberOfTests = 0;
    int testTo = 0;
    for (int y = shadowSoftness; y > 0; y--)
    {
        for (int x = 0; x < numberOfTestsInOneGo; x++)
        {
            TotalNumberOfTests++;
            float2 randomPos = getRandomPosition(y, float2(shadowMapChoords.xy + float2(x, y)));
            float sm = shadowMap.SampleLevel(samp, float2(shadowMapChoords.xy + randomPos * pixelSize), 0).r;
            if (sm + bias < shadowMapChoords.z)
            {
                testTo++;
                shadowReturn += 1.0;
            }
        }
        if (testTo >= numberOfTestsInOneGo)
        {
            break;
            
        }
        testTo = 0;
    }
    return (shadowReturn / (TotalNumberOfTests));

    //SOFT SHADOWS BUT CAN BE HARD
    //for (int y = -shadowSoftness; y <= shadowSoftness; y++)
    //{
    //    for (int x = -shadowSoftness; x <= shadowSoftness; x++)
    //    {
    //        float sm = shadowMap.SampleLevel(samp, float2(shadowMapChoords.xy + float2(x, y) * pixelSize), 0).r;
    //        if (sm + bias < shadowMapChoords.z)
    //        {
    //            shadowReturn += 1.0;
    //        }
    //    }
    //}
   //return (shadowReturn / (4.0 * shadowSoftness * shadowSoftness));
}

float4 main(PixelShaderInput input) : SV_TARGET
{
    //DEBUG
    float3 normal = input.TBN[2];
    if ((materialIndex.x >= MAXNROFMATERIALS || materialIndex.x < 0) ||
        (materialIndex.y >= MAXNROFMATERIALS || materialIndex.y < 0))
    {
        return float4(1, 0, 0, 1);
    }
    if ((materialFlag & MaterialType_Normal) && cameraPos.w < 1)
    {
        float3 normalSample = bindless_textures[materialIndex.y].Sample(samp, input.uv).xyz;
        normalSample = (normalSample * 2.0 - 1.0);
        normal = normalize(mul(normalSample, input.TBN));
    }
    
    //THIS SHOULD BE IN MATERIAL LATER//
    float Shininess = 32;
    float specularStrength = 0.5f;
    ////////////////////////////////////
    
    float4 color = bindless_textures[materialIndex.x].Sample(samp, input.uv);
    float3 viewDir = normalize(cameraPos.xyz - input.fragpos.xyz);
    
    if(color.w == 0)
    {
        discard;
    }
    
    float3 ka = float3(0, 0, 0);
    float3 resultColor = float3(0, 0, 0);
    
    for (uint i = 0; i < nrOfLights; i++)
    {
        //ambient 
        float3 ambientLight = ka.xyz * LightColor[i].xyz * color.xyz;

        const float4 shadowCamera = input.fragpos;
        matrix lightViewProj = ViewProjectionMatrix[i];
        const float4 shadowHomo = mul(shadowCamera, ViewProjectionMatrix[i]);
        float4 shadowMapCoords = shadowHomo * float4(0.5, -0.5, 1.0f, 1.0f) + (float4(0.5f, 0.5f, 0.0f, 0.0f) * shadowHomo.w);
        shadowMapCoords.xyz = shadowMapCoords.xyz / shadowMapCoords.w;
        
        float shadowValue;
        const float bias = 0.00001f;
        if ((shadowMapCoords.x >= 1 || shadowMapCoords.x <= 0 || shadowMapCoords.y >= 1 || shadowMapCoords.y <= 0 || shadowMapCoords.z >= 1 || shadowMapCoords.z <= 0))
        {
            shadowValue = 0;

        }
        else
        {
            shadowValue = shadowLevel(bias, i, shadowMapCoords.xyz);
        }
        
        float3 lightDir = normalize(LightPosoLightType[i].xyz - input.fragpos.xyz);
        float3 halfWayDir = normalize(lightDir - viewDir);

        //Diffuse
        float3 defuse_light;
        float ammount_diffuse = max(dot(normal.xyz, lightDir), 0.0f);
        defuse_light = ammount_diffuse * LightColor[i].xyz * color.xyz;

        float3 reflectDir = reflect(-lightDir, normal.xyz);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
        float3 specular = specularStrength * spec * LightColor[i].xyz;

        resultColor +=  ambientLight + 
                        (defuse_light + specular) * color.xyz * (1.0 - shadowValue);
    }
    
    return float4(saturate(resultColor), color.w);
}