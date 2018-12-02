cd %1
shader_minifier.exe "src/fragment_shader.glsl.pr" --preserve-externals --preserve-all-globals -o "src/fragment_shader.inl"
shader_minifier.exe "src/post_processing_shader.glsl.pr" --preserve-externals --preserve-all-globals -o "src/post_processing_shader.inl"
