echo off
cd /d %~dp0
C:\VulkanSDK\1.2.162.0\Bin\glslangvalidator -V shader.vert -o shadervert.spv
C:\VulkanSDK\1.2.162.0\Bin\glslangvalidator -V shader.frag -o shaderfrag.spv
echo on