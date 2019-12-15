#ifndef DW_MOTION_BLUR_HEADER_H
#define DW_MOTION_BLUR_HEADER_H

static const float Z_OFFSET = 15.0f;
static const float Z_OFFSET_PS = 150.0f;
static const float samples = 8.0f;
static const float BUNBO = 0.002f*(8.0f / samples);//0.001953125f
static const float blurscale = -1.0f;// -0.15f;// 0.4f;

#endif //DW_MOTION_BLUR_HEADER_H