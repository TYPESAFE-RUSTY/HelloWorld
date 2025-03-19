> Note : before running the program compile the shader located at `src/shaders`

## Compile Shader

Simply run compileShader.ps1
> Note : this adds spv files to folder named spirv in base directory

to manually compile type 

```bash 
glslc src/shaders/<name>.vert -o spirv/<name>.vert.spv
```
> Note : Replace <name> with the name of shader