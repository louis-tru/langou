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

#include "./fs.h"
#include "./zlib.h"
#include "./handle.h"
#include "./error.h"
#include "./http.h"
#include "./uv.h"
#include "./dict.h"

namespace flare {

	String inl_format_part_path(cString& path);

	static const String SEPARATOR("@/", 2);

	typedef HttpHelper::ResponseData ResponseData;

	class FileReader::Core {
	 public:

		enum Protocol {
			FILE = 0,
			ZIP,
			HTTP,
			HTTPS,
			FTP,
			FTPS,
			Unknown,
		};

		~Core() {
			ScopeLock lock(zip_mutex_);
			for (auto i: zips_ ) {
				Release(i.value);
			}
		}

		Protocol protocol(cString& path) {
			if ( Path::is_local_file( path ) ) {
				return FILE;
			}
			if ( Path::is_local_zip( path ) ) {
				return ZIP;
			}
			if ((path[0] == 'h' || path[0] == 'H') &&
					(path[1] == 't' || path[1] == 'T') &&
					(path[2] == 't' || path[2] == 'T') &&
					(path[3] == 'p' || path[3] == 'P')) {
				if (path[4] == ':' &&
						path[5] == '/' &&
						path[6] == '/') {
					return HTTP;
				}
				if ((path[4] == 's' || path[4] == 'S') &&
						path[5] == ':' &&
						path[6] == '/' &&
						path[7] == '/') {
					return HTTPS;
				}
			}
			if ((path[0] == 'f' || path[0] == 'F') &&
					(path[1] == 't' || path[1] == 'T') &&
					(path[2] == 'p' || path[2] == 'P')) {
				if (path[3] == ':' &&
						path[4] == '/' &&
						path[5] == '/') {
					return FTP;
				}
				if ((path[3] == 's' || path[3] == 'S') &&
						path[4] == ':' &&
						path[5] == '/' &&
						path[6] == '/') {
					return FTPS;
				}
			}
			return Unknown;
		}

		String zip_path(cString& path) {
			if (path.is_empty())
				return String();
			int i = path.index_of(SEPARATOR);
			if (i != -1)
				return path.substr(0, i);
			if (path[path.length() - 1] == SEPARATOR[0])
				return path.substr(0, path.length() - 1);
			return String();
		}

		ZipReader* get_zip_reader(cString& path) throw(Error) {
			ZipReader* reader = zips_[path];
			if (reader) {
				return reader;
			}
			reader = new ZipReader(path);
			if ( !reader->open() ) {
				Release(reader);
				F_THROW(ERR_FILE_NOT_EXISTS, "Cannot open zip file, `%s`", *path);
			}
			zips_[path] = reader;
			return reader;
		}

		void read_from_zip(RunLoop* loop, cString& zip, cString& path, bool stream, Cb cb) {
			Buffer buffer;
			ScopeLock lock(zip_mutex_);
			try {
				ZipReader* read = get_zip_reader(zip);
				String inl_path = inl_format_part_path(path.substr(zip.length() + SEPARATOR.length()));
				if ( read->jump(inl_path) ) {
					buffer = read->read();
				} else {
					Error err(ERR_FILE_NO_EXIST_IN_ZIP_PKG, "Zip package internal file does not exist, %s", *path);
					async_reject(cb, std::move(err), loop); return;
				}
			} catch (cError& err) {
				Error e(err);
				async_reject(cb, Error(err), loop); return;
			}
			
			if ( stream ) {
				uint32_t len = buffer.length();
				async_resolve<Object>(cb, StreamResponse(buffer, 1, 0, len, len, nullptr), static_cast<PostMessage*>(loop));
			} else {
				async_resolve<Object>(cb, std::move(buffer), static_cast<PostMessage*>(loop));
			}
		}

		uint32_t read(cString& path, Cb cb, bool stream) {
			
			Protocol p = protocol(path);
			uint32_t id = 0;

			switch (p) {
				default:
				case FILE:
					if ( stream ) {
						id = FileHelper::read_stream(path, *(Callback<StreamResponse>*)(&cb));
					} else {
						FileHelper::read_file(path, cb);
					}
					break;
				case ZIP: {
					String zip = zip_path(path);
					if ( zip.is_empty() ) {
						async_reject(cb, Error(ERR_INVALID_FILE_PATH, "Invalid file path, \"%s\"", *path), RunLoop::current());
					} else {
						RunLoop* loop = RunLoop::current();
						loop->work(Cb([this, loop, zip, path, stream, cb](CbData& evt) {
							read_from_zip(loop, zip, path, stream, cb);
						}));
					}
					break;
				}
				case FTP:
				case FTPS:
					async_reject(cb, Error(ERR_NOT_SUPPORTED_FILE_PROTOCOL, "This file protocol is not supported"), RunLoop::current());
					break;
				case HTTP:
				case HTTPS:
					try {
						if ( stream ) {
							id = HttpHelper::get_stream(path, *(Callback<StreamResponse>*)(&cb));
						} else {
							id = HttpHelper::get(path, HttpHelper::Cb([cb](HttpHelper::Cb::Data& e) {
								ResponseData* data = static_cast<ResponseData*>(e.data);
								if (e.error) {
									cb->reject(e.error);
								} else {
									cb->resolve(&data->data);
								}
							}));
						}
					} catch(Error& err) {
						async_reject(cb, Error(err), RunLoop::current());
					}
					break;
			}
			return id;
		}

