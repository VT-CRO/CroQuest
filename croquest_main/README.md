# Super Quest

This will be the documentation for Super Quest game

## File Structure

Each "Quest" File (`.qst`) will have the following format:

```
START_LEVEL_INF
NAME@The First Quest
CREATED@12:16:15PM 4/6/2025
BY@Marco Gonzalez Hauger
LIVES@3
END_LEVEL_INFO

START_LEVEL_BUILD@main
PREAMBLE@You are a freshman student looking for your dorm.
;Avoid getting captured by UVA students.
SKY@regularsky
BG1@skyline
BG2@houses
PALYER@50,50;mario
PLATFORM@10,10;20,20;grass
TEXTURE@20,10;30,20;bricks
CHECKPOINT@30,30;flag;You have made it to the duck pond.
ENEMY@10,20;uva
BRICKBLOCK@30,30;brick
POWERBLOCK@40,30;mysterybox;1-up
FINALFLAG@50,30;flagpole;greenflag
PIPE@60,30;underground,10,10
COINS@20,10
STARCOINS@25,15
END_LEVEL_BUILD@main
```

## Commands
| Command           | Description                                                                                                               | Usage/Example                                           |
|-------------------|---------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------|
| **START_LEVEL_INFO**   | Marks the beginning of the level's info section.                                                                     | `START_LEVEL_INF`                                       |
| **NAME@**         | Specifies the name of the quest or level.                                                                                 | `NAME@The First Quest`                                  |
| **LIVES@**         | Specifies the amount of lives the player starts with                                                                                 | `LIVES@3`                                  |
| **CREATED@**      | Indicates the date/time the level was created.                                                                            | `CREATED@12:16:15PM 4/6/2025`                           |
| **BY@**           | Indicates the creator’s name or identifier.                                                                               | `BY@Marco Gonzalez Hauger`                              |
| **END_LEVEL_INFO**    | Marks the end of the level info section.                                                                              | `END_LEVEL_INFO`                                        |
| **START_LEVEL_BUILD@** | Marks the beginning of the level build section for the name. Each level MUST have a main.                                                                       | `START_LEVEL_BUILD@main`                                     |
| **PREAMBLE@**     | Provides a brief intro or preamble text for the level—often displayed before gameplay starts.                             | `PREAMBLE@You are a freshman student looking for your dorm.` |
| **SKY@**          | Defines the sky theme for the level, which moves very slowly                                                  | `SKY@regularsky`                                        |
| **BG1@**          | Defines the first background layer, which slow.                                                        | `BG1@skyline`                                           |
| **BG2@**          | Defines the second background layer, which at medium speed.                                      | `BG2@houses`                                            |
| **PALYER@**       | Sets the player’s starting coordinates on the level, and their player model. We'll have a couple different characters to choose from.                                                                     | `PALYER@50,50;mario`                                          |
| **PLATFORM@**     | Creates a platform at given coordinates (x,y), and an end coordinate (x,y). It will be drawn as a rectangle, with a specified tile/texture.                                              | `PLATFORM@10,10;20,20;grass`                            |
| **TEXTURE@**      | Same as a platform, except it has no hitbox to the user.                                                         | `TEXTURE@20,10;30,20;bricks`                            |
| **CHECKPOINT@**   | Marks a checkpoint for the player at specified coordinates, with an optional message or flag that will appear.                             | `CHECKPOINT@30,30;flag;You have made it to the duck pond.` |
| **ENEMY@**        | Places an enemy at specified coordinates with a given type. Enemies will be defined game-wide, and won't change level-to-level.                            | `ENEMY@10,20;uva`                                       |
| **BRICKBLOCK@**   | Places a breakable brick block at the specified coordinates.                                                              | `BRICKBLOCK@30,30;brick`                                |
| **POWERBLOCK@**   | Places a power-up block (e.g., a mystery box) at the specified coordinates, and has a specific item in it.                                               | `POWERBLOCK@40,30;mysterybox;1-up`                           |                                   |
| **FINALFLAG@**         | The final flagpole that ends the level. Also has an optional texture.                                                                          | `FINALFLAG@40,30;flagpole;mytexture`                          |
| **PIPE@**              | A green pipe placed at (x,y), transporting the user to a different world at (new x,y).                                                        | `PIPE@10,10;cave,50,50`                                     |
| **COINS@**             | Adds coins to the level to collect and increase score.                                                             | `COINS@10,20`                              |
| **STARCOINS@**         | Collect 3 starcoins for extra points.                                                       | `STARCOINS@10,20`                                                 |
| **END_LEVEL_BUILD**   | Marks the end of the level build section of a certain name.                                                                             | `END_LEVEL_BUILD@main`    
