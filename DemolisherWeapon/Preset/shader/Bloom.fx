//�萔
cbuffer CSCb : register(b0) {
	//�t���[���o�b�t�@�𑜓x
	uint win_x;
	uint win_y;

	//�P�x�������l
	float luminanceThreshold;
};

// ���̓e�N�X�`��
Texture2D<float4> inputTex : register(t1);

// �o�̓e�N�X�`��
RWTexture2D<float4> rwOutputTex : register(u0);

[numthreads(32, 32, 1)]
void CSmain(uint3 run_xy : SV_DispatchThreadID)
{
	uint2 uv = run_xy.xy;

	if (uv.x % 2 == 0) {
		uv.x = win_x - uv.x;
	}
	if (uv.y % 2 == 0) {
		uv.y = win_y - uv.y;
	}

	//�e�N�X�`���T�C�Y�����Ă���o��
	if (uv.x > win_x || uv.y > win_y) {
		return;
	}			
	
	//�F�擾
	float4 color = inputTex[uv];

	//�P�x���ݒ�ȉ��Ȃ甲����
	float luminance = dot(color.xyz, float3(0.2125f, 0.7154f, 0.0721f));
	if (luminance < luminanceThreshold){
		return;
	}

	//�o��
	color.xyz *= luminance - luminanceThreshold;
	color.w = 1.0f;
	rwOutputTex[uv] = color;

	//�P�x�������قǃ��[�v�񐔑�����
	//float loopmax = min(12.0f, luminance / 2.0f);

	//���W�ϊ�
	/*velocity.x *= (float)win_x;
	velocity.y *= (float)win_y;*/

	//[unroll]
	//for (uint i2 = 0; i2 < loopmax; i2++)
	//{
	//	[unroll]
	//	for (float i = 0.0f; i < loopmax; i++)
	//	{
	//		uint2 sampuv = round((float2)uv + ((i + 1.0f) / loopmax) * velocity.xy);

	//		if (sampuv.x < 0 || sampuv.y < 0 || sampuv.x > win_x || sampuv.y > win_y) { break; }

	//		float4 samp = rwVelocityTex[sampuv];

	//		if (velocity.z < samp.w + Z_OFFSET * distanceScale) {//��O�̃s�N�Z������̓T���v�����Ȃ�
	//			rwOutputTex[sampuv] = lerp(rwOutputTex[sampuv], color, 1.0f / (i + 2.0f));//�T���v�����œ����x
	//		}
	//	}
	//}
}