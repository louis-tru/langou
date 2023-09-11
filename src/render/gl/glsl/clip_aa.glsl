#vert
void main() {
	aafuzz = aafuzzIn;
	gl_Position = matrix * vec4(vertexIn.xy, depth, 1.0);
}

#frag
uniform                float     aafuzzWeight;
uniform                float     aafuzzConst;
uniform                sampler2D aaclip;
layout(location=1) out lowp vec4 aaclipOut; // clip anti alias alpha

void main() {
	//
	// Limit the range to 1.0-0.9, which can precisely achieve pruning rollback (division operation). 
	// The range of 1.0-0.9 is already accurate enough for aa
	//
	// lowp float alpha = (1.0 - abs(aafuzz)) * (aafuzzWeight * 0.1) + 0.9;
	// F = (C + Fuzz) * W + C1;
	// F = (C.W + C1) + Fuzz.W
	// C'.W = (C.W + C1)
	// C'   = C + C1/W
	// F = C'.W + Fuzz.W
	lowp float alpha = (aafuzzConst + abs(aafuzz)) * aafuzzWeight;
	lowp float clip = texelFetch(aaclip, ivec2(gl_FragCoord.xy), 0).a;
	// aaclipOut = vec4(1.0,1.0,1.0,clip * alpha);
	aaclipOut = vec4(1.0,0.0,0.0,1.0);
}
