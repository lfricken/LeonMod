
# LeonMod - [click here to learn](https://docs.google.com/document/d/12eoA7EOY_42vgNyLlonejqKW4pcny9x_-KcpemVLlEg/edit)

# Install Instructions
Basically, the files you see above need to be merged into your `Sid Meier's Civilization V\Assets` folder. So the whole path looks like `Sid Meier's Civilization V\Assets\DLC\LEKMOD_V28.2`, but if you use [GitHub Desktop](https://desktop.github.com/) with a [GitHub account](github.com/login) you will be able to get updates or turn `on/off` the mod instantly with the press of a button.
1. Uninstall any other multiplayer mod packs you have (non mod pack mods do not need to be uninstalled, those are mods that show up under the Mods section in Civ)
1. Install [GitHub Desktop](https://desktop.github.com/) and login to it with your [GitHub account](github.com/login)
1. To get the files, click the green `Code` button at the top right of this page and select `Open with GitHub Desktop`
1. Click `Clone` at the bottom, this will take several minutes but only needs to be done once
1. Once done, at the top select `Repository` and `Show in Explorer`, this should open the files you just downloaded
3. You should see `.git, DLC, Maps, UI` folders and some files (if you don't see `.git`, switch your windows Explorer View to enable `Hidden items`)
4. In another window, navigate to your `Sid Meier's Civilization V\Assets` folder where you should see `DLC, Maps, UI` folders
5. Drag all the contents, including `.git` from that download folder to your civ assets folder so it Merges the folders
6. GitHub Desktop will cry about not being able to find the repository
7. Launch `GitHub Desktop > File > Add local repostory > Choose` and navigate to `Sid Meier's Civilization V\Assets` > `Select Folder` and select `Add repository`

In GitHub Desktop (CLOSE CIV!):
* To update, click `Fetch origin` twice (wait a second in between)
* To disable the mod, click `Current branch` and select `no_mod`
* To enable the mod, click `Current branch` and select `main`
* To see update info, click `History`

Did it work?:
* Click `Single Player > Set Up Game` 
* You should see `NonChalant` at the top

### Want to contribute?
* See the issues page. We have issues that can be fixed or tested by people without C++, XML, or Lua experience!



# Compiling Downloads
(it's possible some of these steps can be skipped but IDK which ones and I don't want to play with fire)
* Install the Visual C++ Edition of [Visual Studio 2008](http://download.microsoft.com/download/E/8/E/E8EEB394-7F42-4963-A2D8-29559B738298/VS2008ExpressWithSP1ENUX1504728.iso)
* Install [Visual Studio 2008 Service Pack 1](https://www.microsoft.com/en-us/download/details.aspx?id=13276)
* Install [Visual Studio 2010](https://web.archive.org/web/20140227220734/download.microsoft.com/download/1/E/5/1E5F1C0A-0D5B-426A-A603-1798B951DDAE/VS2010Express1.iso) [(mirror)](https://debian.fmi.uni-sofia.bg/~aangelov/VS2010Express1.iso)
* Install [Service Pack 1 for Visual Studio 2010](https://drive.google.com/file/d/1NevJ0AiNFEDXKDm5FjZn1wzvvvOINF64/view?usp=sharing)
* Install the [Latest Community Version of Visual Studio](https://visualstudio.microsoft.com/) The only workload you need is "Desktop development with C++".

## Not Needed Downloads:
* [Old Microsoft Downloads](https://my.visualstudio.com/Downloads?q=visual%20studio%202010&wt.mc_id=o~msft~vscom~older-downloads)
* [Visual Studio 2008 Express 1397868 iso](http://download.microsoft.com/download/8/B/5/8B5804AD-4990-40D0-A6AA-CE894CBBB3DC/VS2008ExpressENUX1397868.iso)
* [Visual Studio 2008 Pro 90 Day Trial](http://download.microsoft.com/download/8/1/d/81d3f35e-fa03-485b-953b-ff952e402520/VS2008ProEdition90dayTrialENUX1435622.iso)
* [Visual Studio 2019 Community](https://learn.microsoft.com/en-us/visualstudio/releases/2019/release-notes)

# Compiling
* Clone the repo. The Visual Studio solution file is included in the repository folder.
* Significant portions of the mods are Lua / SQL / XML files. Those can be modified without rebuilding the game core.
* You need to install Visual C++ 2008 and Visual C++ 2008 SP1 compiler to actually link the resulting game core DLL. It is possible to use a recent IDE like Visual Studio 2019 Community though, just make sure to use the correct toolset.
* Always install different Visual Studio editions in chronological order, eg 2008 before 2019.
* If prompted on loading the solution file whether you want to retarget projects, select "No Upgrade" for both options and continue.
* If you encounter an "unexpected precompiler header error", install [this hotfix](http://thehotfixshare.net/board/index.php?autocom=downloads&showfile=11640).
* A tutorial with visual aids has been posted here, courtesy of ASCII Guy.
* If building the Release version, Whole Program Optimization is enabled, which will cause a several minute delay at the end of pass 1, but the compiler is still functioning!
* You can disable Whole Program Optimization locally under Project > VoxPopuli Properties > C/C++ > Optimization > Whole Program Optimization (set it to No)
* If the compiler stops responding at the end of pass 2, try deleting the hidden .vs folder as well as the BuildOutput/BuildTemp folders in the project directory, then reopening the solution file.

















