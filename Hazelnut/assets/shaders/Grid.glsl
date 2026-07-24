// Editor grid (XZ plane)

#type vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
	v_TexCoord = a_TexCoord;
}

#type fragment
#version 330 core
layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform float u_GridScale;
uniform float u_GridSize;

float gridFactor(vec2 st, float lineWidth)
{
	vec2 grid = abs(fract(st) - 0.5);
	vec2 deriv = fwidth(st);
	vec2 lines = smoothstep(deriv * lineWidth, deriv * (lineWidth + 1.0), grid);
	return clamp(max(lines.x, lines.y), 0.0, 1.0);
}

void main()
{
	float line = gridFactor(v_TexCoord * u_GridScale, u_GridSize);
	color = vec4(vec3(0.2), 0.35 * line);

	if (color.a < 0.01)
		discard;
}
