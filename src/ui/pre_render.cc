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

#include "./pre_render.h"
#include "./view/view.h"
#include "./text/text_opts.h"
#include "./window.h"
#include "./app.h"
#include "./action/action.h"

namespace qk {

	void PreRender::LevelMarks::clear() {
		_length = 0;
	}

	PreRender::PreRender(Window *win)
		: _mark_total(0)
		, _window(win)
		, _is_render(false), _is_render_Mt(false)
	{}

	void PreRender::mark_layout(View *view, uint32_t level) {
		Qk_ASSERT(level);
		_marks.extend(level + 1);
		auto& arr = _marks[level];
		view->_mark_index = arr.length();
		arr.push(view);
		_mark_total++;
	}

	void PreRender::unmark_layout(View *view, uint32_t level) {
		Qk_ASSERT(level);
		auto& arr = _marks[level];
		auto last = arr[arr.length() - 1];
		if (last != view) {
			arr[view->_mark_index] = last;
		}
		arr.pop();
		view->_mark_index = -1;
		_mark_total--;
		_is_render = true;
	}

	void PreRender::mark_render() {
		_is_render = true;
	}

	void PreRender::mark_render_Mt() {
		_is_render_Mt = true;
	}

	void PreRender::post(Cb cb) {
		_window->host()->loop()->post(cb);
	}

	void PreRender::addtask(Task* task) {
		if ( task->task_id() == Task::ID() ) {
			Task::ID id = _tasks.pushBack(task);
			task->_task_id = id;
			task->_pre = this;
		}
	}

	void PreRender::untask(Task* task) {
		Task::ID id = task->task_id();
		if ( id != Task::ID() ) {
			(*id)->_pre = nullptr;
			(*id)->_task_id = Task::ID();
			(*id) = nullptr;
		}
	}

	void PreRender::solveMarks() {
		// First forward iteration
		for (auto &levelMarks: _marks) {
			for (auto &view: levelMarks) {
				if (view) {
					if (view->_mark_value & View::kStyle_Class) {
						view->applyClass_Rt(view->parentSsclass_Rt());
					}
					if ( view->layout_forward(view->_mark_value) ) {
						// simple delete mark
						view->_mark_index = -1;
						view = nullptr;
						_mark_total--;
					}
				}
			}
		}

		while (_mark_total) {
			Qk_ASSERT(_mark_total > 0);

			// reverse iteration
			for (int i = _marks.length() - 1; i >= 0; i--) {
				auto &levelMarks = _marks[i];
				for (auto &view: levelMarks) {
					if (view) {
						if ( view->layout_reverse(view->_mark_value) ) {
							// simple delete mark recursive
							view->_mark_index = -1;
							view = nullptr;
							_mark_total--;
						}
					}
				}
			}
			if (!_mark_total) break;

			// forward iteration
			for (auto &levelMarks: _marks) {
				for (auto &view: levelMarks) {
					if (view) {
						if ( view->layout_forward(view->_mark_value) ) {
							// simple delete mark
							view->_mark_index = -1;
							view = nullptr;
							_mark_total--;
						}
					}
				}
			}
		}

		for (auto &levelMarks: _marks) {
			levelMarks.clear();
		}
		_is_render = true;
	}

	void PreRender::clearTasks() {
		for (auto t: _tasks) {
			if (t)
				t->_pre = nullptr; // clear task
		}
		_tasks.clear();
	}

	void PreRender::asyncCommit() {
		if (_is_render_Mt) {
			_is_render_Mt = false;
			async_call([](auto self, auto arg) { self->_is_render = true; }, this, 0);
		}
		if (_asyncCall.length()) {
			_asyncCommitMutex.lock();
			_asyncCommit.concat(std::move(_asyncCall));
			_asyncCommitMutex.unlock();
		}
	}

	void PreRender::solveAsyncCall() {
		if (_asyncCommit.length()) {
			_asyncCommitMutex.lock();
			auto calls(std::move(_asyncCommit));
			_asyncCommitMutex.unlock();
			for (auto &i: calls) {
				((void (*)(void*,AsyncCall::Arg))i.exec)(i.ctx, i.arg); // exec async call
			}
		}
	}

	void PreRender::flushAsyncCall() {
		asyncCommit();
		solveAsyncCall();
	}

	bool PreRender::solve(int64_t time) {
		solveAsyncCall();

		_window->actionCenter()->advance_Rt(uint32_t(uint64_t(time) / 1000)); // advance action

		if ( _tasks.length() ) { // solve task
			auto i = _tasks.begin(), end = _tasks.end();
			while ( i != end ) {
				Task* task = *i;
				if ( task ) {
					if ( time > task->task_timeout() ) {
						if ( task->run_task(time) ) {
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
			solveMarks();
		}

		return _is_render ? (_is_render = false, true): false;
	}

	RenderTask::~RenderTask() {
		if (_pre) {
			_pre->untask(this);
		}
	}

	void RenderTask::set_task_timeout(int64_t timeout_us) {
		_task_timeout = timeout_us;
	}

}
