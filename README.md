# Evaluation Assignment 2


I have to say this has a very good set of features and very beautifully laid scenery.

The rendering, though, is not efficient: it uses a lot of draw calls. It would
have been more efficient to use batch rendering (e.g., all walls in one VBO),
OpenGL's instancing for the pellets and enabling face culling.

I have some comments also on your programming.

- You have memory leaks. Here you allocate but never delete:

```txt 
File: main.cpp
 298  player = new Player(glm::vec3(i, 0, j), WIDTH / 2, HEIGHT / 2);
 311  ghosts.push_back(new Ghost(ghostLvl, pos.z, pos.x));
```

- You have a very reduced set of flat classes (no hierarchy or composition relationships).
- You pass objects to functions by value (you should use passing by reference, instead).
- There are a lot of functions in your main. It would have been good to try to
  pack them in classes to follow a full Object-oriented scheme
  
My evaluation of this work is **C**.
