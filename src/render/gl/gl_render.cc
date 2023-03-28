/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "../../app.h"
#include "../../display.h"
#include "./gl_render.h"
#include "./gl_canvas.h"

namespace qk {

	uint32_t gl_set_texture(cPixel* src, GLuint id, bool isGenerateMipmap);

	uint32_t gl_pixel_internal_format(ColorType type) {
		switch (type) {
			case kColor_Type_RGB_565: return GL_RGB565;
			case kColor_Type_RGBA_8888: return GL_RGBA8;
			case kColor_Type_RGB_888X: return GL_RGBA8;
			case kColor_Type_RGBA_1010102: return GL_RGB10_A2;
			case kColor_Type_RGB_101010X: return GL_RGB10_A2;
			default: return 0;
		}
	}

	bool glIsSupportMultisampled() {
		String VENDOR = (const char*)glGetString(GL_VENDOR);
		String RENDERER = (const char*)glGetString(GL_RENDERER);
		String version = (const char*)glGetString(GL_VERSION);
		String extensions = (const char*)glGetString(GL_EXTENSIONS);

		Qk_DEBUG("OGL VENDOR: %s", *VENDOR);
		Qk_DEBUG("OGL RENDERER: %s", *RENDERER);
		Qk_DEBUG("OGL VERSION: %s", *version);
		Qk_DEBUG("OGL EXTENSIONS: %s", *extensions);

		String str = String::format("%s %s %s %s", *VENDOR, *RENDERER, *version, *extensions);

		for (auto s : {"OpenGL ES", "OpenGL", "OpenGL Entity"}) {
			int idx = str.index_of(s);
			if (idx != -1) {
				int num = str.substr(idx + strlen(s)).trim().substr(0,1).to_number<int>();
				if (num > 2)
					return true;
				else if (extensions.index_of( "multisample" ) != -1)
					return false;
			}
		}

		return version.index_of("Metal") != -1;
	}

	GLRender::GLRender(Options opts)
		: GLCanvas(this), Render(opts)
		, _Is_Support_Multisampled(glIsSupportMultisampled())
		, _shaders{&_clear, &_clip, &_color, &_image, &_yuv420p, &_yuv420sp, &_linear, &_radial}
	{
		switch(_opts.colorType) {
			case kColor_Type_BGRA_8888:
				_opts.colorType = kColor_Type_RGBA_8888; break;
			case kColor_Type_BGRA_1010102:
				_opts.colorType = kColor_Type_RGBA_1010102; break;
			case kColor_Type_BGR_101010X:
				_opts.colorType = kColor_Type_RGB_101010X; break;
			default: break;
		}

		for (auto shader: _shaders) {
			shader->build();
		}

		glUseProgram(_image.shader());
		glUniform1i(_image.image(), 0);

		glUseProgram(_yuv420p.shader());
		glUniform1i(_yuv420p.image(), 0);
		glUniform1i(_yuv420p.image_u(), 1);
		glUniform1i(_yuv420p.image_v(), 2);

		glUseProgram(_yuv420sp.shader());
		glUniform1i(_yuv420sp.image(), 0);
		glUniform1i(_yuv420sp.image_uv(), 1);
		
		glUseProgram(0);

		_canvas = this; // set default canvas

		if (!_Is_Support_Multisampled) {
			_opts.msaaSampleCnt = 0;
		}
		// enable and disable test function
		glClearStencil(127);
		glStencilMask(0xFFFFFFFF);
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE); // enable color
		glDisable(GL_STENCIL_TEST); // disable stencil test
		glDisable(GL_DEPTH_TEST); // disable depth test
		glEnable(GL_BLEND); // enable color blend
	}

	GLRender::~GLRender() {}

	Object* GLRender::asObject() {
		return this;
	}

	void GLRender::reload() {
		auto size = getSurfaceSize();
		Mat4 mat;
		if (!_delegate->onRenderBackendReload({Vec2{0,0},size}, size, getDefaultScale(), &mat, &_surfaceScale))
			return;

		auto w = size.x(), h = size.y();

		Qk_ASSERT(w, "Invalid viewport size width");
		Qk_ASSERT(h, "Invalid viewport size height");

		glViewport(0, 0, w, h);

		glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer); // bind frame buffer
		setRenderBuffer(w, h);

		if (_opts.msaaSampleCnt > 1 && !_IsDeviceMsaa) {
			glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);

			do { // enable multisampling
				setMSAABuffer(w, h, _opts.msaaSampleCnt);
				// Test the framebuffer for completeness.
				if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ) {
					if ( _opts.msaaSampleCnt > 1 )
						_IsDeviceMsaa = true;
					break;
				}
				_opts.msaaSampleCnt >>= 1;
			} while (_opts.msaaSampleCnt > 1);
		}

		if (!_IsDeviceMsaa) { // no device msaa
			glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
			setAntiAlias(w, h);
			setDepthBuffer(w, h);
		}

		setStencilBuffer(w, h, _opts.msaaSampleCnt);

		const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(_IsDeviceMsaa ? 1: 2, buffers);

		if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {
			Qk_FATAL("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		}

#if DEBUG
		int width, height;
		// Retrieve the height and width of the color renderbuffer.
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
		Qk_DEBUG("GL_RENDERBUFFER_WIDTH: %d, GL_RENDERBUFFER_HEIGHT: %d", width, height);
#endif

		setRootMatrixBuffer(mat);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void GLRender::setRenderBuffer(int width, int height) {
		glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, gl_pixel_internal_format(_opts.colorType), width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _render_buffer);
	}

	void GLRender::setStencilBuffer(int width, int height, int MSAASample) { // set clip stencil buffer
		glBindRenderbuffer(GL_RENDERBUFFER, _stencil_buffer);
		if (MSAASample > 1) {
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAASample, GL_STENCIL_INDEX8, width, height);
		} else {
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
		}
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _stencil_buffer);
	}

	void GLRender::setMSAABuffer(int width, int height, int MSAASample) {
		glBindRenderbuffer(GL_RENDERBUFFER, _msaa_render_buffer); // render buffer
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAASample, gl_pixel_internal_format(_opts.colorType), width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _msaa_render_buffer);
	}

	void GLRender::setAntiAlias(int width, int height) {
		// set anti alias texture buffer
		glActiveTexture(GL_TEXTURE31);
		glBindTexture(GL_TEXTURE_2D, _aa_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width * 2, height * 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _aa_tex, 0);
		// glBindTexture(GL_TEXTURE_2D, 0);
	}

	void GLRender::setDepthBuffer(int width, int height) {
		// set depth buffer
		glBindRenderbuffer(GL_RENDERBUFFER, _depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_buffer);
	}

	GLuint GLRender::setTexture(cPixel *src, uint32_t id) {
		return gl_set_texture(src, id, true);
	}

	void GLRender::deleteTextures(const uint32_t *IDs, uint32_t count) {
		glDeleteTextures(count, IDs);
	}

}
