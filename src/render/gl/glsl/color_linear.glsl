#vert
uniform   vec4      range;/*start/end range for rect*/
out       float     indexed_f;
void main() {
	vec2 ao = range.zw     - range.xy;
	vec2 bo = vertex_in.xy - range.xy;
	/*indexed_f = clamp(dot(ao,bo) / dot(ao,ao), 0.0, 1.0);*/
	indexed_f = dot(ao,bo) / dot(ao,ao);
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}

#frag
in      lowp float     indexed_f;
uniform      int       count;
uniform lowp vec4      colors[256];/*max 256 color points*/
uniform lowp float     positions[256];
void main() {
	int s = 0;
	int e = count-1;
	while (s+1 < e) {/*dichotomy search color value*/
		int idx = (e - s) / 2 + s;
		if (indexed_f > positions[idx]) {
			s = idx;
		} else if (indexed_f < positions[idx]) {
			e = idx;
		} else {
			s = idx; e = idx+1; break;
		}
	}
	lowp float w = (indexed_f - positions[s]) / (positions[e] - positions[s]);
	fragColor = mix(colors[s], colors[e], w);
}