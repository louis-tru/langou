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

var util = require('./util');
var Buffer = require('buffer').Buffer;
var Path = require('path');
var fs = require('fs');
var mkdir = fs.mkdir;
var mkdirSync = fs.mkdirSync;
var chmod = fs.chmod;
var chown = fs.chown;

exports.__proto__ = fs;

function inl_copy_file(path, target, cancel_handle, options, cb) {

	if ( !options.replace ) {

		fs.exists(target, function(ok){
			if ( ok ) {
				cb(); // cancel copy
			} else {
				inl_copy_file(path, target, cancel_handle, { replace:true }, cb);
			}
		});
		
		return;
	}

	if (cancel_handle.is_cancel) {
		return;
	}

	var read = exports.createReadStream(path);
	var write = exports.createWriteStream(target);

	function error(e) {
		read.destroy();
		write.destroy();
		console.error(e);
		cb(e);
	}
	
	read.on('data', function (buff) {
		if (cancel_handle.is_cancel) {
			read.destroy();
			write.destroy();
		} else {
			write.write(buff);
		}
	});
	read.on('end', function () {
		if (!cancel_handle.is_cancel) {
			write.end();
			cb();
		}
	});
	read.on('error', error);
	write.on('error', error);
}

function inl_copy_dir(path, target, cancel_handle, options, cb) {
	if (cancel_handle.is_cancel) return;
	var list = null;
	
	function shift(err) {
		if (err) { return cb (err) }
		if (!list.length) { return cb() } // Íê³É
		
		var name = list.shift();
		if (options.ignore_hide && name[0] == '.')
			return shift(); // ºöÂÔÒþ²Ø

		var path2 = path + '/' + name;
		var target2 = target + '/' + name;
		
		fs.lstat(path2, function(err, stat) {
			if (err) return cb(err);
			
			if (stat.isSymbolicLink()) {
				fs.readlink(path2, (e,p)=>{
					if (e) 
						return cb(err);
					fs.symlink(target2, p, e=>{
						if (e)
							console.error(e);
						shift();
					});
				});
			}
			else if (stat.isFile()) {
				inl_copy_file(path2, target2, cancel_handle, options, shift);
			} 
			else if (stat.isDirectory()) {
				inl_copy_dir(path2, target2, cancel_handle, options, shift);
			} else {
				shift();
			}
		});
	}
	
	function readdir(err) {
		if (err) 
			return cb(err);
		fs.readdir(path, function (err, ls) {
			if (err) 
				return cb (err);
			list = ls;
			shift();
		});
	}
	
	fs.exists(target, function(exists) { // ´´½¨Ä¿Â¼
		if (exists) {
			readdir();
		} else {
			fs.mkdir(target, readdir);
		}
	});
}

function inl_copy_file_sync(path, target, options, check) {

	if ( !options.replace ) { // Èç¹û´æÔÚ²»Ìæ»»
		if ( fs.existsSync(target) ) {
			return; // ½áÊø
		}
	}

	if (!check(path, target)) return; // È¡Ïû

	var rfd = fs.openSync(path, 'r');
	var wfd = fs.openSync(target, 'w');
	
	var size = 1024 * 100;
	var buff = new Buffer(size); // 100kb
	var len = 0;
	
	do {
		if (!check(path, target)) break; // È¡Ïû
		len = fs.readSync(rfd, buff, 0, size, null);
		fs.writeSync(wfd, buff, 0, len, null);
	} while (len == size);
	
	fs.closeSync(rfd);
	fs.closeSync(wfd);
}

function inl_copy_dir_sync(path, target, options, check) {
	if (!check(path, target)) 
		return; // È¡Ïû
	
	if (!fs.existsSync(target)) { // ´´½¨Ä¿Â¼
		fs.mkdirSync(target);
	}
	
	var ls = fs.readdirSync(path);
	
	for (var i = 0; i < ls.length; i++) {
		var name = ls[i];
		if (options.ignore_hide && name[0] == '.')
			continue; // ºöÂÔÒþ²Ø
			
		var path2 = path + '/' + name;
		var target2 = target + '/' + name;
		var stat = fs.lstatSync(path2);
		
		if (stat.isSymbolicLink()) {
			try {
				fs.symlinkSync(fs.readlinkSync(path2), target2);
			} catch(err) { console.error(err); }
		}
		else if (stat.isFile()) {
			inl_copy_file_sync(path2, target2, options, check);
		}
		else if (stat.isDirectory()) {
			inl_copy_dir_sync(path2, target2, options, check);
		}
	}
}

function inl_rm_sync(path) {
	
	var stat = fs.lstatSync(path);
	if (stat.isFile() || stat.isSymbolicLink()) {
		return fs.unlinkSync(path);
	}
	else if (!stat.isDirectory()) {
		return;
	}
	
	var ls = fs.readdirSync(path);
	
	for(var i = 0; i < ls.length; i++){
		inl_rm_sync(path + '/' + ls[i]);
	}
	fs.rmdirSync(path);
}

