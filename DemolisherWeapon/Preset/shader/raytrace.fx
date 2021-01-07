/***************************************************************************
# Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/

#include "hlslHelpers.hlsl"

static const int MAX_TRACE_RECURSION_DEPTH = 4;

//頂点構造
struct SVertex{    
    float3 position;
    float3 normal;
    float4 tangent;
    float2 uv;
};
//カメラ構造体
//定数バッファなので16バイトアライメントに気を付けること
struct Camera{
    float4x4 mCameraRot;   //カメラの回転行列
    float3 pos;            //カメラ座標。
    float far;             //遠平面。 
    float near;            //近平面。
    float fov;             //視野角
};

cbuffer rayGenCB :register(b0) {
    Camera g_camera; //カメラ。
};

RaytracingAccelerationStructure g_raytracingWorld : register(t0);    //レイトレワールド。

Texture2D<float4> gAlbedoTexture : register(t1);    //アルベドマップ。
Texture2D<float4> g_normalMap : register(t2);       //法線マップ。
Texture2D<float4> g_specularMap : register(t3);     //スペキュラマップ。
Texture2D<float4> g_reflectionMap : register(t4);   //リフレクションマップ。
Texture2D<float4> g_refractionMap : register(t5);   //屈折マップ。

StructuredBuffer<SVertex> g_vertexBuffers : register(t6);   //頂点バッファ。
StructuredBuffer<int> g_indexBuffers : register(t7);        //インデックスバッファ。

StructuredBuffer<float4x4> g_worldMatrixs : register(t8); //ワールド行列

RWTexture2D<float4> gOutput : register(u0);//出力先

SamplerState  s : register(s0);//サンプラー

/// <summary>
/// RayTracingEngine.cpp : RayPayload
/// </summary>
struct RayPayload
{
    float3 color;
    int depth;
};
struct RayPayloadShadow
{
    int hit;
};
struct RayPayloadAO
{
    float3 color;
    float distance;
};

//太陽の色
static const float3 sunRed = float3(1.0f, 57 / 255.0f, 47 / 255.0f);

//二乗する
float square(in float f)
{
    return f * f;
}
//四乗する
float fourth(in float f)
{
    return f * f * f * f;
}

//大気フォグ
float3 AtmosphericSky(float3 solDir, float3 worldPosNorm)
{
    //青さ 
    float3 skyC = float3(36.0 / 255.0, 67.0 / 255.0, 1.0) * saturate(solDir.y);
   	
    //地平線に近いほど白い
    float heightScale = fourth(fourth(1.0f - saturate(worldPosNorm.y)));
    skyC = lerp(skyC, float3(1.0f, 1.0f, 1.0f), square(saturate(solDir.y)) * heightScale);
   
	//夕焼け
    float timeScale = saturate(1.0f - length(solDir.y) * 4.0f);
    float solScale = fourth(fourth(saturate(dot(solDir, worldPosNorm))));
    skyC = lerp(skyC, sunRed, timeScale * saturate(solScale + heightScale));
	
	//太陽
    if (worldPosNorm.y > 0.0f && length(dot(solDir * -1.0f, worldPosNorm)) > 0.9998f)
    {
        if (solDir.y < 0.0f)
        {
            return float3(1.5f, 1.5f, 1.0f);
        }
        else
        {
            float heightScale = fourth(fourth(1.0f - saturate(worldPosNorm.y)));
            return 10.0f * lerp(float3(1.0f, 1.0f, 1.0f), sunRed, heightScale);
        }
    }
	
    return skyC;
}

//フレネル
//specAlbedo = フレネル反射率
//halfVec = ハーフベクトル = normalize(lightDir + viewDir);
//lightDir = ライト方向
float3 Fresnel(in float3 specAlbedo, in float3 halfVec, in float3 lightDir)
{
    return specAlbedo + (1.0f - specAlbedo) * pow((1.0f - dot(lightDir, halfVec)), 5.0f);
}

//UV座標を取得。
float2 GetUV(BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    //プリミティブIDを取得。
    uint primID = PrimitiveIndex();   
    //プリミティブIDから頂点番号を取得する。
    uint v0_id = g_indexBuffers[primID * 3];
    uint v1_id = g_indexBuffers[primID * 3 + 1];
    uint v2_id = g_indexBuffers[primID * 3 + 2];
    float2 uv0 = g_vertexBuffers[v0_id].uv;
    float2 uv1 = g_vertexBuffers[v1_id].uv;
    float2 uv2 = g_vertexBuffers[v2_id].uv;
    
    float2 uv = barycentrics.x * uv0 + barycentrics.y * uv1 + barycentrics.z * uv2;
    
    return uv;
}
//モデルローカル法線を取得。
float3 GetLocalNormal(BuiltInTriangleIntersectionAttributes attribs, float2 uv)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    //プリミティブIDを取得。
    uint primID = PrimitiveIndex();   
    //プリミティブIDから頂点番号を取得する。
    uint v0_id = g_indexBuffers[primID * 3];
    uint v1_id = g_indexBuffers[primID * 3 + 1];
    uint v2_id = g_indexBuffers[primID * 3 + 2];

    float3 normal0 = g_vertexBuffers[v0_id].normal;
    float3 normal1 = g_vertexBuffers[v1_id].normal;
    float3 normal2 = g_vertexBuffers[v2_id].normal;
    float3 normal = barycentrics.x * normal0 + barycentrics.y * normal1 + barycentrics.z * normal2;
    normal = normalize(normal);

    //法線マップ
    /*
    float3 tangent0 = g_vertexBuffers[v0_id].tangent;
    float3 tangent1 = g_vertexBuffers[v1_id].tangent;
    float3 tangent2 = g_vertexBuffers[v2_id].tangent;
    float3 tangent = barycentrics.x * tangent0 + barycentrics.y * tangent1 + barycentrics.z * tangent2;
    tangent = normalize(tangent);

    float3 binormal = normalize(cross(tangent, normal));

    
    float3 binSpaceNormal = g_normalMap.SampleLevel ( s, uv, 0.0f).xyz;
    binSpaceNormal = (binSpaceNormal * 2.0f) - 1.0f;
    
	normal = tangent * binSpaceNormal.x + binormal * binSpaceNormal.y + normal * binSpaceNormal.z; 
    */
    
    return normal;
}

