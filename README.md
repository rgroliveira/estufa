# Estufa Ditatica 1.0
Rogerio ifce.edu.br

# _Sample project_

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This is the simplest buildable example. The example is used by command `idf.py create-project`
that copies the project to user specified path and set it's name. For more information follow the [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project)



## How to use example
We encourage the users to use the example as a template for the new projects.
A recommended way is to follow the instructions on a [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project).

## Example folder contents

The project **sample_project** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.
"# estufa" 

## Brainstorm Menu
├─ 1 Principal
├─ 2 Setpoint
├─ 2.1 Setpoint
├─ 3 Modo
├─ 3.1 Automatico
├─ 3.2 Ligado
├─ 3.3 Desligado
├─ 4 Ajusta Hora
├─ 4.1 Hora
├─ 4.2 Minuto
├─ 4.3 Segundo
├─ 5 Ajusta data
├─ 5.1 Dia
├─ 5.2 Mes
├─ 5.3 Ano