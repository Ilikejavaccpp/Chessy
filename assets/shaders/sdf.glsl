#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Alpha discard threshold
const float alphaThreshold = 0.5;

void main()
{
    // Sample distance field value (stored in alpha channel)
    float distance = texture(texture0, fragTexCoord).a;
    
    // Smooth boundary scaling calculation 
    float alpha = smoothstep(alphaThreshold - 0.1, alphaThreshold + 0.1, distance);

    // Calculate final fragment color
    finalColor = fragColor*vec4(1.0, 1.0, 1.0, alpha);
}
