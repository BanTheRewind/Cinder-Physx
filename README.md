# Cinder-Physx
Nvidia Physx SDK implementation for Cinder. Learn more about Physx here: https://developer.nvidia.com/physx-sdk

### SETUP

##### 1. Register as a Nvidia developer to get access to the Physx source by following the instructions here:
   https://developer.nvidia.com/physx-source-github

##### 2. Clone this repository.
```
git clone https://github.com/BanTheRewind/Cinder-Physx.git
```
##### 3. Add the Physx submodule.
```
cd Cinder-Physx
git submodule update --init
```
##### 4. Build Physx with Visual Studio 2013 Community Edition or better.
  - Open "Cinder-Physx/PhysX-3.3/PhysXSDK/Source/compiler/vc12win64/PhysX.sln"
  - Go to "BUILD" > "Batch Build..."
  - "Select All"
  - "Build"

##### 5. Open and run a sample project.
