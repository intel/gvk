# Pipeline Explorer (*v0.5*)
#### Please see [docs/UserGuide.md](docs/UserGuide.md) for a detailed user guide.
Pipeline Explorer is a standalone GUI that uses a Vulkan layer to gather metrics and perform shader experiments in a live Vulkan application. It also allows for frame analysis when paired with a capture/playback tool such as GITS.

Pipeline Explorer is packaged within GVK. Upon building GVK, you will find the **gvk-pipeline.explorer.exe** binary generated in the gvk-pipeline-explorer directory.


# Quick Live analysis workflow overview:
- Launch the application in Workspace window, and leave it running.
- Then, refresh active pipelines in the Pipelines window
- You can then quickly determine the purpose of each graphics/raytracing pipeline by using highlight column.
- You can also sort the pipelines by average time or executions per frame to find troublesome pipelines.
- To make Shader edits, use the Decompile, Open File (Ctrl-Shift-S to save), and Recompile buttons in the Selected Pipeline Window.
- Then, refresh active metrics in the Metrics Window. Compare the metrics before and after your experiment (toggle the experiment checkbox in the Selected Pipeline window). Or generate a metrics report.
- Close the application when you completed your analysis.

