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

#include "qgr/utils/http.h"
#include "qgr/utils/http-cookie.h"
#include "qgr/utils/fs.h"
#include "qgr/version.h"
#include "qgr/utils/string-builder.h"
#include "qgr/utils/uv-1.h"

XX_NS(qgr)

static uint   http_initialized = 0;
static String http_cache_path = String();
static String http_user_agent = "Mozilla/5.0 qgr/" QGR_VERSION " (KHTML, like Gecko)";

String inl__get_http_user_agent() {
	return http_user_agent;
}

String inl__get_http_cache_path() {
	return http_cache_path;
}

typedef HttpHelper::RequestOptions RequestOptions;
typedef HttpHelper::ResponseData ResponseData;

static uint http_request(RequestOptions& options, cCb& cb, bool stream) throw(HttpError) {
	
	class Task: public AsyncIOTask, public HttpClientRequest::Delegate, public SimpleStream {
	 public:
		Callback      cb;
		bool          stream;
		bool          full_data;
		StringBuilder data;
		HttpClientRequest* client;
		
		Task() {
			client = new HttpClientRequest(loop());
			client->set_delegate(this);
			full_data = 1;
		}
		
		~Task() {
			Release(client);
		}
		
		virtual void trigger_http_error(HttpClientRequest* req, cError& error) {
			HttpError e(error.code(),
									error.message() + ", " + req->url().c(), req->status_code(), req->url());
			sync_callback(cb, &e);
			abort(); // abort and release
		}

		virtual void trigger_http_timeout(HttpClientRequest* req) {
			HttpError e(ERR_HTTP_REQUEST_TIMEOUT,
									String("http request timeout") + ", " + req->url().c(), 0, req->url());
			sync_callback(cb, &e);
			abort(); // abort and release
		}

		virtual void trigger_http_data(HttpClientRequest* req, Buffer buffer) {
			if ( stream ) {
				// TODO 现在还不支持暂停与恢复功能
				IOStreamData data(buffer, 0, id(),
													client->download_size(),
													client->download_total(), nullptr /*, this*/);
				sync_callback(cb, nullptr, &data);
			} else {
				if ( full_data ) {
					data.push(buffer.collapse_string());
				}
			}
		}
		
		virtual void trigger_http_end(HttpClientRequest* req) {
			/*
			 100-199 用于指定客户端应相应的某些动作。
			 200-299 用于表示请求成功。
			 300-399 用于已经移动的文件并且常被包含在定位头信息中指定新的地址信息。
			 400-499 用于指出客户端的错误。
			 500-599 用于支持服务器错误。
			 */
			if ( client->status_code() > 399 || client->status_code() < 100 ) {
				HttpError e(ERR_HTTP_STATUS_ERROR,
										String::format("Http status error, status code:%d, %s",
																	 req->status_code(), req->url().c()),
										req->status_code(), req->url());
				sync_callback(cb, &e);
			} else {
				if ( stream ) {
					// TODO 现在还不支持暂停与恢复功能
					IOStreamData data(Buffer(), 1, id(),
														client->download_size(),
														client->download_total(), nullptr /*, this*/);
					sync_callback(cb, nullptr, &data);
				} else {
					ResponseData rdata;
					rdata.data = data.to_buffer();
					rdata.http_version = client->http_response_version();
					rdata.status_code = client->status_code();
					rdata.response_headers = client->get_all_response_headers();
					sync_callback(cb, nullptr, &rdata);
				}
			}
			abort(); // abort and release
		}
		
		virtual void trigger_http_abort(HttpClientRequest* req) {
			XX_DEBUG("request async abort");
		}
		
		virtual void trigger_http_write(HttpClientRequest* req) {}
		virtual void trigger_http_header(HttpClientRequest* req) {}
		virtual void trigger_http_readystate_change(HttpClientRequest* req) {}
		
		virtual void abort() {
			Release(client); client = nullptr;
			AsyncIOTask::abort();
		}
		
		virtual void pause() {
			if ( client ) client->pause();
		}
		
		virtual void resume() {
			if ( client ) client->resume();
		}
		
	};
	
	Handle<Task> task(new Task());
	
	HttpClientRequest* req = task->client;
	
	try {
		req->set_url(options.url);
		req->set_method(options.method);
		req->set_timeout(options.timeout);
		req->disable_cache(options.disable_cache);
		req->disable_ssl_verify(options.disable_ssl_verify);
		req->disable_cookie(options.disable_cookie);
		task->cb = cb;
		task->stream = stream;
		
		if ( !options.upload.is_empty() ) { // 需要上传文件
			req->set_upload_file("file", options.upload);
		}

		if ( !options.save.is_empty() ) {
			task->full_data = 0;
			req->set_save_path(options.save);
		}
		
		for ( auto& i : options.headers ) {
			req->set_request_header(i.key(), i.value());
		}
		
		req->send(options.post_data);
	} catch (cError& e) {
		throw HttpError(e);
	}
	
	return task.collapse()->id();
}

