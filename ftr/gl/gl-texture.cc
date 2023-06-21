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

#include "ftr/texture.h"
#include "ftr/gl/gl.h"
#include "ftr/sys.h"
#include "native-glsl.h"

FX_NS(ftr)

typedef PixelData::Format PixelFormat;

/**
 * @func get_gl_texture_data_format 获取当前环境对应的OpenGL纹理数据格式
 */
static GLint get_gl_texture_data_format(PixelFormat format) {
	switch (format) {
		case PixelData::RGBA4444:
		case PixelData::RGBX4444: return GL_UNSIGNED_SHORT_4_4_4_4;
		case PixelData::RGBA5551: return GL_UNSIGNED_SHORT_5_5_5_1;
		case PixelData::RGB565: return GL_UNSIGNED_SHORT_5_6_5;
		default: return GL_UNSIGNED_BYTE;
	}
}

/**
 * @func get_gl_UNPACK_ALIGNMENT_size
 */
static uint get_gl_UNPACK_ALIGNMENT_VALUE(PixelFormat format) {
	switch (format) {
		case PixelData::RGBA4444:
		case PixelData::RGBX4444:
		case PixelData::RGB565:
		case PixelData::LUMINANCE_ALPHA88:
		case PixelData::RGBA5551: return 2;
		case PixelData::RGBA8888:
		case PixelData::RGBX8888: return 4;
		case PixelData::RGB888: return 3;
		default: return 1;
	}
}

inline static bool is_support_texture(GLDraw* ctx, PixelFormat pixel_format) {
	return ctx->get_gl_texture_pixel_format(pixel_format);
}

uint GLDraw::set_texture(const Array<PixelData>& data) {
	if ( data.length() == 0 ) {
		return 0;
	}
	cPixelData& pixel_data = data[0];
	
	if ( pixel_data.body().length() == 0 ) {
		return 0;
	}
	
	PixelFormat pixel_format = pixel_data.format();
	if ( ! is_support_texture(this, pixel_format) ) { // 当前GPU是否支持这些格式
		return 0;
	}
	
	uint handle;
	glGenTextures(1, &handle);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, handle);
	
	if ( !glIsTexture(handle) ) {
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &handle);
		return false;
	}
	
	GLint format = get_gl_texture_pixel_format(pixel_format); ASSERT(format);
	
	// 启用各向异性
#if defined(GL_EXT_texture_filter_anisotropic) && GL_EXT_texture_filter_anisotropic == 1
  //  GLfloat largest;
  //  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest);
  //  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest);
#endif 
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, get_gl_UNPACK_ALIGNMENT_VALUE(pixel_format));
	
	int mipmap_level = data.length();
	
	if ( PixelData::is_compressd_format(pixel_format) ) {
		if (mipmap_level > 1) { // mipmap
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_level - 1);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		for (int i = 0; i < mipmap_level; i++) {
			cPixelData& pixel_data = data[i];
			WeakBuffer body = pixel_data.body();
			glCompressedTexImage2D(GL_TEXTURE_2D, i, format,
														 pixel_data.width(),
														 pixel_data.height(), 0, body.length(), *body);
		}
	} else {
		if (mipmap_level > 1) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_level - 1);
			
			for (int i = 0; i < mipmap_level; i++) {
				cPixelData& pixel_data = data[i];
				glTexImage2D(GL_TEXTURE_2D, i, format,
										 pixel_data.width(),
										 pixel_data.height(), 0, format,
										 get_gl_texture_data_format(pixel_format), *pixel_data.body());
			}
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, format,
									 pixel_data.width(),
									 pixel_data.height(), 0, format,
									 get_gl_texture_data_format(pixel_format), *pixel_data.body());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}
	
	// Load texture success
	
	// GL_REPEAT / GL_CLAMP_TO_EDGE / GL_MIRRORED_REPEAT 
	// 纹理重复方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	return handle;
}

uint GLDraw::gen_texture(uint origin_texture, uint width, uint height) {
	GLuint default_frame_buffer = m_current_frame_buffer;
	GLuint handle;
	GLuint frame_buffer;
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	glGenTextures(1, &handle);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle, 0);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
		glViewport(0, 0, width, height);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, origin_texture);
		glUseProgram(shader::gen_texture.shader);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glViewport(0, 0, surface_size()[0], surface_size()[1]);
	} else {
		glDeleteTextures(1, &handle);
		handle = 0;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, default_frame_buffer);
	glDeleteFramebuffers(1, &frame_buffer);
	
	return handle;
}

