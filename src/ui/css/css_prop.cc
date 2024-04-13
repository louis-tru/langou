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

#include "./css.h"
#include "../view/view.h"
#include "../window.h"

namespace qk {

	typedef StyleSheets::Property Property;

	template<typename T>
	T* copy_value_ptr(T* value) {
		Qk_UNIMPLEMENTED();
		return nullptr;
	}

	template<typename T>
	inline void transition_value_ptr(T *v1, T *v2, float t, ViewProp prop, View *target) {
		Qk_UNIMPLEMENTED();
	}

	template<>
	BoxFilter* copy_value_ptr(BoxFilter* value) {
		return value->copy_Rt(nullptr);
	}

	template<>
	void transition_value_ptr(BoxFilter *v1, BoxFilter *v2, float t, ViewProp prop, View *target) {
		auto acc = target->accessor() + prop;
		if (acc->set) {
			auto v = (target->*(BoxFilter* (View::*)())acc->get)();
			auto v_new = v1->transition_Rt(v, v2, t);
			(target->*(void (View::*)(BoxFilter*))acc->set)(v_new);
		}
	}

	template<typename T>
	inline T transition_value(T v1, T v2, float t) {
		auto v = v1 - (v1 - v2) * t;
		return v1;
	}

	template<>
	BoxSize transition_value(BoxSize v1, BoxSize v2, float t) {
		if ( v1.kind == v2.kind ) {
			return { v1.value - (v1.value - v2.value) * t, v1.kind };
		} else {
			return t < 1.0 ? v1 : v2;
		}
	}

	template<>
	Color transition_value(Color v1, Color v2, float t) {
		return Color(v1.r() - (v1.r() - v2.r()) * t,
								v1.g() - (v1.g() - v2.g()) * t,
								v1.b() - (v1.b() - v2.b()) * t, v1.a() - (v1.a() - v2.a()) * t);
	}

	template<>
	TextSize transition_value(TextSize v1, TextSize v2, float t) {
		if ( v1.kind == TextValueKind::kValue && v2.kind == TextValueKind::kValue ) {
			return { v1.value - (v1.value - v2.value) * t, TextValueKind::kValue };
		} else {
			return  t < 1.0 ? v1 : v2;
		}
	}

	template<>
	TextColor transition_value(TextColor v1, TextColor v2, float t) {
		if ( v1.kind == TextValueKind::kValue && v2.kind == TextValueKind::kValue ) {
			return { transition_value(v1.value, v2.value, t), TextValueKind::kValue };
		} else {
			return  t < 1.0 ? v1 : v2;
		}
	}

	template<>
	Shadow transition_value(Shadow v1, Shadow v2, float t) {
		return {
			v1.offset_x - (v1.offset_x - v2.offset_x) * t,
			v1.offset_y - (v1.offset_y - v2.offset_y) * t,
			v1.size - (v1.size - v2.size) * t,
			transition_value(v1.color, v2.color, t),
		};
	}

	template<>
	TextShadow transition_value(TextShadow v1, TextShadow v2, float t) {
		if ( v1.kind == TextValueKind::kValue && v2.kind == TextValueKind::kValue ) {
			return { transition_value(v1.value, v2.value, t), TextValueKind::kValue };
		} else {
			return t < 1.0 ? v1 : v2;
		}
	}

	#define _Define_Enum_transition(Type) template<>\
		Type transition_value(Type f1, Type f2, float t) {\
			return t < 1.0 ? f1: f2;\
		}
	_Define_Enum_transition(int)
	_Define_Enum_transition(Align)
	_Define_Enum_transition(Direction)
	_Define_Enum_transition(ItemsAlign)
	_Define_Enum_transition(CrossAlign)
	_Define_Enum_transition(Wrap)
	_Define_Enum_transition(WrapAlign)
	_Define_Enum_transition(String)
	_Define_Enum_transition(TextAlign)
	_Define_Enum_transition(TextWeight)
	_Define_Enum_transition(TextSlant)
	_Define_Enum_transition(TextDecoration)
	_Define_Enum_transition(TextOverflow)
	_Define_Enum_transition(TextWhiteSpace)
	_Define_Enum_transition(TextWordBreak)
	_Define_Enum_transition(TextFamily)
	_Define_Enum_transition(KeyboardType)
	_Define_Enum_transition(KeyboardReturnType)
	#undef _Define_Enum_transition

	// ----------------------------------------------------------------------------------------------

