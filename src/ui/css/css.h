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

#ifndef __quark__css__css__
#define __quark__css__css__

#include "../../util/util.h"
#include "../../util/dict.h"
#include "../filter.h"
#include "../view_prop.h"

namespace qk {
	class Layout;

	enum CSSType {
		kNone_CSSType = 0,
		kNormal_CSSType, // css pseudo type
		kHover_CSSType,
		kActive_CSSType,
	};

	class Qk_EXPORT CSSName {
	public:
		Qk_DEFINE_PROP_GET(uint64_t, hashCode);
		Qk_DEFINE_PROP_GET(String, name);
		CSSName(cString& name);
	};

	typedef const CSSName cCSSName;

	/**
	 * 
	 * Cascading style sheets
	 * 
	 * @class StyleSheets
	*/
	class Qk_EXPORT StyleSheets: public Object {
		Qk_HIDDEN_ALL_COPY(StyleSheets);
	public:
		class Property {
		public:
			virtual ~Property() = default;
			virtual void apply(Layout *layout) = 0;
		};

		// define props
		#define _Fun(Enum, Type, Name, From) void set_##Name(Type value);
			Qk_View_Props(_Fun)
		#undef _Fun

		Qk_DEFINE_PROP(uint64_t, time);
		Qk_DEFINE_PROP_GET(CSSName, name);
		Qk_DEFINE_PROP_GET(StyleSheets*, parent, NoConst);
		Qk_DEFINE_PROP_GET(StyleSheets*, normal, NoConst); // style sheets for pseudo type
		Qk_DEFINE_PROP_GET(StyleSheets*, hover,  NoConst);
		Qk_DEFINE_PROP_GET(StyleSheets*, active, NoConst);
		Qk_DEFINE_PROP_GET(CSSType, type);
		Qk_DEFINE_PROP_GET(bool, havePseudoType); // normal | hover | active

		/**
		 * @constructor
		*/
		StyleSheets(cCSSName &name, StyleSheets *parent, CSSType type);

		/**
		 * @destructor
		*/
		virtual ~StyleSheets();

		/**
		* @method find children style sheets
		*/
		const StyleSheets* find(cCSSName &name) const;

		/**
		* @method apply style to layout
		*/
		void apply(Layout *layout) const;

	private:
		typedef Dict<CSSName, StyleSheets*> StyleSheetsDict;

		StyleSheets* findAndMake(cCSSName &name, CSSType type, bool isExtend);
		void setProp(uint32_t, Property* prop);

		StyleSheetsDict _substyles; // css name => .self .sub { width: 100px }
		StyleSheetsDict _extends; // css name => .self.extend { width: 100px }
		Dict<uint32_t, Property*> _props; // ViewProperty => Property*

		friend class RootStyleSheets;
		friend class StyleSheetsClass;
	};

	typedef const StyleSheets cStyleSheets;

	class Qk_EXPORT RootStyleSheets: public StyleSheets {
	public:
		RootStyleSheets();

		/**
		*  ".div_cls.div_cls2 .aa.bb.cc, .div_cls.div_cls2:down .aa.bb.cc"
		*
		* @method search()
		*/
		Array<StyleSheets*> search(cString &exp);
	};

	class Qk_EXPORT StyleSheetsClass {
		Qk_HIDDEN_ALL_COPY(StyleSheetsClass);
	public:
		Qk_DEFINE_PROP_GET(bool, havePseudoType); // 当前样式表组支持伪类型
		Qk_DEFINE_PROP_GET(bool, firstApply); // 是否为首次应用样式表,在处理动作时如果为第一次忽略动作
		Qk_DEFINE_PROP_GET(CSSType, status); // 当前伪类应用状态
		Qk_DEFINE_PROP_GET(Layout*, host, NoConst);
		Qk_DEFINE_PROP_GET(StyleSheetsClass*, parent, NoConst); // apply parent ssc

		StyleSheetsClass(Layout *host);
		inline cSet<CSSName>& name() const { return _name; }
		inline bool haveSubstyles() const { return _haveSubstyles.length(); }
		void set(cArray<String> &name);
		void add(cString &name);
		void remove(cString &name);
		void toggle(cString &name);

	private:
		void updateClass();
		void setStatus(CSSType status);
		void apply(StyleSheetsClass *parent);
		void applyFrom(StyleSheetsClass *ss);
		void applySubstyle(StyleSheets *ss);
		void applyExtend(StyleSheets *ss_left);
		void applyStyle(StyleSheets *ss);

		Set<CSSName> _name; // class name
		Array<StyleSheets*> _haveSubstyles; // 当前应用的样式表且拥有子样式表供后代视图查询
		
		friend class Layout;
	};

}
#endif
