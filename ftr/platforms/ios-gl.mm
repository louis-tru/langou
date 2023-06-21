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

#import "ios-gl-1.h"
#import "ftr/app.h"
#import "ftr/display-port.h"
#import "ftr/sys.h"
#import <OpenGLES/ES2/glext.h>

FX_NS(ftr)

/**
 * @class MyGLDraw
 */
template<class Basic> class MyGLDraw: public Basic {
 public:
	MyGLDraw(GUIApplication* host, EAGLContext* ctx,
					 DrawLibrary library,
					 cJSON& options): Basic(host, options), proxy_(this, ctx) {
		this->m_library = library;
		this->initialize();
	}
	virtual void commit_render() {
		proxy_.commit_render();
	}
	virtual GLint get_gl_texture_pixel_format(PixelData::Format pixel_format) {
		return proxy_.get_gl_texture_pixel_format(pixel_format);
	}
	virtual void gl_main_render_buffer_storage() {
		proxy_.gl_main_render_buffer_storage();
	}
	inline GLDrawProxy* proxy() { return &proxy_; }
	
 private:
	GLDrawProxy proxy_;
};

GLDrawProxy* GLDrawProxy::create(GUIApplication* host, cJSON& options) {
	GLDrawProxy* rv = nullptr;
	EAGLContext* ctx = [EAGLContext alloc];
	
	if ( [ctx initWithAPI:kEAGLRenderingAPIOpenGLES3] ) {
		rv = (new MyGLDraw<GLDraw>(host, ctx, DRAW_LIBRARY_GLES3, options))->proxy();
	} else
	if ( [ctx initWithAPI:kEAGLRenderingAPIOpenGLES2] ) {
		rv = (new MyGLDraw<GLDraw>(host, ctx, DRAW_LIBRARY_GLES2, options))->proxy();
	} else {
		FX_FATAL("Unable to initialize OGL device does not support OpenGLES");
	}
	
	return rv;
}

GLDrawProxy::GLDrawProxy(GLDraw* host, EAGLContext* ctx): m_host(host), m_context(ctx) {
	ASSERT([EAGLContext setCurrentContext:ctx], "Failed to set current OpenGL context");
	ctx.multiThreaded = NO;
}

GLDrawProxy::~GLDrawProxy() {
	[EAGLContext setCurrentContext:nullptr];
}

void GLDrawProxy::gl_main_render_buffer_storage() {
	// Create the color renderbuffer and call the rendering context to allocate the storage
	// on our Core Animation layer.
	// The width, height, and format of the renderbuffer storage are derived from the bounds
	// and properties of the CAEAGLLayer object
	// at the moment the renderbufferStorage:fromDrawable: method is called.
	[m_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:m_layer];
}

void GLDrawProxy::commit_render() {
	glBindVertexArray(0); // clear vao
	
	if (m_host->multisample() > 1) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_host->m_msaa_frame_buffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_host->m_frame_buffer);
		GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
		
		if (m_host->library() == DRAW_LIBRARY_GLES2) {
			glResolveMultisampleFramebufferAPPLE();
			glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER, 3, attachments);
		} else {
			Vec2 ssize = m_host->surface_size();
			glBlitFramebuffer(0, 0, ssize.width(), ssize.height(),
												0, 0, ssize.width(), ssize.height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
			glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 3, attachments);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, m_host->m_frame_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_host->m_frame_buffer);
	} else {
		GLenum attachments[] = { GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT, };
		if (m_host->library() == DRAW_LIBRARY_GLES2) {
			glDiscardFramebufferEXT(GL_FRAMEBUFFER, 2, attachments);
		} else {
			glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
		}
	}

	// Assuming you allocated a color renderbuffer to point at a Core Animation layer,
	// you present its contents by making it the current renderbuffer
	// and calling the presentRenderbuffer: method on your rendering context.
	[m_context presentRenderbuffer:GL_FRAMEBUFFER];
}

/**
 * @func get_gl_texture_pixel_format 获取当前环境对应的OpenGL纹理像素格式,如果返回0表示不支持纹理格式
 */
GLint GLDrawProxy::get_gl_texture_pixel_format(PixelData::Format pixel_format) {
	switch (pixel_format) {
		case PixelData::RGBA4444:
		case PixelData::RGBX4444:
		case PixelData::RGBA5551:
		case PixelData::RGBA8888:
		case PixelData::RGBX8888: return GL_RGBA;
		case PixelData::RGB565:
		case PixelData::RGB888: return GL_RGB;
		case PixelData::ALPHA8: return GL_ALPHA;
		case PixelData::LUMINANCE8: return GL_LUMINANCE;
		case PixelData::LUMINANCE_ALPHA88: return GL_LUMINANCE_ALPHA;
			// compressd texture
		case PixelData::PVRTCI_2BPP_RGB: return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		case PixelData::PVRTCI_2BPP_RGBA:
		case PixelData::PVRTCII_2BPP: return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
		case PixelData::PVRTCI_4BPP_RGB: return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
		case PixelData::PVRTCI_4BPP_RGBA:
		case PixelData::PVRTCII_4BPP: return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		case PixelData::ETC1:
		case PixelData::ETC2_RGB: return GL_COMPRESSED_RGB8_ETC2;
		case PixelData::ETC2_RGB_A1:
		case PixelData::ETC2_RGBA: return GL_COMPRESSED_RGBA8_ETC2_EAC;
		default: return 0;
	}
}

//void GLDrawProxy::set_current_context() {
//  ASSERT([EAGLContext setCurrentContext:m_context], "Failed to set current OpenGL context");
//}

void GLDrawProxy::set_surface_view(UIView* view, CAEAGLLayer* layer) {
	ASSERT([EAGLContext setCurrentContext:m_context], "Failed to set current OpenGL context");
	m_surface_view = view;
	m_layer = layer;
	m_host->set_best_display_scale(UIScreen.mainScreen.scale);
}

bool GLDrawProxy::refresh_surface_size(::CGRect rect) {
	float scale = UIScreen.mainScreen.scale;
	Vec2 size(rect.size.width * scale, rect.size.height * scale);
	if ( !size.is_zero() ) {
		return m_host->set_surface_size(size);
	}
	return false;
}

FX_END
