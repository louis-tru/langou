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

#include "./glsl_shader.h"

namespace qk {

	static GLuint compile_shader(cChar* name, const GLchar* code, GLenum shader_type) {
		GLuint shader_handle = glCreateShader(shader_type);
		GLint code_len = (GLint)strlen(code);
		glShaderSource(shader_handle, 1, &code, &code_len);
		glCompileShader(shader_handle);
		GLint ok;
		glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &ok);
		if ( ok != GL_TRUE ) {
			char log[255] = { 0 };
			glGetShaderInfoLog(shader_handle, 254, &ok, log);
			Qk_FATAL("Compile shader error, %s\n\n%s", name, log);
		}
		return shader_handle;
	}

#if Qk_OSX
#define Qk_GL_Version "330"
#else // ios es
#define Qk_GL_Version "300 es"
#endif

	static const String vertexHeader(String::format("\n\
		#version %s\n\
		#define matrix root_matrix * view_matrix\n\
		uniform mat4  root_matrix;\n\
		layout (std140) uniform ubo {\n\
			mat4  view_matrix;\n\
		};\n\
		in      vec2  vertex_in;\n\
	", Qk_GL_Version));

	static const String fragmentHeader(String::format("\n\
		#version %s\n\
		layout(location=0) out lowp vec4 color_o;\n\
	", Qk_GL_Version));

	GLSLShader::GLSLShader(): _shader(0) {
	}

	void GLSLShader::compile(
		cChar* name, cChar* vertexShader, cChar* fragmentShader,
		cChar* _attrs, cChar* _uniforms, GLuint *storeLocation)
	{
		Qk_ASSERT(!_shader);
		GLuint vertex_handle =
			compile_shader(name, (vertexHeader + vertexShader).c_str(), GL_VERTEX_SHADER);
		GLuint fragment_handle =
			compile_shader(name, (fragmentHeader + fragmentShader).c_str(), GL_FRAGMENT_SHADER);
		GLuint program = glCreateProgram();
		glAttachShader(program, vertex_handle);
		glAttachShader(program, fragment_handle);

		GLint status; // query status

		auto attrs = String(_attrs).split(",");
		auto uniforms = String(_uniforms).split(",");
		
		Qk_DEBUG("sizeof(GLSLShader) %d,%d,%d", sizeof(GLSLShader), sizeof(GLSLColor), sizeof(GLSLImage));

		// bind attrib Location
		GLuint attrIdx = 0;
		glBindAttribLocation(program, _vertex_in = attrIdx++, "vertex_in");
		for (auto &i: attrs) {
			if (!i.is_empty()) {
				glBindAttribLocation(program, *storeLocation = attrIdx++, i.c_str());
				storeLocation++;
			}
		}

		glLinkProgram(program);
		glDeleteShader(vertex_handle);
		glDeleteShader(fragment_handle);

		if ((glGetProgramiv(program, GL_LINK_STATUS, &status), status) != GL_TRUE) {
			char log[256] = { 0 };
			glGetProgramInfoLog(program, 255, &status, log);
			Qk_FATAL("Link shader error, %s\n\n%s", name, log);
		}

		// Get uniform block and bind index
		GLuint ubo = glGetUniformBlockIndex(program, "ubo");
		glUniformBlockBinding(program, ubo, 0); // bind uniform block index as zero
#if DEBUG
		GLint bufferSize;
		glGetActiveUniformBlockiv(program, ubo, GL_UNIFORM_BLOCK_DATA_SIZE, &bufferSize);
		Qk_ASSERT(bufferSize == 64);
#endif

		// Get Uniform Location index value
		_root_matrix = glGetUniformLocation(program, "root_matrix");
		// _view_matrix = glGetUniformLocation(program, "view_matrix");
		for (auto &i: uniforms) {
			if (!i.is_empty()) {
				*storeLocation = glGetUniformLocation(program, i.c_str());
				storeLocation++;
			}
		}

		glUseProgram(program);
		glEnableVertexAttribArray(_vertex_in);

		_shader = program;
	}

	void GLSLColor::build() {
		compile("color shader",
		"\n\
			uniform vec4  color;\n\
			out     vec4  color_f;\n\
			void main() {\n\
				color_f = color;\n\
				gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);\n\
			}\n\
		",
		"\n\
			in lowp vec4 color_f;\n\
			void main() {\n\
				color_o = color_f;\n\
			}\n\
		",
		"", "color", &_color);
	}

	static const char *v_image_shader = "\n\
		uniform   float     opacity;\n\
		uniform   vec4      coord;//offset,scale\n\
		out       float     opacity_f;\n\
		out       vec2      coord_f;\n\
		void main() {\n\
			opacity_f = opacity;\n\
			coord_f = (vertex_in.xy - coord.xy) * coord.zw;\n\
			gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);\n\
		}\n\
	";

	void GLSLImage::build() {
		compile("image shader", v_image_shader,
		"\n\
			in lowp   float     opacity_f;\n\
			in lowp   vec2      coord_f;\n\
			uniform   sampler2D image;\n\
			void main() {\n\
				color_o = texture(image, coord_f) * vec4(1.0, 1.0, 1.0, opacity_f);\n\
			}\n\
		",
		"", "opacity,coord,image", &_opacity);
	}

	void GLSLImageYUV420P::build() {
		compile("yuv420p shader", v_image_shader,
		"\n\
			in lowp   float     opacity_f;\n\
			in lowp   vec2      coord_f;\n\
			uniform   sampler2D image;\n\
			uniform   sampler2D image_u;\n\
			uniform   sampler2D image_v;\n\
			void main() {\n\
				lowp float y = texture(image, coord_f).r;\n\
				lowp float u = texture(image_u, coord_f).r;\n\
				lowp float v = texture(image_v, coord_f).r;\n\
				color_o = vec4( y + 1.4075 * (v - 0.5),\n\
												y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),\n\
												y + 1.779  * (u - 0.5),\n\
												opacity_f);\n\
			}\n\
		",
		"", "opacity,coord,image,image_u,image_v", &_opacity);
	}

	void GLSLImageYUV420SP::build() {
		compile("yuv420sp shader", v_image_shader,
		"\n\
			in lowp   float     opacity_f;\n\
			in lowp   vec2      coord_f;\n\
			uniform   sampler2D image;\n\
			uniform   sampler2D image_uv;\n\
			void main() {\n\
				lowp float y = texture(image, coord_f).r;\n\
				lowp float u = texture(image_uv, coord_f).r;\n\
				lowp float v = texture(image_uv, coord_f).a;\n\
				color_o = vec4( y + 1.4075 * (v - 0.5),\n\
												y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),\n\
												y + 1.779  * (u - 0.5),\n\
												opacity_f);\n\
			}\n\
		",
		"", "opacity,coord,image,image_uv", &_opacity);
	}

	void GLSLGradient::build() {

		const char *v_shader = _type == Paint::kLinear_GradientType ?
		// kLinear
		"\n\
			uniform   vec4      range;//start/end range for rect\n\
			out       float     position_f;\n\
			void main() {\n\
				vec2 ao = range.zw     - range.xy;\n\
				vec2 bo = vertex_in.xy - range.xy;\n\
				position_f = clamp(dot(ao,bo) / dot(ao,ao), 0.0, 1.0);\n\
				gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);\n\
			}\n\
		":
		// kRadial
		"\n\
			uniform   vec4      range;//center/radius for circle\n\
			out       float     position_f;\n\
			void main() {\n\
				position_f = min(1.0, length((vertex_in.xy-range.xy)/range.zw));\n\
				gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);\n\
			}\n\
		";

		compile("gradient shader", v_shader,
		"\n\
			in      lowp float     position_f;\n\
			uniform      int       count;\n\
			uniform lowp vec4      colors[256];//max 256 color points\n\
			uniform lowp float     positions[256];\n\
			void main() {\n\
				int s = 0;\n\
				int e = count-1;\n\
				while (s+1 < e) {// dichotomy search color value\n\
					int idx = (e - s) / 2 + s;\n\
					if (position_f > positions[idx]) {\n\
						s = idx;\n\
					} else if (position_f < positions[idx]) {\n\
						e = idx;\n\
					} else { \n\
						s = idx; e = idx+1; break; \n\
					}\n\
				}\n\
				lowp float w = (position_f - positions[s]) / (positions[e] - positions[s]);\n\
				color_o = mix(colors[s], colors[e], w);\n\
			\
			}\n\
		",
		"", "range,count,colors,positions", &_range);
	}

}
