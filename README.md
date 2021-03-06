# Cinder-Physx
Nvidia Physx SDK implementation for Cinder. Learn more about Physx here: https://developer.nvidia.com/physx-sdk

![alt tag](http://bantherewind.com/uploads/physx.png)

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

### BUILD (VISUAL STUDIO 2013)

##### 4. Build Physx with Visual Studio 2013 Community Edition or better.
  - Open "Cinder-Physx/PhysX-3.3/PhysXSDK/Source/compiler/vc12win64/PhysX.sln"
  - Go to "BUILD" > "Batch Build..."
  - "Select All"
  - "Build"

##### 5. Open and run a sample project.

##### 6. (Optional) Download and install Physx Visual Debugger (PVD) from here:
   https://developer.nvidia.com/gameworksdownload#?search=pvd
   
   NOTE: Open and run PVD _before_ running a sample app.
   
### BUILD (XCODE)

##### 4. Change `abs` to `fabs` at "Cinder-Physx/PhysX-3.3/PhysXSDK/Source/LowLevel/Software Source/PxsSolverConstraintExtPF:209".

##### 5. Make the following changes to "Cinder-Physx/PhysX-3.3/PhysXSDK/Source/LowLevelCloth/src/neon/SwCollisionHelpers.h"
```
SwCollisionHelpers.h:56
“uint8x16_t” to “uint8x8_t” (two instances)

SwCollisionHelpers.h:57
“vtbl1q_u8” to “vtbl1_u8”

SwCollisionHelpers.h:58
“vtbl1q_u8” to “vtbl1_u8”

SwCollisionHelpers.h:76
“uint8x16x2_t” to “uint8x8x2_t” (two instances)

SwCollisionHelpers.h:77
“vtbl2q_u8” to “vtbl2_u8”

SwCollisionHelpers.h:78
“vtbl2q_u8” to “vtbl2_u8”
```

##### 6. The following must be added to "Other C Flags" for each configuration on each target:
```
-Wno-reserved-id-macro
-Wno-unused-local-typedefs
```

##### 7. Use the command line tool to build each configuration from "Cinder-Physx/PhysX-3.3/PhysXSDK/Source/compiler/xcode_osx64/"
```
xcodebuild -project PhysX.xcodeproj -alltargets -configuration checked
xcodebuild -project PhysX.xcodeproj -alltargets -configuration debug
xcodebuild -project PhysX.xcodeproj -alltargets -configuration profile
xcodebuild -project PhysX.xcodeproj -alltargets -configuration release
```

##### 8. Repeat steps 6 and 7 for iOS from the "xcode_ios64" folder (first change "Targeted Device Family" to match your device(s)).
