#include"model.fx"

//#define CLOUD

//�_�֊s�e�N�X�`��
#if defined(CLOUD)
Texture2D<float> dissolveTexture : register(t7);
#endif

PSOutput_RenderGBuffer PS_TriPlanarMapping(PSInput In)
{
	PSOutput_RenderGBuffer Out = (PSOutput_RenderGBuffer)0;

	//UV���W
#if defined(USE_LOCALPOS)
	float3 uv = In.Localpos * triPlanarMapUVScale;
#else
	float3 uv = In.Worldpos * triPlanarMapUVScale;
#endif

	//�@������u�����h��
	float3 blendNormal = saturate(pow(In.Normal*1.4f, 4));

	//�_�֊s
#if defined(CLOUD)
	{
		//�T���v��
		float X = dissolveTexture.Sample(Sampler, uv.zy + uvOffset).x;
		float Y = dissolveTexture.Sample(Sampler, uv.zx + uvOffset).x;
		float Z = dissolveTexture.Sample(Sampler, uv.xy + uvOffset).x;
		//����
		Z = lerp(Z, X, blendNormal.x);
		Z = lerp(Z, Y, blendNormal.y);	

		//�����x�N�g��
		float3 eveDir = normalize(In.Worldpos - camWorldPos);
		//�����ɖ@������������قǗ֊s
		Z += 1.0f - abs(dot(In.Normal, eveDir));
		Z*=Z;
		//�������l�Ńs�N�Z���j��
		if (Z > 0.9f){
			discard;
		}
	}
#endif

	//�A���x�h
#if ALBEDO_MAP
	float4 X = albedoTexture.Sample(Sampler, uv.zy + uvOffset);
	float4 Y = albedoTexture.Sample(Sampler, uv.zx + uvOffset);
	float4 Z = albedoTexture.Sample(Sampler, uv.xy + uvOffset);

	Out.albedo = Z;
	Out.albedo = lerp(Out.albedo, X, blendNormal.x);
	Out.albedo = lerp(Out.albedo, Y, blendNormal.y);	
#else
    AlbedoRender(In, Out);
#endif

	//���e�X�g
    if (Out.albedo.a > 0.5f)
    {
        Out.albedo.a = 1.0f; //����������
    }
    else
    {
        discard; //�`�悵�Ȃ�
    }

	//�@��
	//��������
	//https://www.patreon.com/posts/16714688
	//https://medium.com/@bgolus/normal-mapping-for-a-triplanar-shader-10bf39dca05a	
#if NORMAL_MAP
	float3 nX = NormalTexture.Sample(Sampler, uv.zy + uvOffset);
	float3 nY = NormalTexture.Sample(Sampler, uv.zx + uvOffset);
	float3 nZ = NormalTexture.Sample(Sampler, uv.xy + uvOffset);

	Out.normal.xyz = nZ;
	Out.normal.xyz = lerp(Out.normal.xyz, nX, blendNormal.x);
	Out.normal.xyz = lerp(Out.normal.xyz, nY, blendNormal.y);

	Out.normal.xyz = Out.normal.xyz * 2.0f - 1.0f;
	Out.normal.xyz = Out.normal.x * In.Tangent + Out.normal.y * In.Binormal + Out.normal.z * In.Normal;
	Out.normal.xyz = normalize(Out.normal.xyz);
#else
    Out.normal.xyz = In.Normal;
#endif

	//���̑��o��
    PosRender(In, Out);
    ParamRender(In, Out);
    MotionRender(In, Out);
    TranslucentRender(In, Out);

	return Out;
}