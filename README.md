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

## 
