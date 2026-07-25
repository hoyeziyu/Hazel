// Forward mesh shader: albedo texture + Blinn-Phong directional light

#type vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_WorldPosition;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main()
{
	vec4 worldPosition = u_Transform * vec4(a_Position, 1.0);
	v_WorldPosition = worldPosition.xyz;
	v_Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
	v_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * worldPosition;
}

#type fragment
#version 330 core
layout(location = 0) out vec4 color;

in vec3 v_WorldPosition;
in vec3 v_Normal;
in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform vec3 u_AlbedoColor;
uniform int u_UseAlbedoMap;
uniform sampler2D u_AlbedoTexture;

uniform vec3 u_CameraPosition;
uniform vec3 u_LightDirection;
uniform vec3 u_LightRadiance;
uniform float u_LightIntensity;
uniform int u_HasDirectionalLight;

void main()
{
	vec3 albedo = u_AlbedoColor;
	if (u_UseAlbedoMap != 0)
		albedo *= texture(u_AlbedoTexture, v_TexCoord).rgb;

	vec3 normal = normalize(v_Normal);
	vec3 ambient = 0.08 * albedo;

	vec3 lighting = ambient;
	if (u_HasDirectionalLight != 0)
	{
		vec3 lightDir = normalize(-u_LightDirection);
		vec3 viewDir = normalize(u_CameraPosition - v_WorldPosition);
		vec3 halfDir = normalize(lightDir + viewDir);

		float diffuse = max(dot(normal, lightDir), 0.0);
		float specular = pow(max(dot(normal, halfDir), 0.0), 32.0);

		vec3 radiance = u_LightRadiance * u_LightIntensity;
		lighting += radiance * (diffuse * albedo + 0.25 * specular);
	}

	color = vec4(lighting, 1.0) * u_Color;
}
