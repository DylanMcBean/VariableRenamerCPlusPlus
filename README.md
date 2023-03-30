# Project: Variable Renamer
Variable Renamer is a tool designed to help developers manage variable names in their codebases, especially in large projects where manual renaming can become cumbersome and error-prone. This project aims to prevent conflicts that may arise from renaming variables without realizing that a variable with the same name already exists.

---
# Background
Consider the following scenario:

You have two variables:

testBool

testBool2

Now, you decide to rename testBool2 to testBool without realizing that testBool already exists. This may lead to warnings or even worse, the program may compile successfully but break during runtime. This issue can be exacerbated in larger files with thousands of lines and hundreds or thousands of variables to keep track of.

Variable Renamer is here to help you avoid such conflicts.

---
# Getting Started
To get started with Variable Renamer, you can either:

1. Clone the repository and use Qt to open it.
2. Download one of the release builds.

## Clone the Repository
git clone https://github.com/DylanMcBean/VariableRenamerCPlusPlus.git
Once you have cloned the repository, open the project in Qt.

## Download a Release Build
Navigate to the Releases page of the repository and download the latest release build for your platform. Extract the archive and run the executable.

---
# Usage
1. Load the source code file(s) you want to work with.
2. Run the Variable Renamer tool.
3. Click on save
Variable Renamer will analyze your code and perform the renaming while ensuring that no conflicts arise due to duplicate variable names.

---
# Contributing
Contributions are welcome! If you have any ideas, suggestions or bug reports, please feel free to open an issue or submit a pull request.

---
# License
This project is licensed under the MIT License.
