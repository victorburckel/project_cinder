#version 330 core

uniform sampler2D uTex0;
in vec2     textCoords;

out vec4	fragColor;

void main()
{	
	fragColor = texture( uTex0, textCoords );
}