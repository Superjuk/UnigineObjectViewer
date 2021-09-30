/* Copyright (C) 2005-2018, UNIGINE. All rights reserved.
 *
 * This file is a part of the UNIGINE 2.7.2.1 SDK.
 *
 * Your use and / or redistribution of this software in source and / or
 * binary form, with or without modification, is subject to: (i) your
 * ongoing acceptance of and compliance with the terms and conditions of
 * the UNIGINE License Agreement; and (ii) your inclusion of this notice
 * in any version of this software that you use or redistribute.
 * A copy of the UNIGINE License Agreement is available by contacting
 * UNIGINE. at http://unigine.com/
 */

	/*
	float2 uv = IN_DATA(0).xy;
	
	float4 auxiliary = TEXTURE_BIAS_ZERO(TEX_AUXILIARY,uv); 
	
	float4 color = TEXTURE_BIAS_ZERO(TEX_COLOR,uv); 
	float4 selection = TEXTURE_BIAS_ZERO(TEX_SELECTION,uv); 
	
	OUT_COLOR = color + saturate(selection - auxiliary) * 2.0f;
	*/

#include <core/shaders/common/fragment.h>

INIT_TEXTURE(0,TEX_COLOR)
INIT_TEXTURE(1,TEX_AUXILIARY)
INIT_TEXTURE(2,TEX_SELECTION)

STRUCT(FRAGMENT_IN)
	INIT_POSITION
	INIT_IN(float2,0)
	INIT_IN(float3,1)
	INIT_IN(float4,2)
END

MAIN_BEGIN(FRAGMENT_OUT,FRAGMENT_IN)
	
	float2 uv = IN_DATA(0).xy;	
	
	// Get scene color
	float4 color = TEXTURE_BIAS_ZERO(TEX_COLOR, uv); 
	float4 selection = TEXTURE_BIAS_ZERO(TEX_SELECTION, uv); 
	
	// get the inverse resolution of the viewport (1.0 / width, 1.0 / height)
	float2 offset = s_viewport.zw * 1.5f;
	
 	float c0 = TEXTURE_BIAS_ZERO(TEX_AUXILIARY, uv + offset * float2( 1, 0)).r;
	float c1 = TEXTURE_BIAS_ZERO(TEX_AUXILIARY, uv + offset * float2( 0, 1)).r;
	float c2 = TEXTURE_BIAS_ZERO(TEX_AUXILIARY, uv + offset * float2( 1, 1)).r;
	float c3 = TEXTURE_BIAS_ZERO(TEX_AUXILIARY, uv + offset * float2( 1, -1)).r;
	
	float c4 = TEXTURE_BIAS_ZERO(TEX_AUXILIARY, uv + offset * float2(-1, 0)).r;
	float c5 = TEXTURE_BIAS_ZERO(TEX_AUXILIARY, uv + offset * float2( 0, -1)).r;
	float c6 = TEXTURE_BIAS_ZERO(TEX_AUXILIARY, uv + offset * float2(-1, -1)).r;
	float c7 = TEXTURE_BIAS_ZERO(TEX_AUXILIARY, uv + offset * float2(-1, 1)).r;
	
	// find edge with Sobel filter
	float sobel_x = c6 + c4 * 2.0f + c7 - c3 - c0 * 2.0f - c2;
	float sobel_y = c6 + c5 * 2.0f + c3 - c7 - c1 * 2.0f - c2;
	float sobel = sqrt(sobel_x * sobel_x + sobel_y * sobel_y);
	
	// apply threshold
	float edge = saturate(1.0f - dot(sobel, 1.0f));
	
	// Choose between scene and selection colors
	//OUT_COLOR = lerp(color, selection, TEXTURE_BIAS_ZERO(TEX_AUXILIARY, uv).r * (1.0f - edge));
	OUT_COLOR = color + selection * TEXTURE_BIAS_ZERO(TEX_AUXILIARY, uv).r * (1.0f - edge);
	
MAIN_END
