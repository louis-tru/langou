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

#include "./cb.h"

namespace qk { namespace js {

	class WrapFileStat: public WrapObject {
	public:
		static void constructor(FunctionArgs args) {
			if (args.length() == 0 || !args[0]->isString()) {
				New<WrapFileStat>(args, new FileStat());
			} else {
				New<WrapFileStat>(args, new FileStat(args[0]->toStringValue(args.worker())));
			}
		}
		static void is_valid(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->is_valid() );
		}
		static void is_file(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->is_file() );
		}
		static void is_dir(FunctionArgs args)  {
			Js_Self(FileStat);
			Js_Return( self->is_dir() );
		}
		static void is_link(FunctionArgs args)  {
			Js_Self(FileStat);
			Js_Return( self->is_link() );
		}
		static void is_sock(FunctionArgs args)  {
			Js_Self(FileStat);
			Js_Return( self->is_sock() );
		}
		static void mode(FunctionArgs args)  {
			Js_Self(FileStat);
			Js_Return( self->mode() );
		}
		static void type(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->type() );
		}
		static void group(FunctionArgs args)  {
			Js_Self(FileStat);
			Js_Return( self->group() );
		}
		static void owner(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->owner() );
		}
		static void size(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->size() );
		}
		static void nlink(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->nlink() );
		}
		static void ino(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->ino() );
		}
		static void blksize(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->blksize() );
		}
		static void blocks(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->blocks() );
		}
		static void flags(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->flags() );
		}
		static void gen(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->gen() );
		}
		static void dev(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->dev() );
		}
		static void rdev(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->rdev() );
		}
		static void atime(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->atime() / 1000 );
		}
		static void mtime(FunctionArgs args) {
			Js_Self(FileStat);
			Js_Return( self->mtime() / 1000 );
		}
		static void ctime(FunctionArgs args)  {
			Js_Self(FileStat);
			Js_Return( self->ctime() / 1000 );
		}
		static void birthtime(FunctionArgs args)  {
			Js_Self(FileStat);
			Js_Return( self->birthtime() / 1000 );
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(FileStat, (JSClass*)0, constructor, {
				Js_Set_Class_Method(isValid, is_valid);
				Js_Set_Class_Method(isFile, is_file);
				Js_Set_Class_Method(isDir, is_dir);
				Js_Set_Class_Method(isDirectory, is_dir);
				Js_Set_Class_Method(isLink, is_link);
				Js_Set_Class_Method(isSock, is_sock);
				Js_Set_Class_Method(mode, mode);
				Js_Set_Class_Method(type, type);
				Js_Set_Class_Method(group, group);
				Js_Set_Class_Method(owner, owner);
				Js_Set_Class_Method(size, size);
				Js_Set_Class_Method(nlink, nlink);
				Js_Set_Class_Method(ino, ino);
				Js_Set_Class_Method(blksize, blksize);
				Js_Set_Class_Method(blocks, blocks);
				Js_Set_Class_Method(flags, flags);
				Js_Set_Class_Method(gen, gen);
				Js_Set_Class_Method(dev, dev);
				Js_Set_Class_Method(rdev, rdev);
				Js_Set_Class_Method(atime, atime);
				Js_Set_Class_Method(mtime, mtime);
				Js_Set_Class_Method(ctime, ctime);
				Js_Set_Class_Method(birthtime, birthtime);
			});
		}
	};

	static char* get_file_write_params(
		FunctionArgs args, bool sync, int& args_index,
		Buffer& holder, int64_t& size, Callback<Buffer>& cb
	) {
		Js_Worker(args);

		auto afterCollapse = false;
		char* data = nullptr;

		if ( args[args_index]->isString() ) { // 写入字符串
			int index_str = args_index++;
			Encoding en = kUTF8_Encoding;

			if ( args.length() > args_index && args[args_index]->isString() ) { // 第三个参数为编码格式
				if ( ! parseEncoding(args, args[args_index++], en) ) return nullptr;
			}
			holder = args[index_str]->toBuffer(worker, en);
			data = holder.val();
			size = holder.length();
		}
		else { // ArrayBuffer or TypedArray
			auto weakBuffer = args[args_index++]->asBuffer(worker);

			data = const_cast<char*>(*weakBuffer);
			size = weakBuffer.length();

			if (!sync) { // async
				// 这是一个危险的操作,确保buffer不能被释放否则会导致致命错误
				holder = Buffer(data, size);
				afterCollapse = true;
			}

			if ( args.length() > args_index && args[args_index]->isInt32() ) { // size
				int num = args[args_index++]->toInt32Value(worker);
				if ( num >= 0 && num < size ) {
					size = num;
					if (!sync) { // async
						holder.collapse();
						holder = Buffer(data, num);
					}
				}
			}
		}

		if (!sync) {
			// keep war buffer Persistent javascript object
			struct PersistentValue {
				typedef NonObjectTraits Traits;
				PersistentValue(FunctionArgs args): source(args.worker(), args[2]) {}
				Persistent<JSValue> source;
			} *pv = new PersistentValue(args);

			auto cbInt = get_callback_for_int(worker, args[0]);

			cb = Callback<Buffer>([pv, cbInt, afterCollapse](auto &e) {
				Qk_ASSERT( e.data );
				Sp<PersistentValue> (pv);
				auto len = e.data->length();
				if (afterCollapse) {
					// collapse这个buffer因为这是ArrayBuffer所持有的内存空间,不能在这里被释放
					e.data->collapse();
				}
				if (e.error) {
					cbInt->reject(e.error);
				} else {
					Int32 i(len);
					cbInt->resolve(&i);
				}
			});
		}

		return data;
	}

	class WrapFs {
	public:
		static void chmod(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || ! args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method chmodSync(path[,mode])\n"
						"* @param path {String}\n"
						"* @param [mode=default_mode] {uint}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method chmod(path[,mode[,cb]][,cb])\n"
						"* @param path {String}\n"
						"* @param [mode=default_mode] {uint}\n"
						"* @param [cb] {Function}\n"
					);
				}
			}
			int args_index = 1;
			uint32_t mode = fs_default_mode;
			if (args.length() > 1 && args[1]->isUint32()) {
				mode = args[1]->toUint32Value(worker);
				args_index++;
			}
			if ( sync ) {
				try {
					fs_chmod_sync(args[0]->toStringValue(worker), mode);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > args_index ) {
					cb = get_callback_for_none(worker, args[args_index]);
				}
				fs_chmod(args[0]->toStringValue(worker), mode, cb);
			}
		}

		static void chmod_r(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || ! args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method chmodrSync(path[,mode])\n"
						"* @param path {String}\n"
						"* @param [mode=default_mode] {uint}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method chmodr(path[,mode[,cb]][,cb])\n"
						"* @param path {String}\n"
						"* @param [mode=default_mode] {uint}\n"
						"* @param [cb] {Function}\n"
						"* @return {uint} return id\n"
					);
				}
			}
			int args_index = 1;
			uint32_t mode = fs_default_mode;
			if (args.length() > 1 && args[1]->isUint32()) {
				mode = args[1]->toUint32Value(worker);
				args_index++;
			}
			if ( sync ) {
				try {
					fs_chmod_r_sync(args[0]->toStringValue(worker), mode);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > args_index ) {
					cb = get_callback_for_none(worker, args[args_index]);
				}
				Js_Return( fs_chmod_r(args[0]->toStringValue(worker), mode, cb) );
			}
		}

		static void chown(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 3 ||
					!args[0]->isString() ||
					!args[1]->isUint32() || !args[2]->isUint32() ) {
				if ( sync ) {
					Js_Throw(
						"* @method chownSync(path, owner, group)\n"
						"* @param path {String}\n"
						"* @param owner {uint}\n"
						"* @param group {uint}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method chown(path, owner, group[,cb])\n"
						"* @param path {String}\n"
						"* @param owner {uint}\n"
						"* @param group {uint}\n"
						"* @param [cb] {Function}\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_chown_sync(args[0]->toStringValue(worker),
												args[1]->toUint32Value(worker),
												args[2]->toUint32Value(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 3 ) {
					cb = get_callback_for_none(worker, args[3]);
				}
				fs_chown(args[0]->toStringValue(worker),
								args[1]->toUint32Value(worker),
								args[2]->toUint32Value(worker), cb);
			}
		}

		static void chown_r(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( args.length() < 3 ||
					!args[0]->isString() ||
					!args[1]->isUint32() || !args[2]->isUint32() ) {
				if ( sync ) {
					Js_Throw(
						"* @method chownrSync(path, owner, group)\n"
						"* @param path {String}\n"
						"* @param owner {uint}\n"
						"* @param group {uint}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method chownr(path, owner, group[,cb])\n"
						"* @param path {String}\n"
						"* @param owner {uint}\n"
						"* @param group {uint}\n"
						"* @param [cb] {Function}\n"
						"* @return {uint} return id\n"
					);
				}
			}
			
			if ( sync ) {
				try {
					fs_chown_r_sync(args[0]->toStringValue(worker),
													args[1]->toUint32Value(worker),
													args[2]->toUint32Value(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 3 ) {
					cb = get_callback_for_none(worker, args[3]);
				}
				Js_Return( fs_chown_r(args[0]->toStringValue(worker),
															args[1]->toUint32Value(worker),
															args[2]->toUint32Value(worker), cb) );
			}
		}

		static void mkdir(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || !args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method mkdirSync(path[,mode])\n"
						"* @param path {String}\n"
						"* @param [mode=default_mode] {uint}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method mkdir(path[,mode[,cb]][,cb])\n"
						"* @param path {String}\n"
						"* @param [mode=default_mode] {uint}\n"
						"* @param [cb] {Function}\n"
					);
				}
			}
			int args_index = 1;
			uint32_t mode = fs_default_mode;
			if (args.length() > 1 && args[1]->isUint32()) {
				mode = args[1]->toUint32Value(worker);
				args_index++;
			}
			if ( sync ) {
				try {
					fs_mkdir_sync(args[0]->toStringValue(worker), mode);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > args_index ) {
					cb = get_callback_for_none(worker, args[args_index]);
				}
				fs_mkdir(args[0]->toStringValue(worker), mode, cb);
			}
		}

		static void mkdir_p(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || ! args[0]->isString()) {
				if ( sync ){
					Js_Throw(
						"* @method mkdirpSync(path[,mode])\n"
						"* @param path {String}\n"
						"* @param [mode=default_mode] {uint}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method mkdirp(path[,mode[,cb]][,cb])\n"
						"* @param path {String}\n"
						"* @param [mode=default_mode] {uint}\n"
						"* @param [cb] {Function}\n"
						"* @return {uint} return id\n"
					);
				}
			}
			int args_index = 1;
			uint32_t mode = fs_default_mode;
			if (args.length() > 1 && args[1]->isUint32()) {
				mode = args[1]->toUint32Value(worker);
				args_index++;
			}
			if ( sync ) {
				try {
					fs_mkdir_p_sync(args[0]->toStringValue(worker), mode);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > args_index ) {
					cb = get_callback_for_none(worker, args[args_index]);
				}
				fs_mkdir_p(args[0]->toStringValue(worker), mode, cb);
			}
		}

		static void rename(FunctionArgs args, bool sync) {
			Js_Worker(args);

			if (args.length() < 2 || !args[0]->isString() || !args[1]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method renameSync(name,new_name)\n"
						"* @param name {String}\n"
						"* @param new_name {String}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method rename(name,new_name[,cb])\n"
						"* @param name {String}\n"
						"* @param new_name {String}\n"
						"* @param [cb] {Function}\n"
					);
				}
			}

			if ( sync ) {
				try {
					fs_rename_sync(args[0]->toStringValue(worker),
																	args[1]->toStringValue(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 2 ) {
					cb = get_callback_for_none(worker, args[2]);
				}
				fs_rename(args[0]->toStringValue(worker), args[1]->toStringValue(worker), cb);
			}
		}

		static void link(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 2 || !args[0]->isString() || !args[1]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method linkSync(path,newPath)\n"
						"* @param path {String}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method link(path,newPath[,cb])\n"
						"* @param path {String}\n"
						"* @param [cb] {Function}\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_link_sync(
						args[0]->toStringValue(worker),
						args[1]->toStringValue(worker)
					);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 2 ) {
					cb = get_callback_for_none(worker, args[2]);
				}
				fs_link(args[0]->toStringValue(worker), args[1]->toStringValue(worker), cb);
			}
		}

		static void unlink(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || !args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method unlinkSync(path)\n"
						"* @param path {String}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method unlink(path[,cb])\n"
						"* @param path {String}\n"
						"* @param [cb] {Function}\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_unlink_sync(args[0]->toStringValue(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 1 ) {
					cb = get_callback_for_none(worker, args[1]);
				}
				fs_unlink(args[0]->toStringValue(worker), cb);
			}
		}

		static void rmdir(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || !args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method rmdirSync(path)\n"
						"* @param path {String}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method rmdir(path)\n"
						"* @param path {String}\n"
						"* @param [cb] {Function}\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_rmdir_sync(args[0]->toStringValue(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 1 ) {
					cb = get_callback_for_none(worker, args[1]);
				}
				fs_rmdir(args[0]->toStringValue(worker), cb);
			}
		}

		static void remove_r(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || !args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method removerSync(path)\n"
						"* @param path {String}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method remover(path)\n"
						"* @param path {String}\n"
						"* @param [cb] {Function}\n"
						"* @return {uint} return id\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_remove_r_sync(args[0]->toStringValue(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 1 ) {
					cb = get_callback_for_none(worker, args[1]);
				}
				Js_Return( fs_remove_r(args[0]->toStringValue(worker), cb) );
			}
		}

		static void copy(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 2 || !args[0]->isString() || !args[1]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method copySync(path, target)\n"
						"* @param path {String}\n"
						"* @param target {String}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method copy(path, target)\n"
						"* @param path {String}\n"
						"* @param target {String}\n"
						"* @param [cb] {Function}\n"
						"* @return {uint} return id\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_copy_sync(args[0]->toStringValue(worker),
																args[1]->toStringValue(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 2 ) {
					cb = get_callback_for_none(worker, args[2]);
				}
				Js_Return( fs_copy(args[0]->toStringValue(worker),
																		args[1]->toStringValue(worker), cb) );
			}
		}

		static void copy_r(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 2 || !args[0]->isString() || !args[1]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method copyrSync(path, target)\n"
						"* @param path {String}\n"
						"* @param target {String}\n"
						"* @return {void}\n"
					);
				} else {
					Js_Throw(
						"* @method copyr(path, target)\n"
						"* @param path {String}\n"
						"* @param target {String}\n"
						"* @param [cb] {Function}\n"
						"* @return {uint} return id\n"
					);
				}
			}
			if ( sync ) {
				try {
					fs_copy_r_sync(args[0]->toStringValue(worker),
																	args[1]->toStringValue(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				Cb cb;
				if ( args.length() > 2 ) {
					cb = get_callback_for_none(worker, args[2]);
				}
				Js_Return( fs_copy_r(args[0]->toStringValue(worker),
																			args[1]->toStringValue(worker), cb) );
			}
		}

		static void readdir(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || !args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method readdirSync(path)\n"
						"* @param path {String}\n"
						"* @return {Array} return Array<Dirent>\n"
					);
				} else {
					Js_Throw(
						"* @method readdir(path[,cb])\n"
						"* @param path {String}\n"
						"* @param [cb] {Function}\n"
					);
				}
			}
			if ( sync ) {
				Array<Dirent> r;
				try {
					r = fs_readdir_sync(args[0]->toStringValue(worker));
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( r );
			} else {
				Callback<qk::Array<qk::Dirent>> cb;
				if ( args.length() > 1 ) {
					cb = get_callback_for_array_dirent(worker, args[1]);
				}
				fs_readdir( args[0]->toStringValue(worker), cb);
			}
		}

		static void stat(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if (args.length() < 1 || ! args[0]->isString()) {
				if ( sync ) {
					Js_Throw(
						"* @method statSync(path)\n"
						"* @param path {String}\n"
						"* @return {FileStat}\n"
					);
				} else {
					Js_Throw(
						"* @method stat(path[,cb])\n"
						"* @param path {String}\n"
						"* @param [cb] {Function}\n"
					);
				}
			}
			if ( sync ) {
				FileStat r;
				try {
					r = fs_stat_sync( args[0]->toStringValue(worker) );
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( r );
			} else {
				Callback<FileStat> cb;
				if ( args.length() > 1 ) {
					cb = get_callback_for_file_stat(worker, args[1]);
				}
				fs_stat(args[0]->toStringValue(worker), cb);
			}
		}

		static void exists(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"* @method existsSync(path)\n"
						"* @param path {String}\n"
						"* @return {bool}\n"
					);
				}
				Js_Return( fs_exists_sync(args[0]->toStringValue(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"* @method exists(path[,cb])\n"
						"* @param path {String}\n"
						"* @param [cb] {Function}\n"
					);
				}
				fs_exists(args[0]->toStringValue(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void is_file(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"* @method isFileSync(path)\n"
						"* @param path {String}\n"
						"* @return {bool}\n"
					);
				}
				Js_Return( fs_is_file_sync(args[0]->toStringValue(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"* @method isFile(path[,cb])\n"
						"* @param path {String}\n"
						"* @param [cb] {Function}\n"
					);
				}
				fs_is_file(args[0]->toStringValue(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void is_directory(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"* @method isDirectorySync(path)\n"
						"* @param path {String}\n"
						"* @return {bool}\n"
					);
				}
				Js_Return( fs_is_directory_sync(args[0]->toStringValue(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"* @method isDirectory(path[,cb])\n"
						"* @param path {String}\n"
						"* @param [cb] {Function}\n"
					);
				}
				fs_is_directory(args[0]->toStringValue(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void readable(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"* @method readableSync(path)\n"
						"* @param path {String}\n"
						"* @return {bool}\n"
					);
				}
				Js_Return( fs_readable_sync(args[0]->toStringValue(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"* @method readable(path[,cb])\n"
						"* @param path {String}\n"
						"* @param cb {Function}\n"
					);
				}
				fs_readable(args[0]->toStringValue(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void writable(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"* @method writableSync(path)\n"
						"* @param path {String}\n"
						"* @return {bool}\n"
					);
				}
				Js_Return( fs_writable_sync(args[0]->toStringValue(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"* @method writable(path[,cb])\n"
						"* @param path {String}\n"
						"* @param cb {Function}\n"
					);
				}
				fs_writable(args[0]->toStringValue(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void executable(FunctionArgs args, bool sync) {
			Js_Worker(args);
			if ( sync ) {
				if (args.length() < 1 || ! args[0]->isString()) {
					Js_Throw(
						"* @method executableSync(path)\n"
						"* @param path {String}\n"
						"* @return {bool}\n"
					);
				}
				Js_Return( fs_executable_sync(args[0]->toStringValue(worker)) );
			} else {
				if (args.length() < 2 || !args[0]->isString() || !args[1]->isFunction()) {
					Js_Throw(
						"* @method executable(path[,cb])\n"
						"* @param path {String}\n"
						"* @param cb {Function}\n"
					);
				}
				fs_executable(args[0]->toStringValue(worker), get_callback_for_bool(worker, args[1]));
			}
		}

		static void read_stream(FunctionArgs args) {
			Js_Worker(args);
			if (args.length() < 2 || !args[0]->isFunction() || !args[1]->isString()) {
				Js_Throw(
					"* @method readStream(cb,path)\n"
					"* @param cb {Function}\n"
					"* @param path {String}\n"
					"* @return {uint32_t} return abort id\n"
				);
			}
			auto cb = get_callback_for_io_stream(worker, args[0]);
			String path = args[1]->toStringValue(worker);
			Js_Return( fs_read_stream(path, cb) );
		}

		static void read_file(FunctionArgs args, bool sync) {
			Js_Worker(args);

			int args_index = 0;
			if (sync) {
				if (args.length() < 1 || !args[0]->isString()) {
					Js_Throw(
						"* @method readFileSync(path[,encoding])\n"
						"* @param path {String}\n"
						"* @param [encoding] {String}\n"
						"* @return {Buffer} return file buffer\n"
					);
				}
			} else {
				args_index++;
				if (args.length() < 2 || !args[0]->isFunction() || !args[1]->isString()) {
					Js_Throw(
						"* @method readFile(cb,path[,encoding])\n"
						"* @param cb {Function}\n"
						"* @param path {String}\n"
						"* @param [encoding] {String}\n"
					);
				}
			}

			String path = args[args_index++]->toStringValue(worker);
			Encoding encoding = kInvalid_Encoding;

			if (args.length() > args_index && args[args_index]->isString()) { //
				if ( !parseEncoding(args, args[args_index++], encoding) ) return;
			}

			if ( sync ) {
				Buffer r;
				try {
					r = fs_read_file_sync(path, -1);
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( convert_buffer(worker, r, encoding) );
			} else {
				// Qk_LOG("read_file,args,%d", args.length());
				fs_read_file(path, get_callback_for_buffer(worker, args[0], encoding));
			}
		}

		static void write_file(FunctionArgs args, bool sync) {
			Js_Worker(args);

			int args_index = 0;
			if ( sync ) {
				if ( args.length() < 2 || !args[0]->isString() ||
						!(args[1]->isString() || args[1]->isBuffer())
				) { // 参数错误
					Js_Throw(
						"* @method writeFileSync(path,buffer[,size])\n"
						"* @method writeFileSync(path,string[,encoding])\n"
						"* @param path {String}\n"
						"* @param string {String}\n"
						"* @param buffer {Uint8Array|ArrayBuffer}\n"
						"* @param [size] {int} write data size\n"
						"* @param [encoding=utf8] {Encoding}\n"
						"* @return {int}\n"
					);
				}
			} else {
				args_index++;
				if ( args.length() < 3 || !args[0]->isFunction() || !args[1]->isString() ||
						!(args[2]->isString() || args[2]->isBuffer())
					) {
					Js_Throw(
						"* @method writeFile(cb,path,buffer[,size])\n"
						"* @method writeFile(cb,path,string[,encoding])\n"
						"* @param cb {Function}\n"
						"* @param path {String}\n"
						"* @param string {String}\n"
						"* @param buffer {Uint8Array|ArrayBuffer}\n"
						"* @param [size] {int} write data size\n"
						"* @param [encoding=utf8] {Encoding}\n"
					);
				}
			}
			
			String path = args[args_index++]->toStringValue(worker);
			Buffer holder;
			int64_t size;
			Callback<Buffer> cb;

			auto data = get_file_write_params(args, sync, args_index, holder, size, cb);
			if (!data)
				return;

			if ( sync ) {
				int r;
				try {
					r = fs_write_file_sync(path, data, size);
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( r );
			} else {
				fs_write_file(path, holder, cb);
			}
		}

		static void open(FunctionArgs args, bool sync) {
			Js_Worker(args);

			int args_index = 0;
			if (sync) {
				if ( args.length() < 1 || !args[0]->isString() ) {
					Js_Throw(
						"* @method openSync(path[,flag])\n"
						"* @param path {String}\n"
						"* @param [flag=FOPEN_R] {FileOpenFlag}\n"
						"* @return {int} return file handle `success >= 0`\n"
					);
				} 
			} else {
				args_index++;
				if ( args.length() < 2 || !args[0]->isFunction() || !args[1]->isString() ) {
					Js_Throw(
						"* @method open(cb,path[,flag])\n"
						"* @param cb {Function}\n"
						"* @param path {String}\n"
						"* @param [flag=FOPEN_R] {FileOpenFlag}\n"
					);
				}
			}

			String path = args[args_index++]->toStringValue(worker);
			FileOpenFlag flag = FileOpenFlag::FOPEN_R;

			if ( args.length() > args_index && args[args_index]->isUint32() ) {
				uint32_t num = args[args_index++]->toUint32Value(worker);
				flag = (FileOpenFlag)num;
			}

			if ( sync ) {
				int r;
				try {
					r = fs_open_sync(path, flag);
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( r );
			} else {
				fs_open(path, flag, get_callback_for_int(worker, args[0]));
			}
		}

		static void close(FunctionArgs args, bool sync) {
			Js_Worker(args);

			int args_index = 0;
			if (sync) {
				if ( args.length() < 1 || !args[0]->isInt32() ) {
					Js_Throw(
						"* @method closeSync(fd)\n"
						"* @param path {int} file handle\n"
						"* @return {void}\n"
						);
				}
			} else {
				args_index++;
				if ( args.length() < 2 || !args[0]->isFunction() || !args[1]->isInt32() ) {
					Js_Throw(
						"* @method close(cb,fd)\n"
						"* @param cb {Function}\n"
						"* @param fd {int} file handle\n"
						);
				}
			}

			int fd = args[args_index]->toInt32Value(worker);
			if ( sync ) {
				try {
					fs_close_sync(fd);
				} catch(cError& err) {
					Js_Throw(err);
				}
			} else {
				fs_close(fd, get_callback_for_none(worker, args[0]));
			}
		}

		static void read(FunctionArgs args, bool sync) {
			Js_Worker(args);
			
			uint32_t args_index = 0;
			if (sync) {
				if ( args.length() < 2 || !args[0]->isInt32() || !args[1]->isUint8Array() ) {
					Js_Throw(
						"* @method readSync(fd,buffer[,size[,offsetFd]])\n"
						"* @param fd {int} file handle\n"
						"* @param buffer {Buffer} output buffer\n"
						"* @param [size=-1] {int}\n"
						"* @param [offsetFd=-1] {int}\n"
						"* @return {int} return read data length\n"
					);
				}
			} else {
				args_index++;
				if ( args.length() < 3 || !args[0]->isFunction() || !args[1]->isInt32() || !args[2]->isUint8Array() ) {
					Js_Throw(
						"* @method read(cb,fd,buffer[,size[,offsetFd]])\n"
						"* @param cb {Function}\n"
						"* @param fd {int} file handle\n"
						"* @param buffer {Buffer} output buffer\n"
						"* @param [size=-1] {int}\n"
						"* @param [offsetFd=-1] {int}\n"
					);
				}
			}

			int fd = args[args_index++]->toInt32Value(worker);
			auto weakBuffer = args[args_index++]->asBuffer(worker);
			char* data = const_cast<char*>(*weakBuffer);
			uint32_t size = weakBuffer.length();
			int64_t offset = -1;

			// size
			if ( args.length() > args_index && args[args_index]->isInt32() ) {
				int num = args[args_index++]->toInt32Value(worker);
				if ( num >= 0 ) {
					size = Qk_MIN(size, num);
				}
			}

			// offset
			if ( args.length() > args_index && args[args_index]->isInt32() ) {
				offset = args[args_index++]->toInt32Value(worker);
				if ( offset < 0 ) offset = -1;
			}
			
			if ( sync ) {
				int r;
				try {
					r = fs_read_sync(fd, data, size, offset);
				} catch(cError& err) {
					Js_Throw(err);
				}
				Js_Return( r );
			} else {
				// keep war buffer Persistent javascript object
				struct PersistentValue {
					typedef NonObjectTraits Traits;
					Persistent<JSValue> args[2];
				} *pv = new PersistentValue;

				auto cbInt = get_callback_for_int(worker, args[0]);

				fs_read(fd, Buffer(data, size), offset, Callback<Buffer>([pv, cbInt](auto& e) {
					Qk_ASSERT(e.data);
					Sp<PersistentValue> (pv);
					auto len = e.data->length();
					// collapse这个buffer因为这是ArrayBuffer所持有的内存空间,不能在这里被释放
					e.data->collapse();
					if (e.error) {
						cbInt->reject(e.error);
					} else {
						Int32 i(len);
						cbInt->resolve(&i);
					}
				}));
			}
		}

		static void write(FunctionArgs args, bool sync) {
			Js_Worker(args);

			int args_index = 0;
			if (sync) {
				if (args.length() < 2 || !args[0]->isInt32() ||
					!(args[1]->isString() || args[1]->isBuffer())
				) {
					Js_Throw(
						"* @method writeSync(fd,buffer[,size[,offsetFd]])\n"
						"* @method writeSync(fd,string[,encoding[,offsetFd]])\n"
						"* @param fd {int} file handle\n"
						"* @param buffer {Uint8Array|ArrayBuffer} write buffer\n"
						"* @param string {String} write string\n"
						"* @param [size=-1] {int} write size, `-1` use buffer.length\n"
						"* @param [offsetFd=-1] {int}\n"
						"* @param [encoding='utf8'] {String}\n"
						"* @return {int} return write data length\n"
					);
				}
			} else {
				args_index++;
				if (args.length() < 3 || !args[0]->isFunction() || !args[1]->isInt32() ||
					!(args[2]->isString() || args[2]->isBuffer() )
				) {
					Js_Throw(
						"* @method write(cb,fd,buffer[,size[,offsetFd]])\n"
						"* @method write(cb,fd,string[,encoding[,offsetFd]])\n"
						"* @param cb {Function}\n"
						"* @param fd {int} file handle\n"
						"* @param buffer {Uint8Array|ArrayBuffer} write buffer\n"
						"* @param string {String} write string\n"
						"* @param [size=-1] {int} write size, `-1` use buffer.length\n"
						"* @param [offsetFd=-1] {int}\n"
						"* @param [encoding='utf8'] {String}\n"
					);
				}
			}

			int fd = args[args_index++]->toInt32Value(worker);
			Buffer holder;
			int64_t size;
			int64_t offset = -1;
			Callback<Buffer> cb;

			auto data = get_file_write_params(args, sync, args_index, holder, size, cb);
			if (!data)
				return;

			if (args.length() > args_index && args[args_index]->isInt32()) { // offset
				offset = args[args_index++]->toInt32Value(worker);
				if ( offset < 0 )
					offset = -1;
			}

			if ( sync ) {
				int r;
				Js_Try_Catch({
					r = fs_write_sync(fd, data, size, offset);
				}, Error);
				Js_Return( r );
			} else {
				fs_write(fd, holder, offset, cb);
			}
		}

		static void abort(FunctionArgs args) {
			Js_Worker(args);
			if (args.length() < 1 || ! args[0]->isUint32()) {
				Js_Throw(
					"* @method abort(id) abort async io\n"
					"* @param id {uint}\n"
				);
			}
			fs_abort( args[0]->toUint32Value(worker) );
		}
		
		// sync
		static void chmod_sync(FunctionArgs args) { chmod(args, 1); }
		static void chown_sync(FunctionArgs args) { chown(args, 1); }
		static void mkdir_sync(FunctionArgs args) { mkdir(args, 1); }
		static void rename_sync(FunctionArgs args) { rename(args, 1); }
		static void link_sync(FunctionArgs args) { link(args, 1); }
		static void unlink_sync(FunctionArgs args) { unlink(args, 1); }
		static void rmdir_sync(FunctionArgs args) { rmdir(args, 1); }
		static void readdir_sync(FunctionArgs args) { readdir(args, 1); }
		static void stat_sync(FunctionArgs args) { stat(args, 1); }
		static void exists_sync(FunctionArgs args) { exists(args, 1); }
		static void is_file_sync(FunctionArgs args) { is_file(args, 1); }
		static void is_directory_sync(FunctionArgs args) { is_directory(args, 1); }
		static void readable_sync(FunctionArgs args) { readable(args, 1); }
		static void writable_sync(FunctionArgs args) { writable(args, 1); }
		static void executable_sync(FunctionArgs args) { executable(args, 1); }
		static void copy_sync(FunctionArgs args) { copy(args, 1); }
		static void chmod_r_sync(FunctionArgs args) { chmod_r(args, 1); }
		static void chown_r_sync(FunctionArgs args) { chown_r(args, 1); }
		static void mkdir_p_sync(FunctionArgs args) { mkdir_p(args, 1); }
		static void remove_r_sync(FunctionArgs args) { remove_r(args, 1); }
		static void copy_r_sync(FunctionArgs args) { copy_r(args, 1); }
		static void write_file_sync(FunctionArgs args) { write_file(args, 1); }
		static void read_file_sync(FunctionArgs args) { read_file(args, 1); }
		static void open_sync(FunctionArgs args) { open(args, 1); }
		static void close_sync(FunctionArgs args) { close(args, 1); }
		static void read_sync(FunctionArgs args) { read(args, 1); }
		static void write_sync(FunctionArgs args) { write(args, 1); }
		// async
		static void chmod_async(FunctionArgs args) { chmod(args, 0); }
		static void chown_async(FunctionArgs args) { chown(args, 0); }
		static void mkdir_async(FunctionArgs args) { mkdir(args, 0); }
		static void rename_async(FunctionArgs args) { rename(args, 0); }
		static void link_async(FunctionArgs args) { link(args, 0); }
		static void unlink_async(FunctionArgs args) { unlink(args, 0); }
		static void rmdir_async(FunctionArgs args) { rmdir(args, 0); }
		static void readdir_async(FunctionArgs args) { readdir(args, 0); }
		static void stat_async(FunctionArgs args) { stat(args, 0); }
		static void exists_async(FunctionArgs args) { exists(args, 0); }
		static void is_file_async(FunctionArgs args) { is_file(args, 0); }
		static void is_directory_async(FunctionArgs args) { is_directory(args, 0); }
		static void readable_async(FunctionArgs args) { readable(args, 0); }
		static void writable_async(FunctionArgs args) { writable(args, 0); }
		static void executable_async(FunctionArgs args) { executable(args, 0); }
		static void copy_async(FunctionArgs args) { copy(args, 0); }
		static void chmod_r_async(FunctionArgs args) { chmod_r(args, 0); }
		static void chown_r_async(FunctionArgs args) { chown_r(args, 0); }
		static void mkdir_p_async(FunctionArgs args) { mkdir_p(args, 0); }
		static void remove_r_async(FunctionArgs args) { remove_r(args, 0); }
		static void copy_r_async(FunctionArgs args) { copy_r(args, 0); }
		static void write_file_async(FunctionArgs args) { write_file(args, 0); }
		static void read_file_async(FunctionArgs args) { read_file(args, 0); }
		static void open_async(FunctionArgs args) { open(args, 0); }
		static void close_async(FunctionArgs args) { close(args, 0); }
		static void read_async(FunctionArgs args) { read(args, 0); }
		static void write_async(FunctionArgs args) { write(args, 0); }

		static void binding(JSObject* exports, Worker* worker) {
			WrapFileStat::binding(exports, worker);

			Js_Set_Property(DEFAULT_MODE, fs_default_mode);
			// api sync
			Js_Set_Method(chmodSync, chmod_sync);
			Js_Set_Method(chownSync, chown_sync);
			Js_Set_Method(mkdirSync, mkdir_sync);
			Js_Set_Method(renameSync, rename_sync);
			Js_Set_Method(linkSync, link_sync);
			Js_Set_Method(unlinkSync, unlink_sync);
			Js_Set_Method(rmdirSync, rmdir_sync);
			Js_Set_Method(readdirSync, readdir_sync);
			Js_Set_Method(statSync, stat_sync);
			Js_Set_Method(existsSync, exists_sync);
			Js_Set_Method(isFileSync, is_file_sync);
			Js_Set_Method(isDirectorySync, is_directory_sync);
			Js_Set_Method(readableSync, readable_sync);
			Js_Set_Method(writableSync, writable_sync);
			Js_Set_Method(executableSync, executable_sync);
			Js_Set_Method(copySync, copy_sync);
			Js_Set_Method(chmodrSync, chmod_r_sync);
			Js_Set_Method(chownrSync, chown_r_sync);
			Js_Set_Method(mkdirpSync, mkdir_p_sync);
			Js_Set_Method(removerSync, remove_r_sync);
			Js_Set_Method(copyrSync, copy_r_sync);
			// async
			Js_Set_Method(chmod, chmod_async);
			Js_Set_Method(chown, chown_async);
			Js_Set_Method(mkdir, mkdir_async);
			Js_Set_Method(rename, rename_async);
			Js_Set_Method(link, link_async);
			Js_Set_Method(unlink, unlink_async);
			Js_Set_Method(rmdir, rmdir_async);
			Js_Set_Method(readdir, readdir_async);
			Js_Set_Method(stat, stat_async);
			Js_Set_Method(exists, exists_async);
			Js_Set_Method(isFile, is_file_async);
			Js_Set_Method(isDirectory, is_directory_async);
			Js_Set_Method(readable, readable_async);
			Js_Set_Method(writable, writable_async);
			Js_Set_Method(executable, executable_async);
			Js_Set_Method(copy, copy_async);
			Js_Set_Method(chmodr, chmod_r_async);
			Js_Set_Method(chownr, chown_r_async);
			Js_Set_Method(mkdirp, mkdir_p_async);
			Js_Set_Method(remover, remove_r_async);
			Js_Set_Method(copyr, copy_r_async);
			Js_Set_Method(readStream, read_stream);
			Js_Set_Method(abort, abort);
			// read/write file sync
			Js_Set_Method(writeFileSync, write_file_sync);
			Js_Set_Method(readFileSync, read_file_sync);
			Js_Set_Method(openSync, open_sync);
			Js_Set_Method(closeSync, close_sync);
			Js_Set_Method(readSync, read_sync);
			Js_Set_Method(writeSync, write_sync);
			// async
			Js_Set_Method(writeFile, write_file_async);
			Js_Set_Method(readFile, read_file_async);
			Js_Set_Method(open, open_async);
			Js_Set_Method(close, close_async);
			Js_Set_Method(read, read_async);
			Js_Set_Method(write, write_async);
		}
	};

	Js_Set_Module(_fs, WrapFs);
} }