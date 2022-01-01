
# LeonMod
This mod changes a bunch of things you can [read about here](https://docs.google.com/document/d/12eoA7EOY_42vgNyLlonejqKW4pcny9x_-KcpemVLlEg/edit).



### Want to contribute?
* See the issues page. We have issues that can be fixed or tested by people without C++, XML, or Lua experience!



# Compiling Downloads
* [Old Microsoft Downloads](https://my.visualstudio.com/Downloads?q=visual%20studio%202010&wt.mc_id=o~msft~vscom~older-downloads)
* [Visual Studio 2008 Express 1397868 iso](http://download.microsoft.com/download/8/B/5/8B5804AD-4990-40D0-A6AA-CE894CBBB3DC/VS2008ExpressENUX1397868.iso)
* [Visual Studio 2008](https://www.microsoft.com/en-us/download/details.aspx?id=7873)
* [Visual Studio 2008 Pro 90 Day Trial](http://download.microsoft.com/download/8/1/d/81d3f35e-fa03-485b-953b-ff952e402520/VS2008ProEdition90dayTrialENUX1435622.iso)
* [Visual Studio 2008 Service Pack 1](https://www.microsoft.com/en-us/download/details.aspx?id=13276)
* [Visual Studio 2019 Community](https://learn.microsoft.com/en-us/visualstudio/releases/2019/release-notes)



# Compiling
* Clone the repo. The Visual Studio solution file is included in the repository folder.
* Significant portions of the mods are Lua / SQL / XML files. Those can be modified without rebuilding the game core.
* You need to install Visual C++ 2008 and Visual C++ 2008 SP1 compiler to actually link the resulting game core DLL. It is possible to use a recent IDE like Visual Studio 2019 Community though, just make sure to use the correct toolset.
* Always install different Visual Studio editions in chronological order, eg 2008 before 2019.
* If prompted on loading the solution file whether you want to retarget projects, select "No Upgrade" for both options and continue.
* If you encounter an "unexpected precompiler header error", install this hotfix.
* A tutorial with visual aids has been posted here, courtesy of ASCII Guy.
* If building the Release version, Whole Program Optimization is enabled, which will cause a several minute delay at the end of pass 1, but the compiler is still functioning!
* You can disable Whole Program Optimization locally under Project > VoxPopuli Properties > C/C++ > Optimization > Whole Program Optimization (set it to No)
* If the compiler stops responding at the end of pass 2, try deleting the hidden .vs folder as well as the BuildOutput/BuildTemp folders in the project directory, then reopening the solution file.

