/**
 * @func request
 */
uint HttpHelper::request(RequestOptions& options, cCb& cb) throw(HttpError) {
	return http_request(options, cb, false);
}

uint HttpHelper::request_stream(RequestOptions& options, cCb& cb) throw(HttpError) {
	return http_request(options, cb, true);
}

extern RunLoop* get_private_loop();
extern bool has_private_loop_thread();

/**
 * @func request_sync
 */
Buffer HttpHelper::request_sync(RequestOptions& options) throw(HttpError) {
	if (has_private_loop_thread()) {
		throw HttpError(ERR_CANNOT_RUN_SYNC_IO,
										String::format("cannot send sync http request, %s"
																	 , options.url.c()), 0, options.url);
	}

	XX_DEBUG("request_sync %s", options.url.c());
	
	class Client: public HttpClientRequest, public HttpClientRequest::Delegate {
	 public:
		Client(RunLoop* loop): HttpClientRequest(loop)
		, full_data(1), is_error(0), ok(0), m_loop(loop), error(0, "", 0, "") {
			set_delegate(this);
		}
		virtual void trigger_http_error(HttpClientRequest* req, cError& err) {
			XX_DEBUG("request_sync %s err", *url());
			HttpError e(err.code(),
									err.message() + ", " + req->url().c(), 0, req->url());
			end(&e);
		}
		virtual void trigger_http_timeout(HttpClientRequest* req) {
			HttpError e(ERR_HTTP_REQUEST_TIMEOUT,
									String("http request timeout") + ", " + req->url().c(), 0, req->url());
			end(&e);
		}
		virtual void trigger_http_data(HttpClientRequest* req, Buffer buffer) {
			XX_DEBUG("request_sync %s data ..", *url());
			if (full_data) {
				data.push(buffer.collapse_string());
			}
		}
		virtual void trigger_http_end(HttpClientRequest* req) {
			XX_DEBUG("request_sync %s end", *url());
			end(nullptr);
		}
		virtual void trigger_http_abort(HttpClientRequest* req) {
			XX_DEBUG("request_sync %s abort", *url());
		}
		virtual void trigger_http_write(HttpClientRequest* req) {}
		virtual void trigger_http_header(HttpClientRequest* req) {}
		virtual void trigger_http_readystate_change(HttpClientRequest* req) {}

		void end(HttpError* err) {
			ScopeLock scope(mutex);
			if (err) {
				is_error = 1;
				error = move(*err);
			}
			ok = 1;
			cond.notify_one();
		}
		
		void send_sync(Buffer buffer) throw(HttpError) {
			Lock lock(mutex);
			post_data = buffer;
			m_loop->post(Cb([this](Se&) {
				XX_DEBUG("request_sync %s send", *url());
				try {
					send(post_data);
				} catch (Error& err) {
					HttpError e(err.code(),
											err.message() + ", " + url().c(), 0, url());
					end(&e);
				}
			}));
			while (!ok) {
				cond.wait(lock); // wait done
			}
			if (is_error) {
				throw error;
			}
		}
		bool					full_data, is_error, ok;
		RunLoop*			m_loop;
		Buffer				post_data;
		HttpError 		error;
		StringBuilder data;
		Condition			cond;
		Mutex 				mutex;
	};
	
	auto loop = get_private_loop();
	
	Client* cli_ptr = new Client(loop);
	Client& cli = *cli_ptr;
	
	ScopeClear scope([cli_ptr, loop](){
		loop->post(Cb([cli_ptr](Se& e){
			cli_ptr->release();
		}));
	});
	
	try {
		cli.set_url(options.url);
		cli.set_method(options.method);
		cli.set_timeout(options.timeout);
		cli.disable_cache(options.disable_cache);
		cli.disable_ssl_verify(options.disable_ssl_verify);
		
		if ( !options.upload.is_empty() ) { // 需要上传文件
			cli.set_upload_file("file", options.upload);
		}
		if ( !options.save.is_empty() ) {
			cli.full_data = 0;
			cli.set_save_path(options.save);
		}
		for ( auto& i : options.headers ) {
			cli.set_request_header(i.key(), i.value());
		}
	} catch(cError& e) {
		throw HttpError(e);
	}

	XX_DEBUG("request_sync %s ..", options.url.c());
	
	cli.send_sync(options.post_data);

	XX_DEBUG("request_sync %s ok", options.url.c());
	
	if ( cli.is_error ) {
		HttpError e(cli.error.code(),
								cli.error.message() + ", " + cli.url().c(),
								cli.status_code(), cli.url());
		throw e;
	} else {
		if ( cli.status_code() > 399 || cli.status_code() < 100 ) {
			String msg = String::format("Http status error, status code:%d, %s",
																	cli.status_code(), cli.url().c());
			HttpError e(ERR_HTTP_STATUS_ERROR, msg, cli.status_code(), cli.url());
			throw e;
		}
	}
	return cli.data.to_buffer();
}

