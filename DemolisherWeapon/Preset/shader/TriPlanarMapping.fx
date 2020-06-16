#include"model.fx"

//#define Y_ONLY 1
#define LITE 1

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
#if Y_ONLY
#if LITE
	float fdistance = length(In.Viewpos);	
	
	if(fdistance < 50.0f){
		Out.albedo = albedoTexture.Sample(Sampler, uv.zx* 40.0f + uvOffset);		
		Out.albedo = lerp(Out.albedo,albedoTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset), saturate(fdistance/50.0f));
	}else
	if(fdistance < 225.0f){
		Out.albedo = albedoTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset);
		Out.albedo = lerp(Out.albedo,albedoTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset), saturate((fdistance-50.0f)/175.0f));	
	}else
	if(fdistance < 650.0f){
		Out.albedo = albedoTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset);	
		Out.albedo = lerp(Out.albedo,albedoTexture.Sample(Sampler, uv.zx + uvOffset), saturate((fdistance-225.0f)/425.0f));	
	}else{
		Out.albedo = albedoTexture.Sample(Sampler, uv.zx + uvOffset);	
		Out.albedo = lerp(Out.albedo,albedoTexture.Sample(Sampler, uv.zx * 0.1f+ uvOffset), saturate((fdistance-650.0f)/3150.0f));	
		Out.albedo = lerp(Out.albedo,albedoTexture.Sample(Sampler, uv.zx * 0.01f + uvOffset), saturate((fdistance-650.0f)/16000.0f));
	}	
#else
	Out.albedo = albedoTexture.Sample(Sampler, uv.zx * 40.0f + uvOffset);

    float fdistance = length(In.Viewpos);
	
	float s = saturate(fdistance/50.0f);
	Out.albedo = lerp(Out.albedo,albedoTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset),s);

    s = saturate((fdistance-50.0f)/175.0f);
	Out.albedo = lerp(Out.albedo,albedoTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset),s);

	s = saturate((fdistance-225.0f)/425.0f);
	Out.albedo = lerp(Out.albedo,albedoTexture.Sample(Sampler, uv.zx + uvOffset),s);//TODO ���肩�����߂���?
   
    s = saturate((fdistance-650.0f)/3150.0f);
	Out.albedo = lerp(Out.albedo,albedoTexture.Sample(Sampler, uv.zx * 0.1f+ uvOffset),s);
    
    s = saturate((fdistance-650.0f)/16000.0f);
	Out.albedo = lerp(Out.albedo,albedoTexture.Sample(Sampler, uv.zx * 0.01f + uvOffset),s);  
#endif
#else
#if LITE
    float fdistance = length(In.Viewpos);	
	float4 X =0;
	float4 Y =0;
	float4 Z =0;
	if(fdistance < 50.0f){
		X = albedoTexture.Sample(Sampler, uv.zy * 40.0f + uvOffset);
		Y = albedoTexture.Sample(Sampler, uv.zx * 40.0f + uvOffset);
		Z = albedoTexture.Sample(Sampler, uv.xy * 40.0f + uvOffset);
		float s = saturate(fdistance/50.0f);
		X = lerp(X,albedoTexture.Sample(Sampler, uv.zy * 10.0f + uvOffset),s);
		Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset),s);
		Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy * 10.0f + uvOffset),s);
	}else
	if(fdistance < 225.0f){
		X = albedoTexture.Sample(Sampler, uv.zy * 10.0f + uvOffset);
		Y = albedoTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset);
		Z = albedoTexture.Sample(Sampler, uv.xy * 10.0f + uvOffset);
		float s = saturate((fdistance-50.0f)/175.0f);
		X = lerp(X,albedoTexture.Sample(Sampler, uv.zy * 3.0f + uvOffset),s);
		Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset),s);
		Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy * 3.0f + uvOffset),s);
	}else
	if(fdistance < 650.0f){
		X = albedoTexture.Sample(Sampler, uv.zy * 3.0f + uvOffset);
		Y = albedoTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset);
		Z = albedoTexture.Sample(Sampler, uv.xy * 3.0f + uvOffset);
		float s = saturate((fdistance-225.0f)/425.0f);
		X = lerp(X,albedoTexture.Sample(Sampler, uv.zy + uvOffset),s);
		Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx + uvOffset),s);//TODO ���肩�����߂���?
		Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy + uvOffset),s);
	}else
	{
		X = albedoTexture.Sample(Sampler, uv.zy + uvOffset);
		Y = albedoTexture.Sample(Sampler, uv.zx + uvOffset);
		Z = albedoTexture.Sample(Sampler, uv.xy + uvOffset);
		float s = saturate((fdistance-650.0f)/3150.0f);
		X = lerp(X,albedoTexture.Sample(Sampler, uv.zy * 0.1f+ uvOffset),s);
		Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx * 0.1f+ uvOffset),s);
		Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy * 0.1f+ uvOffset),s);
		s = saturate((fdistance-650.0f)/16000.0f);
		X = lerp(X,albedoTexture.Sample(Sampler, uv.zy * 0.01f + uvOffset),s);
		Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx * 0.01f + uvOffset),s);
		Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy * 0.01f + uvOffset),s);  
    }
