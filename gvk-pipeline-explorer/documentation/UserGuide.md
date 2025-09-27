
# Welcome to Pipeline Explorer! (v0.5)

Pipeline Explorer is a standalone GUI that uses a Vulkan layer to gather metrics and perform shader experiments in a live Vulkan application. It also allows for frame analysis when paired with a capture/playback tool such as GITS.

Pipeline Explorer is packaged within GVK. Upon building GVK, you will find the **gvk-pipeline.explorer-gui.exe** binary generated in the gvk-pipeline-explorer directory.
>For example, if building GVK using the Debug target, Pipeline Explorer is found at [GVK repo]/build/gvk-pipeline-explorer/Debug/gvk-pipeline-explorer-gui.exe.

(For docs in regards to how each UI element in Pipeline Explorer functions see the [Window Guide](WindowGuide.md)).


# Getting Started 

## Live analysis workflow

When you first open Pipeline explorer, you will see several windows. These windows are all moveable, resizable, collapsable, and dockable. 
>To move a window, click and hold on the tab describing the window. This tab is at the top of the window. 
>You can also close windows using the *X* button. To restore a window, click the Windows menu at the top of Pipeline Explorer.

### Launch executable and load pipelines

Within the **Workspace** window, insert the absolute path to the executable (including the name and file extension) in the **Launch** text field. 
>You can perform clipboard operations as well as drag and drop files/folders from Windows Explorer into any text field in Pipeline Explorer

Then click the **Launch** button in the same window, and Pipeline Explorer will run your executable.

In the **Pipelines** window, click the **Refresh Active Pipelines** button. This will populate the table below this button with any active Vulkan Pipelines (Graphics, Raytracing, and Compute). Each row of the table represents a single pipeline. 

Pipeline data such as its Universally Unique Identifier are displayed here. The table columns can be customized by right-clicking on a column name. This opens the column customization pop-up, from which you can select pipeline data to display on the table. You can also sort some columns such as _Avg. Time/Frame_ by left-clicking the column name. This may help pin down the pipelines that need attention.
>Some of the pipeline table columns contain interactive elements (Highlight, Experiment, and Metrics) rather than data.

### Determine which pipeline needs attention 
To better understand what each pipeline does, you can use the highlight shader experiment on graphics/raytracing pipelines. Left-click the pipeline you would like to highlight - the row will turn blue to indicate it is selected. 

The cell corresponding to the **Highlight** column in each pipeline row contains a pink (default) color pick box, as well as a checkbox. When you select the checkbox, a shader experiment is performed on that pipeline to set its color output to pink (or the color you've chosen) in order to highlight this pipeline. You will see the results of this experiment immediately in the launched application. 
>This highlight shader experiment replaces the pipeline fragment shader with a simple shader that changes the out Color to the color selected in the color pick box.

### Gather Metrics

Once you have determined the pipeline that needs attention, in the **Metrics** window click **Refresh Available Metrics**. This will populate the table below the *Sample Metrics* button with metrics from the currently selected pipeline. 

To filter the populated metrics to only those you are interested in, you can enter your ';' delimited, case-sensitive filter string in the **Any Of** text field. Then click the **Apply Metrics Filter** button. This will populate the table with only metrics groups that pass your filter.

To refresh the metrics table, click the **Sample Metrics** button. You can also generate a metrics report by checking the checkbox under **Write Metrics Report**. The report will be generated in your workspace once the *Sample Metrics* button is clicked.
>The Metrics Report is generated in JSON format. It will be saved to [workspace]/reports/*.metrics

### Pipeline inspection and Shader Editing

If you'd like to edit the shaders, Pipeline Explorer provides this feature as well! First, make sure the pipeline containing the shaders you'd like to edit is selected. Then head over to the **Selected Pipeline** window and click the **Decompile** button. Your pipeline will be decompiled into your workspace. The pipeline structures will be saved as JSON files, and the shaders will be saved as both GLSL/HLSL and SPIR-V files. 

All these decompiled files are viewable within Pipeline Explorer by clicking the **Open File** button and selecting a file from the pop-up menu that appears. This opens a file in a new Text Viewer window. 
> You can open as many Text Viewer windows as you want within Pipeline Explorer. They can be docked, resized, collapsed, and moved however you would like. You can increase the Font scale anywhere from 1.0 to 2.0 using the text field or the *-* and *+* buttons at the top of the window.

GLSL/HLSL shader files in particular are editable using this Text Viewer. Once you make the desired changes to your shader, you can use the keyboard shortcut **Ctrl-Shift-S** to save your shader. Then click the **Recompile** button to recompile your edited shader into the live pipeline! 
If your edited shader compiles successfully, you will see the results immediately in the launched application. If there are compilation errors, you will see them in the Console window.
> You can alternatively use your own text editor to edit the GLSL/HLSL files. Once your edits are saved, you can  recompile your edited shader into the live pipeline the same way using the Recompile button.

Now, you can head back over to the Metrics window and click the **Sample Metrics** button to refresh the metrics table. These new metrics reflect your edited shaders in the selected pipeline!

### Post-Workflow

When you are ready to close the executable, simply exit the executable as you normally would. Pipeline Explorer will react by setting its windows back to their initial state. From here, you can launch a new application by following the *Launch executable and load pipelines* steps again, or exit Pipeline Explorer. 
Pipeline Explorer will store path-related text fields in memory for ease of use next time you open it. Alternatively, you can click the **Recent** button in the **Workspace** window to bring up a pop-up window with recently ran applications. Selecting an application loads the saved path-related text field memory for that specific application launch.

Happy Exploring!
