
- Handle wrapping (everything *but* dispatchable handles)
- Experiment/compile/recompile state is confusing
- Store list of recent launcher paramers
- Solar Bay
- Report appender
- Shader objects
- Application launch
    - Environment variables
- Refector pre/execute/post handlers for layer vs. static lib
- Performance queries
- Clean .data/ on startup
- SPIR-V toolchain options in GUI
- Pipeline table header
    - Static header
- All info files in UI
- Auto sort by time
- Wrangle logs/messages
- Error logic
- [Decompile]/[Recompile] shouldn't create a metrics report
- std::vector, std::map, std::set as static interfaces over C array
- Debug names/labels
- Sort by metric
- Multi pipeline metrics
- Barriers
- Notify when duplicate UUIDs are encountered
- Notify on basically every action
- Auto scroll log
- Charts
- Extraneous .data/GVK_STRUCTURE_TYPE_PIPELINE_EXPLORER_REQUEST_INFO.info
    - build directory and/or working directory
- gvk-pipeline-explorer.pdb shared between .exe and .lib?
- Double check collection range logic
- Fill out workspace UI with injection info
- Init with various API levels
- Window focus wierdness (RDP?)
- Use VkPerformance structs
    - Custom to_string()
- Disable refresh buttons when no app running
- Button to stop tooled application
- Reset pipeline windows on app shutdown
- Pipeline count in table header
- Sampling metrics shouldn't refresh pipelines/timings/sorting
- Decompile when pipeline is selected
- Selectable metrics in metrics panel
    - Right click copy to clip board
- Wrangle units
- Default to longest running pipeline for sorting and selection
- Wrangle messages
- Look into Zep
    - https://github.com/cmaughan/zep_imgui
    - https://github.com/Rezonality/zep

•  Sparse resources support
   o  High level overview of the solution I’m going with
        Each VkSparseImageMemoryBind represents a 3D rectangular region of an individual VkImageSubresource
        Track bindings as axis-aligned-bounding-boxes via boost::geometry r-tree, a bounding-volume-hierarchy data structure well suited for sparse 3D indexing
        As new bindings (or unbinds) are processed any AABBs intersecting with incoming VkSparseImageMemoryBind are split and the overlap region subtracted

================================================================================
Needs more attention
--------------------------------------------------------------------------------
Extensions/IDs
Save/load workspace
null checks in gvk::layer::Registry

================================================================================

Duplicate logs...
ERROR : Failed to compile "VK_SHADER_STAGE_FRAGMENT_BIT"
ERROR: 0:368: 'subgroup op' : requires SPIR-V 1.3 
ERROR: 0:368: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.

ERROR : Failed to compile "VK_SHADER_STAGE_FRAGMENT_BIT"
ERROR: 0:368: 'subgroup op' : requires SPIR-V 1.3 
ERROR: 0:368: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.