static RequestOptions default_request_options(cString& url) {
	return {
		url,
		HTTP_METHOD_GET,
		Map<String, String>(),
		Buffer(),
		String(),
		String(),
		0,
		false,
		false,
		false,
	};
}

/**
 * @func download
 */
uint HttpHelper::download(cString& url, cString& save, cCb& cb) throw(HttpError) {
	RequestOptions options = default_request_options(url);
	options.save = save;
	return http_request(options, cb, false);
}

/**
 * @func download_sync
 */
void HttpHelper::download_sync(cString& url, cString& save) throw(HttpError) {
	RequestOptions options = default_request_options(url);
	options.save = save;
	request_sync(options);
}

/**
 * @func upload
 */
uint HttpHelper::upload(cString& url, cString& file, cCb& cb) throw(HttpError) {
	RequestOptions options = default_request_options(url);
	options.upload = file;
	options.method = HTTP_METHOD_POST;
	options.disable_cache = true;
	return http_request(options, cb, false);
}

/**
 * @func upload
 */
Buffer HttpHelper::upload_sync(cString& url, cString& file) throw(HttpError) {
	RequestOptions options = default_request_options(url);
	options.upload = file;
	options.method = HTTP_METHOD_POST;
	options.disable_cache = true;
	return request_sync(options);
}

/**
 * @func get
 */
uint HttpHelper::get(cString& url, cCb& cb, bool no_cache) throw(HttpError) {
	RequestOptions options = default_request_options(url);
	options.disable_cache = no_cache;
	return http_request(options, cb, false);
}

/**
 * @func get_stream
 */
uint HttpHelper::get_stream(cString& url, cCb& cb, bool no_cache) throw(HttpError) {
	RequestOptions options = default_request_options(url);
	options.disable_cache = no_cache;
	return http_request(options, cb, true);
}

/**
 * @func post
 */
uint HttpHelper::post(cString& url, Buffer data, cCb& cb) throw(HttpError) {
	RequestOptions options = default_request_options(url);
	options.method = HTTP_METHOD_POST;
	options.post_data = data;
	return http_request(options, cb, false);
}

/**
 * @func get_sync
 */
Buffer HttpHelper::get_sync(cString& url, bool no_cache) throw(HttpError) {
	RequestOptions options = default_request_options(url);
	options.disable_cache = no_cache;
	return request_sync(options);
}

/**
 * @func post_sync
 */
Buffer HttpHelper::post_sync(cString& url, Buffer data) throw(HttpError) {
	RequestOptions options = default_request_options(url);
	options.method = HTTP_METHOD_POST;
	options.post_data = data;
	return request_sync(options);
}

/**
 * @func abort
 */
void HttpHelper::abort(uint id) {
	AsyncIOTask::safe_abort(id);
}

/**
 * @func initialize
 */
void HttpHelper::initialize() {
	if ( ! http_initialized++ ) {
		http_user_agent = String::format("Mozilla/5.0 (%s/%s) qgr/"
																		 QGR_VERSION " (KHTML, like Gecko)", *sys::name(), *sys::version());
		set_cache_path(Path::temp("http_cache"));
	}
}

/**
 * @func user_agent
 */
String HttpHelper::user_agent() {
	return http_user_agent;
}

/**
 * @func set_user_agent
 */
void HttpHelper::set_user_agent(cString& user_agent) {
	http_user_agent = user_agent;
}

/**
 * @func cache_path
 */
String HttpHelper::cache_path() {
	return http_cache_path;
}

/**
 * @func set_cache_path 设置缓存文件路径
 */
void HttpHelper::set_cache_path(cString& path) {
	if ( FileHelper::mkdir_p_sync(path) ) {
		http_cache_path = path;
	}
}

/**
 * @func clean_cache 清理web缓存
 */
void HttpHelper::clear_cache() {
	// delete cache files
	if ( ! http_cache_path.is_empty() ) {
		FileHelper::rm_r_sync(http_cache_path);
		set_cache_path(http_cache_path);
	}
}

void HttpHelper::clear_cookie() {
	http_cookie_clear();
}

XX_END