#else
	float4 X = albedoTexture.Sample(Sampler, uv.zy * 40.0f + uvOffset);
	float4 Y = albedoTexture.Sample(Sampler, uv.zx * 40.0f + uvOffset);
	float4 Z = albedoTexture.Sample(Sampler, uv.xy * 40.0f + uvOffset);

    float fdistance = length(In.Viewpos);
	
	float s = saturate(fdistance/50.0f);
	X = lerp(X,albedoTexture.Sample(Sampler, uv.zy * 10.0f + uvOffset),s);
	Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset),s);
	Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy * 10.0f + uvOffset),s);

    s = saturate((fdistance-50.0f)/175.0f);
    X = lerp(X,albedoTexture.Sample(Sampler, uv.zy * 3.0f + uvOffset),s);
	Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset),s);
	Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy * 3.0f + uvOffset),s);

	s = saturate((fdistance-225.0f)/425.0f);
    X = lerp(X,albedoTexture.Sample(Sampler, uv.zy + uvOffset),s);
	Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx + uvOffset),s);//TODO ���肩�����߂���?
	Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy + uvOffset),s);
   
    s = saturate((fdistance-650.0f)/3150.0f);
    X = lerp(X,albedoTexture.Sample(Sampler, uv.zy * 0.1f+ uvOffset),s);
	Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx * 0.1f+ uvOffset),s);
	Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy * 0.1f+ uvOffset),s);
    
    s = saturate((fdistance-650.0f)/16000.0f);
    X = lerp(X,albedoTexture.Sample(Sampler, uv.zy * 0.01f + uvOffset),s);
	Y = lerp(Y,albedoTexture.Sample(Sampler, uv.zx * 0.01f + uvOffset),s);
	Z = lerp(Z,albedoTexture.Sample(Sampler, uv.xy * 0.01f + uvOffset),s);    
#endif
	Out.albedo = Z;
	Out.albedo = lerp(Out.albedo, X, blendNormal.x);
	Out.albedo = lerp(Out.albedo, Y, blendNormal.y);	
#endif
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
#if Y_ONLY
#if LITE
	float nfdistance = length(In.Viewpos);
	
	if(nfdistance < 50.0f){
		Out.normal.xyz = NormalTexture.Sample(Sampler, uv.zx* 40.0f + uvOffset);		
		Out.normal.xyz = lerp(Out.normal.xyz,NormalTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset), saturate(nfdistance/50.0f));
	}else
	if(nfdistance < 225.0f){
		Out.normal.xyz = NormalTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset);
		Out.normal.xyz = lerp(Out.normal.xyz,NormalTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset), saturate((nfdistance-50.0f)/175.0f));	
	}else
	if(nfdistance < 650.0f){
		Out.normal.xyz = NormalTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset);	
		Out.normal.xyz = lerp(Out.normal.xyz,NormalTexture.Sample(Sampler, uv.zx + uvOffset), saturate((nfdistance-225.0f)/425.0f));	
	}else{
		Out.normal.xyz = NormalTexture.Sample(Sampler, uv.zx + uvOffset);	
		Out.normal.xyz = lerp(Out.normal.xyz,NormalTexture.Sample(Sampler, uv.zx * 0.1f+ uvOffset), saturate((nfdistance-650.0f)/3150.0f));	
		Out.normal.xyz = lerp(Out.normal.xyz,NormalTexture.Sample(Sampler, uv.zx * 0.01f + uvOffset), saturate((nfdistance-650.0f)/16000.0f));
	}  
#else
	Out.normal.xyz = NormalTexture.Sample(Sampler, uv.zx* 40.0f + uvOffset);
	
	float nfdistance = length(In.Viewpos);
	
	float ns = saturate(nfdistance/50.0f);
	Out.normal.xyz = lerp(Out.normal.xyz,NormalTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset),ns);
	
    ns = saturate((nfdistance-50.0f)/175.0f);
	Out.normal.xyz = lerp(Out.normal.xyz,NormalTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset),ns);
	
	ns = saturate((nfdistance-225.0f)/425.0f);
	Out.normal.xyz = lerp(Out.normal.xyz,NormalTexture.Sample(Sampler, uv.zx + uvOffset),ns);//TODO ���肩�����߂���?
	
    ns = saturate((nfdistance-650.0f)/3150.0f);
	Out.normal.xyz = lerp(Out.normal.xyz,NormalTexture.Sample(Sampler, uv.zx * 0.1f+ uvOffset),ns);
	
    ns = saturate((nfdistance-650.0f)/16000.0f);
    Out.normal.xyz = lerp(Out.normal.xyz,NormalTexture.Sample(Sampler, uv.zx * 0.01f + uvOffset),ns);
