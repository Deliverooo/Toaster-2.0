# Toaster 3.0 (Triple T)

A toasty and orbicular graphics engine with even more very cool stuff...

<p float="left" width=100%> 
	<img src="https://github.com/Deliverooo/Toaster-2.0/blob/3.0/github/images/Maclaurin_3T.png" width=25%>
</p>

# Features

- Very nice-looking API (objectively)
- Utilisation of Vulkan 1.4 Descriptor heaps and fully bindless rendering architecture, eliminating the need for
  pipelines and descriptor sets.
- Orbo

# Using / Installing

1. Have Windows. I do not plan on ever supporting Linux or macOS...
2. Make sure you have the newest possible Vulkan SDK installed with Version >= 1.4 -> https://vulkan.lunarg.com/sdk/home
   as well as the latest graphics drivers for your system.
3. Git clone with ```git clone --recursive https://github.com/Deliverooo/Toaster-2.0.git```

If you want to install Toaster as the SDK (you probably should), then run ``install.bat``. This will install the Toaster
SDK in your ``C:/Program Files/ToasterSDK/`` directory. Then, to use the generated CMake targets, simply add the ``bin``
directory in the SDK to your **System** environment varible list.

# Planned Features

- Move more towards Data-Oriented Design and away from coupled OOP
- ¿Video loading? (possibly; ts sounds cool...)
- Other things