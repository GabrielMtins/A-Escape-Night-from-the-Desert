# A-Escape-Night-from-the-Desert

![Imagem do Jogo](https://img.itch.zone/aW1nLzEyMzQwNjcyLnBuZw==/original/lVzNLc.png)

A Escape Night from the Desert é um jogo de tiro em primeira pessoa que utiliza
a técnica de raycasting para renderizar seus gráficos. O diferencial do jogo é
que o raycasting funciona com múltiplas alturas, diferentemente de jogos
clássicos de raycasting como Wolfenstein, em que todos as paredes tem a mesma
altura.

A Engine desse jogo renderiza via software e tem suporte à scripting usando
lua. Toda a IA dos inimigos foram escritas em lua, além de algumas funções de
carregamento de assets. Também há o editor de fases incluso, que permite que
novas fases sejam criadas em 3d, utilizando a mesma engine de renderização que
o jogo.

O jogo está distribuido sob a licença GNU GPLv3+. Seus binários compilados
estão disponíveis na página do [itch.io](https://gabrielmtins.itch.io/a-escape-night-from-the-desert).

## Compilação 

Para compilar o jogo e o editor, você precisa baixar como dependência:
- libsdl2
- libsdl2-image
- libsdl2-ttf
- libsdl2-mixer
- libgtk3
- liblua
- gcc
- cmake

Para compilar a engine e executar, basta digitar os seguintes comandos
no terminal:

```
git clone https://github.com/GabrielMtins/A-Escape-Night-from-the-Desert.git
cmake -B build
cd build && make
cd ..
./build/main
```

Para compilar o editor e executar, basta digitar os seguintes comandos:
```
cd editor
git submodule init
git submodule update
cmake -B build
cd build && make
cd ../..
./editor/build/main
```
