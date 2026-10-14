Decompiles Frostbite 1.5 Shader Database into a more manageable format. This one is for PC version only (little endian, Dx11 Database) 

usage:

2 options - 

1) Use game dump as a base to find proper shader paths, example: 

shader: Objects/Weapons/Handheld/BU_rif_F2000/BU_rif_F2000_Mesh3p/Objects/Weapons/Handheld/BU_rif_F2000/Shaders/LOD2_Metal 
to 
real path: Objects/Weapons/Handheld/BU_rif_F2000/Shaders/LOD2_Metal

type in the path where the dump is located, with a slash (forward or backwards) at the end. the dbx files need to be converted to xml for this to work 

2) Skip the game dump and create the shaders as they appear in the shader database

- type a path where the database (or databases) are located, with a slash (forward or backwards) at the end.

- lastly type the output path where shaders will be saved, with a slash (forward or backwards) at the end.
   
