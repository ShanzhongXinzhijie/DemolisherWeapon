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
	float4 X = albedoTexture.Sample(Sampler, uv.zy * 10.0f + uvOffset);
	float4 Y = albedoTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset);
	float4 Z = albedoTexture.Sample(Sampler, uv.xy * 10.0f + uvOffset);

    float fdistance = length(In.Viewpos);
    float s = saturate(fdistance/500.0f);
    X = lerp(X,albedoTexture.Sample(Sampler, uv.zy * 1.0f + uvOffset),s);
	Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx * 1.0f + uvOffset),s);
	Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy * 1.0f + uvOffset),s);
   
    s = saturate((fdistance-500.0f)/3500.0f);
    X = lerp(X,albedoTexture.Sample(Sampler, uv.zy * 0.1f+ uvOffset),s);
	Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx * 0.1f+ uvOffset),s);//TODO ������O�ɍL��
	Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy * 0.1f+ uvOffset),s);
    
    s = saturate((fdistance-4000.0f)/16000.0f);
    X = lerp(X,albedoTexture.Sample(Sampler, uv.zy * 0.01f + uvOffset),s);
	Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx * 0.01f + uvOffset),s);
	Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy * 0.01f + uvOffset),s);    

	Out.albedo = Z;
	Out.albedo = lerp(Out.albedo, X, blendNormal.x);
	Out.albedo = lerp(Out.albedo, Y, blendNormal.y);	

	//Out.albedo.xyz = lerp(Out.albedo.xyz,blendNormal,0.5f);
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
#if 0//NORMAL_MAP
	float3 nX = NormalTexture.Sample(Sampler, uv.zy + uvOffset);
	float3 nY = NormalTexture.Sample(Sampler, uv.zx + uvOffset);
	float3 nZ = NormalTexture.Sample(Sampler, uv.xy + uvOffset);

 //   nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 80.0f + uvOffset),0.5f);
	//nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 80.0f + uvOffset),0.5f);
	//nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 80.0f + uvOffset),0.5f);

 //   nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 20.0f + uvOffset),0.5f);
	//nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 20.0f + uvOffset),0.5f);
	//nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 20.0f + uvOffset),0.5f);

 //   nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 0.01f + uvOffset),0.5f);
	//nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 0.01f + uvOffset),0.5f);
	//nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 0.01f + uvOffset),0.5f);

 //   float3 nX = NormalTexture.Sample(Sampler, uv.zy * 10.0f + uvOffset);
	//float3 nY = NormalTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset);
	//float3 nZ = NormalTexture.Sample(Sampler, uv.xy * 10.0f + uvOffset);

 //   float fdistanceN = length(In.Viewpos);
 //   float sN = saturate(fdistanceN/1000.0f);
 //   nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 1.0f + uvOffset),sN);
	//nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 1.0f + uvOffset),sN);
	//nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 1.0f + uvOffset),sN);
   
 //   sN = saturate((fdistanceN-1000.0f)/1000.0f);
 //   nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 0.1f+ uvOffset),sN);
	//nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 0.1f+ uvOffset),sN);
	//nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 0.1f+ uvOffset),sN);
    
 //   sN = saturate((fdistanceN-1000.0f)/5000.0f);//TODO 2000
 //   nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 0.01f + uvOffset),sN);
	//nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 0.01f + uvOffset),sN);
	//nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 0.01f + uvOffset),sN);  

 //   nX = lerp(nX,float3(0.5f,0.5f,1.0f),0.5f);
	//nY = lerp(nY,float3(0.5f,0.5f,1.0f),0.5f);
	//nZ = lerp(nZ,float3(0.5f,0.5f,1.0f),0.5f);  

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