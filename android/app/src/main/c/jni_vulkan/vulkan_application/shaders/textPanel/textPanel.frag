#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (binding = 1) uniform sampler2D tex;

layout (location = 0) in vec4 texcoord;
layout (location = 1) in vec3 frag_pos;
layout (location = 2) in vec3 frag_pos_model;

layout (location = 0) out vec4 uFragColor;

const vec3 lightDir = vec3(0.424, 0.566, 0.707);

float smallDecimal = 0.000001;
// The cordinates were cherry-picked in Blender
vec3 bottomLeftBox = vec3(2.585544 - smallDecimal, 0.224669 - smallDecimal, -0.567743 - smallDecimal);
vec3 topRightBox = vec3(-2.529603 + smallDecimal, 0.224669 + smallDecimal, 0.576378 + smallDecimal);

// we must avoid if conditions in shaders by using this function
// https://stackoverflow.com/questions/12751080/glsl-point-inside-box-test
// return 1 if v inside the box, return 0 otherwise

// same as at the link, but we make sure the values to compare are in the right order
// It is like reverting the X axis or the Y axis, independently
float insideBox3D(vec3 v, vec3 bottomLeft, vec3 topRight) {
   vec3 bottomLeft2;
   vec3 topRight2;
   bottomLeft2.x = min(bottomLeft.x, topRight.x);
   topRight2.x = max(bottomLeft.x, topRight.x);
   bottomLeft2.y = min(bottomLeft.y, topRight.y);
   topRight2.y = max(bottomLeft.y, topRight.y);
   bottomLeft2.z = min(bottomLeft.z, topRight.z);
   topRight2.z = max(bottomLeft.z, topRight.z);
   vec3 s = step(bottomLeft2, v) - step(topRight2, v);
   return s.x * s.y * s.z;
}

void main() {
   vec3 dX = dFdx(frag_pos);
   vec3 dY = dFdy(frag_pos);
   vec3 normal = normalize(cross(dX,dY));
   float light = max(0.0, dot(lightDir, normal));
   // 0, 32, 246, the colour was picked using Digital Coulour Meter
   vec4 blue = vec4(0, 1.0 * 32 / 255, 1.0 * 246 / 255, 1);
   //uFragColor = vec4((light * blue).xyz, 1);
   float t = insideBox3D(frag_pos_model, bottomLeftBox, topRightBox);
   //float t = 0;
   //vec4 colorWithTexture = light * texture(tex, texcoord.xy);
   vec4 colorWithTexture = texture(tex, texcoord.xy);
   //vec4 colorWithTexture =  vec4(0, 1.0, 0, 1);
   uFragColor = t * colorWithTexture + (1 - t) * blue;
}
