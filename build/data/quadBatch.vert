#version 130

uniform mat4 view;

in vec2 in_vertex;
in vec3 in_positionRotation;
in vec4 in_color;

out vec4 v_color;

void main()
{
	v_color = in_color;

	float angle = in_positionRotation.z;
	mat2 m = mat2(sin(angle), -cos(angle),
				cos(angle), sin(angle));
	gl_Position = view * vec4((in_vertex.xy * m) + in_positionRotation.xy, 0, 1);
}