static const int SOFT_SHADOW_RAY_MAX = 3;
static const float3 DirLightSizeOffset[] = { 
    float3(0.0f,0.002f,0.0f),
    float3(0.002f,0.0f,0.0f),
    float3(-0.002f,0.0f,0.0f)
};

//アンビエントオクルージョン
float TraceAORay(inout RayPayload raypayload, float3 normal)
{
    if (raypayload.depth < MAX_TRACE_RECURSION_DEPTH)
    {
        float hitT = RayTCurrent();
        float3 rayDirW = WorldRayDirection();
        float3 rayOriginW = WorldRayOrigin();
        // Find the world-space hit position
        float3 posW = rayOriginW + hitT * rayDirW;

        RayDesc ray;
        ray.Origin = posW;
        ray.TMin = 0.01f;
        ray.TMax = 25;
        
        //シード値
        uint seed = initRand((uint) square(posW.x * 10), (uint) square(posW.z * 10));
        
        //ランダムに半球コーン?内をサンプル
        ray.Direction = getConeSample(
            seed,
            normal,
            PI
        );

        RayPayloadAO payloadAO;    
        payloadAO.distance = ray.TMax;
        TraceRay(
            g_raytracingWorld,
            0,
            0xFF,
            2,//ヒットグループのインデックス
            0,
            2,//ミスシェーダのインデックス
            ray,
            payloadAO
        );
        
        float result = (1.0f - (payloadAO.distance / ray.TMax)) * saturate(dot(ray.Direction, normal));
        
        //シード値
        seed = initRand((uint) square(posW.x * 20), (uint) square(posW.z * 30));
        
        //ランダムに半球コーン?内をサンプル
        ray.Direction = getConeSample(
            seed,
            normal,
            PI
        );

        payloadAO.distance = ray.TMax;
        TraceRay(
            g_raytracingWorld,
            0,
            0xFF,
            2, //ヒットグループのインデックス
            0,
            2, //ミスシェーダのインデックス
            ray,
            payloadAO
        );
        
        result += (1.0f - (payloadAO.distance / ray.TMax)) * saturate(dot(ray.Direction, normal));
        result /= 2.0f;
        
        return result; //  cos(); //payload.val = exp(-distance * g_optionsCB.m_falloff);
    }
    return 0.0f;
}

