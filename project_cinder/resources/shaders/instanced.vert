#version 330 core

uniform mat4	ciModelViewProjection;

in vec4			ciPosition;
in vec2			ciTexCoord0;

in vec2			aInstancePosition;
in vec4         aInstanceTextCoords;

out vec2		textCoords;

void main()
{
	vec4 position = ciPosition;
	position.xy += aInstancePosition;
	
	textCoords = vec2( aInstanceTextCoords.x,  aInstanceTextCoords.y ) + ciTexCoord0 * vec2( aInstanceTextCoords.z, aInstanceTextCoords.w );

	gl_Position = ciModelViewProjection * position;
}
