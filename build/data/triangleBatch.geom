#version 150

layout (points) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 g_vert0[];
in vec4 g_vert1[];
in vec4 g_vert2[];
in vec4 g_color[];

out vec4 v_color;

void main()
{
	v_color = g_color[0];

	gl_Position = g_vert0[0]; EmitVertex();
	gl_Position = g_vert1[0]; EmitVertex();
	gl_Position = g_vert2[0]; EmitVertex();
	EndPrimitive();
}