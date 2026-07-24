// Simple forward mesh shader with optional albedo texture

#type vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core
layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform vec3 u_AlbedoColor;
uniform int u_UseAlbedoMap;
uniform sampler2D u_AlbedoTexture;

void main()
{
	vec3 albedo = u_AlbedoColor;
	if (u_UseAlbedoMap != 0)
		albedo *= texture(u_AlbedoTexture, v_TexCoord).rgb;

	color = vec4(albedo, 1.0) * u_Color;
}
