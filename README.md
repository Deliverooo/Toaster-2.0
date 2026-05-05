# Toaster 2.0 - Vulkan

Toaster Engine v2.0 (the definitive edition... but with Vulkan)

A graphics engine based on the OG Toaster architecture, but with better things...

# Features

- C# Scripting using Mono
- Model loading
- Pretty nice looking API
- Orbo
- Allegedly 14674 lines of code (As of 03/05/2026)

# Using / Installing

1. Make sure you have the Mono SDK installed -> https://www.mono-project.com/download/stable
2. Make sure you have the .NET SDK installed -> https://dotnet.microsoft.com/en-us/download
3. Make sure you have the Vulkan SDK installed with Version >= 1.3 -> https://vulkan.lunarg.com/sdk/home
4. Git clone with ```git clone --recursive https://github.com/Deliverooo/Toaster-2.0.git```
5. Goto `Toaster-2.0/scripts` and run `build_scripts.bat` to build the C# DLL
6. Go to your **system** environment variables and add the bin directory to **PATH** e.g. ```C:\dev\Toaster-2.0\bin```
7. Open a terminal in the root directory and type ```toaster_runtime scripts\Toaster\bin\Debug\Toaster.dll``` if you
   want to run the runtime with the C# script demo. If not, type toaster_editor in any terminal to run the editor. :)
8. Enjoy... :)

# Planned Features

- Asset system
- Audio?
- ¿Video loading? (possibly; ts sounds cool...)
- Other things

## Examples

<p float="left" width=25%> 
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/master/examples/images/New_Lighting.png" width=25%>
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/master/examples/images/C%23_Scripting.png" width=25%>
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/master/examples/images/Toaster_development_environment_overview.png" width=25%>
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/master/examples/images/Sponza_Demo.png" width=25%>
</p>
