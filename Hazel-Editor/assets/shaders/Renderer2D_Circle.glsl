#type vertex
#version 450 core

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec2 a_LocalPosition;
layout(location = 2) in float a_Thickness;
layout(location = 3) in float a_Fade;
layout(location = 4) in vec4 a_Color;
layout(location = 5) in int a_EntityId;

layout (std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec2 LocalPosition;
	float Thickness;
	float Fade;
	vec4 Color;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat int v_EntityId;

void main()
{
	Output.LocalPosition = a_LocalPosition;
	Output.Thickness = a_Thickness;
	Output.Fade = a_Fade;
	Output.Color = a_Color;

	v_EntityId = a_EntityId;

	gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityId;

struct VertexOutput
{
	vec2 LocalPosition;
	float Thickness;
	float Fade;
	vec4 Color;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat int v_EntityId;

void main()
{
	float fade = Input.Fade;
	float distance = 1.0 - dot(Input.LocalPosition, Input.LocalPosition);
	float thickness = Input.Thickness;

	float alpha = smoothstep(0.0, fade, distance);
	alpha *= smoothstep(thickness + fade, thickness, distance);

	if (alpha == 0.0)
		discard;

	o_Color = Input.Color;
	o_Color.a *= alpha;

	o_EntityId = v_EntityId;
}