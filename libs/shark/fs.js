/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

exports.__proto__ = require('fs');

export process.binding('shark_fs');

 /**
	* @enum FileType
	* FILE_UNKNOWN,
	* FILE_FILE,
	* FILE_DIR,
	* FILE_LINK,
	* FILE_FIFO,
	* FILE_SOCKET,
	* FILE_CHAR,
	* FILE_BLOCK
	* @end
	*/

 /**
	* DEFAULT_MODE
	*/

 /**
	* @object Dirent
	* name     {String}
	*	pathname {String}
	*	type     {FileType}
	* @end
	*/

 /**
	*
	* @func abort(id) abort async io
	* @arg id {uint}
	*
	* @func chmodR(path[,mode[,cb]])
	* @func chmodR(path[,cb])
	* @arg path {String}
	* @arg [mode=DEFAULT_MODE] {uint}
	* @arg [cb] {Function}
	* @ret {uint} return id
	*
	* @func chmodSyncR(path[,mode])
	* @arg path {String}
	* @arg [mode=DEFAULT_MODE] {uint}
	* @ret {bool}
	*
	* @func chownR(path, owner, group[,cb])
	* @arg path {String}
	* @arg owner {uint}
	* @arg group {uint}
	* @arg [cb] {Function}
	* @ret {uint} return id
	*
	* @func chownSyncR(path, owner, group)
	* @arg path {String}
	* @arg owner {uint}
	* @arg group {uint}
	* @ret {bool}
	*
	* @func mkdirP(path[,mode[,cb]])
	* @func mkdirP(path[,cb])
	* @arg path {String}
	* @arg [mode=default_mode] {uint}
	* @arg [cb] {Function}
	*
	* @func mkdirSyncP(path[,mode])
	* @arg path {String}
	* @arg [mode=default_mode] {uint}
	* @ret {bool}
	*
	* @func removeR(path[,cb])
	* @arg path {String}
	* @arg [cb] {Function}
	* @ret {uint} return id
	*
	* @func removeSyncR(path)
	* @arg path {String}
	* @ret {bool}
	*
	* @func copy(path,target[,cb])
	* @arg path {String}
	* @arg target {String}
	* @arg [cb] {Function}
	* @ret {uint} return id
	*
	* @func copySync(path, target)
	* @arg path {String}
	* @arg target {String}
	* @ret {bool}
	*
	* @func copyR(path, target[,cb])
	* @arg path {String}
	* @arg target {String}
	* @arg [cb] {Function}
	* @ret {uint} return id
	*
	* @func copySyncR(path, target)
	* @arg path {String}
	* @arg target {String}
	* @ret {bool}
	*
	* @func readdir(path[,cb])
	* @func ls(path[,cb])
	* @arg path {String}
	* @arg [cb] {Function}
	*
	* @func readdirSync(path)
	* @func ls_sync(path)
	* @arg path {String}
	* @ret {Array} return Array<Dirent>	
	*
	* @func isFile(path[,cb])
	* @arg path {String}
	* @arg [cb] {Function}
	*
	* @func isFileSync(path)
	* @arg path {String}
	* @ret {bool}
	*
	* @func isDirectory(path[,cb])
	* @arg path {String}
	* @arg [cb] {Function}
	*
	* @func isDirectorySync(path)
	* @arg path {String}
	* @ret {bool}
	*
	*/