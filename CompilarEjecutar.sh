#!/bin/dash


cadena1=' @ ' 
cadena2=' : '
cadena3=' $'
epacio=' '


echo "$(whoami)"$cadena1"$(hostname)"$cadena2 $esapacio "$(pwd)" $cadena3 > Prompt.txt

gcc proyecto.c -o proyecto

./proyecto