function inl_rm(handle, path, cb) {

	cb = util.cb(cb);

	exports.lstat(path, function (err, stat) {
		if (err) {
			return cb(err);
		}
		
		if (stat.isFile() || stat.isSymbolicLink()) {
			if (!handle.is_cancel) { // Ã»ÓÐÈ¡Ïû
				exports.unlink(path, cb);
			}
			return;
		}
		else if (!stat.isDirectory()){
			return cb();
		}
		
		var ls = null;
		
		function shift(err) {
			if (err) {
				return cb(err);
			}
			if (!ls.length) {
				return exports.rmdir(path, cb);
			}
			inl_rm(handle, path + '/' + ls.shift(), shift);
		}

		//dir
		exports.readdir(path, function (err, data) {
			ls = data;
			shift(err);
		});
	});
}

function inl_ls_sync(origin, path, depth, cb) {
	var ls = fs.readdirSync(origin + '/' + path);
	var rev = [];
	
	for (var i = 0; i < ls.length; i++) {
		var name = ls[i];
		var pathname = path ? path + '/' + name : name;
		var stat = fs.statSync(origin + '/' + pathname);
		stat.name = name;

		cb(stat, pathname);
		
		if (stat.isFile()) {
			rev.push(stat);
		} else if (stat.isDirectory()) {
			if (depth) { // 
				stat.children = inl_ls_sync(origin, pathname, depth, cb);
			}
			rev.push(stat);
		}

	}
	return rev;
}

/**
 * set dir and file
 */
exports.chown_r = function (path, uid, gid, cb) {
	path = Path.resolve(path);
	
	cb = err || function (err) {
		if (err) throw util.err(err);
	};
	
	function shift(path, _cb) {
		exports.stat(path, function (err, stat) {
			if (err) { return cb(err) }
			if (!stat.isDirectory()) { return _cb() }
			
			var dir = path + '/';
			
			function shift2(ls) {
				if (!ls.length) { return _cb() }
				path = dir + ls.shift();
				chown(path, uid, gid, function (err) {
					if (err) { return cb(err) }
					shift(path, function () { shift2(ls) });
				});
			}
			exports.readdir(dir, function (err, ls) {
				if (err) { return cb(err) }
				shift2(ls);
			});
		});
	}
	
	chown(path, uid, gid, function (err) {
		if (err) { return cb }
		shift(path, cb);
	});
};

/**
 * set user file weight
 * @param {String}   path
 * @param {String}   mode
 * @param {Function} cb    (Optional)
 */
exports.chmod_r = function (path, mode, cb) {
	path = Path.resolve(path);
	
	cb = cb || function (err) { 
		if (err) throw util.err(err);
	}
	
	function shift (path, _cb) {
		
		exports.stat(path, function (err, stat) {
			if (err) { return cb(err) }
			if (!stat.isDirectory()) { return _cb() }
			
			var dir = path + '/';
			
			function shift2 (ls) {
				if (!ls.length) { return _cb() }
				path = dir + ls.shift();
				chmod(path, mode, function (err) {
					if (err) { return cb(err) }
					shift(path, function () { shift2(ls) })
				});
			}
			
			exports.readdir(dir, function (err, ls) {
				if (err) { return cb(err) }
				shift2(ls);
			});
		});
	}
	
	chmod(path, mode, function (err) {
		if (err) { return cb(err) }
		shift(path, cb);
	});
};

/**
	* remove file
	*/
exports.rm = function (path, cb) {
	return exports.unlink(path, cb);
};

/**
 * remove all file async
 * @param {String}   path
 * @param {Function} cb   (Optional)
 */
exports.rm_r = function (path, cb) {
	var handle = { is_cancel: false };
	cb = cb || function (err) { 
		if (err) throw util.err(err);
	};
	inl_rm(handle, path, cb);
	return {
		cancel: function () { // È¡Ïûdelete
			handle.is_cancel = true; 
			cb(null, null, true);
		}
	};
};

/**
	* Í¬²½É¾³ýÎÄ¼þ
	*/
exports.rm_sync = function (path) {
	return exports.unlinkSync(path);
};

/**
 * É¾³ýÎÄ¼þÓëÎÄ¼þ¼Ð
 */
exports.rm_r_sync = function (path) {
	if ( fs.existsSync(path) ) {
		inl_rm_sync(path);
	}
};

/**
 * copy all file 
 * @param {String}   path
 * @param {String}   target
 * @param {Object}   options  (Optional)
 * @param {Function} cb   (Optional)
 */
exports.cp = function (path, target, options, cb) {
	var cancel_handle = {
		is_cancel: false,
	};
	path = Path.resolve(path);
	target = Path.resolve(target);
	
	if (typeof options == 'function') {
		cb = options;
		options = null;
	}
	options = util.assign({ 
		ignore_hide: false, // ºöÂÔÒþ²Ø
		replace: true, // Èç¹û´æÔÚÌæ»»Ä¿±ê
	}, options);
	
	cb = cb || function (err) { 
		if (err) throw util.err(err);
	};
	
	if (options.ignore_hide && Path.basename(path)[0] == '.')
		return cb(); // ºöÂÔÒþ²Ø
	
	fs.stat(path, function (err, stat) {
		if (err) {
			return cb (err);
		}
		if (stat.isFile()) {
			inl_copy_file(path, target, cancel_handle, options, cb);
		} 
		else if (stat.isDirectory()) {
			inl_copy_dir(path, target, cancel_handle, options, cb);
		} 
		else {
			cb ();
		}
	});
	
	return {
		cancel: function () {  // È¡Ïûcp
			cancel_handle.is_cancel = true;
			cb(null, null, true);
		}
	};
};

