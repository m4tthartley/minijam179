//
//  Video for Apple devices
//
//  Created by Matt Hartley on 22/02/2025.
//  Copyright 2025 GiantJelly. All rights reserved.
//

#include <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#include <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <core/core.h>
#include <core/sysvideo.h>
#include <core/math.h>

#include "system.h"
#include "game.h"

#include <core/sysvideo.h>


typedef struct {
	// NSApplication* app;
	// NSWindow* window;

	// id<MTLDevice> device;
	// CAMetalLayer* metalLayer;
	// id<MTLCommandQueue> commandQueue;

	id<MTLRenderPipelineState> pipeline;

	id<MTLTexture> framebufferTexture;
} sys_objc_state_t;


NSString* shaderSource =
@"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertexOut {\n"
"	float4 pos [[position]];\n"
"	float2 texCoord;\n"
"};\n"
"vertex VertexOut vertex_main(uint vertexID [[vertex_id]]) {\n"
"	float4 vertices[] = {\n"
"		{-0.5, -0.5, 0.0, 1.0},\n"
"		{ 0.5, -0.5, 0.0, 1.0},\n"
"		{ 0.5,  0.5, 0.0, 1.0},\n"
"		{-0.5,  0.5, 0.0, 1.0},\n"
"	};\n"
"	float2 texCoords[] = {\n"
"		{0.0, 0.0},\n"
"		{1.0, 0.0},\n"
"		{1.0, 1.0},\n"
"		{0.0, 1.0},\n"
"	};\n"
"	VertexOut out;\n"
"	out.pos = vertices[vertexID];\n"
"	out.texCoord = texCoords[vertexID];\n"
"	return out;\n"
"}\n"
"fragment float4 fragment_main(VertexOut in [[stage_in]], texture2d<float> texture [[texture(0)]]) {\n"
"	constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);\n"
"	return texture.sample(textureSampler, in.texCoord);\n"
"	//return float4(1.0, 0.0, 1.0, 1.0);\n"
"}\n"
;

SYS_FUNC void Sys_InitMetal(sys_window_t* win, video_t* video) {
	sys_objc_state_t* state = (sys_objc_state_t*)video->objc_state;
	// sys_window_t* win = &video->window;

	sys_init_metal(win);

	NSWindow* window = win->sysWindow;
	id<MTLDevice> device = win->mtlDevice;
	CAMetalLayer* layer = win->mtlLayer;

	video->framebufferSize = int2(320, 200);
	video->framebuffer = malloc(sizeof(u32) * video->framebufferSize.x * video->framebufferSize.y);
	video->scaledFramebuffer = malloc(sizeof(u32) * video->screenSize.x * video->screenSize.y);

	NSError* error = NULL;

	id<MTLLibrary> lib = [device 
		newLibraryWithSource: shaderSource
		options:nil
		error:&error];
	if (!lib) {
		print_error((char*)[[error localizedDescription] UTF8String]);
		exit(1);
	}

	id<MTLFunction> vertex = [lib newFunctionWithName: @"vertex_main"];
	id<MTLFunction> fragment = [lib newFunctionWithName: @"fragment_main"];
	MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
	// desc.rasterSampleCount = 1;
	desc.vertexFunction = vertex;
	desc.fragmentFunction = fragment;
	desc.colorAttachments[0].pixelFormat = layer.pixelFormat;
	state->pipeline = [[device
		newRenderPipelineStateWithDescriptor: desc
		error: &error
	] retain];
	if (!state->pipeline) {
		print_error((char*)[[error localizedDescription] UTF8String]);
		exit(1);
	}
	// video->pipeline = pipeline;

	[lib release];
	[vertex release];
	[fragment release];
	[desc release];

	MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
	texDesc.pixelFormat = layer.pixelFormat;
	texDesc.width = video->screenSize.x;
	texDesc.height = video->screenSize.y;
	texDesc.usage = MTLTextureUsageShaderRead;
	texDesc.textureType = MTLTextureType2D;
	state->framebufferTexture = [[device newTextureWithDescriptor: texDesc] retain];
	[texDesc release];
	// state->framebufferTexture = texture;

	window.contentView.layer = layer;
}

SYS_FUNC void Sys_OutputFrameAndSync(sys_window_t* win, video_t* video) {
	sys_objc_state_t* state = (sys_objc_state_t*)video->objc_state;
	// sys_window_t* win = &video->window;
	CAMetalLayer* layer = win->mtlLayer;
	id<MTLCommandQueue> commandQueue = win->mtlCommandQueue;

	// id<MTLTexture> framebufferTexture = video->framebufferTexture;
	// CAMetalLayer* metalLayer = video->metalLayer;
	// id<MTLCommandQueue> commandQueue = video->commandQueue;

	// Scale framebuffer up to window framebuffer size
	float xd = (float)video->framebufferSize.x / (float)video->screenSize.x;
	float yd = (float)video->framebufferSize.y / (float)video->screenSize.y;
	float diff = xd / yd;
	int relativeWidth = ((float)video->screenSize.y / (float)video->framebufferSize.y) * (float)video->framebufferSize.x; //(float)video->screenSize.x * diff;
	int xoffset = (video->screenSize.x-relativeWidth)/2;
	for (int iy=0; iy<video->screenSize.y; ++iy){
		for (int ix=xoffset; ix<xoffset+relativeWidth; ++ix) {
			int x = ((float)(ix-xoffset) / relativeWidth) * (float)video->framebufferSize.x;
			int y = ((float)(iy) / video->screenSize.y) * (float)video->framebufferSize.y;
			video->scaledFramebuffer[iy*video->screenSize.x+ix] = video->framebuffer[(video->framebufferSize.y-y-1)*video->framebufferSize.x+x];
		}
	}

	MTLRegion region = {
		.origin = {0, 0, 0,},
		.size = {video->screenSize.x, video->screenSize.y, 1},
	};
	[state->framebufferTexture
		replaceRegion: region
		mipmapLevel: 0
		withBytes: video->scaledFramebuffer
		bytesPerRow: sizeof(u32) * video->screenSize.x
	];

	id<CAMetalDrawable> drawable = [layer nextDrawable];
	// MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];
	// pass.colorAttachments[0].texture = drawable.texture;
	// pass.colorAttachments[0].loadAction = MTLLoadActionClear;
	// pass.colorAttachments[0].clearColor = MTLClearColorMake(0, 0.5, 0, 1);
	// pass.colorAttachments[0].storeAction = MTLStoreActionStore;

	id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
	// id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor: pass];
	// [encoder setRenderPipelineState: video->pipeline];

	// [encoder setFragmentTexture: video->framebufferTexture atIndex:0];
	// [encoder drawPrimitives: MTLPrimitiveTypeTriangle vertexStart: 0 vertexCount: 3];

	// [encoder endEncoding];

	id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
	[blitEncoder 
		copyFromTexture: state->framebufferTexture
		sourceSlice: 0
		sourceLevel: 0
		sourceOrigin: (MTLOrigin){0, 0, 0}
		sourceSize: (MTLSize){video->screenSize.x, video->screenSize.y, 1}
		toTexture: drawable.texture
		destinationSlice: 0
		destinationLevel: 0
		destinationOrigin: (MTLOrigin){0, 0, 0}
	];
	[blitEncoder endEncoding];

	[commandBuffer presentDrawable: drawable];
	[commandBuffer commit];
}

