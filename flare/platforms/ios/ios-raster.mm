/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#import <OpenGLES/ES3/gl.h>
#import <UIKit/UIKit.h>

#include "../../render/gl.h"
#include "../../display.h"
#include "../apple/apple-render.h"

namespace flare {

	class RasterRenderIOS: public GLRender, public RenderApple {
	 public:
		RasterRenderIOS(Application* host, EAGLContext* ctx, const Options& params)
			: GLRender(host, params), _ctx(ctx)
		{
		}

		~RasterRenderIOS() {
			[EAGLContext setCurrentContext:nullptr];
		}

		void setView(UIView* view) override {
			F_ASSERT(!_view);
			[EAGLContext setCurrentContext:_ctx];
			F_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
			_view = view;
			_layer = (CAEAGLLayer*)view.layer;
			_layer.drawableProperties = @{
				kEAGLDrawablePropertyRetainedBacking : @NO,
				kEAGLDrawablePropertyColorFormat     : kEAGLColorFormatRGBA8
			};
			_layer.opaque = YES;
		}

		Render* render() override { return this; }

		Class layerClass() override { return [CAEAGLLayer class]; }

		bool is_gpu() override { return false; }

		void commit() override {
			if (!_RasterSurface) return;
			// We made/have an off-screen surface. Get the contents as an SkImage:
			sk_sp<SkImage> snapshot = _RasterSurface->makeImageSnapshot();
			SkSurface* gpuSurface = GLRender::surface();
			gpuSurface->getCanvas()->drawImage(snapshot, 0, 0);
			gpuSurface->flushAndSubmit();

			GLenum attachments[] = { GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
			glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);

			// Assuming you allocated a color renderbuffer to point at a Core Animation layer,
			// you present its contents by making it the current renderbuffer
			// and calling the presentRenderbuffer: method on your rendering context.
			[_ctx presentRenderbuffer:GL_FRAMEBUFFER];
		}

		SkSurface* surface() override {
			if (!_RasterSurface) {
				// make the offscreen image
				auto region = _host->display()->surface_region();
				auto info = SkImageInfo::Make(region.width, region.height,
																			SkColorType(_opts.colorType), kPremul_SkAlphaType, nullptr);
				_RasterSurface = SkSurface::MakeRaster(info);
			}
			return _RasterSurface.get();
		}

		void gl_renderbuffer_storage() override {
			[_ctx renderbufferStorage:GL_RENDERBUFFER fromDrawable:_layer];
		}

		void reload() override {
			_RasterSurface.reset();
			GLRender::reload();
		}

	private:
		EAGLContext* _ctx;
		UIView* _view;
		CAEAGLLayer* _layer;
		sk_sp<SkSurface> _RasterSurface;
	};

	RenderApple* MakeRasterRender(Application* host, const Render::Options& parems) {
		EAGLContext* ctx = [EAGLContext alloc];
		if ( [ctx initWithAPI:kEAGLRenderingAPIOpenGLES3] ) {
			[EAGLContext setCurrentContext:ctx];
			F_ASSERT([EAGLContext currentContext], "Failed to set current OpenGL context");
			ctx.multiThreaded = NO;
			return new RasterRenderIOS(host, ctx, parems);
		} else {
			return nullptr;
		}
	}

}  // namespace flare