//光源に向けてレイを飛ばす。
float TraceLightRay(inout RayPayload raypayload, float3 normal)
{
    //int result = SOFT_SHADOW_RAY_MAX;
    if (raypayload.depth < MAX_TRACE_RECURSION_DEPTH)
    {
        float hitT = RayTCurrent();
        float3 rayDirW = WorldRayDirection();
        float3 rayOriginW = WorldRayOrigin();

        // Find the world-space hit position
        float3 posW = rayOriginW + hitT * rayDirW;

        RayDesc ray;
        ray.Origin = posW;
        ray.TMin = 0.01f;
        ray.TMax = 2500;        
        
        float3 toLight = normalize(float3(0.5, 0.5, 0.2));
        // Calculate a vector perpendicular to L
        float3 perpL = cross(toLight, float3(0.f, 1.0f, 0.f));
        // Handle case where L = up -> perpL should then be (1,0,0)
        if (all(perpL == 0.0f)){ perpL.x = 1.0; }
        // Use perpL to get a vector from worldPosition to the edge of the light sphere
        const float radius = 10.0f;
        float3 lightPosition = posW + toLight * 2500.0f;
        float3 toLightEdge = normalize((lightPosition + perpL * radius) - posW);
        // Angle between L and toLightEdge. Used as the cone angle when sampling shadow rays
        float coneAngle = acos(dot(toLight, toLightEdge)) * 2.0f;        
       
        //シード値
        uint seed = initRand((uint) square(posW.x * 10), (uint) square(posW.z * 10));
        
        //ランダムにコーン内をサンプル
        ray.Direction = getConeSample(
            seed,
            toLight,
            coneAngle
        );

        RayPayloadShadow payloadShadow;
        payloadShadow.hit = 1;
        TraceRay(
            g_raytracingWorld,
            RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
            0xFF,
            1,
            0,
            1,
            ray,
            payloadShadow
        );
        
        return 1.0f - payloadShadow.hit;
        
        /*
        [unroll]
        for (int i = 0; i < SOFT_SHADOW_RAY_MAX; i++)
        {        
            raypayload.hit = 1;
            ray.Direction = normalize(float3(0.5, 0.5, 0.2) + DirLightSizeOffset[i]);
            TraceRay(
                g_raytracingWorld,
                RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
                0xFF,
                1,
                0,
                1,
                ray,
                raypayload
            );
            result -= raypayload.hit;
        }
        */
    }
    return 1.0f;
    //return (float) result / SOFT_SHADOW_RAY_MAX;
}
//反射レイを飛ばす。
void TraceReflectionRay(inout RayPayload raypayload, float3 normal)
{
    if (raypayload.depth < MAX_TRACE_RECURSION_DEPTH)
    {
        float hitT = RayTCurrent();
        float3 rayDirW = WorldRayDirection();
        float3 rayOriginW = WorldRayOrigin();

        //反射ベクトルを求める。
        float3 refDir = reflect(rayDirW, normal);
        // Find the world-space hit position
        float3 posW = rayOriginW + hitT * rayDirW;

        RayDesc ray;
        ray.Origin = posW;
        ray.Direction = refDir;
        ray.TMin = 0.01f;
        ray.TMax = 10000;    
         
        raypayload.depth++;
        TraceRay(
            g_raytracingWorld,
            0,
            0xFF,
            0,
            0,
            0,//1,
            ray,
            raypayload
        );
        raypayload.depth--;      
        
        //フレネル        
        raypayload.color *= Fresnel(0.03f, normalize(refDir + rayDirW*-1.0f), refDir);
    }
}
[shader("raygeneration")]
void rayGen()
{
    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();

    float2 crd = float2(launchIndex.xy);
    float2 dims = float2(launchDim.xy);

    float2 d = ((crd / dims) * 2.f - 1.f);
    float aspectRatio = dims.x / dims.y;

	//ピクセル方向に打ち出すレイを作成する。
    RayDesc ray;
    ray.Origin = g_camera.pos;
    ray.Direction = float3(d.x * aspectRatio, -d.y, 1.0f);
    ray.Direction.xy *= tan(g_camera.fov / 2.0f); 
    ray.Direction = normalize(mul(g_camera.mCameraRot, ray.Direction));
    ray.TMin = g_camera.near;
    ray.TMax = g_camera.far;

    //TraceRay
    RayPayload payload;
    payload.depth = 0;    
    TraceRay(g_raytracingWorld, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);
    
    //Output
    gOutput[launchIndex.xy] = float4(payload.color, 1.0f);
}

