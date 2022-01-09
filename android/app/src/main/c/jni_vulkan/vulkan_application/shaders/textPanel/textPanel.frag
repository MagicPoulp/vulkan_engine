#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (binding = 1) uniform sampler2D tex;

layout (location = 0) in vec4 texcoord;
layout (location = 1) in vec3 frag_pos;
layout (location = 2) in vec3 frag_pos_model;
layout (location = 0) out vec4 uFragColor;

const vec3 lightDir= vec3(0.424, 0.566, 0.707);

// we must avoid if conditions in shaders by using this trick
// https://stackoverflow.com/questions/12751080/glsl-point-inside-box-test
// return 1 if v inside the box, return 0 otherwise
float insideBox(vec2 v, vec2 bottomLeft, vec2 topRight) {
   vec2 s = step(bottomLeft, v) - step(topRight, v);
   return s.x * s.y;
}

float insideBox3D(vec3 v, vec3 bottomLeft, vec3 topRight) {
   vec3 s = step(bottomLeft, v) - step(topRight, v);
   return s.x * s.y * s.z;
}

vec3 bottomLeft = vec3(2.585544, 0.224669 - 0.001, -0.567743);
vec3 topRight = vec3(-2.529603, 0.224669 + 0.001, 0.576378);

void main() {
   vec3 dX = dFdx(frag_pos);
   vec3 dY = dFdy(frag_pos);
   vec3 normal = normalize(cross(dX,dY));
   float light = max(0.0, dot(lightDir, normal));
   // 0, 32, 246, the colour was picked using Digital Coulour Meter
   vec4 blue = vec4(0, 1.0 * 32 / 255, 1.0 * 246 / 255, 1);
   //uFragColor = vec4((light * blue).xyz, 1);
   float t = insideBox3D(frag_pos_model, bottomLeft, topRight);
   //float t = 0;
   //vec4 colorWithTexture = light * texture(tex, texcoord.xy);
   vec4 colorWithTexture =  vec4(0, 1.0, 0, 1);
   uFragColor = t * colorWithTexture + (1 - t) * blue;
}
