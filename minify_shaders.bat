set ProjectDir=4kIntro/
EXIT /B
cd %1
shader_minifier.exe "%ProjectDir%/src/fragment_shader.glsl.pr" --preserve-externals --preserve-all-globals -o "%ProjectDir%src/fragment_shader.inl"
shader_minifier.exe "%ProjectDir%/src/post_processing_shader.glsl.pr" --preserve-externals --preserve-all-globals -o "%ProjectDir%src/post_processing_shader.inl"
