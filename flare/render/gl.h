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

#ifndef __flare__render__gl__
#define __flare__render__gl__

#include "./render.h"
#include "skia/gpu/gl/GrGLInterface.h"
#include "skia/core/SkRefCnt.h"
#include "skia/core/SkSurface.h"

namespace flare {

	class GLRender: public Render {
	public:
		sk_sp<SkSurface> getBackbufferSurface() override;

		bool isValid() override;

		void resize(Vec2 size, Rect surface_region) override;
		void swapBuffers() override;

		void setDisplayParams(const DisplayParams& params) override;

	protected:
		GLRender(Application* host, const DisplayParams& params);
		// This should be called by subclass constructor. It is also called when window/display
		// parameters change. This will in turn call onInitializeContext().
		void initializeContext();
		virtual sk_sp<const DisplayParams> onInitializeContext() = 0;

		// This should be called by subclass destructor. It is also called when window/display
		// parameters change prior to initializing a new GL context. This will in turn call
		// onDestroyContext().
		void destroyContext();
		virtual void onDestroyContext() = 0;

		virtual void onSwapBuffers() = 0;

		bool isGpuContext() override { return true; }

		sk_sp<const GrGLInterface> fBackendContext;
		sk_sp<SkSurface>           fSurface;
	};

}   // namespace flare

#endif
