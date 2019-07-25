# DX9Demo

## About

This was a the final project for one of my University classes. It is basicly a Demo engine build in DirectX 9, with two models
(a tiger and a Teapot) in front of a mirrored cube with an explosion of particles in the background, a "sky" box and
a FPS counter. I decided to upload this so that I can back it up and if I ever decide to return to this I can use it as 
a base.

This is presented *AS IS* and I take absolutely no responsiblity for any issues that may occure form it's usage.

## Controls
### Camera Movement:

| Keys     | Command                                                                                            |
|----------|----------------------------------------------------------------------------------------------------|
| "W" Key: | move camra forward                                                                                 |
| "S" Key: | move camra backward                                                                                |
| "D" Key: | move camra right                                                                                   |
| "A" Key: | move camra left                                                                                    |
| Mouse    | When pick mode("P" Key) is not enabled mouse rotates the camera, both vertically and horizontally. |

### Model Manipulation
#### Selection

| Keys       | Command                                                                                                                      |
|------------|------------------------------------------------------------------------------------------------------------------------------|
| "1" Key    | Select Tiger model                                                                                                           |
| "2" Key    | Select Teapot model                                                                                                          | 
| MouseWheel | Rotates the model up or down based on direction                                                                              |
| Numpad 8   | Moves the Model vertically up                                                                                                |
| Numpad 2   | Moves the Model vertically down                                                                                              |
| Numpad 4   | Moves the model left                                                                                                         |
| Numpad 6   | Moves the model right                                                                                                        |
| Numpad 9   | Moves the model into the scene (z+)                                                                                          |
| Numpad 3   | Moves the model out of the scene (z-)                                                                                        |
| "P" Key    | Enbles pick mode, this will allow you to pick, using the mouse, a model and move it in the direction of the mouses movement. |

#### Lighting Controls

| Keys     | Command                                     |
|----------|---------------------------------------------|
| "8" Key  | Set Lighting method to Directional          |
| "9" Key  | Set Lighting method to Point lighting       |
| "0" Key  | Set Lighting method to Spot light           |
| "7" Key  | Toggle lighting method to Ambeient Lighting |