		Buffer read_sync(cString& path) throw(Error) {
			Buffer rv;

			switch ( protocol(path) ) {
				default:
				case FILE:
					F_CHECK(FileHelper::exists_sync(path),
										ERR_FILE_NOT_EXISTS, "Unable to read file contents, \"%s\"", *path);
					rv = FileHelper::read_file_sync(path);
					break;
				case ZIP: {
					String zip = zip_path(path);
					F_CHECK(!zip.is_empty(), ERR_FILE_NOT_EXISTS, "Invalid file path, \"%s\"", *path);
					
					ScopeLock lock(zip_mutex_);
					
					ZipReader* read = get_zip_reader(zip);
					String inl_path = inl_format_part_path( path.substr(zip.length() + SEPARATOR.length()) );
					
					if ( read->jump(inl_path) ) {
						rv = read->read();
					} else {
						F_THROW(ERR_ZIP_IN_FILE_NOT_EXISTS,
							"Zip package internal file does not exist, %s", *path);
					}
					break;
				}
				case FTP:
				case FTPS:
					F_THROW(ERR_NOT_SUPPORTED_FILE_PROTOCOL, "This file protocol is not supported");
					break;
				case HTTP:
				case HTTPS: rv = HttpHelper::get_sync(path); break;
			}
			return rv;
		}

		void abort(uint32_t id) {
			AsyncIOTask::safe_abort(id);
		}

		bool exists_sync(cString& path, bool file, bool dir) {
			switch ( protocol(path) ) {
				default:
				case FILE:
					if ( file && FileHelper::is_file_sync(path) )
						return true;
					if ( dir  && FileHelper::is_directory_sync(path) )
						return true;
					return false;
				case ZIP: {
					String zip = zip_path(path);
					if ( !zip.is_empty() ) {
						F_IGNORE_ERR({
							ScopeLock lock(zip_mutex_);
							ZipReader* read = get_zip_reader(zip);
							String inl_path = inl_format_part_path( path.substr(zip.length() + SEPARATOR.length()) );
							if ( file && read->is_file( inl_path ) )
								return true;
							if ( dir && read->is_directory( inl_path ) )
								return true;
						});
					}
					return false;
				}
			}
			return false;
		}

		Array<Dirent> readdir_sync(cString& path) throw(Error) {
			Array<Dirent> rv;
			switch ( protocol(path) ) {
				default:
				case FILE:
					rv = FileHelper::readdir_sync(path);
				case ZIP: {
					String zip = zip_path(path);
					if ( !zip.is_empty() ) {
						F_IGNORE_ERR({
							ScopeLock lock(zip_mutex_);
							ZipReader* read = get_zip_reader(zip);
							String inl_path = inl_format_part_path( path.substr(zip.length() + SEPARATOR.length()) );
							rv = read->readdir(inl_path);
						});
					}
					break;
				}
			}
			return std::move(rv);
		}

		String format(cString& path) {
			int index = -1;
			switch ( protocol(path) ) {
				default:
				case ZIP:
				case FILE: return Path::format("%s", *path);
				case HTTP: index = path.index_of('/', 8); break;
				case HTTPS:index = path.index_of('/', 9); break;
				case FTP:  index = path.index_of('/', 7); break;
				case FTPS: index = path.index_of('/', 8); break;
			}
			if (index == -1) {
				return path;
			}
			String s = inl_format_part_path(path.substr(index));
			if (s.is_empty()) {
				return path.substr(0, index);
			} else {
				return path.substr(0, index + 1) + s;
			}
		}
		
		bool is_absolute(cString& path) {
			
			if ( Path::is_local_absolute(path) ) {
				return true;
			} else {
				switch ( protocol(path) ) {
					case ZIP:
					case FILE:
					case HTTP:
					case HTTPS:
					case FTP:
					case FTPS: return true;
					default: return false;
				}
			}
		}
		
		void clear() {
			ScopeLock lock(zip_mutex_);
			for ( auto& i: zips_ ) {
				Release(i.value);
			}
			zips_.clear();
		}
		
	 private:
		Mutex zip_mutex_;
		Dict<String, ZipReader*> zips_;
	};

	FileReader::FileReader(): _core(new Core()) { }

	FileReader::FileReader(FileReader&& reader): _core(reader._core) {
		reader._core = nullptr;
	}

	FileReader::~FileReader() {
		delete _core;
		_core = nullptr;
	}

	uint32_t FileReader::read_file(cString& path, Cb cb) {
		return _core->read(path, cb, false);
	}
	uint32_t FileReader::read_stream(cString& path, Callback<StreamResponse> cb) {
		return _core->read(path, *(Cb*)&cb, true);
	}
	Buffer FileReader::read_file_sync(cString& path) throw(Error) {
		return _core->read_sync(path);
	}
	void FileReader::abort(uint32_t id) {
		_core->abort(id);
	}
	bool FileReader::exists_sync(cString& path) {
		return _core->exists_sync(path, 1, 1);
	}
	bool FileReader::is_file_sync(cString& path) {
		return _core->exists_sync(path, 1, 0);
	}
	bool FileReader::is_directory_sync(cString& path) {
		return _core->exists_sync(path, 0, 1);
	}
	Array<Dirent> FileReader::readdir_sync(cString& path) {
		try {
			return _core->readdir_sync(path);
		} catch(Error& err) {
			F_ERR(FS, err);
		}
		return Array<Dirent>();
	}
	String FileReader::format(cString& path) {
		return _core->format(path);
	}
	bool FileReader::is_absolute(cString& path) {
		return _core->is_absolute(path);
	}
	void FileReader::clear() {
		return _core->clear();
	}

	static FileReader* shared_instance = nullptr;

	void FileReader::set_shared_instance(FileReader* reader) {
		if (shared_instance != reader) {
			Release(shared_instance);
			shared_instance = reader;
		}
	}

	FileReader* FileReader::shared() {
		if ( !shared_instance ) {
			shared_instance = new FileReader();
		}
		return shared_instance;
	}

}
