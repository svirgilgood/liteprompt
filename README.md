# liteprompt
All you need, nothing you don't.


## What Does Liteprompt Do

liteprompt is a small c program to use for zsh prompts. It provides the
following information:

1. updates: this is the line count for the file ~/.updates. Which can be
   populated by having the following cronjobs for a debian based distro:
    ```
    10 * * * * apt list --upgradable > ~/.updates 
    # and in the sudo crontab 
    15 * * * * apt update 
    ```
1. ssh hostname: if you are currently in an ssh session, it will print the
   hostname for the server 
1. Virtual Env: if you have a virtual environment activated, it will display
   the name of the directory that holds the virtual environment. 
1. Git Branch: if you are in a directory that is under git version control, it
   will display the name of the git branch in green if the branch is up to
   date, and red if the branch is not. 
1. Path: The path is in a condensed form. `~/D/n/calc>` would be the path for
   the directory `/home/user/Documents/notebooks/calc`. 

If this is added to the root zshrc, the display will be
`%F{yellow}{hostname}%f:%F{red}#` when the user logs in as root. 

## How to Install 

Installation of the prompt is pretty simple. 

### Dependencies 

The only dependency for this program is libgit2. Install that on debian
distros with `# apt install libgit2-1.1`.


### Building the package 

To build the program, run `gcc -o $HOME/bin/prompt liteprompt.c -lgit2`. This will
create an executable in the directory ~/bin called prompt. 

### Adding the File the .zshrc 

To your zshrc, add:
```
prcmd() {
  PROMPT=`prompt`
}
```

Source the updated zshrc, and the prompt should be displaying in full color.

### For updates

To have the updates file. Add the following to the user crontab:
```
10 * * * * apt list --upgradable > ~/.updates 
```
And to a super user crontab add:
```
15 * * * * apt update 
```

