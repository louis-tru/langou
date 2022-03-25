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


#ifndef __flare__render__metal__
#define __flare__render__metal__

#include "./render.h"

#include <QuartzCore/CAMetalLayer.h>
#include <Metal/Metal.h>
#include <MetalKit/MTKView.h>

F_NAMESPACE_START

class MetalRender: public Render {
public:
	virtual ~MetalRender();
	virtual void reload() override;
	virtual void begin() override;
	virtual void submit() override;
	virtual void activate(bool isActive) override;
protected:
	virtual void onReload() = 0;
	virtual void onBegin() = 0;
	virtual void onSubmit() = 0;
	MetalRender(Application* host, const Options& opts);
	MTKView*            _view;
	CAMetalLayer*       _layer;
	id<MTLDevice>       _device;
	id<MTLCommandQueue> _queue; // sk_cfp<id<MTLCommandQueue>>
	id<CAMetalDrawable> _drawable;
	id               _pipelineArchive; // id<MTLBinaryArchive>
};

F_NAMESPACE_END
#endif