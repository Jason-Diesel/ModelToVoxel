# ModelToVoxel
<div style="display:flex;">
  <img src="https://github.com/Jason-Diesel/FBXToVoxel/blob/master/FBXToVoxel/Frank.PNG" alt="Image 1" style="width:49%;">
  <img src="https://github.com/Jason-Diesel/FBXToVoxel/blob/master/FBXToVoxel/TheInn.PNG" alt="Image 2" style="width:49%;">
</div>

<strong>
Oops, this code is not meant to be readeble to other humans, but give it a try if you want to
</strong><br><br>

<strong>Installation</strong>
<br>
Open Up the visual studio project in Visual studio 2022, set FBXToVoxel as the startup project, Go into the DirectX12NewEngine folder -> Library -> Assimp -> lib and copy the assimp-vc143-mtd.dll and paste it into ($SolutionDir)/x64/Debug or ($SolutionDir)/x64/Release folder. (($SolutionDir)/x64/Release and ($SolutionDir)/x64/Debug will probably not exist, and there for you will have to create these directories by yourself, or run the program once and let it fail).
Now run program and you are done.
<br>

An application that creates voxel models from other models.
<br>
The application can take everything that the assimp library can take (2024) https://github.com/assimp/assimp, but not the ownFileType for the engine.
<br>
The engine is my own created with DirectX 12 (Sorry Linux and Mac users)
<br>
The .vox files looks like this:
Size.x (uint32_t)<br>
Size.y (uint32_t)<br>
Size.z (uint32_t)<br>
Voxels (uint16_t[3]) * Size.x * Size.y * Size.z (Voxels are 3 uint16_t with the first value being red color, second green color, and last blue, but 0,0,0 is air/transparent). 
<br>

<strong>Move with WASD, Shift(Down), Space(up) and CTRL(faster speed).</strong>
<br>
also use mouse click to interact with IMGUI elements.
<br>
(Tab can also be used to get the objects position, size and rotation, and will stop the mouse rotation on the camera! Press Tab again to remove it)
<br>
<strong>The Important files that tackels the problem is: </strong>
<ul>
  <li>FBXToVoxel/ModelToVoxel.h/.cpp</li>
  <li>FBXToVoxel/ComputeVoxels.hlsl (Not recommended)</li>
</ul> 

Watch video on the topic
<a href="https://youtu.be/x9e2ksB7l9E">Link To Youtube video</a>
<br>
