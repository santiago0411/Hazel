#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_EntityId;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) out VertexOutput Output;
layout (location = 2) out flat int v_EntityId;

void main()
{
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;

	v_EntityId = a_EntityId;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityId;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) in VertexOutput Input;
layout (location = 2) in flat int v_EntityId;

layout (binding = 0) uniform sampler2D u_FontAtlas;

float screenPxRange()
{
	const float pxRange = 2.0;
	vec2 unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlas, 0));
	vec2 screenTexSize = vec2(1.0) / fwidth(Input.TexCoord);
	return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b)
{
	return max(min(r, g), min(max(r, g), b));
}

void main()
{
	vec3 msd = texture(u_FontAtlas, Input.TexCoord).rgb;
	float sd = median(msd.r, msd.g, msd.b);
	float screenPxDistance = screenPxRange() * (sd - 0.5);
	float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

	if (opacity == 0.0)
		discard;

	vec4 bgColor = vec4(0.0);
	o_Color = mix(bgColor, Input.Color, opacity);

	if (o_Color.a == 0.0)
		discard;

	o_EntityId = v_EntityId;
}