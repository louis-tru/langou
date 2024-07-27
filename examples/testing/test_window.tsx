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

import { LOG, Mv, Pv, Mvcb } from './tool';
import { _CVD, Application, Window } from 'quark';
import * as screen from 'quark/screen';
import * as types from 'quark/types';

const resolve = require.resolve;
const app = new Application();
const s = app.screen;
const win = new Window({
	frame: types.newRect(0,0,500,500),
}).activate();

win.render(
	<box width={200} height={200} backgroundColor="#f00">
		<text textColor="#f00">ABCDEFG你好</text>
		<image src={resolve('res/cc.jpg')} width="match" height="match" opacity={0.5} />
	</box>
)

LOG('\nTest Window:\n')
Mv(screen, 'mainScreenScale', [], e=>[1,2,3].indexOf(e)!=-1)
Pv(win, 'atomPixel', e=>[0.5,1].indexOf(e)!=-1)
Mvcb(win, 'nextFrame', [()=>console.log('win.nextFrame')])
Pv(win, 'size', e=>e.x==500&&e.y==500, e=>e.size=types.parseVec2([500,500]))
Pv(win, 'atomPixel', 1)
Pv(win, 'surfaceSize', e=>true)
Pv(win, 'scale', 1)
Mv(win, 'setCursorStyle', [types.CursorStyle.Arrow])
Pv(win, 'backgroundColor', e=>e.toString()=='#000000')
