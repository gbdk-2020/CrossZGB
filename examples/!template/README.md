# ZGB-template
A template for projects using ZGB, A little engine for creating games

# VSCode debugging using Emulicious
- Download CrossZGB to e.g. `~/CrossZGB`
- Download [Emulicious](https://emulicious.net/downloads/) to e.g. `~/Emulicious.jar`
- Install a java runtime environment (if you haven't already): `sudo apt install openjdk-21-jre`
- Add to your `.bashrc`, replacing the example paths with yours
```
export ZGB_PATH=~/CrossZGB/ZGB/common
export EMULICIOUS_PATH=~/Emulicious.jar
```
- Open your copy of the template in VSCode
- Install the workspace recommended extensions:
  - [C/C++ for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
  - [Emulicious Debugger](https://marketplace.visualstudio.com/items?itemName=emulicious.emulicious-debugger)

Now open the Run and Debug tab, select Debug or DebugColor, and press the play button to start debugging. Emulicious will be launched automatically and breakpoints will be hit. Enjoy!

> Note if using WSL: make sure your code is in your home directory rather than shared from windows, otherwise emulicious debugging won't work.
