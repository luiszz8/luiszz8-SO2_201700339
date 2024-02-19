# **Manual Técnico  Practica 1 Sistemas Operativos 2**

### Información destacada

El manual técnico hace referencia a la información necesaria con el fin de orientar al personal de desarrollo en la concepción, planteamiento análisis programación 
e instalación del sistema. Es de notar que la redacción propia del manual técnico está orientada a personal con conocimientos en sistemas y tecnologías de información, 
conocimientos en C y procesos de linux.

La practica consiste en escribir, leer y posicionar en un archivo de texto mediante 2 proeceos hijos. Estos procesos siendo capturados por systemtap y llevar el control de estos.

## Estructura de la raiz del proyecto
La **Practica 1** cuenta con los siguientes módulos (raiz):  
[![indice.png](https://i.postimg.cc/tTSSjw5v/indice.png)](https://postimg.cc/sQW9Pnb5)  
Divido en:
parent: que contiene la logica del proceso padre.  
[![padre.png](https://i.postimg.cc/x1DsnKvQ/padre.png)](https://postimg.cc/LnDkVgQQ)  
child: contiene la logica del proceso hijo.  
[![hijo.png](https://i.postimg.cc/jjRZJt3k/hijo.png)](https://postimg.cc/yDrFM4HF)  
trace: contine la logica para capturar los preocesos mediante systemtap.  
[![trace.png](https://i.postimg.cc/zDQtPm3V/trace.png)](https://postimg.cc/c6QBrbRS)  
**Ejemplo de salidas esperadas:**  
[![proceso.png](https://i.postimg.cc/Gt6XBD6k/proceso.png)](https://postimg.cc/1VcG28mt)  
[![texto.png](https://i.postimg.cc/4yY8NP2f/texto.png)](https://postimg.cc/BPG5mTGk)
