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


#ifndef __ftr__render_skia_render__
#define __ftr__render_skia_render__

#include "../../util/macros.h"

#if F_APPLE
#include "../metal.h"
#endif

#if !F_APPLE || F_ENABLE_GL
#include "../gl.h"
#endif

#include "./skia_canvas.h"
#include "skia/core/SkSurface.h"
#include "skia/gpu/GrBackendSurface.h"
#include "skia/gpu/GrDirectContext.h"

F_NAMESPACE_START

/**
* @class SkiaRender
*/
class SkiaRender: public ViewVisitor {
public:
	SkiaRender();
	virtual void visitView(View* box);
	virtual void visitBox(Box* box);
	virtual void visitImage(Image* image);
	virtual void visitVideo(Video* video);
	virtual void visitScroll(Scroll* scroll);
	virtual void visitInput(Input* input);
	virtual void visitText(Text* text);
	virtual void visitLabel(Label* label);
	virtual void visitRoot(Root* root);
	virtual void visitFlowLayout(FlowLayout* flow);
	virtual void visitFlexLayout(FlexLayout* flex);
private:
	typedef void (*DrawFn)(SkiaRender* render, Box* v);
	void solveBox(Box* v, DrawFn fn);
	void solvePaintImage(Box *host, PaintImage* paint);
	void solvePaintGradient(Box *host, PaintGradient* paint);
	void solvePaintShadow(Box *host, PaintShadow* paint);
protected:
	sk_sp<GrDirectContext> _direct;
	sk_sp<SkSurface> _surface;
	sk_sp<SkSurface> _rasterSurface;
	SkiaCanvas*      _canvas;
	union {
		uint32_t  _alpha32;
		struct {
			uint8_t val; // Little Endian
			uint8_t __[3];
		} alpha;
	};
	bool _raster; // software raster
};

#if !F_APPLE || F_ENABLE_GL
/**
* @class SkiaGLRender
*/
class SkiaGLRender: public GLRender, public SkiaRender {
public:
	virtual ViewVisitor* visitor() override;
protected:
	virtual void onReload() override;
	virtual void onSubmit() override;
	SkiaGLRender(Application* host, const Options& opts, bool raster);
};
#endif

#ifdef F_APPLE
/**
* @class SkiaMetalRender
*/
class SkiaMetalRender: public MetalRender, public SkiaRender {
public:
	virtual ViewVisitor* visitor() override;
protected:
	virtual void onReload() override;
	virtual void onBegin() override;
	virtual void onSubmit() override;
	SkiaMetalRender(Application* host, const Options& opts, bool raster);
};
#endif

F_NAMESPACE_END
#endif
