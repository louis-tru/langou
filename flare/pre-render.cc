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

#include "./pre-render.h"
#include "./layout/layout.h"
#include "./app.h"

namespace flare {

	F_DEFINE_INLINE_MEMBERS(PreRender, Inl) {
		public:

		/**
		* @func add_task
		*/
		void add_task(Task* task) {
			if ( task->get_task_id() == Task::ID() ) {
				Task::ID id = _tasks.push_back(task);
				task->set_task_id( id );
			}
		}
		
		/**
		* @func del_task
		*/
		void del_task(Task* task) {
			Task::ID id = task->get_task_id();
			if ( id != Task::ID() ) {
				(*id)->set_task_id( Task::ID() );
				(*id) = nullptr;
			}
		}

		void solve_mark() {
			do {
				{ // forward iteration
					for (auto levelMarks: _marks) {
						for (auto layout: levelMarks) {
							if (layout) {
								if ( !layout->layout_forward(layout->layout_mark()) ) {
									// simple delete mark
									layout->_mark_index = -1;
									layout = nullptr;
									_mark_total--;
								}
							}
						}
					}
				}
				if (_mark_total > 0) { // reverse iteration
					for (int i = _marks.length() - 1; i >= 0; i--) {
						auto levelMarks = _marks[i];
						for (auto layout: levelMarks) {
							if (layout) {
								if ( !layout->layout_reverse(layout->layout_mark()) ) {
									// simple delete mark recursive
									layout->_mark_index = -1;
									layout = nullptr;
									_mark_total--;
								}
							}
						}
					}
				}
				F_ASSERT(_mark_total >= 0);
			} while (_mark_total);

			for (auto levelMarks: _marks) {
				levelMarks.clear();
			}
			_is_render = true;
		}

		void solve_mark_recursive() {
			for (auto levelMarks: _mark_recursives) {
				for (auto layout: levelMarks) {
					layout->layout_recursive(layout->layout_mark());
					layout->_recursive_mark_index = -1;
				}
				levelMarks.clear(); // clear level marks
			}
			_mark_recursive_total = 0;
			_is_render = true;
		}
		
	};

	/**
	* @constructor
	*/
	PreRender::PreRender()
		: _is_render(false)
		, _mark_total(0)
		, _mark_recursive_total(0)
		, _marks(8)
		, _mark_recursives(8)
	{
	}

	/**
	* @destructor
	*/
	PreRender::~PreRender() {
	}

	/**
	* 解决标记需要更新的视图
	*/
	bool PreRender::solve(int64_t now_time) {

		if ( _tasks.length() ) { // solve task
			auto i = _tasks.begin(), end = _tasks.end();
			while ( i != end ) {
				Task* task = *i;
				if ( task ) {
					if ( now_time > task->get_task_timeout() ) {
						if ( task->run_task(now_time) ) {
							_is_render = true;
						}
					}
					i++;
				} else {
					_tasks.erase(i++);
				}
			}
		}

		if (_mark_total) { // solve marks
			Inl_PreRender(this)->solve_mark();
		}

		if (_mark_recursive_total) { // solve mark recursive
			Inl_PreRender(this)->solve_mark_recursive();
		}

		if (_is_render) {
			_is_render = false;
			return true;
		} else {
			return false;
		}
	}

	void PreRender::mark(Layout *layout, uint32_t depth) {
		F_ASSERT(depth);
		_marks.extend(depth + 1);
		auto arr = _marks[depth];
		layout->_mark_index = arr.length();
		arr.push(layout);
		_mark_total++;
	}

	void PreRender::mark_recursive(Layout *layout, uint32_t depth) {
		F_ASSERT(depth);
		_mark_recursives.extend(depth + 1);
		auto arr = _mark_recursives[depth];
		layout->_recursive_mark_index = arr.length();
		arr.push(layout);
		_mark_recursive_total++;
	}

	void PreRender::delete_mark(Layout *layout, uint32_t depth) {
		F_ASSERT(depth);
		auto arr = _marks[depth];
		auto last = arr[arr.length() - 1];
		if (last != layout) {
			arr[layout->_mark_index] = last;
		}
		arr.pop();
		layout->_mark_index = -1;
		_mark_total--;
		_is_render = true;
	}

	void PreRender::delete_mark_recursive(Layout *layout, uint32_t depth) {
		F_ASSERT(depth);
		auto arr = _mark_recursives[depth];
		auto last = arr[arr.length() - 1];
		if (last != layout) {
			arr[layout->_recursive_mark_index] = last;
		}
		arr.pop();
		layout->_recursive_mark_index = -1;
		_mark_recursive_total--;
		_is_render = true;
	}

	PreRender::Task::~Task() {
		unregister_task();
	}

	void PreRender::Task::register_task() {
		if ( app() ) {
			Inl_PreRender(app()->pre_render())->add_task(this);
		}
	}

	void PreRender::Task::unregister_task() {
		if ( app() ) {
			Inl_PreRender(app()->pre_render())->del_task(this);
		}
	}

}
