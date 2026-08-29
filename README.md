# Toaster 2.0 - Vulkan

Toaster Engine v2.0 (the definitive edition... but with Vulkan)

A toasty and orbicular graphics engine with very cool stuff...

# Features

- Very nice looking API
- Extremely modern Vulkan, using descriptor heaps and an OpenGL-like rendering API
- Orbo

# Using / Installing

1. Make sure you have the Mono SDK installed -> https://www.mono-project.com/download/stable
2. Make sure you have the .NET SDK installed -> https://dotnet.microsoft.com/en-us/download
3. Make sure you have the Vulkan SDK installed with Version >= 1.4 -> https://vulkan.lunarg.com/sdk/home.
4. Because of recent changes to the rendering architecture, it is required that you have the latest possible graphics
   driver and Vulkan SDK installed
5. Git clone with ```git clone --recursive https://github.com/Deliverooo/Toaster-2.0.git```
6. In the master directory where you cloned it, run ```install.bat```. This will install the Toaster SDK.
7. Go to where the SDK is installed at ```C:/Program Files/Toaster-SDK/``` and add the ```bin``` directory to your
   windows *system* environment variable list
8. There you go...

# Creating a new project

Pick a directory to create your project, e.g. ```C:/ToasterEngine/```. Next open a terminal/command prompt in
that directory and type ```tstb new cppProj -n YourProjectName```. This will create a new project in that directory with
the
specified name. Then run the ```build_project.bat``` file that has been created to build the sample code into an
executable. From there, you can do whatever you want to do... Ts is a game engine, so maybe make a game... :)

# Planned Features

- Remove C# because it is inferior to ***C++***
- Physics system via Jolt
- ¿Video loading? (possibly; ts sounds cool...)
- Other things

## Examples

<p float="left" width=25%> 
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/2.0/examples/github_images/Orbo_Backrooms.png" width=25%>
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/2.0/examples/github_images/Sigeon.png" width=25%>
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/2.0/examples/github_images/Brick.png" width=25%>
</p>
