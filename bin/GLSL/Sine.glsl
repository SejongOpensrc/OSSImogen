layout (std140) uniform SineBlock
{
	float freq;
	float angle;
} SineParam;

float Sine()
{
	vec2 nuv = vUV - vec2(0.5);
	nuv = vec2(cos(SineParam.angle), sin(SineParam.angle)) * (nuv * SineParam.freq * PI * 2.0);
    return cos(nuv.x + nuv.y) * 0.5 + 0.5;
}
