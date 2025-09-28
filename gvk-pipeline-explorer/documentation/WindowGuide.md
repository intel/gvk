# Pipeline Explorer (v0.5) Window Guide

Use this window guide as detailed documentation in regards to how each UI element in Pipeline Explorer functions. For a more workflow based guide, please see the [User Guide](UserGuide.md)

*Note: UD = Under Development*
## Workspace Window

 - Workspace
	 - Auto: Check to allow modification to the Workspace text field. 
	 - *Text field*: Location of the Pipeline Explorer Workspace for this application (Metrics report and decompiled pipelines will be saved here)
 - Clear: Clears saved path-related text fields from all Pipeline Explorer windows
 - Recent: Brings up a pop-up window with recently ran applications. Selecting an application loads the path-related saved text field memory for that specific application launch.
 - Wait For Debugger: Check to enable a dialogue box before application launch, allowing the user time to attach a debugger.
 - Launch: Run the executable in the Launch text field.
 - *Text field* : Location of the application to launch.
 - Target and *Text field*: UD
 - Args *Text field* : Arguments to provide to the application.
 - Working Directory
	 - Auto: Check to allow modification to the Working Directory text field.
	 - *Text field*: Location of the Working Directory (usually directory the executable lives in)

## Pipelines Window
 - Refresh Active Pipelines: Populates the table below this button with any active Vulkan Pipelines

## Selected Pipeline Window
*These UI elements will only be interactive when a pipeline is selected in the Pipelines window*

 - Write Pipeline Info
	 - *Checkbox*: UD.
	 - *Text field*: Location of the pipeline folder where pipeline Info and shaders will be written to upon decompilation.
 - UUID *text field*: (not interactive) The Universally Unique Identifier for this pipeline 
 - Driver UUID *text field*: (not interactive) The Universally Unique Identifier for this pipeline per the driver
 - Decompile:  Decompiles the selected pipeline to its pipeline folder in your workspace.
 - Recompile: Recompiles any edited GLSL/HLSL shaders from the pipeline folder into the live pipeline
 - Open File: Opens a pop-up menu where you can select a file to open in a new Text Viewer window. 
 - Experiment *checkbox*: Enables or disables the shader editing experiment on selected pipeline (if user has recompiled one)
 - Metrics *checkbox*: UD
 - Highlight *checkbox*: When checked, a shader experiment is performed on the selected pipeline to set its color output to the chosen color from the color picker in order to highlight this pipeline.
 - Color *color picker* : When clicked on, will open a RGB/HSV/Hex color editing pop-up to allow selection of a new color for the highlighting shader experiment. Hovering over the color picker will display a color summary.

## Metrics pipeline

 - Refresh Available Metrics: This will populate the table below the *Sample Metrics* button with metrics from the currently selected pipeline. 
 - Write Metrics Report:
	 - *Checkbox*: Enables metrics report generation when *Sample metrics* button is clicked.
	 - *Text field*: (not interactive) Location of the folder the metrics report will be saved too
 - Warmup Frame Count *text field*: How many warmup frames to repeat before metrics collection will begin. Can be modified by interacting with the textbox or using the *-* and *+* buttons. 
 - Sample Frame Count *text field*: How many frames to average over to calculate the sampled metrics. Can be modified by interacting with the textbox or using the *-* and *+* buttons.
 - Apply Metrics Filter: Populate the table with only metrics groups that pass your filter(s) in the Any Of and All Of text fields.
 - Any Of *text field*: Metrics filter to filter metrics groups by. Case sensitive and ';' delimited.
 - All Of *text field*: Metrics filter to filter metrics groups by. Case sensitive and ';' delimited.
 - Available Metrics Groups: Label that displays the number of currently available metrics groups 
 - Sample Metrics: Will refresh the metrics table with sampled metrics, using the specified Warmup Frame Count and Sample Frame Count.

## Console Window
- Clear: Clears the Console text
- Console: Displays any errors in Pipeline Explorer (ex. from shader recompilation) to the user.

## Text Viewer Window
Encounter this window by opening a file from a decompiled pipeline in the Selected Pipeline window.
- Font Scale *text field*: The font scale in this text viewer window. Can be any value from 1.0 to 2.0. Change by editing the text field or by using the text field or the *-* and *+* buttons.
- Viewer: View any code or Pipeline info. You can also edit GLSL/HLSL shaders here. To save edited shaders, press **Ctrl-Shift-S**
