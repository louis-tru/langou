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

#include "../app.h"
#include "./css.h"
#include "../window.h"
#include "../view/view.h"

namespace qk {
	#define _async_call _host->window()->preRender().async_call

	CStyleSheetsClass::CStyleSheetsClass(View *host)
		: _havePseudoType(false)
		, _firstApply(true)
		, _host(host)
		, _parent(nullptr)
		, _status(kNormal_CSSType)
		, _setStatus(kNormal_CSSType)
	{
		Qk_ASSERT(host);
	}

	void CStyleSheetsClass::set(cArray<String> &name) {
		_async_call([](auto ctx, auto val) {
			Sp<Array<String>> valp(val.arg);
			ctx->_nameHash_Rt.clear();
			for ( auto &j: **valp )
				ctx->_nameHash_Rt.add(CSSName(j).hashCode());
			ctx->updateClass_Rt();
		}, this, new Array<String>(name));
	}

	void CStyleSheetsClass::add(cString &name) {
		_async_call([](auto ctx, auto hash) {
			if (!ctx->_nameHash_Rt.has(hash.arg)) {
				ctx->_nameHash_Rt.add(hash.arg);
				ctx->updateClass_Rt();
			}
		}, this, CSSName(name).hashCode());
	}

	void CStyleSheetsClass::remove(cString &name) {
		_async_call([](auto ctx, auto hash) {
			auto it = ctx->_nameHash_Rt.find(hash.arg);
			if (it != ctx->_nameHash_Rt.end()) {
				ctx->_nameHash_Rt.erase(it);
				ctx->updateClass_Rt();
			}
		}, this, CSSName(name).hashCode());
	}

	void CStyleSheetsClass::toggle(cString &name) {
		_async_call([](auto ctx, auto hash) {
			auto it = ctx->_nameHash_Rt.find(hash.arg);
			if (it == ctx->_nameHash_Rt.end()) {
				ctx->_nameHash_Rt.add(hash.arg);
			} else {
				ctx->_nameHash_Rt.erase(it);
			}
			ctx->updateClass_Rt();
		}, this, CSSName(name).hashCode());
	}

	void CStyleSheetsClass::updateClass_Rt() {
		_host->mark_layout(View::kStyle_Class, true);
		_status = kNone_CSSType; // force apply update
	}

	void CStyleSheetsClass::setStatus_Rt(CSSType status) {
		if ( _setStatus != status ) {
			_setStatus = status;
			if ( _havePseudoType ) {
				_host->mark_layout(View::kStyle_Class, true);
			}
		}
	}

	bool CStyleSheetsClass::apply_Rt(CStyleSheetsClass *parent) {
		if (_setStatus == _status) {
			return false;
		}
		auto hash = _stylesHash_Rt.hashCode();
		// reset env
		_status = _setStatus;
		_styles_Rt.clear();
		_stylesHash_Rt = Hash5381();
		_havePseudoType = false;
		_parent = parent;

		if (_nameHash_Rt.length()) {
			applyFrom_Rt(parent);
		}
		_firstApply = false;

		// affects children CStyleSheetsClass
		return _stylesHash_Rt.hashCode() != hash;
	}

	void CStyleSheetsClass::applyFrom_Rt(CStyleSheetsClass *ssc) {
		if (ssc) {
			Qk_ASSERT(ssc->_styles_Rt.length());
			applyFrom_Rt(ssc->_parent);
			for (auto ss: ssc->_styles_Rt) {
				applyFindSubstyle_Rt(ss);
			}
		} else {
			applyFindSubstyle_Rt(shared_app()->styleSheets()); // apply global style
		}
	}

	void CStyleSheetsClass::applyFindSubstyle_Rt(CStyleSheets *ss) {
		for (auto &n: _nameHash_Rt) {
			qk::CStyleSheets *sss;
			if (ss->_substyles.get(n.key, sss)) {
				applyStyle_Rt(sss);
			}
		}
	}

	// CSS Sample
	/**
	.a {
		width: 100;
		.a_a {
			height: 110;
			&:hover {
				height: 170;
			}
		}
		.a_b {
			color: #f00;
		}
		&:hover {
			width: 150;
			.a_a {
				height: 160;
				// ------------------ error ------------------
				&:hover {
					height: 171;
				}
				// ------------------
			}
			.a_b {
				color: #ff0;
			}
		}
	}
	<box ssclass="a">
		<box ssclass="a_a">test a<box>
		<box ssclass="a_a a_b">test b<box>
	</bod>
	*/

	void CStyleSheetsClass::applyStyle_Rt(CStyleSheets *ss) {
		ss->apply(_host);

		if (ss->_substyles.length()) {
			_stylesHash_Rt.updateu64(uint64_t(ss));
			_styles_Rt.push(ss);
		}

		// apply pseudo class
		if (ss->_havePseudoType) {
			_havePseudoType = true;

			CStyleSheets *ss_pt = nullptr;
			switch (_status) {
				case kNormal_CSSType: ss_pt = ss->_normal; break;
				case kHover_CSSType: ss_pt = ss->_hover; break;
				case kActive_CSSType: ss_pt = ss->_active; break;
			}
			if (ss_pt) applyStyle_Rt(ss_pt);
		}

		if (ss->_extends.length()) { // apply extend
			for (auto &i: ss->_extends) { // test right extend
				if (_nameHash_Rt.has(i.key)) { // test ok
					applyStyle_Rt(i.value);
				}
			}
		}
	}

}
