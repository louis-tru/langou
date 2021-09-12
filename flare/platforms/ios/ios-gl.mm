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

#import <OpenGLES/ES3/gl.h>
#import <UIKit/UIKit.h>

#include "../../render/gl.h"
#include "../mac/_mac-render_.h"

namespace flare {

	class GLRenderIOS: public GLRender, public RenderMAC {
	public:
		GLRenderIOS(GUIApplication* host, EAGLContext* ctx, const DisplayParams& params, bool gles2)
			: GLRender(host, params), _glctx(ctx), _gles2(gles2)
		{
			ASSERT([EAGLContext setCurrentContext:ctx], "Failed to set current OpenGL context");
			ctx.multiThreaded = NO;
			initialize();
		}

		~GLRenderIOS() {
			[EAGLContext setCurrentContext:nullptr];
		}

		void setView(UIView* view) {
			ASSERT(!_view);
			_view = view;
			_layer = view.layer;
			_layer.drawableProperties = @{
				kEAGLDrawablePropertyRetainedBacking : @NO,
				kEAGLDrawablePropertyColorFormat     : kEAGLColorFormatRGBA8
			};
			// _layer.opaque = YES;
			// _layer.frame = frameRect;
			// _layer.contentsGravity = kCAGravityTopLeft;
			_host->display()->set_best_display_scale(UIScreen.mainScreen.scale);
		}

		Class layerClass() { return [CAEAGLLayer class]; }

		Render* render() { return this; }

		void glRenderbufferStorageMain() {
			[_glctx renderbufferStorage:GL_RENDERBUFFER fromDrawable:_layer];
		}

		void commit() {
			if (gpuMSAASample()) {
				glBindFramebuffer(GL_READ_FRAMEBUFFER, _msaa_frame_buffer);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frame_buffer);
				GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
				if (_gles2) {
					glResolveMultisampleFramebufferAPPLE();
					glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER, 3, attachments);
				} else {
					auto region = _host->display()->surface_region();
					glBlitFramebuffer(0, 0, region.width, region.height,
														0, 0, region.width, region.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
					glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 3, attachments);
				}
				glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
				glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
			} else {
				GLenum attachments[] = { GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
				if (_gles2) {
					glDiscardFramebufferEXT(GL_FRAMEBUFFER, 2, attachments);
				} else {
					glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
				}
			}

			// Assuming you allocated a color renderbuffer to point at a Core Animation layer,
			// you present its contents by making it the current renderbuffer
			// and calling the presentRenderbuffer: method on your rendering context.
			[_glctx presentRenderbuffer:GL_FRAMEBUFFER];
		}

	private:
		EAGLContext* _glctx;
		UIView* _view;
		CAEAGLLayer* _layer;
		bool _gles2;
	};

	RenderMAC* MakeMetalRender(GUIApplication* host, const DisplayParams& parems) {
		EAGLContext* ctx = [EAGLContext alloc];
		if ( [ctx initWithAPI:kEAGLRenderingAPIOpenGLES3] ) {
			return new GLRenderIOS(host, ctx, parems, false);
		} else if ( [ctx initWithAPI:kEAGLRenderingAPIOpenGLES2] ) {
			return new GLRenderIOS(host, ctx, parems, true);
		} else {
			return nullptr;
		}
	}

}  // namespace flare