[shader("miss")]
void miss(inout RayPayload payload)
{
    payload.color = AtmosphericSky(normalize(float3(0.5, 0.1, 0.2)), WorldRayDirection()); //float3(66.0 / 255.0, 167.0 / 255.0, 1.0);
}

[shader("closesthit")]
void chs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.depth++;
    
    //ヒットしたプリミティブのUV座標を取得。
    float2 uv = GetUV(attribs);    
    //ヒットしたプリミティブの法線を取得。
    float3 normal = GetLocalNormal(attribs, uv);    
    //法線をワールド空間に変換
    normal = mul(g_worldMatrixs[InstanceID()], normal);
    
    float lig = 0.0f;
    
    //シャドウ
    //光源にむかってレイを飛ばす。
    lig = TraceLightRay(payload, normal);
    
    //アンビエントオクルージョン
    //float3 AOC = 1.0f - TraceAORay(payload, normal);;
    float ao = 1.0f - TraceAORay(payload, normal);
    
    //ディフューズライティング
    float3 ligDir = normalize(float3(0.5, 0.5, 0.2));
    float t = max(0.0f, dot(ligDir, normal));
    lig *= t;
    
    //環境光
    lig += 0.615f * ao;
    
    //反射レイ。
    RayPayload refPayload;
    refPayload.depth = payload.depth;
    refPayload.color = 0;
    TraceReflectionRay(refPayload, normal);
    
    //このプリミティブの反射率を取得。
    float reflectRate = 0.35f;//g_reflectionMap.SampleLevel(s, uv, 0.0f).r;
    float3 color = gAlbedoTexture.SampleLevel(s, uv, 0.0f).rgb ;
    color *= lig;
    payload.color = color; //payload.color = lerp( color, refPayload.color, reflectRate);     
    payload.color += refPayload.color * reflectRate * reflectRate; //shininess * shininess    
    
    /*
    payload.color.rg = uv;
    payload.color.b = 0.0f;
    
    payload.color = normal;    
    */
    
    payload.depth--;
}

[shader("anyhit")]
void any(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    //ヒットしたプリミティブのUV座標を取得。
    float2 uv = GetUV(attribs);

    //αテスト
    float opacity = gAlbedoTexture.SampleLevel(s, uv, 0.0f).a;
    if (opacity < 0.33)
    {
        IgnoreHit();
    }
}

//シャドウレイ
[shader("closesthit")]
void shadowChs(inout RayPayloadShadow payload, in BuiltInTriangleIntersectionAttributes attribs)
{
   payload.hit = 1;
}

[shader("miss")]
void shadowMiss(inout RayPayloadShadow payload)
{
   payload.hit = 0;
}

[shader("anyhit")]
void shadowAny(inout RayPayloadShadow payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    //ヒットしたプリミティブのUV座標を取得。
    float2 uv = GetUV(attribs);    

    //αテスト
    float opacity = gAlbedoTexture.SampleLevel(s, uv, 0.0f).a;
    if (opacity < 0.33)
    {
        IgnoreHit();
    }
}

//AOレイ
[shader("closesthit")]
void aoChs(inout RayPayloadAO payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.distance = RayTCurrent();
}

[shader("miss")]
void aoMiss(inout RayPayloadAO payload)
{
}

[shader("anyhit")]
void aoAny(inout RayPayloadAO payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    //ヒットしたプリミティブのUV座標を取得。
    float2 uv = GetUV(attribs);

    //αテスト
    float opacity = gAlbedoTexture.SampleLevel(s, uv, 0.0f).a;
    if (opacity < 0.33)
    {
        IgnoreHit();
    }
}
