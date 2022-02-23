// @private head
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


#ifndef __ftr__render_render__
#define __ftr__render_render__

#include "../util/loop.h"
#include "../util/json.h"
#include "../math.h"
#include "../source.h"

#define SK_GL 1

#if F_IOS || F_OSX
# define SK_METAL 1
#elif F_ANDROID
# define SK_VULKAN 1
#elif F_WIN
# define SK_DIRECT3D 1
#endif

#include <skia/core/SkCanvas.h>
#include <skia/core/SkSurface.h>
#include <skia/gpu/GrDirectContext.h>
#include <skia/gpu/gl/GrGLInterface.h> // gl

namespace flare {

	class Application;

	class Canvas: public SkCanvas {
		public:
			void setMatrix(const flare::Mat& mat);
	};

	/**
	* @class Render
	*/
	class F_EXPORT Render: public Object, public PostMessage {
		F_HIDDEN_ALL_COPY(Render);
		public:
			typedef SkSurfaceProps::Flags Flags;
			struct Options {
				ColorType colorType;
				Flags     flags;
				int  msaaSampleCnt, stencilBits;
				bool enableBinaryArchive;
				bool enableGpu, disableMetal, disableVulkan;
			};
			static Options parseOptions(cJSON& opts);
			static Render* Make(Application* host, const Options& opts);

			Canvas* canvas();
			GrDirectContext* direct();

			virtual ~Render();
			virtual SkSurface* surface() = 0;
			virtual bool is_gpu() = 0;
			virtual void reload() = 0;
			virtual void submit() = 0;
			virtual void activate(bool isActive);
			inline Application* host() { return _host; }
			virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0) override;

		protected:
			Render(Application* host, const Options& params);
			Application*  _host;
			Options       _opts;
			sk_sp<GrDirectContext> _direct;
	};

	class RasterRender: public Render {
		public:
			virtual SkSurface* surface() override;
			virtual bool is_gpu() override { return false; }
			virtual void reload() override;
			virtual void submit() override;
		protected:
			virtual void onSubmit(SkPixmap* pixmap) = 0;
			RasterRender(Application* host, const Options& opts);
			sk_sp<SkSurface> _rasterSurface;
	};

}

#endif
