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

#include "../../util/fs.h"
#include "../../util/hash.h"
#include "./pool.h"

namespace qk {

	// ---------------------- F o n t . P o o l --------------------------

	FontPool::FontPool(): _tf65533GlyphID(0) {}

	void FontPool::init() {
		FontStyle style; // default style

		// Find english character set
		auto tf = match(String(), style);
		_defaultFamilyNames.push(tf->getFamilyName());

		Qk_DEBUG("FontPool::init _defaultFamilyNames, %s", *_defaultFamilyNames[0]);

		// Find chinese character set, 楚(26970)
		auto tf1 = matchCharacter(String(), style, 26970);
		if (tf1) {
			_defaultFamilyNames.push(tf1->getFamilyName());
			tf1->getMetrics(&_MaxMetrics64, 64);
			Qk_DEBUG("FontPool::init _defaultFamilyNames, %s", *_defaultFamilyNames[1]);
		} else {
			tf->getMetrics(&_MaxMetrics64, 64);
		}
		// find �(65533) character tf
		_tf65533 = matchCharacter(String(), style, 65533);
		Qk_ASSERT(_tf65533);
		Qk_DEBUG("FontPool::init _tf65533, %s", *_tf65533->getFamilyName());

		_tf65533GlyphID = _tf65533->unicharToGlyph(65533);
		_defaultFontFamilys = getFontFamilys(Array<String>());
	}

	FFID FontPool::getFontFamilys(cString& familys) {
		return familys.isEmpty() ? _defaultFontFamilys: getFontFamilys(familys.split(","));
	}

	FFID FontPool::getFontFamilys(cArray<String>& familys) {
		Hash5381 hash;
		for (auto& i: familys) {
			hash.updatestr(i.trim());
		}
		auto it = _fontFamilys.find(hash.hashCode());
		if (it != _fontFamilys.end()) {
			return *it->value;
		}
		return *_fontFamilys.set(hash.hashCode(), new FontFamilys(this, familys));
	}

	void FontPool::addFontFamily(cBuffer& buff, cString& alias) {
		ScopeLock scope(_Mutex);
		for (int i = 0; ;i++) {
			auto tf = onAddFontFamily(buff, i);
			if (!tf)
				break;
			_ext.get(tf->getFamilyName()).set(tf->fontStyle(), tf);
			if (!alias.isEmpty()) {
				_ext.get(alias).set(tf->fontStyle(), tf);
			}
		}
	}

	cArray<String>& FontPool::defaultFamilyNames() const {
		return _defaultFamilyNames;
	}

	uint32_t FontPool::countFamilies() const {
		return onCountFamilies();
	}

	String FontPool::getFamilyName(int index) const {
		return onGetFamilyName(index);
	}

	Sp<Typeface> FontPool::match(cString& familyName, FontStyle style) {
		if (familyName.isEmpty()) {
			return onMatch(nullptr, style);
		}
		// find extend font family
		if (_ext.length()) {
			ScopeLock scope(_Mutex);
			auto it0 = _ext.find(familyName);
			if (it0 != _ext.end()) {
				auto it = it0->value.find(style);
				if (it != it0->value.end())
					return it->value.value();
				return it0->value.begin()->value.value();
			}
		}
		return onMatch(familyName.c_str(), style);
	}

	Sp<Typeface> FontPool::matchCharacter(cString& familyName, FontStyle style,
																		 Unichar character) const {
		cChar* c_familyName = familyName.isEmpty() ? nullptr: familyName.c_str();
		return onMatchCharacter(c_familyName, style, character);
	}

}
