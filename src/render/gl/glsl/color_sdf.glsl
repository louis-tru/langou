#vert
in  float sdf_in; // signed distance field
out float sdf_f;

void main() {
	sdf_f = sdf_in;
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}

#frag
in      lowp float sdf_f;
uniform lowp float sdf_range[3]; // range -0.5 => 0, sdf increase
uniform lowp vec4  color;

void main() {
	// gl_InstanceID
	// gl_VertexID
	// sdf value range: -0.5 => 0, alpha range: 0 => 1
	lowp float alpha = smoothstep(sdf_range[0], sdf_range[1], abs(sdf_range[2] + sdf_f));
	fragColor = vec4(color.rgb, color.a * alpha);
}