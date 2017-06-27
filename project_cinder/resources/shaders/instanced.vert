#version 150

uniform mat4 ciModelViewProjection;

uniform vec4[4] uImageCoords;

in vec4 ciPosition;
in vec2 ciTexCoord0;

in vec2 aInstancePosition;
in int aInstanceImageIndex;

out vec2 textCoords;

void main()
{
	vec4 position = ciPosition;
	position.xy += aInstancePosition;

	vec4 imageCoords = uImageCoords[ aInstanceImageIndex ];
	textCoords = vec2( imageCoords.x,  imageCoords.y ) + ciTexCoord0 * vec2( imageCoords.z, imageCoords.w );
	
	gl_Position = ciModelViewProjection * position;
}
