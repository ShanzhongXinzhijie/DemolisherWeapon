#define ALBEDO_MAP 1
#define NORMAL_MAP 1
//#define MOTIONBLUR 1

#include"model.fx"

PSOutput_RenderGBuffer PS_TriPlanarMapping(PSInput In)
{
	//�ʏ탌���_�����O
	PSOutput_RenderGBuffer Out = PSMain_RenderGBuffer(In);

	float3 uv = In.Worldpos * 0.001f;

	//�A���x�h
	float4 X = albedoTexture.Sample(Sampler, uv.zy);
	float4 Y = albedoTexture.Sample(Sampler, uv.zx);
	float4 Z = albedoTexture.Sample(Sampler, uv.xy);

	float3 blendNormal = saturate(pow(In.Normal*1.4,4));
	Out.albedo = Z;
	Out.albedo = lerp(Out.albedo, X, blendNormal.x);
	Out.albedo = lerp(Out.albedo, Y, blendNormal.y);

	//���e�X�g
	if (Out.albedo.a > 0.5f) {
		Out.albedo.a = 1.0f;//����������
	}
	else {
		discard;//�`�悵�Ȃ�
	}

	//�@��
	//��������
	//https://www.patreon.com/posts/16714688
	//https://medium.com/@bgolus/normal-mapping-for-a-triplanar-shader-10bf39dca05a
	
	float3 nX = NormalTexture.Sample(Sampler, uv.zy);
	float3 nY = NormalTexture.Sample(Sampler, uv.zx);
	float3 nZ = NormalTexture.Sample(Sampler, uv.xy);

	Out.normal = nZ;
	Out.normal = lerp(Out.normal, nX, blendNormal.x);
	Out.normal = lerp(Out.normal, nY, blendNormal.y);

	Out.normal = Out.normal * 2.0f - 1.0f;
	Out.normal = Out.normal.x * In.Tangent + Out.normal.y * In.Binormal + Out.normal.z * In.Normal;
	Out.normal = normalize(Out.normal);

	return Out;
}