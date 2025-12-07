#version 430
#pragma stage : vert

layout(location = 0) out vec3 v_Position;
vec3 positions[3] = vec3[](
	vec3(-0.5, -0.5, 0.0),
	vec3(0.5, -0.5, 0.0),
	vec3(0.0, 0.5, 0.0)
);

void main()
{
	v_Position = positions[gl_VertexIndex];
	gl_Position = vec4(positions[gl_VertexIndex], 1.0);
}

#version 430
#pragma stage : frag

layout(location = 0) out vec4 finalColor;
layout(location = 0) in vec3 v_Position;

void main()
{
	finalColor = vec4((v_Position * 0.5 + 0.5), 1.0);// * u_Color.xyz, 1.0);
}