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
4. Yes, I am sorry I have made you install so many things...
5. Git clone with ```git clone --recursive https://github.com/Deliverooo/Toaster-2.0.git```
6. Goto `Toaster-2.0/scripts` and run `build_scripts.bat` to build the C# DLL
7. Go to your **system** environment variables and add the bin directory to **PATH** e.g. ```C:\dev\Toaster-2.0\bin```
8. There you go...

# Creating a new project

Pick a directory to create your project, e.g. ```C:/ToasterEngine```. Next open a terminal/command prompt in
that directory and type ```tstb new -n YourProjectName```. This will create a new project in that directory with the
specified name. If you want to change the default scene name, instead type
```tstb new -n YourProjectName -sn YourSceneName```. After that, multiple directories will have been created, aswell as
a C# project. If you want to run to make sure you did everything correctly, type the command
```toaster_runtime -p Path/to/your/project.tproj```. Additionally, I am working on editing the Windows registry to add
*Open With* functionality. If you have assets you want to use, drag them into their respective folders under the
project (it actually doesn't matter which folders they go into) and to add them to the asset registry type
```tstb buildAssets``` in the same directory as your .tproj file.

At the moment, you will have to match the asset id's manually, which can be annoying, but eventually the toaster_editor
will be able to take care of this for you... :)

# Planned Features

- Use Hostfxr and CoreClr instead of Mono because it is ~6-7x faster
- Physics system via Jolt
- Asset system
- Audio?
- ¿Video loading? (possibly; ts sounds cool...)
- Other things

## Examples

<p float="left" width=25%> 
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/master/examples/images/Test_scene.png" width=25%>
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/master/examples/images/Heavenly_Orbo.png" width=25%>
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/master/examples/images/SSAO_Irradiance.png" width=25%>
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/master/examples/images/New_Lighting.png" width=25%>
</p>