	template<typename T>
	struct PropImpl: Property {
		inline PropImpl(ViewProp prop, T value): _prop(prop), _value(value) {}
		void apply(View *target) override {
			auto set = (void (View::*)(T))(target->accessor() + _prop)->set;
			if (set)
				(target->*set)(_value);
		}
		void transition(View *target, Property *to, float y) override {
			Qk_ASSERT(static_cast<PropImpl*>(to)->_prop == _prop);
			auto set = (void (View::*)(T))(target->accessor() + _prop)->set;
			if (set)
				(target->*set)(transition_value(_value, static_cast<PropImpl*>(to)->_value, y));
		}
		Property* copy() override {
			return new PropImpl<T>(_prop, _value);
		}
		ViewProp _prop;
		T _value;
	};

	// @template Object or BoxFilter
	template<typename T>
	struct PropImpl<T*>: Property {
		PropImpl(ViewProp prop, T* value): _prop(prop), _value(value) {
			Qk_ASSERT(_value);
			static_assert(T::Traits::isObject, "Property value must be a object type");
			// value->retain(); // @line SetProp<Object*>::asyncExec, value->retain();
		}
		~PropImpl() {
			_value->release();
		}
		void apply(View *target) override {
			auto set = (void (View::*)(T*))(target->accessor() + _prop)->set;
			if (set)
				(target->*set)(_value);
		}
		void transition(View *target, Property *to, float t) override {
			Qk_ASSERT(static_cast<PropImpl*>(to)->_prop == _prop);
			transition_value_ptr(
				static_cast<BoxFilter*>(_value),
				static_cast<BoxFilter*>(static_cast<PropImpl*>(to)->_value),
				t, _prop, target
			);
		}
		Property* copy() override {
			return new PropImpl(_prop, copy_value_ptr(static_cast<BoxFilter*>(_value)));
		}
		ViewProp _prop;
		T* _value;
	};

	// ---- SetProp ----

	template<typename T>
	struct SetProp: StyleSheets {
		void set(ViewProp key, T value) {
			Property *prop;
			if (_props.get(key, prop)) {
				static_cast<PropImpl<T>*>(prop)->_value = value;
			} else {
				onMake(key, _props.set(key, new PropImpl<T>(key, value)));
			}
		}
		template<ViewProp key>
		void asyncSet(T value) {
			auto win = getWindowForAsyncSet();
			if (win) {
				win->preRender().async_call([](auto self, auto arg) {
					self->set(key, arg.arg);
				}, this, value);
			} else {
				set(key, value);
			}
		}
		template<ViewProp key>
		void asyncSetLarge(T &value) {
			auto win = getWindowForAsyncSet();
			if (win) {
				win->preRender().async_call([](auto self, auto arg) {
					Sp<T> h(arg.arg);
					self->set(key, *arg.arg);
				}, this, new T(value));
			} else {
				set(key, value);
			}
		}
	};

	template<>
	template<ViewProp key>
	void SetProp<String>::asyncSet(String value) {
		asyncSetLarge<key>(value);
	}

	template<>
	template<ViewProp key>
	void SetProp<TextShadow>::asyncSet(TextShadow value) {
		asyncSetLarge<key>(value);
	}

	template<>
	template<ViewProp key>
	void SetProp<TextFamily>::asyncSet(TextFamily value) {
		asyncSetLarge<key>(value);
	}

	template<>
	struct SetProp<BoxFilter*>: StyleSheets {
		void set(ViewProp key, BoxFilter* value) {
			Property *prop;
			value = BoxFilter::assign_Rt(nullptr, value, nullptr);
			if (_props.get(key, prop)) {
				auto p = static_cast<PropImpl<BoxFilter*>*>(prop);
				if (p->_value != value) {
					p->_value->release();
					p->_value = value;
				} else {
					value->release();
				}
			} else {
				onMake(key, _props.set(key, new PropImpl<BoxFilter*>(key, value)));
			}
		}
		template<ViewProp key>
		void asyncSet(BoxFilter* value) {
			// value->retain();
			auto win = getWindowForAsyncSet();
			if (win) {
				win->preRender().async_call([](auto self, auto arg) {
					self->set(key, arg.arg);
				}, this, value);
			} else {
				set(key, value);
			}
		}
	};

	template<typename T>
	struct SetProp<T*>: StyleSheets {
		template<ViewProp key>
		inline void asyncSet(T* value) {
			static_cast<SetProp<BoxFilter*>*>(static_cast<StyleSheets*>(this))->asyncSet<key>(value);
		}
	};

	#define _Fun(Enum, Type, Name, _) void StyleSheets::set_##Name(Type value) {\
		static_cast<SetProp<Type>*>(this)->asyncSet<k##Enum##_ViewProp>(value);\
	}
	Qk_View_Props(_Fun)

	#undef _Fun

}