static bool load_yuv_texture2(Draw* draw, uint handle,
															GLenum format,
															uint pixel_storei,
															uint offset_x, uint offset_y, 
															uint width, uint height, cchar* buffer, 
															bool new_gen, uint nw = 0, uint nh = 0) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, handle);

	if ( ! glIsTexture(handle) ) {
		return false;
	}
	
	// 纹理重复方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, pixel_storei);
	
	if (new_gen) {
		glTexImage2D(GL_TEXTURE_2D, 0, format,
								 nw ? nw : width,
								 nh ? nh : height, 0, format, GL_UNSIGNED_BYTE, nullptr);
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0,
									offset_x, offset_y,
									width, height, format, GL_UNSIGNED_BYTE, buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

bool GLDraw::set_yuv_texture(TextureYUV* yuv_tex, cPixelData& data) {
	PixelFormat format = data.format();
	ASSERT(format == PixelData::YUV420P || format == PixelData::YUV420SP ||
					 format == PixelData::YUV411P || format == PixelData::YUV411SP);
	
	if ( format != PixelData::YUV420P && format != PixelData::YUV420SP ) {
		return false; // 暂时只支持 420p/420sp
	}
	
	uint tex_y, tex_uv;
	bool gen = false;
	
	if (yuv_tex->m_handle[0] &&
			yuv_tex->width() == data.width() &&
			yuv_tex->height() == data.height() ) {
		tex_y = yuv_tex->m_handle[0];
		tex_uv = yuv_tex->m_handle[1];
	} else {
		gen = true;
		glGenTextures(1, &tex_y);
		glGenTextures(1, &tex_uv);
	}
	
#if FX_OSX
#define GL_LUMINANCE GL_RED
#define GL_LUMINANCE_ALPHA GL_RED
#endif
	
	// set yuv texure
	if ( load_yuv_texture2(this, tex_y, GL_LUMINANCE, 1,
												 0, 0, data.width(), data.height(), *data.body(), gen, 0, 0) ) {  // y
		bool ok = true;
		
		uint uv_w = ceilf(data.width() / 2.0);
		uint uv_h = ceilf(data.height() / 2.0);
		
		if ( format == PixelData::YUV420P ) { // 420p
			ok = load_yuv_texture2(this, tex_uv, GL_LUMINANCE, 1,                                   // u
														 0, 0, uv_w, uv_h, *data.body(1), gen, uv_w, uv_h * 2);
			if ( ok ) {                                                                             // v
				glTexSubImage2D(GL_TEXTURE_2D, 0,
												0, uv_h,
												uv_w, uv_h, GL_LUMINANCE, GL_UNSIGNED_BYTE, *data.body(2));
			}
		} else { // 420sp
			ok = load_yuv_texture2(this, tex_uv, GL_LUMINANCE_ALPHA, 2,                             // uv
														 0, 0, uv_w, uv_h, *data.body(1), gen, 0, 0);
		}
		
		if ( ok ) {
			// Load texture success
			
			if ( yuv_tex->m_handle[0] ) {
				if ( yuv_tex->m_handle[0] != tex_y ) {
					glDeleteTextures(2, yuv_tex->m_handle); // Delete old texture
				}
			}
			
			glBindTexture(GL_TEXTURE_2D, 0);
			yuv_tex->m_handle[0] = tex_y;
			yuv_tex->m_handle[1] = tex_uv;
			return true;
		}
	}
	
	if ( gen ) {
		glDeleteTextures(1, &tex_y);
		glDeleteTextures(1, &tex_uv);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	
	return false;
}

void GLDraw::del_texture(uint id) {
	glDeleteTextures(1, &id);
}

void GLDraw::use_texture(uint id, Repeat repeat, uint slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, id);
	
	switch (repeat) {
		default: // NONE
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		case Repeat::REPEAT:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		case Repeat::REPEAT_X:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		case Repeat::REPEAT_Y:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		case Repeat::MIRRORED_REPEAT:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			break;
		case Repeat::MIRRORED_REPEAT_X:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		case Repeat::MIRRORED_REPEAT_Y:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			break;
	}
}

void GLDraw::use_texture(uint id, uint slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, id);
}

FX_END
