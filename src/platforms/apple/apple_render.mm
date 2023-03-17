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

#import "./apple_app.h"
#import "../../display.h"
#import "../../render/gl/gl_render.h"

using namespace qk;

uint32_t Render::post_message(Cb cb, uint64_t delay_us) {
	if (_renderLoop) {
		return _renderLoop->post(cb, delay_us);
	} else {
#if Qk_USE_DEFAULT_THREAD_RENDER || Qk_OSX
		auto main = dispatch_get_main_queue();
		if (main == dispatch_get_current_queue()) {
			cb->resolve();
		} else {
			auto core = cb.Handle::collapse();
			dispatch_async(main, ^{
				core->resolve();
				core->release();
			});
		}
		return 0;
#else
		return _host->loop()->post(cb, delay_us);
#endif
	}
}

// ------------------- Metal ------------------
#if Qk_ENABLE_METAL
#import "../../render/metal/metal_render.h"

@interface MTView: UIView
@end

@implementation MTView
+ (Class)layerClass {
	if (@available(iOS 13.0, *))
		return CAMetalLayer.class;
	return nil;
}
@end

class AppleMetalRender: public MetalRender, public QkAppleRender {
public:
	AppleMetalRender(Application* host, bool independentThread): MetalRender(host, independentThread)
	{}
	UIView* make_surface_view(CGRect rect) override {
		_view = [[MTKView alloc] initWithFrame:rect device:nil];
		_view.layer.opaque = YES;
		return _view;
	}
	Render* render() override {
		return this;
	}
};
#endif

// ------------------- OpenGL ------------------
#if Qk_ENABLE_GL
QkAppleRender* makeAppleGLRender(Application* host, bool independentThread);
#endif

Render* Render::Make(Application* host) {
	QkAppleRender* r = nullptr;
	bool independentThread = host->options().independentThread;

	if (independentThread) {
#if Qk_USE_DEFAULT_THREAD_RENDER || Qk_OSX
		independentThread = false; // use default thread render
#endif
	}

#if Qk_ENABLE_METAL
	if (@available(macOS 10.11, iOS 13.0, *))
		r = new AppleMetalRender(host, independentThread);
#endif
#if Qk_ENABLE_GL
	if (!r)
		r = makeAppleGLRender(host, independentThread);
#endif
	Qk_ASSERT(r, "create render object fail");

	return r->render();
}
