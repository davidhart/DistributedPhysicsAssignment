#version 150

uniform mat4 view;

in vec2 in_vert0;
in vec2 in_vert1;
in vec4 in_color;

out vec4 g_vert0;
out vec4 g_vert1;
out vec4 g_color;

void main()
{
	g_color = in_color;
	g_vert0 = view * vec4(in_vert0.xy, 0, 1);
	g_vert1 = view * vec4(in_vert1.xy, 0, 1);
}