#endif
	Out.normal.xyz = Out.normal.xyz * 2.0f - 1.0f;
	Out.normal.xyz = Out.normal.x * In.Tangent + Out.normal.y * In.Binormal + Out.normal.z * In.Normal;
	Out.normal.xyz = normalize(Out.normal.xyz);
#else
#if LITE
	float nfdistance = length(In.Viewpos);	
	float3 nX =0;
	float3 nY =0;
	float3 nZ =0;
	if(nfdistance < 50.0f){
		nX = NormalTexture.Sample(Sampler, uv.zy * 40.0f + uvOffset);
		nY = NormalTexture.Sample(Sampler, uv.zx * 40.0f + uvOffset);
		nZ = NormalTexture.Sample(Sampler, uv.xy * 40.0f + uvOffset);
		float s = saturate(nfdistance/50.0f);
		nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 10.0f + uvOffset),s);
		nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset),s);
		nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 10.0f + uvOffset),s);
	}else
	if(nfdistance < 225.0f){
		nX = NormalTexture.Sample(Sampler, uv.zy * 10.0f + uvOffset);
		nY = NormalTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset);
		nZ = NormalTexture.Sample(Sampler, uv.xy * 10.0f + uvOffset);
		float s = saturate((nfdistance-50.0f)/175.0f);
		nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 3.0f + uvOffset),s);
		nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset),s);
		nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 3.0f + uvOffset),s);
	}else
	if(nfdistance < 650.0f){
		nX = NormalTexture.Sample(Sampler, uv.zy * 3.0f + uvOffset);
		nY = NormalTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset);
		nZ = NormalTexture.Sample(Sampler, uv.xy * 3.0f + uvOffset);
		float s = saturate((nfdistance-225.0f)/425.0f);
		nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy + uvOffset),s);
		nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx + uvOffset),s);//TODO ���肩�����߂���?
		nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy + uvOffset),s);
	}else
	{
		nX = NormalTexture.Sample(Sampler, uv.zy + uvOffset);
		nY = NormalTexture.Sample(Sampler, uv.zx + uvOffset);
		nZ = NormalTexture.Sample(Sampler, uv.xy + uvOffset);
		float s = saturate((nfdistance-650.0f)/3150.0f);
		nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 0.1f+ uvOffset),s);
		nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 0.1f+ uvOffset),s);
		nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 0.1f+ uvOffset),s);
		s = saturate((nfdistance-650.0f)/16000.0f);
		nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 0.01f + uvOffset),s);
		nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 0.01f + uvOffset),s);
		nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 0.01f + uvOffset),s);  
    }
#else
	float3 nX = NormalTexture.Sample(Sampler, uv.zy* 40.0f + uvOffset);
	float3 nY = NormalTexture.Sample(Sampler, uv.zx* 40.0f + uvOffset);
	float3 nZ = NormalTexture.Sample(Sampler, uv.xy* 40.0f + uvOffset);
   
	float nfdistance = length(In.Viewpos);
	
	float ns = saturate(nfdistance/50.0f);
	nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 10.0f + uvOffset),ns);
	nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 10.0f + uvOffset),ns);
	nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 10.0f + uvOffset),ns);

    ns = saturate((nfdistance-50.0f)/175.0f);
    nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 3.0f + uvOffset),ns);
	nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 3.0f + uvOffset),ns);
	nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 3.0f + uvOffset),ns);

	ns = saturate((nfdistance-225.0f)/425.0f);
    nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy + uvOffset),ns);
	nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx + uvOffset),ns);//TODO ���肩�����߂���?
	nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy + uvOffset),ns);
   
    ns = saturate((nfdistance-650.0f)/3150.0f);
    nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 0.1f+ uvOffset),ns);
	nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 0.1f+ uvOffset),ns);
	nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 0.1f+ uvOffset),ns);
    
    ns = saturate((nfdistance-650.0f)/16000.0f);
    nX = lerp(nX,NormalTexture.Sample(Sampler, uv.zy * 0.01f + uvOffset),ns);
	nY = lerp(nY,NormalTexture.Sample(Sampler, uv.zx * 0.01f + uvOffset),ns);
	nZ = lerp(nZ,NormalTexture.Sample(Sampler, uv.xy * 0.01f + uvOffset),ns);  

	Out.normal.xyz = nZ;
	Out.normal.xyz = lerp(Out.normal.xyz, nX, blendNormal.x);
	Out.normal.xyz = lerp(Out.normal.xyz, nY, blendNormal.y);
#endif
	Out.normal.xyz = Out.normal.xyz * 2.0f - 1.0f;
	Out.normal.xyz = Out.normal.x * In.Tangent + Out.normal.y * In.Binormal + Out.normal.z * In.Normal;
	Out.normal.xyz = normalize(Out.normal.xyz);
#endif
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