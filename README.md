# computer-graphics-1
Salford University Year 3 - Computer Graphics - Assignment 1. 

The set assignment was to create a data visualisation tool using OpenGL. in the C language. A basic template was provided, that had to be adapted to fulfill the requirements of the assignment.

This required reading a data file and representing said data with labeled primative shapes, altering their colour and size based off the relative values in the given data row, and visualizing there connections to other data rows, including magnatude (which is represented by the length of the drawn arc). 

A Spring based simulation was then created, using equations such as Hooke's law and Newton's laws, to move the primative shapes to consequently make the arcs between data rows to be correctly representative of their relative magnitude. The graph is considered complete when all primative shapes come to rest.

A damping coefficient is used to remove energy from the system, to aid in the eventual rest case of the network. 

A basic hud and multi level menu was created to aid the user whith the simulation controls and ability to alter simuation values, such as the Coulombs constant & range of affect and damping value.

Multiple static views were also created to view the dataset in terms of related data values. 

The solution to open the code in an editor is located in Template/raaComputerGraphicsAssignment1.sln. Note: The solution may require retargeting to compile. Windows SDK 10.0, Platform Toolset v143 should work for this.
