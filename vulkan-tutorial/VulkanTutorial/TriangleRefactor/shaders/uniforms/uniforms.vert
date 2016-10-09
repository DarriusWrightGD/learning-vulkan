#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    //fragColor = vec3(ubo.model[0].x,0,1);
    fragColor = inColor;
}


//#version 450
//#extension GL_ARB_separate_shader_objects : enable

//layout (location = 0) in vec3 position;
//layout (location = 1) in vec3 color;

//out gl_PerVertex {
//	vec4 gl_Position;
//};

//layout (binding = 0) uniform UniformBufferObject {
//	mat4 model;
//	mat4 view;
//	mat4 projection;
//} ubo;

//layout (location = 0) out vec3 outColor;

//void main(){
//	gl_Position = ubo.projection *ubo.view * ubo.model * vec4(position, 1);
//	outColor = color;
//}

//#version 450
//#extension GL_ARB_separate_shader_objects : enable

//layout (location = 0) in vec3 position;
//layout (location = 1) in vec3 color;

//out gl_PerVertex {
//	vec4 gl_Position;
//};

//layout(location = 0) out vec3 fragColor;


//void main(){
//	gl_Position = vec4(position, 1.0);
//	fragColor = color;
//}