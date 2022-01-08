#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (binding = 1) uniform sampler2D tex;

//layout (location = 0) in vec4 texcoord;
layout (location = 0) in vec3 frag_pos;
layout (location = 0) out vec4 uFragColor;

const vec3 lightDir= vec3(0.424, 0.566, 0.707);

void main() {
   vec3 dX = dFdx(frag_pos);
   vec3 dY = dFdy(frag_pos);
   vec3 normal = normalize(cross(dX,dY));
   float light = max(0.0, dot(lightDir, normal));
   // 0, 32, 246, the colour was picked using Digital Coulour Meter
   vec4 blue = vec4(0, 1.0 * 32 / 255, 1.0 * 246 / 255, 1);
   uFragColor = vec4((light * blue).xyz, 1);

}
