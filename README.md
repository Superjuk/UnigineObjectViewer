# UnigineObjectViewer
Simple 3D object viewer on Unigine 2.7.2 (Sim License) engine.

## Building
For build you need:
- Qt5.11+
- Unigine 2.7.2 Sim License
- On Windows: MSVC compiler
  On Linux: GCC 6.4.0+
  
 Build system - qmake;
 Easy way - build from Qt Creator :) It finds all Qt dependencies and so on.
 
 ## Features
 - rotating object by pressing left mouse button or using viewcube (in top-right corner);
 - zooming in and zooming out (mouse wheel scrolling or -/= keyboard buttons for slow zoom and CTRL+ -/= for fast zoom);
 - select part of object: by left mouse button click, select from part list (checkbox tree) on left;
 - fit zoom by space keyboard button click;
 - selected part have orange outline (made by shader);
 - discarding checkbox hides this part from view;
 - transparent mode (make all parts except selected transparent);
 - by double clicking on part name in part names list, you can rename part and save it:
 - available list of initial names;
 - import models from STEP files (beta, in debug).

## To-Do List
- upgrade project from commercial Unigine 2.7.2 Sim to free Unigine 2.14.1 Community;
- fix "gimbal lock" in viewcube;
- fix rotate velocity (make it fixed);
- remove "magic numbers";
- split WorldLogic on classes;
- replace pointers to smart pointers;
- remove document import function.

## Prerun
In Unigine SDK Browser you need:
1. Add existing project (.project file);
2. Reconfigure it: Other Actions -> Configure Project;
3. Open Editor in Unigine Editor and run world file (need to compile shaders and create cache).

## Run Options
- --editable - changeable part names;
- --importable - ability to import STEP files if --model doesn`t specify;
- --model <name> </path/to/model.node> - load model from "data" folder. Path is relative. Root folder is "data";
- --soffice </path/to/soffice/binary> - need to import office documents into html format (must deprecate in future).
