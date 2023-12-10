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

#ifndef __quark__layout__image__
#define __quark__layout__image__

#include "./box.h"
#include "../../render/source.h"

namespace qk {

	class Qk_EXPORT ImageLayout: public BoxLayout, public ImageSourceHolder {
	public:
		ImageLayout(Window *win);
		virtual bool layout_forward(uint32_t mark) override;
		virtual void draw(UIRender *render) override;
	protected:
		virtual float solve_layout_content_width(Size &parent_layout_size) override;
		virtual float solve_layout_content_height(Size &parent_layout_size) override;
		virtual void onSourceState(Event<ImageSource, ImageSource::State>& evt) override;
		virtual ImagePool* imgPool() override;
	};

	class Qk_EXPORT Image: public Box {
	public:
		Qk_Define_View(Image, Box);
		Qk_DEFINE_PROP_ACC(String, src);
		Qk_DEFINE_PROP_ACC(ImageSource*, source, NoConst);
	};

}
#endif