/**
	* copy all file sync
	* @param {String}   path
	* @param {String}   target
	* @param {Object}   options  (Optional)
	*/
exports.cp_sync = function (path, target, options) {
	path = Path.resolve(path);
	target = Path.resolve(target);
	
	options = util.assign({ 
		ignore_hide: false, // ºöÂÔÒþ²Ø
		replace: true, // Èç¹û´æÔÚÌæ»»Ä¿±ê
		check: function() { return true; },
	}, options);
	
	var check = options.check;
	
	if (options.ignore_hide && Path.basename(path)[0] == '.')
		return; // ºöÂÔÒþ²Ø
		
	var stat = fs.statSync(path);
	
	exports.mkdir_p_sync(Path.dirname(target));
	
	if (stat.isFile()) {
		inl_copy_file_sync(path, target, options, check);
	} 
	else if (stat.isDirectory()) {
		inl_copy_dir_sync(path, target, options, check);
	}
};

/**
	* create all file dir
	* @param {String}   path
	* @param {String}   mode  (Optional)
	* @param {Function} cb    (Optional)
	*/
exports.mkdir_p = function (path, mode, cb) {

	if(typeof mode == 'function'){
		cb = mode;
		mode = null;
	}
	
	cb = cb || function (err) { 
		if (err) throw util.err(err);
	};
	
	path = Path.resolve(path);
	exports.exists(path, function (exists) {
		if (exists) { return cb() }

		var prefix = path.match(/^(\w+:)?\//)[0];
		var ls = path.substr(prefix.length).split('/');
		
		function shift (err) {
			if (err) { return cb(err) }
			if (!ls.length) { return cb() }
			
			prefix += ls.shift() + '/';
			exports.exists(prefix, function (exists) {
				if (exists) { return shift() }
				mkdir(prefix, mode, shift);
			});
		}
		shift();
	});
};

/**
	* create all file dir sync
	* @param {String}   path
	* @param {String}   mode  (Optional)
	*/
exports.mkdir_p_sync = function (path, mode){
	
	path = Path.resolve(path);
	
	if(fs.existsSync(path)){
		return;
	}
	
	var prefix = path.match(/^(\w+:)?\//)[0];
	var ls = path.substr(prefix.length).split('/');
	
	for(var i = 0; i < ls.length; i++){
		prefix += ls[i] + '/';
		if(!fs.existsSync(prefix)){
			mkdirSync(prefix, mode);
		}
	}
};

/**
	* get all info
	* @param {String}   path
	* @param {Boolean}  depth
	* @param {Function} cb
	*/
exports.ls = function (path, depth, cb) {

	path = Path.resolve(path);
	
	if (typeof depth == 'function') {
		cb = depth;
		depth = false;
	}
	
	cb = cb || function (err) {
		if (err) throw util.err(err);
	}
	
	function shift (path, _depth, _cb, stat) {

		if (!stat.isDirectory() || !_depth){
			return _cb(null, stat);
		}
		
		var cls = stat.children = [];
		
		function shift2 (err, ls) {
			if (err) {
				return cb (err);
			}
			
			if(!ls.length){
				return _cb(null, stat);
			}
			
			var name = ls.shift();
			var path2 =  path + '/' + name;
			
			fs.stat(path2, function (err, stat) {
				if (err) { return cb(err) }
				shift(path2, depth, function (err, stat) {
					stat.name = name;
					cls.push(stat);
					shift2(null, ls);
				}, stat);
			});
		}
		
		fs.readdir(path, shift2);
	}
	
	fs.stat(path, function (err, stat) {
		if (err) { return cb(err) }
		shift(path, true, function (err, stat) { 
			cb(err, stat.children || null);
		}, stat);
	});
};

/**
	* get dir info
	*/
exports.ls_sync = function(path, depth, cb) {
	path = Path.resolve(path);
	
	var rev = null;
	var stat = fs.statSync(path);
	
	if (stat.isDirectory()) {
		rev = inl_ls_sync(path, '', !!depth, util.cb(cb));
	}
	return rev;
};

exports.list = exports.ls;
exports.listSync = exports.ls_sync;
exports.mkdirSyncP = exports.mkdir_p_sync;
exports.mkdirP = exports.mkdir_p;
exports.copySync = exports.cp_sync;
exports.copy = exports.cp;
exports.removeSyncR = exports.rm_r_sync;
exports.removeR = exports.rm_r;
exports.chmodR = exports.chmod_r;
exports.chownR = exports.chown_r;
