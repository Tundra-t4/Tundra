### Avionix !

Welcome to Avionix, a programming language that:
1) Emphasizes the ability to do anything whether low or high level
2) Opt-in safety, Avionix won't be aggressive against unsafe actions unless you ask it to!
3) Ensures readability and a smooth learning curve

Currently Avionix is **EXPERIMENTAL** and not recommended for production usage,

You can check out the docs [here](https://Avionix-t4.github.io/Docs)

### Download
Check releases (for macos)

### Building from source
Simply:
```bash
git clone https://github.com/Avionix-t4/Avionix/
cd Avionix
bash assemble.sh <your operating system>
```
**<sup><sub>(Assemble.sh may need sudo depending on your OS)</sup></sub>**

Now there will be a file called `Avionix-2025b-<your operating system>.tar.gz` in `Release/2025b/`

### Installation
Once you have compiled and bundled the .tar.gz whether from **Download** or **Building from source** ,

```bash
mkdir /usr/local/bin/Avionix
tar -xvzf <path to Avionix-2025b-<your operating system>.tar.gz> -C /usr/local/bin/Avionix
```
lastly, you will need to add the folder to PATH for macos users this is navigating to `~/.zshrc` (or your preferred shell) and adding the line:
```bash
export PATH="/usr/local/bin/Avionix:$PATH"
```

Verify the installation with `Avionixc version`:
`Avionixc: Anthology no. 2025b (Arcadia)`

### Project Structure
```
Mercury/ - the project management component
Avionixc/ - the compiler management component
polaroid/ - the llvm codegen component
std/ - the Avionix std
```

